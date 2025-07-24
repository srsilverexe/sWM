#include "../include/windowManager.h"
#include "../include/ewmh.h"
#include "../include/bar.h"
#include "../include/client.h"
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef debug
#define debug_print(...) fprintf(stderr, "[WM] " __VA_ARGS__)
#else
#define debug_print(...)
#endif

bool initWindowManager(WindowManager *wm) {
  if (!(wm->dpy = XOpenDisplay(NULL)))
    return false;

  wm->root = DefaultRootWindow(wm->dpy);
  wm->workspaces = NULL;
  wm->masterRatio = DEFAULT_MASTER_RATIO;
  wm->currentWorkspace = 0;
  wm->currentLayout = MASTER;

  wm->supportedWmCheckWindow = XCreateSimpleWindow(wm->dpy, wm->root, 0, 0, 1, 1, 0, 0, 0);

  if (!(wm->cursor = XCreateFontCursor(wm->dpy, XC_X_cursor)))
    return false;

  XDefineCursor(wm->dpy, wm->root, wm->cursor);

  XSelectInput(wm->dpy, wm->root,
               SubstructureRedirectMask | SubstructureNotifyMask |
                   KeyPressMask | FocusChangeMask);

  initBar(wm);

  return true;
}

bool applyConfigsInWindowManager(WindowManager *wm) {
  wm->workspaces = calloc(wm->config.nWorkspaces, sizeof(Workspace));
  if (!wm->workspaces) {
    fprintf(stderr, "Memory allocation failed for workspaces\n");
    return false;
  }

  for (size_t i = 0; i < wm->config.nWorkspaces; i++) {
    wm->workspaces[i].clients = NULL;
    wm->workspaces[i].focused = NULL;
    wm->workspaces[i].master = NULL;
    wm->workspaces[i].fullscreenClient = NULL;
  }

  initEWMH(wm);

  return true;
}

void cleanupWindowManager(WindowManager *wm) {
  freeClients(wm);
  free(wm->workspaces);

  XFreeCursor(wm->dpy, wm->cursor);
  XDestroyWindow(wm->dpy, wm->supportedWmCheckWindow);
  XDestroyWindow(wm->dpy, wm->bar.window);
  XCloseDisplay(wm->dpy);
}

void arrangeWindows(WindowManager *wm) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];
  if (!currentWorkspace->clients) return;

  if (currentWorkspace->fullscreenClient) {
    fullscreenLayout(wm);
  } else {
    switch (wm->currentLayout) {
      case MASTER: masterLayout(wm); break;
      case MONOCLE: monocleLayout(wm); break;
    }
  }
}

void masterLayout(WindowManager *wm) {
  Workspace *ws = &wm->workspaces[wm->currentWorkspace];
  if (!ws->clients) return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);
  const int screenWidth = rootAttr.width;
  const int screenHeight = rootAttr.height;
  
  // Account for bar and gaps
  const int topOffset = wm->config.barHeight + wm->config.gaps;
  const int usableWidth = screenWidth - 2 * wm->config.gaps;
  const int usableHeight = screenHeight - topOffset - wm->config.gaps;

  // Count clients and ensure master is set
  int nClients = 0;
  for (Client *c = ws->clients; c; c = c->workspaceNext) nClients++;
  
  if (!ws->master) ws->master = ws->clients;

  // Special case: single window
  if (nClients == 1) {
    XMoveResizeWindow(wm->dpy, ws->clients->window,
                      wm->config.gaps, topOffset,
                      usableWidth, usableHeight);
    return;
  }

  // Calculate master and stack areas
  const int masterWidth = usableWidth * wm->masterRatio;
  const int stackWidth = usableWidth - masterWidth - wm->config.gaps;
  
  // Calculate stack window heights
  const int totalGaps = (nClients - 2) * wm->config.gaps;
  const int stackWinHeight = (usableHeight - totalGaps) / (nClients - 1);
  const int remainder = (usableHeight - totalGaps) % (nClients - 1);

  // Position master window
  XMoveResizeWindow(wm->dpy, ws->master->window,
                    wm->config.gaps, topOffset,
                    masterWidth, usableHeight);

  // Position stack windows
  int stackX = wm->config.gaps + masterWidth + wm->config.gaps;
  int y = topOffset;
  int stackIndex = 0;
  
  for (Client *c = ws->clients; c; c = c->workspaceNext) {
    if (c == ws->master) continue;
    
    const int height = stackWinHeight + (stackIndex == nClients - 2 ? remainder : 0);
    XMoveResizeWindow(wm->dpy, c->window, stackX, y, stackWidth, height);
    
    y += height + wm->config.gaps;
    stackIndex++;
  }
}

void monocleLayout(WindowManager *wm) {
  Workspace *ws = &wm->workspaces[wm->currentWorkspace];
  if (!ws->clients) return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);
  
  // Consistent with master layout
  const int topOffset = wm->config.barHeight + wm->config.gaps;
  const int usableWidth = rootAttr.width - 2 * wm->config.gaps;
  const int usableHeight = rootAttr.height - topOffset - wm->config.gaps;

  // Ensure focus is set
  if (!ws->focused) ws->focused = ws->clients;

  for (Client *c = ws->clients; c; c = c->workspaceNext) {
    if (c == ws->focused) {
      XMapWindow(wm->dpy, c->window);
      XMoveResizeWindow(wm->dpy, c->window,
                        wm->config.gaps, topOffset,
                        usableWidth, usableHeight);
    } else {
      XUnmapWindow(wm->dpy, c->window);
    }
  }
}

void fullscreenLayout(WindowManager *wm) {
  Workspace *ws = &wm->workspaces[wm->currentWorkspace];
  if (!ws->clients || !ws->fullscreenClient) return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);

  for (Client *c = ws->clients; c; c = c->workspaceNext) {
    if (c == ws->fullscreenClient) {
      XMoveResizeWindow(wm->dpy, c->window, 0, 0,
                        rootAttr.width, rootAttr.height);
      XMapRaised(wm->dpy, c->window);
    } else {
      XUnmapWindow(wm->dpy, c->window);
    }
  }
  
  setFocus(wm, ws->fullscreenClient);
}
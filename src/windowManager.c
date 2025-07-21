#include "../include/windowManager.h"
#include "../include/actions.h"
#include "../include/atoms.h"
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

  if (!(wm->cursor = XCreateFontCursor(wm->dpy, XC_X_cursor)))
    return false;

  XDefineCursor(wm->dpy, wm->root, wm->cursor);

  XSelectInput(wm->dpy, wm->root,
               SubstructureRedirectMask | SubstructureNotifyMask |
                   KeyPressMask | FocusChangeMask);

  initAtoms(wm);
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

      setNumberOfDesktopsAtom(wm, wm->config.nWorkspaces);

    return true;
}

void cleanupWindowManager(WindowManager *wm) {
    freeClients(wm);
    free(wm->workspaces);
    
    XFreeCursor(wm->dpy, wm->cursor);
    XDestroyWindow(wm->dpy, wm->bar.window);
    XCloseDisplay(wm->dpy);
}

void arrangeWindows(WindowManager *wm) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

#ifdef debug
  int nClients = 0;


  Client *c = currentWorkspace->clients;
  while (c) {
    nClients++;

    c = c->next;
  }
  debug_print("Arranging %zu windows in workspace %zu\n", nClients,
              wm->currentWorkspace);
#endif

  if (!currentWorkspace->clients)
    return;

  if (currentWorkspace->fullscreenClient) {
    fullscreenLayout(wm);
  } else {

    switch (wm->currentLayout) {
    case MASTER:
      masterLayout(wm);
      break;
    case MONOCLE:
      monocleLayout(wm);
      break;
    }
  }
}

void masterLayout(WindowManager *wm) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

#ifdef debug
  debug_print("Master layout: master=0x%lx\n",
              currentWorkspace->master->window);
#endif
  if (!currentWorkspace->clients)
    return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);
  int screenWidth = rootAttr.width;
  int screenHeight = rootAttr.height;

  int topExtraSpace = wm->config.barHeight + wm->config.gaps;
  int usableWidth = screenWidth - 2 * wm->config.gaps;
  int usableHeight = screenHeight - topExtraSpace - wm->config.gaps;

  int nClients = 0;
  Client *c = currentWorkspace->clients;

  while (c) {
    nClients++;

    c = c->next;
  }

  if (!currentWorkspace->master) {
    currentWorkspace->master = currentWorkspace->clients;
  }

  if (nClients == 1) {
    XMoveResizeWindow(wm->dpy, currentWorkspace->clients->window,
                      wm->config.gaps, topExtraSpace, usableWidth,
                      usableHeight);
    return;
  }

  int masterWidth = usableWidth * wm->masterRatio;
  int stackWidth = usableWidth - masterWidth - wm->config.gaps;

  XMoveResizeWindow(wm->dpy, currentWorkspace->master->window, wm->config.gaps,
                    topExtraSpace, masterWidth, usableHeight);
  
  int stackWinHeight = 0;
  int remainder = 0;
  if (nClients > 1) {
      int totalGaps = (nClients - 2) * wm->config.gaps;
      stackWinHeight = (usableHeight - totalGaps) / (nClients - 1);
      remainder = (usableHeight - totalGaps) % (nClients - 1);
  }

  int stackX = wm->config.gaps + masterWidth + wm->config.gaps;
  int y = topExtraSpace;

  c = currentWorkspace->clients;
  int stackIndex = 0;

  while (c) {
      if (c != currentWorkspace->master) {
          int height = stackWinHeight;
          if (stackIndex == nClients - 2) {
              height += remainder;
          }
          
          XMoveResizeWindow(wm->dpy, c->window, stackX, y, stackWidth, height);
          y += height + wm->config.gaps;
          stackIndex++;
      }
      c = c->next;
  }
}

void monocleLayout(WindowManager *wm) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (!currentWorkspace->clients)
    return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);
  int screenWidth = rootAttr.width;
  int screenHeight = rootAttr.height;

  int topExtraSpace = wm->config.barHeight + wm->config.gaps + 3;
  int usableWidth = screenWidth - 2 * wm->config.gaps;
  int usableHeight = screenHeight - topExtraSpace - wm->config.gaps;

  if (!currentWorkspace->focused) {
    currentWorkspace->focused = currentWorkspace->clients;
  }

  Client *c = currentWorkspace->clients;

  while (c) {
    if (c == currentWorkspace->focused) {
      XMapWindow(wm->dpy, c->window);

      XMoveResizeWindow(wm->dpy, c->window, wm->config.gaps, topExtraSpace,
                        usableWidth, usableHeight);

    } else {
      XUnmapWindow(wm->dpy, c->window);
    }

    c = c->next;
  }
}

void fullscreenLayout(WindowManager *wm) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (!currentWorkspace->clients)
    return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);

  Client *c = currentWorkspace->clients;

  while (c) {
    if (c == currentWorkspace->fullscreenClient) {
      XMoveResizeWindow(wm->dpy, c->window, 0, 0, rootAttr.width,
                        rootAttr.height);
      XMapRaised(wm->dpy, c->window);
    } else {
      XUnmapWindow(wm->dpy, c->window);
    }

    c = c->next;
  }

  setFocus(wm, currentWorkspace->fullscreenClient);
}

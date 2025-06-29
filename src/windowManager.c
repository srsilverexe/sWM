#include "../include/windowManager.h"
#include "../include/atoms.h"
#include "../include/bar.h"
#include "../include/client.h"
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

bool initWindowManager(WindowManager *wm) {
  if (!(wm->dpy = XOpenDisplay(NULL)))
    return false;

  wm->root = DefaultRootWindow(wm->dpy);
  for (size_t i = 0; i < 10; i++) {
    wm->workspaces[wm->currentWorkspace].clients = NULL;
    wm->workspaces[wm->currentWorkspace].focused = NULL;
    wm->workspaces[wm->currentWorkspace].master = NULL;
  }

  wm->masterRatio = DEFAULT_MASTER_RATIO;
  wm->currentWorkspace = 0;

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

void cleanupWindowManager(WindowManager *wm) {
  XFreeCursor(wm->dpy, wm->cursor);
  XCloseDisplay(wm->dpy);
  freeClients(wm);
  XDestroyWindow(wm->dpy, wm->bar.window);
}

void arrangeWindows(WindowManager *wm) {
  if (!wm->workspaces[wm->currentWorkspace].clients)
    return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);
  int screenWidth = rootAttr.width;
  int screenHeight = rootAttr.height;

  int topExtraSpace = wm->config.barHeight + wm->config.gaps + 3;
  int usableWidth = screenWidth - 2 * wm->config.gaps;
  int usableHeight = screenHeight - topExtraSpace - wm->config.gaps;

  int nClients = 0;
  for (Client *c = wm->workspaces[wm->currentWorkspace].clients; c;
       c = c->next) {
    nClients++;
  }

  if (!wm->workspaces[wm->currentWorkspace].master) {
    wm->workspaces[wm->currentWorkspace].master =
        wm->workspaces[wm->currentWorkspace].clients;
  }

  if (nClients == 1) {
    XMoveResizeWindow(
        wm->dpy, wm->workspaces[wm->currentWorkspace].clients->window,
        wm->config.gaps, topExtraSpace, usableWidth, usableHeight);
    return;
  }

  int masterWidth = usableWidth * wm->masterRatio;
  int stackWidth = usableWidth - masterWidth - wm->config.gaps;

  // Position master window
  XMoveResizeWindow(wm->dpy,
                    wm->workspaces[wm->currentWorkspace].master->window,
                    wm->config.gaps, topExtraSpace, masterWidth, usableHeight);

  int stackX = wm->config.gaps + masterWidth + wm->config.gaps;
  int stackY = topExtraSpace;
  int stackWinHeight =
      (usableHeight - (nClients - 2) * wm->config.gaps) / (nClients - 1);

  Client *c = wm->workspaces[wm->currentWorkspace].clients;
  int stackIndex = 0;

  while (c) {
    if (c != wm->workspaces[wm->currentWorkspace].master) {
      int y = stackY + stackIndex * (stackWinHeight + wm->config.gaps);
      int height = (stackIndex == nClients - 2)
                       ? (usableHeight - y + wm->config.gaps)
                       : stackWinHeight;

      XMoveResizeWindow(wm->dpy, c->window, stackX, y, stackWidth, height);

      stackIndex++;
    }
    c = c->next;
  }
}

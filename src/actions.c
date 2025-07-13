#include "../include/actions.h"
#include "../include/bar.h"
#include "../include/client.h"
#include <X11/X.h>
#include <X11/Xatom.h>
#include <stdio.h>

void moveFocusedWindow(WindowManager *wm, Directions direction) {
  if (!wm->workspaces[wm->currentWorkspace].focused)
    return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);

  XWindowAttributes attr;
  if (!XGetWindowAttributes(
          wm->dpy, wm->workspaces[wm->currentWorkspace].focused->window,
          &attr)) {
    removeClient(wm, wm->workspaces[wm->currentWorkspace].focused);
    wm->workspaces[wm->currentWorkspace].focused =
        wm->workspaces[wm->currentWorkspace].clients;
    return;
  }

  int x = attr.x;
  int y = attr.y;

  switch (direction) {
  case LEFT:
    x -= 5;
    break;
  case RIGHT:
    x += 5;
    break;
  case UP:
    y -= 5;
    break;
  case DOWN:
    y += 5;
    break;
  }

  // Apply clamping
  x = CLAMP(0, rootAttr.width - attr.width, x);
  y = CLAMP(0, rootAttr.height - attr.height, y);

  XMoveWindow(wm->dpy, wm->workspaces[wm->currentWorkspace].focused->window, x,
              y);
}

void resizeFocusedWindow(WindowManager *wm, ResizeTypes type) {
  if (!wm->workspaces[wm->currentWorkspace].focused)
    return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);

  XWindowAttributes attr;
  if (!XGetWindowAttributes(
          wm->dpy, wm->workspaces[wm->currentWorkspace].focused->window,
          &attr)) {
    removeClient(wm, wm->workspaces[wm->currentWorkspace].focused);
    wm->workspaces[wm->currentWorkspace].focused =
        wm->workspaces[wm->currentWorkspace].clients;
    return;
  }

  int w = attr.width;
  int h = attr.height;

  switch (type) {
  case EXPAND_HORIZONTAL:
    w += 5;
    break;
  case SHRINK_HORIZONTAL:
    w -= 5;
    break;
  case EXPAND_VERTICAL:
    h += 5;
    break;
  case SHRINK_VERTICAL:
    h -= 5;
    break;
  }

  w = CLAMP(MIN_WINDOW_SIZE, rootAttr.width - attr.x, w);
  h = CLAMP(MIN_WINDOW_SIZE, rootAttr.height - attr.y, h);

  XResizeWindow(wm->dpy, wm->workspaces[wm->currentWorkspace].focused->window,
                w, h);
}

void focusToDirection(WindowManager *wm, Directions direction) {
  if (!wm->workspaces[wm->currentWorkspace].focused ||
      !wm->workspaces[wm->currentWorkspace].clients)
    return;

  Client *target = NULL;

  if (direction == LEFT || direction == UP) {
    target = wm->workspaces[wm->currentWorkspace].focused->prev;
    if (!target) {
      Client *last = wm->workspaces[wm->currentWorkspace].clients;
      while (last && last->next)
        last = last->next;
      target = last;
    }
  } else {
    target = wm->workspaces[wm->currentWorkspace].focused->next;
    if (!target) {
      target = wm->workspaces[wm->currentWorkspace].clients;
    }
  }

  if (target) {
    setFocus(wm, target);
  }
}

void killFocusedWindow(WindowManager *wm) {
  if (!wm->workspaces[wm->currentWorkspace].focused)
    return;

  // Get atoms for protocols
  Atom wmProtocols = XInternAtom(wm->dpy, "WM_PROTOCOLS", False);
  Atom wmDeleteWindow = XInternAtom(wm->dpy, "WM_DELETE_WINDOW", False);

  Atom *protocols = NULL;
  int count;

  // Check if window supports WM_DELETE_WINDOW protocol
  if (XGetWMProtocols(wm->dpy,
                      wm->workspaces[wm->currentWorkspace].focused->window,
                      &protocols, &count)) {
    for (size_t i = 0; i < count; i++) {
      if (protocols[i] == wmDeleteWindow) {
        // Send polite close request
        XEvent ev = {0};

        ev.xclient.type = ClientMessage;
        ev.xclient.window =
            wm->workspaces[wm->currentWorkspace].focused->window;
        ev.xclient.message_type = wmProtocols;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = wmDeleteWindow;
        ev.xclient.data.l[1] = CurrentTime;

        XSendEvent(wm->dpy,
                   wm->workspaces[wm->currentWorkspace].focused->window, False,
                   NoEventMask, &ev);
        XFree(protocols);
        return;
      }
    }
  }

  // If don't has the protocol support use the EWMH fallback method
  Atom netCloseWindow = XInternAtom(wm->dpy, "_NET_CLOSE_WINDOW", False);
  XEvent ev = {0};

  ev.xclient.type = ClientMessage;
  ev.xclient.window = wm->workspaces[wm->currentWorkspace].focused->window;
  ev.xclient.message_type = netCloseWindow;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime;
  ev.xclient.data.l[1] = 2; // Source: application

  XSendEvent(wm->dpy, wm->root, False,
             SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

void changeWorkspace(WindowManager *wm, size_t targetWorkspace) {
  if (targetWorkspace >= 10) {
    fprintf(stderr, "Invalid workspace index: %zu\n", targetWorkspace);
    return;
  }

  if (targetWorkspace == wm->currentWorkspace)
    return;

  for (Client *c = wm->workspaces[wm->currentWorkspace].clients; c;
       c = c->next) {
    XUnmapWindow(wm->dpy, c->window);
  }

  size_t previusWorkspace = wm->currentWorkspace;
  wm->currentWorkspace = targetWorkspace;
  updateBar(wm);

  for (Client *c = wm->workspaces[wm->currentWorkspace].clients; c;
       c = c->next) {
    XMapWindow(wm->dpy, c->window);
  }

  setFocus(wm, wm->workspaces[wm->currentWorkspace].focused);

  arrangeWindows(wm);

  Atom netCurrentDesktop = XInternAtom(wm->dpy, "_NET_CURRENT_DESKTOP", False);
  unsigned long data[] = {wm->currentWorkspace};
  XChangeProperty(wm->dpy, wm->root, netCurrentDesktop, XA_CARDINAL, 32,
                  PropModeReplace, (unsigned char *)data, 1);
}

void moveFocusedWindowToWorkspace(WindowManager *wm, size_t targetWorkspace) {
  if (targetWorkspace >= WORKSPACE_COUNT) {
    fprintf(stderr, "Invalid workspace index: %zu\n", targetWorkspace);
    return;
  }
  if (targetWorkspace == wm->currentWorkspace ||
      !wm->workspaces[wm->currentWorkspace].focused)
    return;
  Client *focusedClient = wm->workspaces[wm->currentWorkspace].focused;
  focusToDirection(wm, RIGHT);

  Window focusedWin = focusedClient->window;

  removeClientFromAWorkspace(wm, focusedClient, wm->currentWorkspace);
  addClientFromAWorkspace(wm, focusedWin, targetWorkspace);

  Client *newClient = wm->workspaces[targetWorkspace].clients;
  setFocusFromAWorkspace(wm, newClient, targetWorkspace);
}

void handleMapRequest(WindowManager *wm, XMapRequestEvent ev) {
  Atom type = None;
  Atom actualType;
  int actualFormat;
  size_t nItems, bytesAfter;
  unsigned char *data = NULL;

  if (XGetWindowProperty(wm->dpy, ev.window, wm->netWmWindowType, 0, 1, False,
                         XA_ATOM, &actualType, &actualFormat, &nItems,
                         &bytesAfter, &data) == Success &&
      data) {
    if (nItems > 0) {
      type = ((Atom *)data)[0];
    }
    XFree(data);
  }

  if (type == wm->netWmWindowTypeDesktop || type == wm->netWmWindowTypeDock) {
    XMapWindow(wm->dpy, ev.window);

    XSelectInput(wm->dpy, ev.window, PropertyChangeMask);
    return;
  }

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);

  int topOffset = wm->config.barHeight + wm->config.gaps;
  XMoveResizeWindow(wm->dpy, ev.window, rootAttr.width / 4,
                    topOffset, // BELOW BAR
                    400, 300);

  XMapWindow(wm->dpy, ev.window);

  addClient(wm, ev.window);
}

void handleDestroyNotify(WindowManager *wm, XDestroyWindowEvent ev) {
  Client *c = findClient(wm, ev.window);
  if (c) {
    removeClient(wm, c);
  }
}

void handleUnmapNotify(WindowManager *wm, XUnmapEvent ev) {
  // Client *c = findClient(wm, ev.window);
  // if (c) {
  //   removeClient(wm, c);
  // }
}

void handleFocusChange(WindowManager *wm, XFocusChangeEvent ev) {
  if (ev.mode == NotifyGrab || ev.mode == NotifyUngrab)
    return;

  if (ev.type != FocusIn)
    return;

  Client *newFocus = findClient(wm, ev.window);
  if (newFocus && newFocus != wm->workspaces[wm->currentWorkspace].focused) {
    setFocus(wm, newFocus);
  }
}

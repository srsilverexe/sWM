#include "../include/actions.h"
#include "../include/bar.h"
#include "../include/client.h"
#include <X11/X.h>
#include <X11/Xatom.h>
#include <stdio.h>

#ifdef debug
#define debug_print(...) fprintf(stderr, "[ACTIONS] " __VA_ARGS__)
#else
#define debug_print(...)
#endif

void moveFocusedWindow(WindowManager *wm, Directions direction) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

#ifdef debug
  debug_print("Moving window: 0x%lx\n", currentWorkspace->focused->window);
#endif

  if (!currentWorkspace->focused)
    return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);

  XWindowAttributes attr;
  if (!XGetWindowAttributes(wm->dpy, currentWorkspace->focused->window,
                            &attr)) {
    removeClient(wm, currentWorkspace->focused);
    currentWorkspace->focused = currentWorkspace->clients;
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
  default:
    break;
  }

  // Apply clamping
  x = CLAMP(0, rootAttr.width - attr.width, x);
  y = CLAMP(0, rootAttr.height - attr.height, y);

  XMoveWindow(wm->dpy, currentWorkspace->focused->window, x, y);
}

void resizeFocusedWindow(WindowManager *wm, ResizeTypes type) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (!currentWorkspace->focused)
    return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);

  XWindowAttributes attr;
  if (!XGetWindowAttributes(wm->dpy, currentWorkspace->focused->window,
                            &attr)) {
    removeClient(wm, currentWorkspace->focused);
    currentWorkspace->focused = currentWorkspace->clients;
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

  XResizeWindow(wm->dpy, currentWorkspace->focused->window, w, h);
}

void focusToDirection(WindowManager *wm, Directions direction) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (!currentWorkspace->focused || !currentWorkspace->clients)
    return;

  Client *target = NULL;

  if (direction == LEFT || direction == UP) {
    target = currentWorkspace->focused->prev;
    if (!target) {
      Client *last = currentWorkspace->clients;
      while (last && last->next)
        last = last->next;
      target = last;
    }
  } else {
    target = currentWorkspace->focused->next;
    if (!target) {
      target = currentWorkspace->clients;
    }
  }

  if (target) {
    setFocus(wm, target);
  }
}

void killFocusedWindow(WindowManager *wm) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (!currentWorkspace->focused)
    return;

  // Get atoms for protocols
  Atom wmProtocols = XInternAtom(wm->dpy, "WM_PROTOCOLS", False);
  Atom wmDeleteWindow = XInternAtom(wm->dpy, "WM_DELETE_WINDOW", False);

  Atom *protocols = NULL;
  int count;

  // Check if window supports WM_DELETE_WINDOW protocol
  if (XGetWMProtocols(wm->dpy, currentWorkspace->focused->window, &protocols,
                      &count)) {
    for (int i = 0; i < count; i++) {
      if (protocols[i] == wmDeleteWindow) {
        // Send polite close request
        XEvent ev = {0};

        ev.xclient.type = ClientMessage;
        ev.xclient.window = currentWorkspace->focused->window;
        ev.xclient.message_type = wmProtocols;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = wmDeleteWindow;
        ev.xclient.data.l[1] = CurrentTime;

        XSendEvent(wm->dpy, currentWorkspace->focused->window, False,
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
  ev.xclient.window = currentWorkspace->focused->window;
  ev.xclient.message_type = netCloseWindow;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime;
  ev.xclient.data.l[1] = 2; // Source: application

  XSendEvent(wm->dpy, wm->root, False,
             SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

void killWindow(WindowManager *wm, Client *c) {
  if (!c)
    return;

  // Get atoms for protocols
  Atom wmProtocols = XInternAtom(wm->dpy, "WM_PROTOCOLS", False);
  Atom wmDeleteWindow = XInternAtom(wm->dpy, "WM_DELETE_WINDOW", False);

  Atom *protocols = NULL;
  int count;

  // Check if window supports WM_DELETE_WINDOW protocol
  if (XGetWMProtocols(wm->dpy, c->window, &protocols, &count)) {
    for (int i = 0; i < count; i++) {
      if (protocols[i] == wmDeleteWindow) {
        // Send polite close request
        XEvent ev = {0};

        ev.xclient.type = ClientMessage;
        ev.xclient.window = c->window;
        ev.xclient.message_type = wmProtocols;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = wmDeleteWindow;
        ev.xclient.data.l[1] = CurrentTime;

        XSendEvent(wm->dpy, c->window, False, NoEventMask, &ev);
        XFree(protocols);
        return;
      }
    }
  }

  // If don't has the protocol support use the EWMH fallback method
  Atom netCloseWindow = XInternAtom(wm->dpy, "_NET_CLOSE_WINDOW", False);
  XEvent ev = {0};

  ev.xclient.type = ClientMessage;
  ev.xclient.window = c->window;
  ev.xclient.message_type = netCloseWindow;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime;
  ev.xclient.data.l[1] = 2; // Source: application

  XSendEvent(wm->dpy, wm->root, False,
             SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

void changeWorkspace(WindowManager *wm, size_t targetWorkspace) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (targetWorkspace >= 10) {
    fprintf(stderr, "Invalid workspace index: %zu\n", targetWorkspace);
    return;
  }

  if (targetWorkspace == wm->currentWorkspace)
    return;

  Client *c = currentWorkspace->clients;

  while (c) {
    XUnmapWindow(wm->dpy, c->window);

    c = c->next;
  }

  wm->currentWorkspace = targetWorkspace;
  updateBar(wm);

  c = currentWorkspace->clients;

  while (c) {
    XMapWindow(wm->dpy, c->window);

    c = c->next;
  }

  setFocus(wm, currentWorkspace->focused);

  arrangeWindows(wm);

  Atom netCurrentDesktop = XInternAtom(wm->dpy, "_NET_CURRENT_DESKTOP", False);
  unsigned long data[] = {wm->currentWorkspace};
  XChangeProperty(wm->dpy, wm->root, netCurrentDesktop, XA_CARDINAL, 32,
                  PropModeReplace, (unsigned char *)data, 1);
}

void moveFocusedWindowToWorkspace(WindowManager *wm, size_t targetWorkspace) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (targetWorkspace >= WORKSPACE_COUNT) {
    fprintf(stderr, "Invalid workspace index: %zu\n", targetWorkspace);
    return;
  }
  if (targetWorkspace == wm->currentWorkspace || !currentWorkspace->focused)
    return;
  Client *focusedClient = currentWorkspace->focused;
  focusToDirection(wm, RIGHT);

  Window focusedWin = focusedClient->window;

  removeClientFromAWorkspace(wm, focusedClient, wm->currentWorkspace);
  addClientFromAWorkspace(wm, focusedWin, targetWorkspace);

  Client *newClient = wm->workspaces[targetWorkspace].clients;
  setFocusFromAWorkspace(wm, newClient, targetWorkspace);
}

void updateFullscreenState(WindowManager *wm, Client *c, bool fullscreen) {
  Workspace *ws = &wm->workspaces[wm->currentWorkspace];

  if (fullscreen) {
    if (ws->fullscreenClient && ws->fullscreenClient != c) {
      updateFullscreenState(wm, ws->fullscreenClient, false);
    }
    c->fullscreen = true;
    ws->fullscreenClient = c;
  } else {
    c->fullscreen = false;
    if (ws->fullscreenClient == c) {
      ws->fullscreenClient = NULL;
    }
  }
  arrangeWindows(wm);
}

void handleClientMessage(WindowManager *wm, XClientMessageEvent ev) {
  if (ev.message_type == wm->netWmState) {
    Atom fullscreen = wm->netWmStateFullscreen;

    if (ev.data.l[1] == fullscreen || ev.data.l[2] == fullscreen) {
      Client *c = findClient(wm, ev.window);
      if (c) {
        bool makeFullscreen = (ev.data.l[0] == 1);

        updateFullscreenState(wm, c, makeFullscreen);
      }
    }
  }
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
#ifdef debug
  debug_print("DestroyNotify: window=0x%lx\n", ev.window);
#endif
  Client *c = findClient(wm, ev.window);
  if (c) {
    removeClient(wm, c);
  }
}

void handleUnmapNotify(WindowManager *wm, XUnmapEvent ev) {
  (void)wm;
  (void)ev;
  // Client *c = findClient(wm, ev.window);
  // if (c) {
  //   removeClient(wm, c);
  // }
}

void handleFocusChange(WindowManager *wm, XFocusChangeEvent ev) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (ev.mode == NotifyGrab || ev.mode == NotifyUngrab)
    return;
  if (ev.type != FocusIn)
    return;

  // Validate window before processing
  XWindowAttributes attr;
  if (!XGetWindowAttributes(wm->dpy, ev.window, &attr)) {
    return;
  }

  Client *newFocus = findClient(wm, ev.window);
  if (newFocus && newFocus != currentWorkspace->focused) {
    setFocus(wm, newFocus);
  }
}

#include "../include/actions.h"
#include "../include/client.h"
#include <X11/X.h>
#include <X11/Xatom.h>

void moveFocusedWindow(WindowManager *wm, Directions direction) {
  if (!wm->focused)
    return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);

  XWindowAttributes attr;
  if (!XGetWindowAttributes(wm->dpy, wm->focused->window, &attr)) {
    removeClient(wm, wm->focused);
    wm->focused = wm->clients;
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

  XMoveWindow(wm->dpy, wm->focused->window, x, y);
}

void resizeFocusedWindow(WindowManager *wm, ResizeTypes type) {
  if (!wm->focused)
    return;

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);

  XWindowAttributes attr;
  if (!XGetWindowAttributes(wm->dpy, wm->focused->window, &attr)) {
    removeClient(wm, wm->focused);
    wm->focused = wm->clients;
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

  XResizeWindow(wm->dpy, wm->focused->window, w, h);
}

void focusToDirection(WindowManager *wm, Directions direction) {
  if (!wm->focused || !wm->clients)
    return;

  Client *target = NULL;

  if (direction == LEFT || direction == UP) {
    target = wm->focused->prev;
    if (!target) {
      Client *last = wm->clients;
      while (last && last->next)
        last = last->next;
      target = last;
    }
  } else {
    target = wm->focused->next;
    if (!target) {
      target = wm->clients;
    }
  }

  if (target) {
    setFocus(wm, target);
  }
}

void killFocusedWindow(WindowManager *wm) {
  if (!wm->focused)
    return;

  // Get atoms for protocols
  Atom wmProtocols = XInternAtom(wm->dpy, "WM_PROTOCOLS", False);
  Atom wmDeleteWindow = XInternAtom(wm->dpy, "WM_DELETE_WINDOW", False);

  Atom *protocols = NULL;
  int count;

  // Check if window supports WM_DELETE_WINDOW protocol
  if (XGetWMProtocols(wm->dpy, wm->focused->window, &protocols, &count)) {
    for (size_t i = 0; i < count; i++) {
      if (protocols[i] == wmDeleteWindow) {
        // Send polite close request
        XEvent ev = {0};

        ev.xclient.type = ClientMessage;
        ev.xclient.window = wm->focused->window;
        ev.xclient.message_type = wmProtocols;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = wmDeleteWindow;
        ev.xclient.data.l[1] = CurrentTime;

        XSendEvent(wm->dpy, wm->focused->window, False, NoEventMask, &ev);
        XFree(protocols);
        return;
      }
    }
  }

  // If don't has the protocol support use the EWMH fallback method
  Atom netCloseWindow = XInternAtom(wm->dpy, "_NET_CLOSE_WINDOW", False);
  XEvent ev = {0};

  ev.xclient.type = ClientMessage;
  ev.xclient.window = wm->focused->window;
  ev.xclient.message_type = netCloseWindow;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime;
  ev.xclient.data.l[1] = 2; // Source: application

  XSendEvent(wm->dpy, wm->root, False,
             SubstructureRedirectMask | SubstructureNotifyMask, &ev);
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
  Client *c = findClient(wm, ev.window);
  if (c) {
    removeClient(wm, c);
  }
}

void handleFocusChange(WindowManager *wm, XFocusChangeEvent ev) {
  if (ev.mode == NotifyGrab || ev.mode == NotifyUngrab)
    return;

  if (ev.type != FocusIn)
    return;

  Client *newFocus = findClient(wm, ev.window);
  if (newFocus && newFocus != wm->focused) {
    setFocus(wm, newFocus);
  }
}

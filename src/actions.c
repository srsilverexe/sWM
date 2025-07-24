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

void focusToDirection(WindowManager *wm, Directions direction) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (!currentWorkspace->focused || !currentWorkspace->clients)
    return;

  Client *target = NULL;

  if (direction == LEFT || direction == UP) {
    target = currentWorkspace->focused->workspacePrev;
    if (!target) {
      Client *last = currentWorkspace->clients;
      while (last && last->workspaceNext)
        last = last->workspaceNext;
      target = last;
    }
  } else {
    target = currentWorkspace->focused->workspaceNext;
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

  Atom wmProtocols = XInternAtom(wm->dpy, "WM_PROTOCOLS", False);
  Atom wmDeleteWindow = XInternAtom(wm->dpy, "WM_DELETE_WINDOW", False);

  Atom *protocols = NULL;
  int count;

  if (XGetWMProtocols(wm->dpy, currentWorkspace->focused->window, &protocols,
                      &count)) {
    for (int i = 0; i < count; i++) {
      if (protocols[i] == wmDeleteWindow) {
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

  Atom netCloseWindow = XInternAtom(wm->dpy, "_NET_CLOSE_WINDOW", False);
  XEvent ev = {0};

  ev.xclient.type = ClientMessage;
  ev.xclient.window = currentWorkspace->focused->window;
  ev.xclient.message_type = netCloseWindow;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime;
  ev.xclient.data.l[1] = 2;

  XSendEvent(wm->dpy, wm->root, False,
             SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

void killWindow(WindowManager *wm, Client *c) {
  if (!c)
    return;

  Atom wmProtocols = XInternAtom(wm->dpy, "WM_PROTOCOLS", False);
  Atom wmDeleteWindow = XInternAtom(wm->dpy, "WM_DELETE_WINDOW", False);

  Atom *protocols = NULL;
  int count;

  if (XGetWMProtocols(wm->dpy, c->window, &protocols, &count)) {
    for (int i = 0; i < count; i++) {
      if (protocols[i] == wmDeleteWindow) {
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

  Atom netCloseWindow = XInternAtom(wm->dpy, "_NET_CLOSE_WINDOW", False);
  XEvent ev = {0};

  ev.xclient.type = ClientMessage;
  ev.xclient.window = c->window;
  ev.xclient.message_type = netCloseWindow;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime;
  ev.xclient.data.l[1] = 2;

  XSendEvent(wm->dpy, wm->root, False,
             SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

void changeWorkspace(WindowManager *wm, size_t targetWorkspace) {
  if (targetWorkspace >= wm->config.nWorkspaces) {
    fprintf(stderr, "Invalid workspace index: %zu\n", targetWorkspace);
    return;
  }

  if (targetWorkspace == wm->currentWorkspace)
    return;

  size_t oldWorkspace = wm->currentWorkspace;
  wm->currentWorkspace = targetWorkspace;

  Client *c = wm->workspaces[oldWorkspace].clients;
  while (c) {
    XUnmapWindow(wm->dpy, c->window);
    c = c->workspaceNext;
  }

  Workspace *newWorkspace = &wm->workspaces[targetWorkspace];
  c = newWorkspace->clients;
  while (c) {
    XMapWindow(wm->dpy, c->window);
    c = c->workspaceNext;
  }

  updateBar(wm);
  
  if (newWorkspace->focused) {
    setFocus(wm, newWorkspace->focused);
  } else if (newWorkspace->clients) {
    setFocus(wm, newWorkspace->clients);
  }

  if (newWorkspace->fullscreenClient) {
    fullscreenLayout(wm);
  } else {
    arrangeWindows(wm);
  }

  updateNetCurrentDesktop(wm);
}

void moveFocusedWindowToWorkspace(WindowManager *wm, size_t targetWorkspace) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (targetWorkspace >= wm->config.nWorkspaces) {
    fprintf(stderr, "Invalid workspace index: %zu\n", targetWorkspace);
    return;
  }
  if (targetWorkspace == wm->currentWorkspace || !currentWorkspace->focused)
    return;
  
  Client *focusedClient = currentWorkspace->focused;
  Window focusedWin = focusedClient->window;

  // Remove from current workspace
  if (currentWorkspace->master == focusedClient) {
    currentWorkspace->master = (focusedClient->workspaceNext) ? focusedClient->workspaceNext : focusedClient->workspacePrev;
  }

  if (focusedClient->workspacePrev) {
    focusedClient->workspacePrev->workspaceNext = focusedClient->workspaceNext;
  } else {
    currentWorkspace->clients = focusedClient->workspaceNext;
  }
  
  if (focusedClient->workspaceNext) {
    focusedClient->workspaceNext->workspacePrev = focusedClient->workspacePrev;
  }

  // Update focus in current workspace
  Client *newFocus = focusedClient->workspaceNext;
  if (!newFocus) newFocus = focusedClient->workspacePrev;
  
  if (newFocus) {
    setFocus(wm, newFocus);
  } else {
    currentWorkspace->focused = NULL;
    XSetInputFocus(wm->dpy, wm->root, RevertToPointerRoot, CurrentTime);
  }

  // Add to target workspace
  Workspace *target = &wm->workspaces[targetWorkspace];
  focusedClient->workspacePrev = NULL;
  focusedClient->workspaceNext = target->clients;
  focusedClient->workspaceIdx = targetWorkspace;
  
  if (target->clients) {
    target->clients->workspacePrev = focusedClient;
  }
  target->clients = focusedClient;

  if (!target->focused) {
    setFocusToAWorkspace(wm, focusedClient, targetWorkspace);
  }

  if (targetWorkspace == wm->currentWorkspace) {
    XMapWindow(wm->dpy, focusedWin);
  } else {
    XUnmapWindow(wm->dpy, focusedWin);
  }
  arrangeWindows(wm);

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
  EWMH *ewmh = &wm->ewmh;

  if (ev.message_type == ewmh->netWmState) {
    Atom fullscreen = ewmh->netWmStateFullscreen;

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
  EWMH *ewmh = &wm->ewmh;

  Atom type = None;
  Atom actualType;
  int actualFormat;
  size_t nItems, bytesAfter;
  unsigned char *data = NULL;

  if (XGetWindowProperty(wm->dpy, ev.window, ewmh->netWmWindowType, 0, 1, False,
                         XA_ATOM, &actualType, &actualFormat, &nItems,
                         &bytesAfter, &data) == Success &&
      data) {
    if (nItems > 0) {
      type = ((Atom *)data)[0];
    }
    XFree(data);
  }

  if (type == ewmh->netWmWindowTypeDialog || type == ewmh->netWmWindowTypeDock) {
    XMapWindow(wm->dpy, ev.window);

    XSelectInput(wm->dpy, ev.window, PropertyChangeMask);
    return;
  }

  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);

  int topOffset = wm->config.barHeight + wm->config.gaps;
  XMoveResizeWindow(wm->dpy, ev.window, rootAttr.width / 4, topOffset, 400,
                    300);

  XMapWindow(wm->dpy, ev.window);

  createClient(wm, ev.window, wm->currentWorkspace);
  updateNetClientList(wm);
  
  // Set initial focus if needed
  Workspace *ws = &wm->workspaces[wm->currentWorkspace];
  if (!ws->focused) {
    setFocus(wm, findClient(wm, ev.window));
  }
}

void handleDestroyNotify(WindowManager *wm, XDestroyWindowEvent ev) {
#ifdef debug
  debug_print("DestroyNotify: window=0x%lx\n", ev.window);
#endif
  Client *c = findClientInGlobalList(wm, ev.window);
  if (c) {
    destroyClient(wm, c);
  }
  updateNetClientList(wm);
}

void handleUnmapNotify(WindowManager *wm, XUnmapEvent ev) {
  if (ev.send_event || ev.event == wm->root)
    return;

  Client *c = findClient(wm, ev.window);
  if (c && c != wm->workspaces[wm->currentWorkspace].fullscreenClient) {
    destroyClient(wm, c);
    updateNetClientList(wm);
  }
}

void handleFocusChange(WindowManager *wm, XFocusChangeEvent ev) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (ev.mode == NotifyGrab || ev.mode == NotifyUngrab)
    return;
  if (ev.type != FocusIn)
    return;

  XWindowAttributes attr;
  if (!XGetWindowAttributes(wm->dpy, ev.window, &attr)) {
    return;
  }

  Client *newFocus = findClient(wm, ev.window);
  if (newFocus && newFocus != currentWorkspace->focused) {
    setFocus(wm, newFocus);
  }
}

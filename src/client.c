#include "../include/client.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdlib.h>

Client *findClient(WindowManager *wm, Window window) {
  for (Client *c = wm->workspaces[wm->currentWorkspace].clients; c;
       c = c->next) {
    if (c->window == window) {
      return c;
    }
  }
  return NULL;
}

void addClient(WindowManager *wm, Window window) {
  Client *c = calloc(1, sizeof(Client));
  if (!c)
    return;
  c->window = window;
  if (wm->workspaces[wm->currentWorkspace].clients) {
    wm->workspaces[wm->currentWorkspace].clients->prev = c;
    c->next = wm->workspaces[wm->currentWorkspace].clients;
  }
  wm->workspaces[wm->currentWorkspace].clients = c;

  XSetWindowBorder(wm->dpy, window, 0x000000);

  setFocus(wm, c);

  XSelectInput(wm->dpy, window, StructureNotifyMask | FocusChangeMask);

  if (!wm->workspaces[wm->currentWorkspace].master)
    wm->workspaces[wm->currentWorkspace].master = c;

  arrangeWindows(wm);
}

void addClientFromAWorkspace(WindowManager *wm, Window window,
                             size_t workspacesIdx) {
  if (workspacesIdx >= 10)
    return;

  Client *c = calloc(1, sizeof(Client));

  if (!c)
    return;

  c->window = window;
  if (wm->workspaces[workspacesIdx].clients) {
    wm->workspaces[workspacesIdx].clients->prev = c;
    c->next = wm->workspaces[workspacesIdx].clients;
  }
  wm->workspaces[workspacesIdx].clients = c;

  XSetWindowBorder(wm->dpy, window, 0x000000);

  setFocusFromAWorkspace(wm, c, workspacesIdx);

  XSelectInput(wm->dpy, window, StructureNotifyMask | FocusChangeMask);

  if (!wm->workspaces[workspacesIdx].master)
    wm->workspaces[workspacesIdx].master = c;

  arrangeWindows(wm);
}

void removeClient(WindowManager *wm, Client *c) {
  if (!c)
    return;

  if (wm->workspaces[wm->currentWorkspace].focused == c) {
    Client *newFocus = (c->next) ? c->next : c->prev;
    if (wm->workspaces[wm->currentWorkspace].focused) {
      setFocus(wm, newFocus);
    } else {
      wm->workspaces[wm->currentWorkspace].focused = NULL;
      XSetInputFocus(wm->dpy, wm->root, RevertToPointerRoot, CurrentTime);
    }
  }

  if (c == wm->workspaces[wm->currentWorkspace].master) {
    wm->workspaces[wm->currentWorkspace].master =
        wm->workspaces[wm->currentWorkspace].clients;
    if (wm->workspaces[wm->currentWorkspace].master == c)
      wm->workspaces[wm->currentWorkspace].master = c->next;
  }

  if (c->prev)
    c->prev->next = c->next;
  if (c->next)
    c->next->prev = c->prev;
  if (wm->workspaces[wm->currentWorkspace].clients == c)
    wm->workspaces[wm->currentWorkspace].clients = c->next;

  free(c);

  arrangeWindows(wm);
}

void removeClientFromAWorkspace(WindowManager *wm, Client *c,
                                size_t workspacesIdx) {
  if (workspacesIdx >= 10 || !c)
    return;

  if (wm->workspaces[workspacesIdx].focused == c) {
    Client *newFocus = (c->next) ? c->next : c->prev;

    if (wm->workspaces[workspacesIdx].focused) {
      setFocusFromAWorkspace(wm, newFocus, workspacesIdx);
    } else {
      wm->workspaces[workspacesIdx].focused = NULL;
      XSetInputFocus(wm->dpy, wm->root, RevertToPointerRoot, CurrentTime);
    }
  }

  if (c == wm->workspaces[workspacesIdx].master) {
    wm->workspaces[workspacesIdx].master =
        wm->workspaces[workspacesIdx].clients;
    if (wm->workspaces[workspacesIdx].master == c)
      wm->workspaces[workspacesIdx].master = c->next;
  }

  if (c->prev)
    c->prev->next = c->next;
  if (c->next)
    c->next->prev = c->prev;
  if (wm->workspaces[workspacesIdx].clients == c)
    wm->workspaces[workspacesIdx].clients = c->next;

  XUnmapWindow(wm->dpy, c->window);
  free(c);

  arrangeWindows(wm);
}

void setFocus(WindowManager *wm, Client *c) {
  if (!c || c == wm->workspaces[wm->currentWorkspace].focused)
    return;

  if (wm->workspaces[wm->currentWorkspace].focused) {
    XSetWindowBorder(wm->dpy,
                     wm->workspaces[wm->currentWorkspace].focused->window,
                     wm->config.unfocusedBorderColor);
    XSetWindowBorderWidth(wm->dpy,
                          wm->workspaces[wm->currentWorkspace].focused->window,
                          wm->config.borderSize);
  }

  wm->workspaces[wm->currentWorkspace].focused = c;
  XSetWindowBorder(wm->dpy, c->window, wm->config.focusedBorderColor);
  XSetWindowBorderWidth(wm->dpy, c->window, wm->config.borderSize);

  XSetInputFocus(wm->dpy, c->window, RevertToPointerRoot, CurrentTime);

  XRaiseWindow(wm->dpy, c->window);
  XFlush(wm->dpy);
}

void setFocusFromAWorkspace(WindowManager *wm, Client *c,
                            size_t workspacesIdx) {
  if (!c || c == wm->workspaces[workspacesIdx].focused)
    return;

  if (wm->workspaces[workspacesIdx].focused) {
    XSetWindowBorder(wm->dpy, wm->workspaces[workspacesIdx].focused->window,
                     wm->config.unfocusedBorderColor);
    XSetWindowBorder(wm->dpy, wm->workspaces[workspacesIdx].focused->window,
                     wm->config.borderSize);
  }

  wm->workspaces[workspacesIdx].focused = c;
  XSetWindowBorder(wm->dpy, c->window, wm->config.focusedBorderColor);
  XSetWindowBorderWidth(wm->dpy, c->window, wm->config.borderSize);

  if (workspacesIdx == wm->currentWorkspace) {
    XSetInputFocus(wm->dpy, c->window, RevertToPointerRoot, CurrentTime);
    XRaiseWindow(wm->dpy, c->window);
    XFlush(wm->dpy);
  }
}

void updateClients(WindowManager *wm) {
  for (size_t workspaceIdx = 0; workspaceIdx < 10; workspaceIdx++) {
    for (Client *c = wm->workspaces[workspaceIdx].clients; c; c = c->next) {
      XUnmapWindow(wm->dpy, c->window);
    }
  }

  for (Client *c = wm->workspaces[wm->currentWorkspace].clients; c;
       c = c->next) {
    XMapWindow(wm->dpy, c->window);
  }

  setFocus(wm, wm->workspaces[wm->currentWorkspace].focused);
  arrangeWindows(wm);
}

void freeClients(WindowManager *wm) {
  while (wm->workspaces[wm->currentWorkspace].clients) {
    Client *next = wm->workspaces[wm->currentWorkspace].clients->next;
    free(wm->workspaces[wm->currentWorkspace].clients);
    wm->workspaces[wm->currentWorkspace].clients = next;
  }
}

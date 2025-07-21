#include "../include/client.h"
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef debug
#define debug_print(...) fprintf(stderr, "[CLIENT] " __VA_ARGS__)
#else
#define debug_print(...)
#endif

static void safeSetWindowBorder(Display *dpy, Window window,
                                unsigned long color, unsigned int width) {
  if (window == 0)
    return;

  XWindowAttributes attr;
  if (!XGetWindowAttributes(dpy, window, &attr)) {
    return;
  }

  XSetWindowBorder(dpy, window, color);
  XSetWindowBorderWidth(dpy, window, width);
}

Client *findClient(WindowManager *wm, Window window) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

#ifdef debug
  debug_print("Searching for window: 0x%lx\n", window);
#endif

  Client *c = currentWorkspace->clients;

  while (c) {
    if (c->window == window) {
      return c;
    }

    c = c->next;
  }
  return NULL;
}

void addClient(WindowManager *wm, Window window) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

#ifdef debug
  debug_print("Adding client: window=0x%lx\n", window);
#endif
  Client *c = calloc(1, sizeof(Client));
  if (!c)
    return;
  c->window = window;
  if (currentWorkspace->clients) {
    currentWorkspace->clients->prev = c;
    c->next = currentWorkspace->clients;
  }
  currentWorkspace->clients = c;

  XSetWindowBorder(wm->dpy, window, 0x000000);

  setFocus(wm, c);

  XSelectInput(wm->dpy, window, StructureNotifyMask | FocusChangeMask);

  if (!currentWorkspace->master)
    currentWorkspace->master = c;

  arrangeWindows(wm);
}

void addClientToAWorkspace(WindowManager *wm, Window window,
                           size_t workspacesIdx) {
#ifdef debug
  debug_print("Adding client: window=0x%lx to workspace: %lu\n", window,
              workspacesIdx);
#endif
  if (workspacesIdx >= wm->config.nWorkspaces)
    return;
  Workspace *targetWorkspace = &wm->workspaces[workspacesIdx];

  Client *c = calloc(1, sizeof(Client));

  if (!c)
    return;

  c->window = window;
  if (targetWorkspace->clients) {
    targetWorkspace->clients->prev = c;
    c->next = targetWorkspace->clients;
  }
  targetWorkspace->clients = c;

  XSetWindowBorder(wm->dpy, window, 0x000000);

  setFocusToAWorkspace(wm, c, workspacesIdx);

  XSelectInput(wm->dpy, window, StructureNotifyMask | FocusChangeMask);

  if (!targetWorkspace->master)
    targetWorkspace->master = c;

  arrangeWindows(wm);
}

void removeClient(WindowManager *wm, Client *c) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

#ifdef debug
  debug_print("Removing client: window=0x%lx\n", c->window);
#endif

  if (!c)
    return;

  if (currentWorkspace->focused == c) {
    Client *newFocus = (c->next) ? c->next : c->prev;

    if (newFocus) {
      XWindowAttributes attr;
      if (!XGetWindowAttributes(wm->dpy, newFocus->window, &attr)) {
        newFocus = NULL;
      }
    }

    if (newFocus && newFocus->window) {
      XWindowAttributes attr;
      if (XGetWindowAttributes(wm->dpy, newFocus->window, &attr)) {
        setFocus(wm, newFocus);
      }
    } else {
      currentWorkspace->focused = NULL;
      XSetInputFocus(wm->dpy, wm->root, RevertToPointerRoot, CurrentTime);
    }
  }

  if (currentWorkspace->master == c) {
    currentWorkspace->master = currentWorkspace->clients;
    if (currentWorkspace->master == c)
      currentWorkspace->master = c->next;
  }

  if (currentWorkspace->fullscreenClient == c) {
    currentWorkspace->fullscreenClient = NULL;
    c->fullscreen = false;
  }

  if (currentWorkspace->focused == c) {
    currentWorkspace->focused = NULL;
    XSetInputFocus(wm->dpy, wm->root, RevertToPointerRoot, CurrentTime);
  }

  if (c->prev)
    c->prev->next = c->next;
  if (c->next)
    c->next->prev = c->prev;
  if (currentWorkspace->clients == c)
    currentWorkspace->clients = c->next;

  free(c);

  arrangeWindows(wm);
}

void removeClientToAWorkspace(WindowManager *wm, Client *c,
                              size_t workspacesIdx) {
#ifdef debug
  debug_print("Removing client: window=0x%lx\n", c->window);
#endif

  if (workspacesIdx >= wm->config.nWorkspaces || !c)
    return;

  Workspace *targetWorkspace = &wm->workspaces[workspacesIdx];

  if (targetWorkspace->focused == c) {
    Client *newFocus = (c->next) ? c->next : c->prev;

    if (newFocus) {
      XWindowAttributes attr;
      if (!XGetWindowAttributes(wm->dpy, newFocus->window, &attr)) {
        newFocus = NULL;
      }
    }

    if (newFocus && newFocus->window) {
      XWindowAttributes attr;
      if (XGetWindowAttributes(wm->dpy, newFocus->window, &attr)) {
        setFocusToAWorkspace(wm, newFocus, workspacesIdx);
      }
    } else {
      targetWorkspace->focused = NULL;
      XSetInputFocus(wm->dpy, wm->root, RevertToPointerRoot, CurrentTime);
    }
  }

  if (targetWorkspace->master == c) {
    targetWorkspace->master = targetWorkspace->clients;
    if (targetWorkspace->master == c)
      targetWorkspace->master = c->next;
  }

  if (targetWorkspace->fullscreenClient == c) {
    targetWorkspace->fullscreenClient = NULL;
    c->fullscreen = NULL;
  }

  if (targetWorkspace->focused == c) {
    targetWorkspace->focused = NULL;
    XSetInputFocus(wm->dpy, wm->root, RevertToPointerRoot, CurrentTime);
  }

  if (c->prev)
    c->prev->next = c->next;
  if (c->next)
    c->next->prev = c->prev;
  if (targetWorkspace->clients == c)
    targetWorkspace->clients = c->next;

  XUnmapWindow(wm->dpy, c->window);
  free(c);

  arrangeWindows(wm);
}

void setFocus(WindowManager *wm, Client *c) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (!c || currentWorkspace->focused == c)
    return;

  XWindowAttributes attr;
  if (!XGetWindowAttributes(wm->dpy, c->window, &attr)) {
    debug_print("ERROR: Window 0x%lx is invalid\n", c->window);
    return;
  }

  Client *old = currentWorkspace->focused;
  debug_print("Setting focus: window=0x%lx (old=0x%lx)\n", c->window,
              old ? old->window : 0);

  if (old) {
    safeSetWindowBorder(wm->dpy, old->window, wm->config.unfocusedBorderColor,
                        wm->config.borderSize);
  }
  currentWorkspace->focused = c;

  XWindowAttributes focusedAttr;
  XGetWindowAttributes(wm->dpy, currentWorkspace->focused->window,
                       &focusedAttr);

  if (focusedAttr.map_state == IsUnmapped) {
    XMapWindow(wm->dpy, currentWorkspace->focused->window);
  }

  Atom netActiveWindow = XInternAtom(wm->dpy, "_NET_ACTIVE_WINDOW", False);
  unsigned long data[] = {c->window};
  XChangeProperty(wm->dpy, wm->root, netActiveWindow, XA_WINDOW, 32,
                  PropModeReplace, (unsigned char *)data, 1);

  XSetWindowBorder(wm->dpy, c->window, wm->config.focusedBorderColor);
  XSetWindowBorderWidth(wm->dpy, c->window, wm->config.borderSize);

  XSetInputFocus(wm->dpy, c->window, RevertToPointerRoot, CurrentTime);

  XRaiseWindow(wm->dpy, c->window);
  XFlush(wm->dpy);
}

void setFocusToAWorkspace(WindowManager *wm, Client *c, size_t workspacesIdx) {
  if (workspacesIdx >= wm->config.nWorkspaces)
    return;

  if (!c || wm->workspaces[workspacesIdx].focused == c)
    return;

  Workspace *targetWorkspace = &wm->workspaces[workspacesIdx];

  XWindowAttributes attr;
  if (!XGetWindowAttributes(wm->dpy, c->window, &attr)) {
    debug_print("ERROR: Window 0x%lx is invalid\n", c->window);
    return;
  }

  Client *old = targetWorkspace->focused;
  debug_print("Setting focus in workspace %zu: window=0x%lx (old=0x%lx)\n",
              workspacesIdx, c->window, old ? old->window : 0);

  if (old) {
    safeSetWindowBorder(wm->dpy, old->window, wm->config.unfocusedBorderColor,
                        wm->config.borderSize);
  }

  targetWorkspace->focused = c;

  XWindowAttributes focusedAttr;
  XGetWindowAttributes(wm->dpy, targetWorkspace->focused->window, &focusedAttr);

  XSetWindowBorder(wm->dpy, c->window, wm->config.focusedBorderColor);
  XSetWindowBorderWidth(wm->dpy, c->window, wm->config.borderSize);

  if (workspacesIdx == wm->currentWorkspace) {
    XSetInputFocus(wm->dpy, c->window, RevertToPointerRoot, CurrentTime);
    XRaiseWindow(wm->dpy, c->window);
    XFlush(wm->dpy);
  }
}

void updateClients(WindowManager *wm) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  for (size_t workspaceIdx = 0; workspaceIdx < wm->config.nWorkspaces;
       workspaceIdx++) {
    Client *c = wm->workspaces[workspaceIdx].clients;
    while (c) {
      if (workspaceIdx != wm->currentWorkspace ||
          wm->currentLayout == MONOCLE) {
        XUnmapWindow(wm->dpy, c->window);
      }
      c = c->next;
    }
  }

  Client *c = currentWorkspace->clients;
  while (c) {
    XMapWindow(wm->dpy, c->window);

    c = c->next;
  }

  setFocus(wm, currentWorkspace->focused);
  arrangeWindows(wm);
}

void freeClients(WindowManager *wm) {
  for (size_t i = 0; i < wm->config.nWorkspaces; i++) {
    while (wm->workspaces[i].clients) {
      Client *next = wm->workspaces[i].clients->next;
      free(wm->workspaces[i].clients);
      wm->workspaces[i].clients = next;
    }
  }
}

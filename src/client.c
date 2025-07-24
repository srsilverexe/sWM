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

    c = c->workspaceNext;
  }
  return NULL;
}

Client *findClientInGlobalList(WindowManager *wm, Window window) {
  #ifdef debug
  debug_print("Searching for window in global list: 0x%lx\n", window);
#endif

  Client *c = wm->allClients;

  while (c) {
      if (c->window == window) {
        return c;
      }

      c = c->globalNext;
  }

  return NULL;
}

void createClient(WindowManager *wm, Window window, size_t targetWorkspaceIdx) {
  if (targetWorkspaceIdx >= wm->config.nWorkspaces) return;
  
  // Skip if window already managed
  if (findClientInGlobalList(wm, window)) {
    debug_print("Window 0x%lx already managed\n", window);
    return;
  }

  Client *c = calloc(1, sizeof(Client));
  if (!c) return;

  // Initialize client
  c->window = window;
  c->workspaceIdx = targetWorkspaceIdx;
  c->fullscreen = false;

  // Add to workspace list (head insertion)
  Workspace *target = &wm->workspaces[targetWorkspaceIdx];
  if (target->clients) {
    target->clients->workspacePrev = c;
    c->workspaceNext = target->clients;
  }
  target->clients = c;

  // Add to global list (head insertion)
  if (wm->allClients) {
    wm->allClients->globalPrev = c;
    c->globalNext = wm->allClients;
  }
  wm->allClients = c;

  // Configure window
  XSelectInput(wm->dpy, window, StructureNotifyMask | FocusChangeMask);
  XSetWindowBorder(wm->dpy, window, wm->config.unfocusedBorderColor);
  XSetWindowBorderWidth(wm->dpy, window, wm->config.borderSize);

  // Handle current workspace
  if (targetWorkspaceIdx == wm->currentWorkspace) {
    XMapWindow(wm->dpy, window);
    setFocus(wm, c);
    arrangeWindows(wm);
  } 
  // Handle background workspace
  else {
    // Only set focus if no other client exists
    if (!target->focused) {
      setFocusToAWorkspace(wm, c, targetWorkspaceIdx);
    }
    // Window remains unmapped
  }
}

void destroyClient(WindowManager *wm, Client *c) {
  if (!c) return;

  Workspace *workspace = &wm->workspaces[c->workspaceIdx];

  // Remove from workspace list
  if (c->workspacePrev) {
    c->workspacePrev->workspaceNext = c->workspaceNext;
  } else {
    workspace->clients = c->workspaceNext;
  }
  
  if (c->workspaceNext) {
    c->workspaceNext->workspacePrev = c->workspacePrev;
  }

  // Update workspace state
  if (workspace->focused == c) {
    Client *newFocus = c->workspaceNext ? c->workspaceNext : c->workspacePrev;
    
    if (newFocus) {
      if (c->workspaceIdx == wm->currentWorkspace) {
        setFocus(wm, newFocus);
      } else {
        setFocusToAWorkspace(wm, newFocus, c->workspaceIdx);
      }
    } else {
      workspace->focused = NULL;
      if (c->workspaceIdx == wm->currentWorkspace) {
        XSetInputFocus(wm->dpy, wm->root, RevertToPointerRoot, CurrentTime);
      }
    }
  }

  if (workspace->master == c) {
    workspace->master = workspace->clients;
  }

  if (workspace->fullscreenClient == c) {
    workspace->fullscreenClient = NULL;
  }

  // Remove from global list
  if (c->globalPrev) {
    c->globalPrev->globalNext = c->globalNext;
  } else {
    wm->allClients = c->globalNext;
  }
  
  if (c->globalNext) {
    c->globalNext->globalPrev = c->globalPrev;
  }

  // Update layout if needed
  if (c->workspaceIdx == wm->currentWorkspace) {
    arrangeWindows(wm);
  }

  free(c);
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
      c = c->workspaceNext;
    }
  }

  Client *c = currentWorkspace->clients;
  while (c) {
    XMapWindow(wm->dpy, c->window);

    c = c->workspaceNext;
  }

  setFocus(wm, currentWorkspace->focused);
  arrangeWindows(wm);
}

void freeClients(WindowManager *wm) {
  for (size_t i = 0; i < wm->config.nWorkspaces; i++) {
    while (wm->workspaces[i].clients) {
      Client *next = wm->workspaces[i].clients->workspaceNext;
      free(wm->workspaces[i].clients);
      wm->workspaces[i].clients = next;
    }
  }
}

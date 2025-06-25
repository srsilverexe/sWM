#include "../include/client.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdlib.h>

Client *findClient(WindowManager *wm, Window window) {
  for (Client *c = wm->clients; c; c = c->next) {
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
  if (wm->clients) {
    wm->clients->prev = c;
    c->next = wm->clients;
  }
  wm->clients = c;

  XSetWindowBorder(wm->dpy, window, 0x000000);

  setFocus(wm, c);

  XSelectInput(wm->dpy, window, StructureNotifyMask | FocusChangeMask);

  if (!wm->master) {
    wm->master = c;
  }

  arrangeWindows(wm);
}

void removeClient(WindowManager *wm, Client *c) {
  if (!c)
    return;

  if (wm->focused == c) {
    Client *newFocus = (c->next) ? c->next : c->prev;
    if (wm->focused) {
      setFocus(wm, newFocus);
    } else {
      wm->focused = NULL;
      XSetInputFocus(wm->dpy, wm->root, RevertToPointerRoot, CurrentTime);
    }
  }

  if (c == wm->master) {
    wm->master = wm->clients;
    if (wm->master == c)
      wm->master = c->next;
  }

  if (c->prev)
    c->prev->next = c->next;
  if (c->next)
    c->next->prev = c->prev;
  if (wm->clients == c)
    wm->clients = c->next;

  free(c);

  arrangeWindows(wm);
}

void setFocus(WindowManager *wm, Client *c) {
  if (!c || c == wm->focused)
    return;

  if (wm->focused) {
    XSetWindowBorder(wm->dpy, wm->focused->window,
                     wm->config.unfocusedBorderColor);
    XSetWindowBorderWidth(wm->dpy, wm->focused->window, wm->config.borderSize);
  }

  wm->focused = c;
  XSetWindowBorder(wm->dpy, c->window, wm->config.focusedBorderColor);
  XSetWindowBorderWidth(wm->dpy, c->window, wm->config.borderSize);

  XSetInputFocus(wm->dpy, c->window, RevertToPointerRoot, CurrentTime);

  XRaiseWindow(wm->dpy, c->window);
  XFlush(wm->dpy);
}

void freeClients(WindowManager *wm) {
  while (wm->clients) {
    Client *next = wm->clients->next;
    free(wm->clients);
    wm->clients = next;
  }
}

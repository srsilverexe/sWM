#include "../include/atoms.h"
#include <stdint.h>

void initAtoms(WindowManager *wm) {
  wm->netWmWindowType = XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE", False);
  wm->netWmWindowTypeDesktop =
      XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
  wm->netWmWindowTypeDock =
      XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
  wm->netNumberOfDesktops =
      XInternAtom(wm->dpy, "_NET_NUMBER_OF_DESKTOPS", False);

  setNumberOfDesktopsAtom(wm, WORKSPACE_COUNT);
}

void setNumberOfDesktopsAtom(WindowManager *wm, size_t nDesktops) {
  Window rootWindow = wm->root;
  unsigned long data = nDesktops;

  XChangeProperty(wm->dpy, rootWindow, wm->netNumberOfDesktops, XA_CARDINAL, 32,
                  PropModeReplace, (unsigned char *)&data, 1);
}

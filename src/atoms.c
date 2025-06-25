#include "../include/atoms.h"

void initAtoms(WindowManager *wm) {
	wm->netWmWindowType = XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE", False);
	wm->netWmWindowTypeDesktop =
		XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
	wm->netWmWindowTypeDock =
		XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
}


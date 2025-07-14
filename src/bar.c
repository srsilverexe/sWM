#include "../include/bar.h"
#include <X11/Xatom.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef debug
#define debug_print(...) fprintf(stderr, "[BAR] " __VA_ARGS__)
#else
#define debug_print(...)
#endif

void initBar(WindowManager *wm) {
  XWindowAttributes rootAttr;
  XGetWindowAttributes(wm->dpy, wm->root, &rootAttr);

  wm->config.barHeight = 24;

  wm->bar.window =
      XCreateSimpleWindow(wm->dpy, wm->root, 0, 0, rootAttr.width,
                          wm->config.barHeight, 0, 0x000000, 0x333333);

  Atom wmWindowType = XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE", False);
  Atom wmWindowTypeDock =
      XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);

  XChangeProperty(wm->dpy, wm->bar.window, wmWindowType, XA_ATOM, 32,
                  PropModeReplace, (unsigned char *)&wmWindowTypeDock, 1);

  long struts[12] = {0};
  struts[2] = wm->config.barHeight;

  Atom netWmStrutPartial = XInternAtom(wm->dpy, "_NET_WM_STRUT_PARTIAL", False);
  XChangeProperty(wm->dpy, wm->bar.window, netWmStrutPartial, XA_CARDINAL, 32,
                  PropModeReplace, (unsigned char *)struts, 4);

  Atom netWmStrut = XInternAtom(wm->dpy, "_NET_WM_STRUT", False);
  XChangeProperty(wm->dpy, wm->bar.window, netWmStrut, XA_CARDINAL, 32,
                  PropModeReplace, (unsigned char *)struts, 4);

  XSelectInput(wm->dpy, wm->bar.window, ExposureMask | ButtonPressMask);

  wm->bar.gc = XCreateGC(wm->dpy, wm->bar.window, 0, NULL);
  XSetForeground(wm->dpy, wm->bar.gc, 0xFFFFFF);

  XMapWindow(wm->dpy, wm->bar.window);

  drawBar(wm);
  XFlush(wm->dpy);
}

void drawBar(WindowManager *wm) {
#ifdef debug
  debug_print("Drawing bar (workspace %zu)\n", wm->currentWorkspace + 1);
#endif

  XWindowAttributes bar_attr;
  XGetWindowAttributes(wm->dpy, wm->bar.window, &bar_attr);

  // Clear bar
  XClearWindow(wm->dpy, wm->bar.window);

  // Get current time
  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  char time_str[64];
  strftime(time_str, sizeof(time_str), "%H:%M:%S", tm);

  // Draw time
  XDrawString(wm->dpy, wm->bar.window, wm->bar.gc, bar_attr.width / 2 + 5,
              wm->config.barHeight / 2 + 5, time_str, strlen(time_str));

  // Draw current workspace
  char currentWorkspaceStr[12];
  snprintf(currentWorkspaceStr, sizeof(currentWorkspaceStr), "%zu",
           wm->currentWorkspace + 1);
  int x = 10;

  XDrawString(wm->dpy, wm->bar.window, wm->bar.gc, x,
              wm->config.barHeight / 2 + 5, currentWorkspaceStr,
              strlen(currentWorkspaceStr));
}

void updateBar(WindowManager *wm) {
  XSetForeground(wm->dpy, wm->bar.gc, 0xFFFFFF);
  drawBar(wm);
}

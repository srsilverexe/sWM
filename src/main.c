#include "../include/bar.h"
#include "../include/config.h"
#include "../include/events.h"
#include "../include/keybindings.h"
#include "../include/windowManager.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>

int main(int argc, char *argv[]) {
  WindowManager wm = {0};

  // Initialize window manager
  if (!initWindowManager(&wm)) {
    fprintf(stderr, "Failed to initialize window manager\n");
    exit(EXIT_FAILURE);
  }

  if (parseConfigFile(&wm, argc > 1 ? argv[1] : NULL)) {
    fprintf(stderr, "Using default configuration\n");
  }

  // Setup keybindings
  setupKeybindings(&wm);

  time_t lastBarUpdate = 0;
  // Event loop
  while (true) {
    // Check for events with timeout
    fd_set in_fds;
    struct timeval tv;
    FD_ZERO(&in_fds);
    FD_SET(ConnectionNumber(wm.dpy), &in_fds);
    tv.tv_usec = 100000; // 100ms timeout

    if (XPending(wm.dpy) ||
        select(ConnectionNumber(wm.dpy) + 1, &in_fds, NULL, NULL, &tv)) {
      XEvent ev;
      XNextEvent(wm.dpy, &ev);
      if (!handleEvent(&wm, ev))
        break;
    }

    // Update bar every second
    time_t now = time(NULL);
    if (now != lastBarUpdate) {
      updateBar(&wm);
      lastBarUpdate = now;
    }
  }

  // Cleanup
  freeConfig(&wm);
  cleanupWindowManager(&wm);
  exit(EXIT_SUCCESS);
}

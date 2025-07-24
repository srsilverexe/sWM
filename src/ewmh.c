#include "../include/ewmh.h"
#include "../include/windowManager.h"
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helpers
void setNetSupported(WindowManager *wm) {
  EWMH *ewmh = &wm->ewmh;

  Atom Supported[] = {
      // WM identification
      ewmh->netSupported, ewmh->netSupportedWmCheck, ewmh->netWmName,

      // Desktop manager
      ewmh->netNumberOfDesktops, ewmh->netCurrentDesktop, ewmh->netDesktopNames,

      // Core Window Types
      ewmh->netWmWindowType, ewmh->netWmWindowTypeNormal,
      ewmh->netWmWindowTypeDialog, ewmh->netWmWindowTypeDock,
      ewmh->netWmWindowTypeUtility,

      // Essential States
      ewmh->netWmStateMaximizedVert, ewmh->netWmStateMaximizedHorz,
      ewmh->netWmStateFullscreen, ewmh->netWmStateHidden,

      // Event handling
      ewmh->netCloseWindow, ewmh->netActiveWindow, ewmh->netWmState,

      // Dynamic tracking
      ewmh->netClientList, ewmh->netClientListStacking};

  XChangeProperty(wm->dpy, wm->root, ewmh->netSupported, XA_ATOM, 32,
                  PropModeReplace, (unsigned char *)Supported,
                  sizeof(Supported) / sizeof(Atom));
}

void setNetSupportedWmCheck(WindowManager *wm) {
  EWMH *ewmh = &wm->ewmh;

  Window data = wm->supportedWmCheckWindow;

  // Set propriety in root window
  XChangeProperty(wm->dpy, wm->root, ewmh->netSupportedWmCheck, XA_WINDOW, 32,
                  PropModeReplace, (unsigned char *)&data, 1);

  // Set propriety in child window
  XChangeProperty(wm->dpy, wm->supportedWmCheckWindow,
                  ewmh->netSupportedWmCheck, XA_WINDOW, 32, PropModeReplace,
                  (unsigned char *)&data, 1);
}

void setNetWmName(WindowManager *wm) {
  EWMH *ewmh = &wm->ewmh;

  const char *data = "sWM";

  XChangeProperty(wm->dpy, wm->supportedWmCheckWindow, ewmh->netWmName,
                  XInternAtom(wm->dpy, "UTF8_STRING", False), 8,
                  PropModeReplace, (unsigned char *)data, strlen(data));
}

void setNetNumberOfDesktops(WindowManager *wm) {
  EWMH *ewmh = &wm->ewmh;

  long data = wm->config.nWorkspaces;

  XChangeProperty(wm->dpy, wm->root, ewmh->netNumberOfDesktops, XA_CARDINAL, 32,
                  PropModeReplace, (unsigned char *)&data, 1);
}

void setNetCurrentDesktop(WindowManager *wm) {
  EWMH *ewmh = &wm->ewmh;

  long data = 0;

  XChangeProperty(wm->dpy, wm->root, ewmh->netCurrentDesktop, XA_CARDINAL, 32,
                  PropModeReplace, (unsigned char *)&data, 1);
}

void setNetDesktopNames(WindowManager *wm) {
   EWMH *ewmh = &wm->ewmh;
    Atom utf8_string = XInternAtom(wm->dpy, "UTF8_STRING", False);
    
    size_t total_bytes = 0;
    for (size_t i = 0; i < wm->config.nWorkspaces; i++) {
        total_bytes += snprintf(NULL, 0, "%zu", i) + 1;
    }
    
    char *buffer = malloc(total_bytes);
    if (!buffer) return;
    
    char *ptr = buffer;
    
    for (size_t i = 0; i < wm->config.nWorkspaces; i++) {
        int len = sprintf(ptr, "%zu", i);
        ptr += len;
        *ptr = '\0';
        ptr++;
    }
    
    XChangeProperty(wm->dpy, wm->root, ewmh->netDesktopNames,
                    utf8_string, 8, PropModeReplace,
                    (unsigned char *)buffer, total_bytes);
    
    free(buffer);
}

void initEWMH(WindowManager *wm) {
  EWMH *ewmh = &wm->ewmh;

  // WM identification
  ewmh->netSupported = XInternAtom(wm->dpy, "_NET_SUPPORTED", False);
  ewmh->netSupportedWmCheck =
      XInternAtom(wm->dpy, "_NET_SUPPORTING_WM_CHECK", False);
  ewmh->netWmName = XInternAtom(wm->dpy, "_NET_WM_NAME", False);

  // Desktop manager
  ewmh->netNumberOfDesktops =
      XInternAtom(wm->dpy, "_NET_NUMBER_OF_DESKTOPS", False);
  ewmh->netCurrentDesktop = XInternAtom(wm->dpy, "_NET_CURRENT_DESKTOP", False);
  ewmh->netDesktopNames = XInternAtom(wm->dpy, "_NET_DESKTOP_NAMES", False);

  // Core Window Types
  ewmh->netWmWindowType = XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE", False);
  ewmh->netWmWindowTypeNormal =
      XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE_NORMAL", False);
  ewmh->netWmWindowTypeDialog =
      XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
  ewmh->netWmWindowTypeDock =
      XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
  ewmh->netWmWindowTypeUtility =
      XInternAtom(wm->dpy, "_NET_WM_WINDOW_TYPE_UTILITY", False);

  // Essential States
  ewmh->netWmStateMaximizedVert =
      XInternAtom(wm->dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
  ewmh->netWmStateMaximizedHorz =
      XInternAtom(wm->dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
  ewmh->netWmStateFullscreen =
      XInternAtom(wm->dpy, "_NET_WM_STATE_FULLSCREEN", False);
  ewmh->netWmStateHidden = XInternAtom(wm->dpy, "_NET_WM_STATE_HIDDEN", False);

  // Event handling
  ewmh->netCloseWindow = XInternAtom(wm->dpy, "_NET_CLOSE_WINDOW", False);
  ewmh->netActiveWindow = XInternAtom(wm->dpy, "_NET_ACTIVE_WINDOW", False);
  ewmh->netWmState = XInternAtom(wm->dpy, "_NET_WM_STATE", False);

  // Dynamic tracking
  ewmh->netClientList = XInternAtom(wm->dpy, "_NET_CLIENT_LIST", False);
  ewmh->netClientListStacking =
      XInternAtom(wm->dpy, "_NET_CLIENT_LIST_STACKING", False);

  setNetSupportedWmCheck(wm);
  setNetNumberOfDesktops(wm);
  setNetDesktopNames(wm);
  setNetCurrentDesktop(wm);

  setNetSupported(wm);

  updateNetClientList(wm);
  
  Window none = 0;
  updateNetActiveWindow(wm, none);
}

void updateNetActiveWindow(WindowManager *wm, Window window) {
    EWMH *ewmh = &wm->ewmh;
    XChangeProperty(wm->dpy, wm->root, ewmh->netActiveWindow, XA_WINDOW, 32, PropModeReplace, (unsigned char *)&window, 1);
}

void updateNetClientList(WindowManager *wm) {
    EWMH *ewmh = &wm->ewmh;
    
    size_t nClients = 0;
    for (size_t i = 0; i < wm->config.nWorkspaces; i++) {
        Client *c = wm->workspaces[i].clients;
        while (c) {
            nClients++;
            c = c->workspaceNext;
        }
    }

    Window *data = calloc(nClients, sizeof(Window));
    if (!data) return;
    
    size_t index = 0;
    for (size_t i = 0; i < wm->config.nWorkspaces; i++) {
        Client *c = wm->workspaces[i].clients;
        while (c) {
            data[index++] = c->window;
            c = c->workspaceNext;
        }
    }

    XChangeProperty(wm->dpy, wm->root, ewmh->netClientList, 
                    XA_WINDOW, 32, PropModeReplace,
                    (unsigned char *)data, nClients);
    
    free(data);
}

void updateNetCurrentDesktop(WindowManager *wm) {
    EWMH *ewmh = &wm->ewmh;

    unsigned int data = wm->currentWorkspace;

    XChangeProperty(wm->dpy, wm->root, ewmh->netCurrentDesktop, 
                    XA_CARDINAL, 32, PropModeReplace,
                    (unsigned char *)&data, 1);
}
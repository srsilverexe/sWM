#ifndef EWMH_H
#define EWMH_H

#include <X11/X.h>
#include <X11/Xatom.h>

typedef struct WindowManager WindowManager;
typedef struct EWMH EWMH;

struct EWMH {
    // WM identification
    Atom netSupported;
    Atom netSupportedWmCheck;
    Atom netWmName;

    // Desktop manager
    Atom netNumberOfDesktops;
    Atom netCurrentDesktop;
    Atom netDesktopNames;

    // Core Window Types
    Atom netWmWindowType;
    Atom netWmWindowTypeNormal;
    Atom netWmWindowTypeDialog;
    Atom netWmWindowTypeDock;
    Atom netWmWindowTypeUtility;

    // Essential states
    Atom netWmStateMaximizedVert;
    Atom netWmStateMaximizedHorz;
    Atom netWmStateFullscreen;
    Atom netWmStateHidden;

    // Event Handling
    Atom netCloseWindow;
    Atom netActiveWindow;
    Atom netWmState;

    // Dynamic tracking
    Atom netClientList;
    Atom netClientListStacking;
};

void initEWMH(WindowManager *wm);

void updateNetActiveWindow(WindowManager *wm, Window window);
void updateNetClientList(WindowManager *wm);
void updateNetCurrentDesktop(WindowManager *wm);

#endif /* EWMH_H */
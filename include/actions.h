#ifndef ACTIONS_H
#define ACTIONS_H

#include "events.h"
#include "windowManager.h"

void moveFocusedWindow(WindowManager *wm, Directions direction);
void resizeFocusedWindow(WindowManager *wm, ResizeTypes type);
void focusToDirection(WindowManager *wm, Directions direction);
void killFocusedWindow(WindowManager *wm);
void changeWorkspace(WindowManager *wm, size_t targetWorkspace);
void moveFocusedWindowToWorkspace(WindowManager *wm, size_t targetWorkspace);

void handleMapRequest(WindowManager *wm, XMapRequestEvent ev);
void handleDestroyNotify(WindowManager *wm, XDestroyWindowEvent ev);
void handleUnmapNotify(WindowManager *wm, XUnmapEvent ev);
void handleFocusChange(WindowManager *wm, XFocusChangeEvent ev);

#endif /* ACTIONS_H */

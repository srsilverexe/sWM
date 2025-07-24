#ifndef ACTIONS_H
#define ACTIONS_H

#include "events.h"
#include "windowManager.h"

void focusToDirection(WindowManager *wm, Directions direction);
void killWindow(WindowManager *wm, Client *c);
void killFocusedWindow(WindowManager *wm);
void changeWorkspace(WindowManager *wm, size_t targetWorkspace);
void moveFocusedWindowToWorkspace(WindowManager *wm, size_t targetWorkspace);
void updateFullscreenState(WindowManager *wm, Client *c, bool fullscreen);

void handleClientMessage(WindowManager *wm, XClientMessageEvent ev);
void handleMapRequest(WindowManager *wm, XMapRequestEvent ev);
void handleDestroyNotify(WindowManager *wm, XDestroyWindowEvent ev);
void handleUnmapNotify(WindowManager *wm, XUnmapEvent ev);
void handleFocusChange(WindowManager *wm, XFocusChangeEvent ev);

#endif /* ACTIONS_H */

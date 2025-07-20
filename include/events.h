#ifndef EVENTS_H
#define EVENTS_H

#include "windowManager.h"

typedef enum { UP, DOWN, LEFT, RIGHT, KEY_PRESS, KEY_RELEASE } Directions;

typedef enum {
  EXPAND_HORIZONTAL,
  SHRINK_HORIZONTAL,
  EXPAND_VERTICAL,
  SHRINK_VERTICAL
} ResizeTypes;

bool handleEvent(WindowManager *wm, XEvent ev);

#endif /* EVENTS_H */

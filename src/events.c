#include "../include/events.h"
#include "../include/actions.h"
#include "../include/keybindings.h"
#include <X11/X.h>

bool handleEvent(WindowManager *wm, XEvent ev) {
  switch (ev.type) {
  case MapRequest:
    handleMapRequest(wm, ev.xmaprequest);
    break;
  case DestroyNotify:
    handleDestroyNotify(wm, ev.xdestroywindow);
    break;
  case UnmapNotify:
    handleUnmapNotify(wm, ev.xunmap);
    break;
  case FocusIn:
    handleFocusChange(wm, ev.xfocus);
    break;
  case KeyPress:
    return handleKeyPress(wm, ev.xkey);
  case KeyRelease:
    return handleKeyRelease(wm, ev.xkey);
  default:
    break;
  }
  return true;
}

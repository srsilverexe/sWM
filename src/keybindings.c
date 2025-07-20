#define _POSIX_C_SOURCE 200809L
#include "../include/keybindings.h"
#include "../include/actions.h"
#include <X11/keysym.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void setupKeybindings(WindowManager *wm) {
  for (Keybinding *k = wm->config.keybindings; k; k = k->next) {
    XGrabKey(wm->dpy, k->keycode, k->mods, wm->root, True, GrabModeAsync,
             GrabModeAsync);
  }
}

bool handleKeyPress(WindowManager *wm, XKeyEvent ev) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  Keybinding *kb = findKeybinding(wm, ev.state, ev.keycode);
  if (kb) {
    switch (kb->action) {
    case QUIT: {
      return false;
      break;
    }
    case KILL_FOCUSED_WINDOW: {
      killFocusedWindow(wm);
      break;
    }
    case ADD_MASTER_RATIO: {
      wm->masterRatio += 0.05;
      if (wm->masterRatio > 0.8)
        wm->masterRatio = 0.8;
      arrangeWindows(wm);
      break;
    }
    case REMOVE_MASTER_RATIO: {
      wm->masterRatio -= 0.05;
      if (wm->masterRatio < 0.2)
        wm->masterRatio = 0.2;
      arrangeWindows(wm);
      break;
    }
    case CHANGE_MASTER: {
      if (currentWorkspace->focused &&
          currentWorkspace->focused != currentWorkspace->master) {
        currentWorkspace->master = currentWorkspace->focused;
        arrangeWindows(wm);
      }
      break;
    }
    case EXEC: {
      pid_t pid = fork();
      if (pid == 0) {
        // Double fork to detach from WM
        if (fork() == 0) {
          execl("/bin/sh", "sh", "-c", (char *)kb->complement, (char *)NULL);
          perror("exec failed");
          exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
      }
      waitpid(pid, NULL, 0); // Clean up intermediate process
      break;
    }
    case CHANGE_LAYOUT_TO_MASTER: {
      wm->currentLayout = MASTER;
      arrangeWindows(wm);
      break;
    }
    case CHANGE_LAYOUT_TO_MONOCLE: {
      wm->currentLayout = MONOCLE;
      arrangeWindows(wm);
      break;
    }
    case CHANGE_FOCUS_NEXT: {
      focusToDirection(wm, RIGHT);
      break;
    }
    case CHANGE_FOCUS_PREV: {
      focusToDirection(wm, LEFT);
      break;
    }
    case CHANGE_CURRENT_WORKSPACE: {
      changeWorkspace(wm, *((unsigned int *)kb->complement));
      break;
    }
    case MOVE_FOCUSED_WINDOW_TO_WORKSPACE: {
      moveFocusedWindowToWorkspace(wm, *((unsigned int *)kb->complement));
      break;
    }
    }
  } else {
    if (currentWorkspace->focused) {
      XKeyEvent keyEvent = ev;
      keyEvent.window = currentWorkspace->focused->window;
      XSendEvent(wm->dpy, keyEvent.window, True, KeyPressMask,
                 (XEvent *)&keyEvent);
      XFlush(wm->dpy);
    }
  }

  return true;
}

bool handleKeyRelease(WindowManager *wm, XKeyEvent ev) {
  Workspace *currentWorkspace = &wm->workspaces[wm->currentWorkspace];

  if (currentWorkspace->focused) {
    XKeyEvent keyEvent = ev;
    keyEvent.window = currentWorkspace->focused->window;
    XSendEvent(wm->dpy, keyEvent.window, True, KeyReleaseMask,
               (XEvent *)&keyEvent);
    XFlush(wm->dpy);
  }
  return true;
}

Keybinding *findKeybinding(WindowManager *wm, unsigned int mods,
                           KeyCode keycode) {
  for (Keybinding *k = wm->config.keybindings; k; k = k->next) {
    if (k->mods == mods && k->keycode == keycode) {
      return k;
    }
  }
  return NULL;
}

void addKeybinding(WindowManager *wm, unsigned int mods, KeyCode keycode,
                   Actions action, void *complement) {
  Keybinding *k = calloc(1, sizeof(Keybinding));

  if (!k)
    return;

  k->mods = mods;
  k->keycode = keycode;
  k->action = action;
  k->complement = complement;

  if (wm->config.keybindings) {
    wm->config.keybindings->prev = k;
    k->next = wm->config.keybindings;
  }
  wm->config.keybindings = k;
}

unsigned int parseModifiers(const char *modsString) {
  unsigned int mods = 0;
  char *token, *str = strdup(modsString);
  char *savePtr = NULL;

  for (token = strtok_r(str, "|", &savePtr); token;
       token = strtok_r(NULL, "|", &savePtr)) {
    if (strcmp(token, "mod4") == 0) {
      mods |= Mod4Mask;
    } else if (strcmp(token, "shift") == 0) {
      mods |= ShiftMask;
    } else if (strcmp(token, "ctrl") == 0 || strcmp(token, "control") == 0) {
      mods |= ControlMask;
    } else if (strcmp(token, "mod1") == 0 || strcmp(token, "alt") == 0) {
      mods |= Mod1Mask;
    } else if (strcmp(token, "mod3") == 0) {
      mods |= Mod3Mask;
    }
  }

  free(str);
  return mods;
}

KeyCode parseKey(WindowManager *wm, const char *keystr) {
  if (strlen(keystr) == 1 && isalpha(keystr[0])) {
    return XKeysymToKeycode(wm->dpy, XStringToKeysym(keystr));
  }

  const struct {
    const char *name;
    KeySym keysym;
  } keymap[] = {{"Enter", XK_Return},  {"Return", XK_Return},
                {"Space", XK_space},   {"Tab", XK_Tab},
                {"Escape", XK_Escape}, {"Left", XK_Left},
                {"Right", XK_Right},   {"Up", XK_Up},
                {"Down", XK_Down},     {"F1", XK_F1},
                {"F2", XK_F2},         {"F3", XK_F3},
                {"F4", XK_F4},         {"F5", XK_F5},
                {"F6", XK_F6},         {"F7", XK_F7},
                {"F8", XK_F8},         {"F9", XK_F9},
                {"F10", XK_F10},       {"F11", XK_F11},
                {"F12", XK_F12},       {NULL, 0}};

  for (int i = 0; keymap[i].name; i++) {
    if (strcmp(keystr, keymap[i].name) == 0) {
      return XKeysymToKeycode(wm->dpy, keymap[i].keysym);
    }
  }

  return XKeysymToKeycode(wm->dpy, XStringToKeysym(keystr));
}

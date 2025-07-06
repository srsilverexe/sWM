#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdbool.h>

// Constants
#define CLAMP(min, max, value) (value < min) ? min : (value > max) ? max : value
#define MIN_WINDOW_SIZE 50
#define DEFAULT_GAP 5
#define DEFAULT_MASTER_RATIO 0.6f

// Forward declarations
typedef struct Client Client;
typedef struct Keybinding Keybinding;
typedef struct Configs Configs;
typedef struct Bar Bar;
typedef struct Workspace Workspace;
typedef struct WindowManager WindowManager;

// Client structure
struct Client {
  Window window;
  Client *next;
  Client *prev;
};

typedef enum {
  QUIT,
  KILL_FOCUSED_WINDOW,
  CHANGE_MASTER,
  ADD_MASTER_RATIO,
  REMOVE_MASTER_RATIO,
  CHANGE_FOCUS_NEXT,
  CHANGE_FOCUS_PREV,
  CHANGE_CURRENT_WORKSPACE,
  MOVE_FOCUSED_WINDOW_TO_WORKSPACE,
  EXEC
} Actions;

struct Keybinding {
  unsigned int mods;
  KeyCode keycode;
  Actions action;
  void *complement;
  Keybinding *next;
  Keybinding *prev;
};

struct Configs {
  unsigned int gaps;

  unsigned int borderSize;
  unsigned long focusedBorderColor;
  unsigned long unfocusedBorderColor;

  unsigned int barHeight;

  Keybinding *keybindings;
};

struct Bar {
  Window window;
  GC gc;
};

struct Workspace {
  Client *clients;
  Client *focused;
  Client *master;
};

// Main WM structure
struct WindowManager {
  Display *dpy;
  Window root;
  Cursor cursor;

  Bar bar;

  Workspace workspaces[10];
  size_t currentWorkspace;

  // Layout
  float masterRatio;

  // Configuration
  Configs config;

  // Atoms
  Atom netWmWindowType;
  Atom netWmWindowTypeDesktop;
  Atom netWmWindowTypeDock;
  Atom netWmWindowStrutPartial;
  Atom netWmWindowStrut;
};

// Core functions
bool initWindowManager(WindowManager *wm);
void cleanupWindowManager(WindowManager *wm);
void arrangeWindows(WindowManager *wm);

#endif /* WINDOW_MANAGER_H */

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

  bool fullscreen;
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

  // To implement
  CHANGE_LAYOUT_TO_MASTER,
  CHANGE_LAYOUT_TO_MONOCLE,

  EXEC
} Actions;

typedef enum { MASTER, MONOCLE } Layouts;

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

  unsigned int nWorkspaces;

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

  Client *fullscreenClient;
};

// Main WM structure
struct WindowManager {
  Display *dpy;
  Window root;
  Cursor cursor;

  Bar bar;

  Workspace *workspaces;
  size_t currentWorkspace;

  // Layout
  Layouts currentLayout;
  float masterRatio;

  // Configuration
  Configs config;

  // Atoms
  Atom netWmWindowType;
  Atom netWmWindowTypeDesktop;
  Atom netWmWindowTypeDock;
  Atom netWmWindowStrutPartial;
  Atom netWmWindowStrut;
  Atom netNumberOfDesktops;
  Atom netWmState;
  Atom netWmStateFullscreen;
};

// Core functions
bool initWindowManager(WindowManager *wm);
bool applyConfigsInWindowManager(WindowManager *wm);

void cleanupWindowManager(WindowManager *wm);
void arrangeWindows(WindowManager *wm);

void masterLayout(WindowManager *wm);
void monocleLayout(WindowManager *wm);
void fullscreenLayout(WindowManager *wm);

#endif /* WINDOW_MANAGER_H */

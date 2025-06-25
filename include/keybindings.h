#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include "windowManager.h"
#include <X11/keysym.h>

void setupKeybindings(WindowManager *wm);
bool handleKeyPress(WindowManager *wm, XKeyEvent ev);

// Helper functions
Keybinding *findKeybinding(WindowManager *wm, unsigned int mods,
                           KeyCode keycode);
void addKeybinding(WindowManager *wm, unsigned int mods, KeyCode keycode,
                   Actions action, void *complement);

KeyCode parseKey(WindowManager *wm, const char *keystr);
unsigned int parseModifiers(const char *modstr);

#endif /* KEYBINDINGS_H */

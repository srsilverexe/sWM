#ifndef ATOMS_H
#define ATOMS_H

#include "windowManager.h"
#include <X11/Xatom.h>

void initAtoms(WindowManager *wm);

void setNumberOfDesktopsAtom(WindowManager *wm, size_t nDesktops);

#endif /* ATOMS_H */

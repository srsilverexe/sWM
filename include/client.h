#ifndef CLIENT_H
#define CLIENT_H

#include "windowManager.h"

Client *findClient(WindowManager *wm, Window window);
// Client *findClientFromAWorkspace(WindowManager *wm, Window window, size_t
// workspace);

void addClient(WindowManager *wm, Window window);
void addClientFromAWorkspace(WindowManager *wm, Window window,
                             size_t workspaceIdx);

void removeClient(WindowManager *wm, Client *c);
void removeClientFromAWorkspace(WindowManager *wm, Client *c,
                                size_t workspaceIdx);

void setFocus(WindowManager *wm, Client *c);
void setFocusFromAWorkspace(WindowManager *wm, Client *c, size_t workspaceIdx);

void updateClients(WindowManager *wm);

void freeClients(WindowManager *wm);

#endif /* CLIENT_H */

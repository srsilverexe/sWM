#ifndef CLIENT_H
#define CLIENT_H

#include "windowManager.h"

Client *findClient(WindowManager *wm, Window window);

void addClient(WindowManager *wm, Window window);
void addClientToAWorkspace(WindowManager *wm, Window window,
                           size_t workspaceIdx);

void removeClient(WindowManager *wm, Client *c);
void removeClientToAWorkspace(WindowManager *wm, Client *c,
                              size_t workspaceIdx);

void setFocus(WindowManager *wm, Client *c);
void setFocusToAWorkspace(WindowManager *wm, Client *c, size_t workspaceIdx);

void updateClients(WindowManager *wm);

void freeClients(WindowManager *wm);

#endif /* CLIENT_H */

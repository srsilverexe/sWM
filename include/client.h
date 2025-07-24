#ifndef CLIENT_H
#define CLIENT_H

#include "windowManager.h"

Client *findClient(WindowManager *wm, Window window);
Client *findClientInGlobalList(WindowManager *wm, Window window);

void createClient(WindowManager *wm, Window window, size_t targetWorkspaceIdx);
void destroyClient(WindowManager *wm, Client *c);

void setFocus(WindowManager *wm, Client *c);
void setFocusToAWorkspace(WindowManager *wm, Client *c, size_t workspaceIdx);

void updateClients(WindowManager *wm);

void freeClients(WindowManager *wm);

#endif /* CLIENT_H */

#ifndef CLIENT_H
#define CLIENT_H

#include "windowManager.h"

Client *findClient(WindowManager *wm, Window window);
void addClient(WindowManager *wm, Window window);
void removeClient(WindowManager *wm, Client *c);
void setFocus(WindowManager *wm, Client *c);
void freeClients(WindowManager *wm);

#endif /* CLIENT_H */

#ifndef CONFIG_H
#define CONFIG_H

#include "windowManager.h"

int parseConfigFile(WindowManager *wm, char *path);
void freeConfig(WindowManager *wm);

#endif /* CONFIG_H */

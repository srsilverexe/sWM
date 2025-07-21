#define _POSIX_C_SOURCE 200809L
#include "../include/bar.h"
#include "../include/config.h"
#include "../include/events.h"
#include "../include/keybindings.h"
#include "../include/windowManager.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifdef debug
static int errorHandler(Display *dpy, XErrorEvent *ee) {
  char buf[1024];
  XGetErrorText(dpy, ee->error_code, buf, sizeof(buf));
  fprintf(stderr, "X11 ERROR: %s\n", buf);
  fprintf(stderr, "  Request: %d.%d\n", ee->request_code, ee->minor_code);
  fprintf(stderr, "  ResourceID: 0x%lx\n", (long)ee->resourceid);
  return 0;
}
#define debug_print(...) fprintf(stderr, "[MAIN] " __VA_ARGS__)
#else
#define debug_print(...)
#endif

#define DEFAULT_SYS_CONFIG_PATH "/usr/share/sWM/config.cfg"
#define GITHUB_REPO_LINK "https://github.com/srsilverexe/sWM/"

int main(void) {
#ifdef debug
  XSetErrorHandler(errorHandler);
  setvbuf(stderr, NULL, _IONBF, 0); // Unbuffer stderr
  debug_print("Starting in debug mode\n");
#endif

  WindowManager wm = {0};

  if (!initWindowManager(&wm)) {
    fprintf(stderr, "Failed to initialize window manager\n");
    exit(EXIT_FAILURE);
  }

  char *homeDir = getenv("HOME");
  if (!homeDir) {
    fprintf(stderr, "HOME enviroment variable not set\n");
    cleanupWindowManager(&wm);
    exit(EXIT_FAILURE);
  }

  size_t path_len = strlen(homeDir) + strlen("/.config/sWM/config.cfg") + 1;
  char *fullConfigPath = malloc(path_len);
  if (!fullConfigPath) {
    perror("Failed to allocate config path");
    cleanupWindowManager(&wm);
    exit(EXIT_FAILURE);
  }
  snprintf(fullConfigPath, path_len, "%s/.config/sWM/config.cfg", homeDir);

  char *dirPath = strdup(fullConfigPath);
  char *lastSlash = strrchr(dirPath, '/');
  if (lastSlash) {
    *lastSlash = '\0';
    mkdir(dirPath, 0755);
  }
  free(dirPath);

  if (access(fullConfigPath, F_OK) == 0) {
    printf("Using config file in: %s\n", fullConfigPath);
  } else {
    printf("Creating new config file: %s\n", fullConfigPath);
    FILE *sysConfigFile = fopen(DEFAULT_SYS_CONFIG_PATH, "r");
    if (!sysConfigFile) {
      fprintf(stderr, "System config file missing: %s\nPlease check: %s\n",
              DEFAULT_SYS_CONFIG_PATH, GITHUB_REPO_LINK);

      free(fullConfigPath);
      cleanupWindowManager(&wm);
      exit(EXIT_FAILURE);
    }

    FILE *userConfigFile = fopen(fullConfigPath, "w");
    if (!userConfigFile) {
      fclose(sysConfigFile);
      fprintf(stderr, "Failed to create user config: %s\n", fullConfigPath);

      free(fullConfigPath);
      cleanupWindowManager(&wm);
      exit(EXIT_FAILURE);
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), sysConfigFile))) {
      fwrite(buffer, 1, bytes, userConfigFile);
    }

    fclose(sysConfigFile);
    fclose(userConfigFile);
  }

  parseConfigFile(&wm, fullConfigPath);

  if (!applyConfigsInWindowManager(&wm)) {
    fprintf(stderr, "Failed to initialize workspaces\n");
    cleanupWindowManager(&wm);
    exit(EXIT_FAILURE);
  }
  free(fullConfigPath);

  setupKeybindings(&wm);

  time_t lastBarUpdate = 0;

  while (true) {
    fd_set in_fds;
    struct timeval tv;
    FD_ZERO(&in_fds);
    FD_SET(ConnectionNumber(wm.dpy), &in_fds);
    tv.tv_usec = 100000;

    if (XPending(wm.dpy) ||
        select(ConnectionNumber(wm.dpy) + 1, &in_fds, NULL, NULL, &tv)) {
      XEvent ev;
      XNextEvent(wm.dpy, &ev);
      if (!handleEvent(&wm, ev))
        break;
    }

    time_t now = time(NULL);
    if (now != lastBarUpdate) {
      updateBar(&wm);
      lastBarUpdate = now;
    }
  }

  freeConfig(&wm);
  cleanupWindowManager(&wm);
  exit(EXIT_SUCCESS);
}

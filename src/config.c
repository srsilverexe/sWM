#define _GNU_SOURCE

#include "../include/config.h"
#include "../include/keybindings.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hexToDecimal(const char *hex) {
  if (strlen(hex) != 6) {
    fprintf(stderr, "Invalid color format: %s\n", hex);
    return 0x000000; // Default black
  }

  int decimal = 0;
  int base = 1;
  int len = strlen(hex);

  for (int i = len - 1; i >= 0; i--) {
    if (hex[i] >= '0' && hex[i] <= '9') {
      decimal += (hex[i] - '0') * base;
    } else if (hex[i] >= 'A' && hex[i] <= 'F') {
      decimal += (hex[i] - 'A' + 10) * base;
    } else if (hex[i] >= 'a' && hex[i] <= 'f') {
      decimal += (hex[i] - 'a' + 10) * base;
    }
    base *= 16;
  }
  return decimal;
}

int parseConfigFile(WindowManager *wm, char *path) {
  FILE *cfgFile = fopen(path, "r");
  if (!cfgFile) {
    perror("Failed to open config file: ");
    return 1;
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int lineNum = 0;

  wm->config.borderSize = 3;
  wm->config.gaps = 10;
  wm->config.unfocusedBorderColor = 0X000000;
  wm->config.focusedBorderColor = 0XFF00FF;
  wm->config.keybindings = NULL;
  wm->config.nWorkspaces = 10;

  while ((read = getline(&line, &len, cfgFile)) != -1) {
    lineNum++;

    if (read <= 1 || line[0] == '#')
      continue;

    line[strcspn(line, "\n")] = '\0';

    char *key = strtok(line, " ");
    if (!key)
      continue;

    if (strcmp(key, "bind") == 0) {
      char *modstr = strtok(NULL, " ");
      char *keystr = strtok(NULL, " ");
      char *action = strtok(NULL, " ");

      if (!modstr || !keystr || !action) {
        fprintf(stderr, "Invalid bind syntax at line %d\n", lineNum);
        continue;
      }

      unsigned int mods = parseModifiers(modstr);

      KeyCode keycode = parseKey(wm, keystr);
      if (!keycode) {
        fprintf(stderr, "Unknown key '%s' at line %d\n", keystr, lineNum);
        continue;
      }

      if (strcmp(action, "exec") == 0) {
        char *cmd = strtok(NULL, "");
        if (!cmd) {
          fprintf(stderr, "Missing command at line %d\n", lineNum);
          continue;
        }

        if (cmd[0] == '"' && cmd[strlen(cmd) - 1] == '"') {
          cmd[strlen(cmd) - 1] = '\0';
          cmd++;
        }

        char *command = strdup(cmd);
        addKeybinding(wm, mods, keycode, EXEC, command);
      } else if (strcmp(action, "focus_next") == 0) {
        addKeybinding(wm, mods, keycode, CHANGE_FOCUS_NEXT, NULL);
      } else if (strcmp(action, "focus_prev") == 0) {
        addKeybinding(wm, mods, keycode, CHANGE_FOCUS_PREV, NULL);
      } else if (strcmp(action, "quit") == 0) {
        addKeybinding(wm, mods, keycode, QUIT, NULL);
      } else if (strcmp(action, "kill_focused_window") == 0) {
        addKeybinding(wm, mods, keycode, KILL_FOCUSED_WINDOW, NULL);
      } else if (strcmp(action, "change_master") == 0) {
        addKeybinding(wm, mods, keycode, CHANGE_MASTER, NULL);
      } else if (strcmp(action, "add_master_ratio") == 0) {
        addKeybinding(wm, mods, keycode, ADD_MASTER_RATIO, NULL);
      } else if (strcmp(action, "remove_master_ratio") == 0) {
        addKeybinding(wm, mods, keycode, REMOVE_MASTER_RATIO, NULL);
      } else if (strcmp(action, "change_current_workspace") == 0) {
        char *value = strtok(NULL, " ");
        if (!value) {
          fprintf(stderr, "Missing value at line %d\n", lineNum);
          continue;
        }

        bool valid = true;
        for (char *p = value; *p; p++) {
          if (!isdigit(*p)) {
            valid = false;
            break;
          }
        }
        if (!valid) {
          fprintf(stderr, "Invalid numeric value at line %d\n", lineNum);
          continue;
        }

        unsigned int *number = malloc(sizeof(unsigned int));
        (*number) = atoi(value);

        addKeybinding(wm, mods, keycode, CHANGE_CURRENT_WORKSPACE, number);
      } else if (strcmp(action, "move_focused_window_to_workspace") == 0) {
        char *value = strtok(NULL, " ");
        if (!value) {
          fprintf(stderr, "Missing value at line %d\n", lineNum);
          continue;
        }

        bool valid = true;
        for (char *p = value; *p; p++) {
          if (!isdigit(*p)) {
            valid = false;
            break;
          }
        }
        if (!valid) {
          fprintf(stderr, "Invalid numeric value at line %d\n", lineNum);
          continue;
        }

        unsigned int *number = malloc(sizeof(unsigned int));
        (*number) = atoi(value);

        addKeybinding(wm, mods, keycode, MOVE_FOCUSED_WINDOW_TO_WORKSPACE,
                      number);
      } else if (strcmp(action, "change_layout_to_master") == 0) {
        addKeybinding(wm, mods, keycode, CHANGE_LAYOUT_TO_MASTER, NULL);
      } else if (strcmp(action, "change_layout_to_monocle") == 0) {
        addKeybinding(wm, mods, keycode, CHANGE_LAYOUT_TO_MONOCLE, NULL);
      } else {
        fprintf(stderr, "Unknown action '%s' at line %d\n", action, lineNum);
      }
      printf("New binding\n \
             mod: %s\n \
             key: %s\n \
             action: %s\n",
             modstr, keystr, action);

    } else {
      char *value = strtok(NULL, " ");
      if (!value) {
        fprintf(stderr, "Missing value at line %d\n", lineNum);
        continue;
      }

      for (char *p = value; *p; p++) {
        if (!isdigit(*p)) {
          fprintf(stderr, "Invalid numeric value at line: %d\n", lineNum);
          break;
        }
      }

      unsigned int number = atoi(value);

      if (strcmp(key, "gaps") == 0) {
        wm->config.gaps = number;
      } else if (strcmp(key, "borderSize") == 0) {
        wm->config.borderSize = number;
      } else if (strcmp(key, "focusedBorderColor") == 0) {
        wm->config.focusedBorderColor = hexToDecimal(value);
      } else if (strcmp(key, "unfocusedBorderColor") == 0) {
        wm->config.unfocusedBorderColor = hexToDecimal(value);
      } else if (strcmp(key, "nWorkspaces") == 0) {
        wm->config.nWorkspaces = (number <= 0) ? 1 : number;
      } else {
        fprintf(stderr, "Unknown config key '%s' at line %d\n", key, lineNum);
      }
    }
  }

  free(line);
  fclose(cfgFile);
  return 0;
}

void freeConfig(WindowManager *wm) {
  while (wm->config.keybindings) {
    Keybinding *next = wm->config.keybindings->next;
    free(wm->config.keybindings->complement);
    free(wm->config.keybindings);
    wm->config.keybindings = next;
  }
}

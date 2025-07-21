# sWM - Silver ~~Simple~~ Window Manager

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build Status](https://github.com/srsilverexe/sWM/actions/workflows/build.yml/badge.svg)](https://github.com/srsilverexe/sWM/actions)

## Description

sWM is a minimal, lightweight X11 tiling window manager written in C. Designed for simplicity and efficiency, it provides essential window management features with a small codebase footprint.

## Features

- Master-stack tiling layout
- Monocle (fullscreen) layout
- Dynamic virtual workspaces
- Key-driven navigation and control
- Dynamic window focus management
- Gap support between windows
- Customizable status bar
- EWMH compliance for basic desktop features
- Simple configuration file

## Installation

### Dependencies
- Xlib headers
- GNU Make
- GCC or Clang

### Build & Install
```bash
git clone https://github.com/srsilverexe/sWM.git
cd sWM
sudo make install
```

## Uninstallation

```bash
sudo make uninstall
```

## Run

```bash
exec sWM
```

## Configuration
Default config: ~/.config/sWM/config.cfg

```cfg
# Window appearance
borderSize 3
unfocusedBorderColor 000000
focusedBorderColor FF00FF

gaps 10
nWorkspaces 10

# Keybindings
# Format: bind <modifiers> <key> <action> [argument]
bind mod4|shift q quit
bind mod4 m change_master
bind mod4 w kill_focused_window
bind mod4|control Left remove_master_ratio
bind mod4|control Right add_master_ratio

bind mod4 1 change_current_workspace 0
bind mod4 2 change_current_workspace 1
bind mod4 3 change_current_workspace 2
bind mod4 4 change_current_workspace 3
bind mod4 5 change_current_workspace 4
bind mod4 6 change_current_workspace 5
bind mod4 7 change_current_workspace 6
bind mod4 8 change_current_workspace 7
bind mod4 9 change_current_workspace 8
bind mod4 0 change_current_workspace 9

bind mod4|shift 1 move_focused_window_to_workspace 0
bind mod4|shift 2 move_focused_window_to_workspace 1
bind mod4|shift 3 move_focused_window_to_workspace 2
bind mod4|shift 4 move_focused_window_to_workspace 3
bind mod4|shift 5 move_focused_window_to_workspace 4
bind mod4|shift 6 move_focused_window_to_workspace 5
bind mod4|shift 7 move_focused_window_to_workspace 6
bind mod4|shift 8 move_focused_window_to_workspace 7
bind mod4|shift 9 move_focused_window_to_workspace 8
bind mod4|shift 0 move_focused_window_to_workspace 9

bind mod4|control m change_layout_to_master
bind mod4|control o change_layout_to_monocle

bind mod4 Enter exec "xterm &"

bind mod4 Left focus_prev
bind mod4 Right focus_next
```
### Keybindings

|       Keybindings      |                Action                 |
|------------------------|---------------------------------------|
| Mod4 + Shift + q       | Quit window manager                   |
| Mod4 + m               | Change Master window in Master Layout |
| Mod4 + w               | Kill focused window                   |
| Mod4 + Control + Left  | Shrink master window                  |
| Mod4 + Control + Right | Expand master window                  |
| Mod4 + 1               | Change current workspace to 0         |
| Mod4 + 2               | Change current workspace to 1         |
| Mod4 + 3               | Change current workspace to 2         |
| Mod4 + 4               | Change current workspace to 3         |
| Mod4 + 5               | Change current workspace to 4         |
| Mod4 + 6               | Change current workspace to 5         |
| Mod4 + 7               | Change current workspace to 6         |
| Mod4 + 8               | Change current workspace to 7         |
| Mod4 + 9               | Change current workspace to 8         |
| Mod4 + 0               | Change current workspace to 9         |
| Mod4 + Shift + 1       | Move focused window to workspace 0    |
| Mod4 + Shift + 2       | Move focused window to workspace 1    |
| Mod4 + Shift + 3       | Move focused window to workspace 2    |
| Mod4 + Shift + 4       | Move focused window to workspace 3    |
| Mod4 + Shift + 5       | Move focused window to workspace 4    |
| Mod4 + Shift + 6       | Move focused window to workspace 5    |
| Mod4 + Shift + 7       | Move focused window to workspace 6    |
| Mod4 + Shift + 8       | Move focused window to workspace 7    |
| Mod4 + Shift + 9       | Move focused window to workspace 8    |
| Mod4 + Shift + 0       | Move focused window to workspace 9    |
| Mod4 + Control + m     | Change layout to master layout        |
| Mod4 + control + o     | Change layout to monocle Layout       |
| Mod4 + Enter           | Exec Xterm                            |
| Mod4 + Left            | Change focus to previous window        |
| Mod4 + Right           | Change focus to next window           |


## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License
MIT - See [LICENSE](LICENSE).

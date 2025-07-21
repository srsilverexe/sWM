# sWM - Silver Window Manager

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build Status](https://github.com/srsilverexe/sWM/actions/workflows/build.yml/badge.svg)](https://github.com/srsilverexe/sWM/actions)

## Description

sWM is a minimal, lightweight X11 tiling window manager written in C. Designed for simplicity and efficiency, it provides essential window management features with a small codebase footprint.

## Features

- Master-stack tiling layout
- Monocle (fullscreen) layout
- Dynamic set virtual workspaces
- Key-driven navigation and control
- Dynamic window focus management
- Gap support between windows
- Customizable status bar
- EWMH compliance for basic desktop features
- Simple configuration file
- Window borders with configurable colors
- Master window ratio control
- Workspace-specific window management

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
Default config: `~/.config/sWM/config.cfg`

Example configuration:
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
bind mod4 r exec "rofi -show drun"

bind mod4 Left focus_prev
bind mod4 Right focus_next
```

### Keybindings Reference

| Key Combination              | Action                                 |
|------------------------------|----------------------------------------|
| <kbd>Mod4</kbd>+<kbd>Shift</kbd>+<kbd>q</kbd> | Quit window manager                   |
| <kbd>Mod4</kbd>+<kbd>m</kbd> | Change master window in master layout  |
| <kbd>Mod4</kbd>+<kbd>w</kbd> | Kill focused window                   |
| <kbd>Mod4</kbd>+<kbd>Ctrl</kbd>+<kbd>←</kbd> | Shrink master area (ratio -0.05)      |
| <kbd>Mod4</kbd>+<kbd>Ctrl</kbd>+<kbd>→</kbd> | Expand master area (ratio +0.05)      |
| <kbd>Mod4</kbd>+<kbd>1-0</kbd> | Switch to workspace 0-9              |
| <kbd>Mod4</kbd>+<kbd>Shift</kbd>+<kbd>1-0</kbd> | Move window to workspace 0-9        |
| <kbd>Mod4</kbd>+<kbd>Ctrl</kbd>+<kbd>m</kbd> | Switch to master layout              |
| <kbd>Mod4</kbd>+<kbd>Ctrl</kbd>+<kbd>o</kbd> | Switch to monocle layout             |
| <kbd>Mod4</kbd>+<kbd>Enter</kbd> | Launch xterm                        |
| <kbd>Mod4</kbd>+<kbd>r</kbd> | Launch rofi application launcher     |
| <kbd>Mod4</kbd>+<kbd>←</kbd> | Focus previous window                |
| <kbd>Mod4</kbd>+<kbd>→</kbd> | Focus next window                    |

## Window Management Features

- **Master Layout**: Main window occupies larger area, other windows tile vertically
- **Monocle Layout**: Only focused window is visible (fullscreen)
- **Dynamic Resizing**: Adjust master area ratio with keybindings
- **Workspace Management**: 10 independent workspaces
- **Window Movement**: Move windows between workspaces
- **Focus Navigation**: Quickly switch between windows
- **Gap Support**: Configurable gaps between windows
- **Border Customization**: Different colors for focused/unfocused windows

## Status Bar Features

- Workspace indicator
- System clock
- Positioned at top of screen
- Auto-resizes with screen
- Dock type window (reserves space)

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License
MIT - See [LICENSE](LICENSE).

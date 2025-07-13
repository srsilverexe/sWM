# Configuration Guide

## File Locations
- System default: `/usr/share/sWM/config.cfg`
- User override: `~/.config/sWM/config.cfg`

## Appearance Settings
```ini
# Pixel gap between windows
gaps 10

# Border width in pixels
borderSize 3

# Border colors (RRGGBB hex format)
focusedBorderColor FF0000   # Red
unfocusedBorderColor 0000FF # Blue
```

## Keybinding Syntax
```
bind [MODIFIERS]+[KEY] [ACTION] [ARGUMENT?]
```

### Modifiers
- `mod4`: Windows/Super key
- `shift`
- `ctrl`/`control`
- `alt`/`mod1`

### Key Names
- Alphabetic keys: `a`-`z`
- Function keys: `F1`-`F12`
- Special keys: `Return`, `Space`, `Tab`, `Escape`, `Left`, `Right`, `Up`, `Down`

### Actions
| Action                            | Description                  |
|-----------------------------------|------------------------------|
| `exec "COMMAND"`                  | Execute shell command        |
| `quit`                            | Exit window manager          |
| `focus_next`/`focus_prev`         | Cycle window focus           |
| `kill_focused_window`             | Close focused window         |
| `change_master`                   | Promote window to master     |
| `add_master_ratio`                | Increase master area         |
| `remove_master_ratio`             | Decrease master area         |
| `change_current_workspace N`      | Switch to workspace N (1-10) |
| `change_layout_to_master`         | Use master-stack layout      |
| `change_layout_to_monocle`        | Use fullscreen layout        |

## Example Configuration
```ini
# Basic settings
gaps 5
borderSize 2
focusedBorderColor 00FF00
unfocusedBorderColor 333333

# Keybindings
bind mod4+Return exec "xterm"
bind mod4+q quit
bind mod4+c kill_focused_window
bind mod4+j focus_next
bind mod4+k focus_prev
bind mod4+1 change_current_workspace 1
bind mod4|shift+1 move_focused_window_to_workspace 1
```

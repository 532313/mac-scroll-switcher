# scrollswitch

`scrollswitch` is a small macOS background tool written in C.

It keeps trackpad scrolling natural while making a traditional mouse wheel feel like Windows:

- Enable macOS natural scrolling in System Settings.
- Run `scrollswitch`.
- Trackpad scroll events pass through unchanged.
- Discrete mouse-wheel events are inverted.

This is needed because macOS exposes natural scrolling as a global setting for both trackpads and mice.

## Build

```sh
make
```

## Run

```sh
./scrollswitch
```

The first run needs Accessibility permission:

1. Open System Settings.
2. Go to Privacy & Security.
3. Open Accessibility.
4. Allow the terminal app you used to start `scrollswitch`, or allow the installed `scrollswitch` binary.

Keep "Natural scrolling" enabled in macOS settings.

## Install as a LaunchAgent (auto-start on login)

Install the binary and register it as a LaunchAgent:

```sh
install -d ~/.local/bin
install -m 0755 scrollswitch ~/.local/bin/scrollswitch

# Copy and configure the plist
sed "s|@BINDIR@|$HOME/.local/bin|g" launchd/com.local.scrollswitch.plist \
  > ~/Library/LaunchAgents/com.local.scrollswitch.plist
chmod 0644 ~/Library/LaunchAgents/com.local.scrollswitch.plist

# Register the agent (takes effect on next login)
launchctl bootstrap "gui/$(id -u)" ~/Library/LaunchAgents/com.local.scrollswitch.plist
```

To start immediately without rebooting:

```sh
launchctl kickstart -k "gui/$(id -u)/com.local.scrollswitch"
```

To stop and unregister:

```sh
launchctl bootout "gui/$(id -u)/com.local.scrollswitch"
```

Default paths:

```text
~/.local/bin/scrollswitch
~/Library/LaunchAgents/com.local.scrollswitch.plist
```

### How auto-start and auto-stop work

The [launchd plist](launchd/com.local.scrollswitch.plist) uses two keys:

| Key | Value | Purpose |
| --- | --- | --- |
| `RunAtLoad` | `true` | Starts scrollswitch automatically when you log in. |
| `KeepAlive` | `true` | Restarts the process if it crashes unexpectedly. |

On **shutdown or logout**, macOS's launchd sends `SIGTERM` to all running user agents. The code handles this signal ([scrollswitch.c:174](src/scrollswitch.c#L174)) and cleanly exits the run loop, so no stale process remains.

## Options

```sh
scrollswitch --help
scrollswitch --verbose
scrollswitch --invert-continuous
```

`--invert-continuous` also inverts continuous scroll devices such as a Magic Mouse.
Do not use it if your trackpad is already correct.

## Notes

This tool uses a `CGEventTap`, so macOS may disable it until Accessibility permission is granted.
It does not change system preferences and does not need root privileges.


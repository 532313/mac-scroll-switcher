# scrollswitch

Keep your Mac's trackpad natural scrolling, while inverting the direction of a traditional mouse wheel.

In macOS, "natural scrolling" is a single global setting that applies to both trackpads and mice.
This tool intercepts scroll events at the system level and only inverts the mouse wheel, so your trackpad stays natural.

## How to set up

### 1. System Settings

Open **System Settings → Trackpad → Scroll & Zoom**, make sure **Natural scrolling** is enabled.

Open **System Settings → Mouse**, make sure **Natural scrolling** is **enabled** here too.
(Yes, enable it for both. scrollswitch will invert the mouse wheel for you.)

### 2. Build

```sh
make
```

If `make` fails, install **Xcode Command Line Tools** first:

```sh
xcode-select --install
```

### 3. Run once (to grant permission)

```sh
./scrollswitch

# Optional: add --verbose to see log output
./scrollswitch --verbose
```

The first time, macOS will show a popup asking for **Accessibility** permission.

1. Click **Open System Settings** in the popup (or go to **System Settings → Privacy & Security → Accessibility**).
2. Find your terminal app (e.g. Terminal, iTerm2) in the list and enable it.
3. Run `./scrollswitch` again.

Once it's running, use your mouse — the scroll direction should be inverted, while your trackpad stays natural.
Press `Ctrl+C` to stop.

### 4. Install for auto-start on login

This registers scrollswitch as a **LaunchAgent**, so it starts automatically when you log in and stops when you shut down.

```sh
install -d ~/.local/bin
install -m 0755 scrollswitch ~/.local/bin/scrollswitch

sed "s|@BINDIR@|$HOME/.local/bin|g" launchd/com.local.scrollswitch.plist \
  > ~/Library/LaunchAgents/com.local.scrollswitch.plist
chmod 0644 ~/Library/LaunchAgents/com.local.scrollswitch.plist

launchctl bootstrap "gui/$(id -u)" ~/Library/LaunchAgents/com.local.scrollswitch.plist
```

To start immediately without rebooting:

```sh
launchctl kickstart -k "gui/$(id -u)/com.local.scrollswitch"
```

### 5. Verify it's running

```sh
launchctl list | grep scrollswitch
```

If you see a line with `com.local.scrollswitch` and no error code, it's running.

## How it works

### Auto-start on login

macOS's launchd watches `~/Library/LaunchAgents/`. When you log in, it finds the plist file,
sees `RunAtLoad = true`, and immediately starts scrollswitch.

### Auto-stop on shutdown

When you shut down, log out, or restart, macOS's launchd sends a `SIGTERM` signal to every
running user agent. scrollswitch catches this signal and exits cleanly — no stale processes.

### Crash recovery

If scrollswitch crashes unexpectedly, `KeepAlive = true` tells launchd to restart it automatically.

## Manage the service

| What you want | Command |
| --- | --- |
| Start now (or restart after stopping) | `launchctl kickstart -k "gui/$(id -u)/com.local.scrollswitch"` |
| Stop and unregister (won't start on next login until you re-run the install step) | `launchctl bootout "gui/$(id -u)/com.local.scrollswitch"` |
| Check if running | `launchctl list \| grep scrollswitch` |
| Re-apply plist after modifying it | `launchctl bootout "gui/$(id -u)/com.local.scrollswitch"` / `launchctl bootstrap "gui/$(id -u)" ~/Library/LaunchAgents/com.local.scrollswitch.plist` |

### How to completely remove

```sh
# Stop the service
launchctl bootout "gui/$(id -u)/com.local.scrollswitch"

# Remove the files
rm ~/Library/LaunchAgents/com.local.scrollswitch.plist
rm ~/.local/bin/scrollswitch
```

## Options

```sh
./scrollswitch --help
./scrollswitch --verbose
./scrollswitch --invert-continuous
```

`--invert-continuous` also inverts scroll events from continuous devices (e.g. Magic Mouse).
Don't use this if your trackpad is already working correctly.

## Notes

- This tool uses `CGEventTap`, which is why macOS requires Accessibility permission.
- It does not change any system settings — it only modifies scroll events in real time.
- No root privileges needed.

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

## Install as a LaunchAgent

```sh
make install
make load
```

Default install path:

```text
/usr/local/bin/scrollswitch
~/Library/LaunchAgents/com.local.scrollswitch.plist
```

To uninstall:

```sh
make uninstall
```

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


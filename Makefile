CC := clang
CFLAGS := -std=c99 -Wall -Wextra -Werror -pedantic -O2
LDFLAGS := -framework ApplicationServices -framework CoreFoundation

PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin
LAUNCH_AGENT := $(HOME)/Library/LaunchAgents/com.local.scrollswitch.plist

BIN := scrollswitch
SRC := src/scrollswitch.c
PLIST := launchd/com.local.scrollswitch.plist

.PHONY: all clean install uninstall load unload

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f $(BIN)

install: $(BIN)
	install -d "$(BINDIR)"
	install -m 0755 "$(BIN)" "$(BINDIR)/$(BIN)"
	install -d "$(HOME)/Library/LaunchAgents"
	sed "s|@BINDIR@|$(BINDIR)|g" "$(PLIST)" > "$(LAUNCH_AGENT)"
	chmod 0644 "$(LAUNCH_AGENT)"

uninstall: unload
	rm -f "$(BINDIR)/$(BIN)"
	rm -f "$(LAUNCH_AGENT)"

load:
	launchctl bootstrap "gui/$$(id -u)" "$(LAUNCH_AGENT)" || true
	launchctl kickstart -k "gui/$$(id -u)/com.local.scrollswitch"

unload:
	launchctl bootout "gui/$$(id -u)" "$(LAUNCH_AGENT)" || true


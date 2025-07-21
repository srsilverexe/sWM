CC = gcc
CFLAGS = -std=c2x -pedantic -Wall -Wextra -Wpedantic -O2
LDFLAGS = -lX11

DEBUG_CFLAGS = -g -O0 -rdynamic -Ddebug
RELEASE_CFLAGS = -O3

BIN = sWM
SRC_DIR = src
BUILD_DIR = build

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d)

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(BIN)

install: $(BIN)
	install -Dm755 $(BIN) $(DESTDIR)/usr/bin/$(BIN)
	install -Dm644 sWM.desktop $(DESTDIR)/usr/share/xsessions/sWM.desktop
	mkdir -p $(DESTDIR)/usr/share/sWM
	cp -rf config.cfg $(DESTDIR)/usr/share/sWM/config.cfg
	mkdir -p ~/.config/sWM
	cp -rf config.cfg ~/.config/sWM/config.cfg

-include $(DEPS)

.PHONY: all clean install

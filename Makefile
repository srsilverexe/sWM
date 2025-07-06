CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -O2
LDFLAGS = -lX11

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
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(BIN)

install: $(BIN)
	install -Dm755 $(BIN) $(DESTDIR)/usr/bin/$(BIN)
	install -Dm644 sWM.desktop $(DESTDIR)/usr/share/xsessions/sWM.desktop
	mkdir -p ~/.config/sWM
	install -Dm644 config.cfg ~/.config/sWM/config.cfg

-include $(DEPS)

.PHONY: all clean install

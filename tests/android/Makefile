# Makefile for R47 Android Native Invariant Tests

CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -DANDROID_BUILD=1 -DPC_BUILD=1 -DLINUX -DOS64BIT -DWITH_PULSEAUDIO -pthread
CFLAGS += -Iinclude -I../../src/c47 -I../../src/c47/hal -I../../src/generated -I../../build.sim/src/c47 -I../../build.sim/src/generated -I../../build.sim/src/generateConstants -I../../build.sim/src/generateCatalogs -I../../android/app/src/main/cpp/generated -I../../dep/decNumberICU
CFLAGS += -I../../android/app/src/main/cpp/c47-android -I../../android/app/src/main/cpp/c47-android/hal
CFLAGS += $(shell pkg-config --cflags gtk+-3.0)

SRC = test_android_invariants.c \
      ../../android/app/src/main/cpp/c47-android/hal/io.c \
      ../../android/app/src/main/cpp/c47-android/android_helpers.c \
      ../../android/app/src/main/cpp/c47-android/hal/print_ir.c

TARGET = test_android_invariants

all: $(TARGET)
	./$(TARGET)

items_android.c: ../../src/c47/items.c
	(sed -n '40,118p' ../../src/c47/items.c; echo "#endif") > $@

items_table_full.c: ../../src/c47/items.c
	(echo '#include "c47.h"'; \
	 sed -n '1752,$$p' ../../src/c47/items.c | \
	 sed -E '/^#define UNIT_CONV/,/RESULT_IN_X \}/c\#define UNIT_CONV(unit, invert, cat, menu) { NULL, 0, cat, menu, 0, CAT_NONE | SLS_ENABLED | US_ENABLED | EIM_DISABLED | PTP_NONE | RESULT_IN_X }' | \
	 sed -E 's/^(\/\*[[:space:]]*[0-9]+[[:space:]]*\*\/[[:space:]]*\{)[[:space:]]*[^,]+,[[:space:]]*[^,]+,/\1 NULL, 0,/') > $@

items_table_full.o: items_table_full.c
	$(CC) $(CFLAGS) -c -o $@ $<

keyboard_keycode.c: ../../src/c47/keyboard.c
	(echo '#include "c47.h"'; echo 'extern uint8_t lastKeyCode;'; echo 'extern uint8_t calcModel;'; sed -n '/^void setLastKeyCode/,/^}/p' ../../src/c47/keyboard.c) > $@

$(TARGET): items_android.c items_table_full.o keyboard_keycode.c $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) items_table_full.o keyboard_keycode.c $(shell pkg-config --libs gtk+-3.0) -lm

clean:
	rm -f $(TARGET) items_android.c items_table_full.c items_table_full.o keyboard_keycode.c *.o


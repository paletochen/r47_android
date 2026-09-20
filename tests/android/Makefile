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

$(TARGET): items_android.c $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(shell pkg-config --libs gtk+-3.0) -lm

clean:
	rm -f $(TARGET) items_android.c *.o

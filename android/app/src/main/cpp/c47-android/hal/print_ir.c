// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"
#include "charString.h"
#include <stdio.h>
#include <string.h>

extern const char *get_android_base_path(void);
extern const uint16_t hp82240CharMap[256];

static uint16_t android_ir_line_delay = 0;
static int esc_state = 0;
static int esc_bytes_expected = 0;

uint32_t getLineDelay(void) {
  return android_ir_line_delay;
}

void setLineDelay(uint16_t delay) {
  android_ir_line_delay = delay;
}

#include "jni_bridge.h"
#include <unistd.h>

static char print_line_buf[2048];
static size_t print_line_len = 0;

static char pretty_line_buf[4096];
static size_t pretty_line_len = 0;

static void appendToFile(int fileType, const char *fileName, const char *data, size_t len) {
  if(!data || len == 0) {
    return;
  }
  FILE *f = NULL;
  int directFd = openDirectDocumentFd(fileType, fileName, "wa");
  if(directFd >= 0) {
    f = fdopen(directFd, "a");
  }

  if(!f) {
    const char *base_path = get_android_base_path();
    if(base_path && base_path[0]) {
      char dir[1024];
      snprintf(dir, sizeof(dir), "%s/PRINT", base_path);
      mkdir(dir, 0777);
      char path[1024];
      snprintf(path, sizeof(path), "%s/PRINT/%s", base_path, fileName);
      f = fopen(path, "a");
    }
  }

  if(!f) {
    LOGE("appendToFile: Unable to open %s", fileName);
    return;
  }

  fwrite(data, 1, len, f);
  fflush(f);
  fclose(f);
}

static void flushPrinterBuffer(void) {
  if(print_line_len == 0) {
    return;
  }
  appendToFile(5 /* PRINT */, "printer_output.txt", print_line_buf, print_line_len);
  print_line_len = 0;
}

static void flushPrettyBuffer(void) {
  if(pretty_line_len == 0) {
    return;
  }
  appendToFile(5 /* PRINT */, "prettyprinter_output.txt", pretty_line_buf, pretty_line_len);
  appendToFile(5 /* PRINT */, "prettyprint_output.txt", pretty_line_buf, pretty_line_len);
  pretty_line_len = 0;
}

void printPrettyLine(const char *str) {
  if(!str) {
    return;
  }
  size_t len = strlen(str);
  if(pretty_line_len + len + 2 >= sizeof(pretty_line_buf)) {
    flushPrettyBuffer();
  }
  memcpy(pretty_line_buf + pretty_line_len, str, len);
  pretty_line_len += len;
  pretty_line_buf[pretty_line_len++] = '\n';
  flushPrettyBuffer();
}

void sendByteIR(uint8_t byte) {
  // Handle printer escape sequences (e.g. graphics or margins)
  if(esc_state == 1) {
    esc_bytes_expected = byte;
    if(esc_bytes_expected == 0) {
      esc_state = 0;
    }
    else {
      esc_state = 2;
    }
    return;
  }
  else if(esc_state == 2) {
    if(--esc_bytes_expected <= 0) {
      esc_state = 0;
    }
    return;
  }
  else if(byte == 27) { // ESC
    esc_state = 1;
    return;
  }

  if(print_line_len + 8 >= sizeof(print_line_buf)) {
    flushPrinterBuffer();
  }

  bool is_newline = false;
  if(byte == '\n' || byte == 0x04) {
    print_line_buf[print_line_len++] = '\n';
    is_newline = true;
  }
  else if(byte >= 32 && byte <= 126) {
    print_line_buf[print_line_len++] = (char)byte;
  }
  else {
    uint16_t unicode = hp82240CharMap[byte];
    if(unicode == 0x240A || unicode == 0x2404) {
      print_line_buf[print_line_len++] = '\n';
      is_newline = true;
    }
    else if(unicode != 0) {
      uint8_t utf8[8];
      codePointToUtf8(unicode, utf8);
      size_t ulen = strlen((char *)utf8);
      if(print_line_len + ulen < sizeof(print_line_buf)) {
        memcpy(print_line_buf + print_line_len, utf8, ulen);
        print_line_len += ulen;
      }
    }
  }

  if(is_newline) {
    flushPrinterBuffer();
    void yieldToAndroidWithMs(int ms);
    yieldToAndroidWithMs(1);
  }
}

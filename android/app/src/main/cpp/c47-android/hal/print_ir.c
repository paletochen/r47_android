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

static void flushPrinterBuffer(void) {
  if(print_line_len == 0) {
    return;
  }

  FILE *f = NULL;
  int directFd = openDirectDocumentFd(5 /* PRINT */, "printer_output.txt", "wa");
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
      snprintf(path, sizeof(path), "%s/PRINT/printer_output.txt", base_path);
      f = fopen(path, "a");
    }
  }

  if(!f) {
    LOGE("flushPrinterBuffer: Unable to open printer_output.txt");
    print_line_len = 0;
    return;
  }

  fwrite(print_line_buf, 1, print_line_len, f);
  fflush(f);
  fclose(f);
  print_line_len = 0;
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
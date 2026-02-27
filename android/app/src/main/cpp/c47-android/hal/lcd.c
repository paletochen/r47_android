#include "c47.h"
#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "R47Lcd"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Android specific globals for display
uint32_t *screenData = NULL; // ARGB8888 buffer: 400x240
extern uint8_t *lcd_buffer; // Defined in c47.c

// Dirty flags for Kotlin layer
bool screenDataDirty = false;

// bits=1 -> BACKGROUND (Light), bits=0 -> TEXT (Dark)
uint32_t textPixel = 0xFF303030;       // Default Black-ish
uint32_t backgroundPixel = 0xFFDFF5CC; // Default Vintage BG

// Precomputed lookup table for 8-pixel expansion
// Converts 1 byte (8 bits) into 8 uint32_t pixels
static uint32_t pixelLookup[256][8];
static bool lookupInitialized = false;

static void updateLookupTable() {
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 8; j++) {
            // R47 core bit-order: Bit 0 is the RIGHTMOST pixel in the 8-pixel segment.
            // Our lookup table chunk[0..7] maps to increasing memory addresses.
            // So chunk[7] should be Bit 0, chunk[0] should be Bit 7.
            pixelLookup[i][j] = (i & (1 << (7 - j))) ? textPixel : backgroundPixel;
        }
    }
    lookupInitialized = true;
    screenDataDirty = true;
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_setLcdColors(JNIEnv* env, jobject thiz, jint text, jint bg) {
    textPixel = (uint32_t)text;
    backgroundPixel = (uint32_t)bg;
    updateLookupTable();
    // Force a full redraw of screenData from lcd_buffer
    if (lcd_buffer) {
        for (int r=0; r<SCREEN_HEIGHT; r++) {
            lcd_buffer[r*52] = 1u; // Mark all rows dirty
        }
        lcd_refresh();
    }
}

void init_lcd_buffers() {
    if (!screenData) {
        screenData = (uint32_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    }
    
    if (!lookupInitialized) {
        updateLookupTable();
    }

    for(int i=0; i<SCREEN_WIDTH*SCREEN_HEIGHT; i++) screenData[i] = backgroundPixel;

    if (!lcd_buffer) {
        lcd_buffer = (uint8_t*)malloc(SCREEN_HEIGHT * 52);
        memset(lcd_buffer, 255, SCREEN_HEIGHT * 52);
        for(int r=0; r<SCREEN_HEIGHT; r++) {
            lcd_buffer[r*52] = 1u; // Dirty
            lcd_buffer[r*52 + 1] = SCREEN_HEIGHT - r - 1; 
        }
    }
}

uint8_t * lcd_line_addr (int row) {
  if (!lcd_buffer) return NULL;
  lcd_buffer[52 * (row)] = 1u;
  return lcd_buffer + row * 52 + 2;
}

void LCD_write_line (uint8_t *line_buf) {
  if (!screenData || !lookupInitialized) return;
  int i, row = line_buf[1];
  if (row < 0 || row >= SCREEN_HEIGHT) return;
  
  // Mapping core row to Android screen row (Top-to-Bottom)
  uint32_t *lineStart = screenData + (SCREEN_HEIGHT - row - 1) * SCREEN_WIDTH;
  
  for (i=0; i<50; i++) {
    // Expand 1 byte to 8 pixels using precomputed table
    uint8_t val = line_buf[i+2];
    uint32_t *chunk = pixelLookup[val];
    // R47 core packs bytes 0..49 from RIGHT to LEFT.
    // i=0 (byte 0) goes to pixels 392..399
    // i=49 (byte 49) goes to pixels 0..7
    uint32_t *dest = lineStart + (49 - i) * 8;
    memcpy(dest, chunk, 8 * sizeof(uint32_t));
  }
  line_buf[0] = 0u; 
  screenDataDirty = true;
}

void lcd_clear_buf () {
  if (!lcd_buffer) return;
  for (uint8_t row = 0; row < SCREEN_HEIGHT; row++) {
    uint8_t *line_buf = lcd_buffer + 52 * row;
    for (uint8_t c = 2; c < 52; c++) {
      line_buf[c] = 0u; // 0 = BACKGROUND
    }
    line_buf[1] = SCREEN_HEIGHT - row - 1;
    line_buf[0] = 1u; 
    LCD_write_line(line_buf);
  }
}

void lcd_refresh () {
  if (!lcd_buffer) return;
  for (uint8_t row = 0; row < SCREEN_HEIGHT; row++) {
    if (lcd_buffer[52 * row]) { 
      LCD_write_line(&lcd_buffer[52 * row]);
    }
  }
}

void lcd_refresh_lines (uint8_t ln, uint8_t cnt) {
  if (!lcd_buffer) return;
  for (uint8_t row = ln; row < ln + cnt; row++) {
    LCD_write_line(&lcd_buffer[52 * row]);
  }
}

void bitblt24 (uint32_t x, uint32_t dx, uint32_t y, uint32_t val, int blt_op, int fill) {
  if (!lcd_buffer) return;
  if (dx < 1 || dx > 24) return;
  if (x >= SCREEN_WIDTH || x + dx > SCREEN_WIDTH) return;
  x = SCREEN_WIDTH - dx - x;
  const uint32_t byte_i   = x >> 3;
  const uint32_t bit_off  = x & 7u;
  const uint32_t lowmask  = (1u << dx) - 1u;
  uint32_t srcbits;
  if (fill == BLT_SET && blt_op != BLT_XOR) {
    srcbits = (blt_op == BLT_ANDN) ? lowmask << bit_off : 0u;
  } else {
    srcbits = (val & lowmask) << bit_off;
  }
  uint8_t srcbytes[4] = { (uint8_t)(srcbits), (uint8_t)(srcbits >> 8), (uint8_t)(srcbits >> 16), (uint8_t)(srcbits >> 24) };
  uint8_t *j = &lcd_buffer[y * (52) + byte_i + 2];
  switch (blt_op) {
    case BLT_OR:   for (int i = 0; i < 4; i++) j[i] |=  srcbytes[i]; break;
    case BLT_XOR:  for (int i = 0; i < 4; i++) j[i] ^=  srcbytes[i]; break;
    case BLT_ANDN: for (int i = 0; i < 4; i++) j[i] &= ~srcbytes[i]; break;
    default: return;
  }
  lcd_buffer[y * (52)] = 1u;
}

void lcd_fill_rect(uint32_t x, uint32_t y, uint32_t dx, uint32_t dy, int val) {
  uint32_t line, col, cols, endX = x + dx, endY = y + dy;
  if(endX > SCREEN_WIDTH || endY > SCREEN_HEIGHT) return;
  int blt_op = val ? BLT_OR : BLT_ANDN;
  for (col = x; col < endX; col += 24) {
    cols = (24 < endX - col) ? 24 : (endX - col);
    for(line = y; line < endY; line++) {
      bitblt24(col, cols, line, 0xFFFFFF, blt_op, BLT_NONE);
    }
  }
}

void _lcdRefresh(void) { lcd_refresh(); }
void _lcdBandRefresh(uint32_t y, uint32_t dy) { (void)y; (void)dy; lcd_refresh(); }
void _lcdSBRefresh(void) { lcd_refresh(); }
void refresh_gui(void) {}
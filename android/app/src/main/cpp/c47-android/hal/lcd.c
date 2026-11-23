#if !defined(LOG_TAG)
#define LOG_TAG "R47Lcd"
#endif // !defined(LOG_TAG)

#include "jni_bridge.h"
#include <string.h>
#include <stdlib.h>

// Android specific globals for display
uint32_t *screenData = NULL; // ARGB8888 buffer: 400x240
extern uint8_t *lcd_buffer; // Defined in c47.c

// Pixel state guarded by lcdPixelMutex (lock order: screenMutex -> lcdPixelMutex)
static bool screenDataDirty = false;
static pthread_mutex_t lcdPixelMutex = PTHREAD_MUTEX_INITIALIZER;

// bits=1 -> BACKGROUND (Light), bits=0 -> TEXT (Dark)
static uint32_t textPixel = 0xFF303030;       // Default Black-ish
static uint32_t backgroundPixel = 0xFFDFF5CC; // Default Vintage BG

// Precomputed lookup table for 8-pixel expansion
// Converts 1 byte (8 bits) into 8 uint32_t pixels
static uint32_t pixelLookup[256][8];
static bool lookupInitialized = false;

static void updateLookupTableLocked(void) {
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

/**
 * Expands one 52-byte 1-bpp scanline record into ARGB8888 screenData while lcdPixelMutex is locked.
 */
static void writeLineLocked(uint8_t *line_buf, bool clearDirtyByte) {
  if(!screenData || !lookupInitialized) {
    return;
  }
  int row = line_buf[1];
  if(row < 0 || row >= SCREEN_HEIGHT) {
    return;
  }

  uint32_t *lineStart = screenData + (SCREEN_HEIGHT - row - 1) * SCREEN_WIDTH;
  for(int i = 0; i < 50; i++) {
    uint8_t val = line_buf[i + 2];
    uint32_t *chunk = pixelLookup[val];
    uint32_t *dest = lineStart + (49 - i) * 8;
    memcpy(dest, chunk, 8 * sizeof(uint32_t));
  }
  if(clearDirtyByte) {
    line_buf[0] = 0u;
  }
  screenDataDirty = true;
}

JNIEXPORT void JNICALL
Java_org_rpncalculators_r47_MainActivity_setLcdColors(JNIEnv* env,
                                                      jobject thiz,
                                                      jint text,
                                                      jint bg) {
  (void)env;
  (void)thiz;
  pthread_mutex_lock(&lcdPixelMutex);
  const uint32_t prevText = textPixel;
  const uint32_t prevBg = backgroundPixel;
  textPixel = (uint32_t)text;
  backgroundPixel = (uint32_t)bg;
  updateLookupTableLocked();
  if(screenData) {
    if(prevText != prevBg) {
      for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        screenData[i] = (screenData[i] == prevText) ? textPixel : backgroundPixel;
      }
      screenDataDirty = true;
    } else if(lcd_buffer) {
      for(int r = 0; r < SCREEN_HEIGHT; r++) {
        writeLineLocked(&lcd_buffer[r * 52], false);
      }
    }
  }
  pthread_mutex_unlock(&lcdPixelMutex);
}

void init_lcd_buffers() {
  pthread_mutex_lock(&lcdPixelMutex);
  if(!screenData) {
    screenData = (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
  }

  if(!lookupInitialized) {
    updateLookupTableLocked();
  }

  if(screenData) {
    for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
      screenData[i] = backgroundPixel;
    }
    screenDataDirty = true;
  }

  if(!lcd_buffer) {
    lcd_buffer = (uint8_t *)malloc(SCREEN_HEIGHT * 52);
    if(lcd_buffer) {
      memset(lcd_buffer, 255, SCREEN_HEIGHT * 52);
      for(int r = 0; r < SCREEN_HEIGHT; r++) {
        lcd_buffer[r * 52] = 1u;  // Dirty
        lcd_buffer[r * 52 + 1] = SCREEN_HEIGHT - r - 1;
      }
    }
  }
  pthread_mutex_unlock(&lcdPixelMutex);
}

/**
 * Copies screenData into pixels under lcdPixelMutex when dirty, without contending on screenMutex.
 */
bool copyDirtyDisplayPixels(JNIEnv *env, jintArray pixels) {
  pthread_mutex_lock(&lcdPixelMutex);
  if(!screenData || !screenDataDirty) {
    pthread_mutex_unlock(&lcdPixelMutex);
    return false;
  }
  (*env)->SetIntArrayRegion(env, pixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT, (const jint *)screenData);
  screenDataDirty = false;
  pthread_mutex_unlock(&lcdPixelMutex);
  return true;
}

uint8_t * lcd_line_addr (int row) {
  if (!lcd_buffer) return NULL;
  lcd_buffer[52 * (row)] = 1u;
  return lcd_buffer + row * 52 + 2;
}

bool_t lcd_buffer_pixel_on(uint32_t x, uint32_t y) {
  if(x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
    return false;
  }
  if (!lcd_buffer) return false;
  const uint8_t *line_buf = lcd_buffer + 52 * y;
  const uint32_t bitIndex = SCREEN_WIDTH - 1 - x;
  const uint32_t byte_i = bitIndex >> 3;
  const uint32_t bit_j = bitIndex & 7u;
  return (line_buf[2 + byte_i] >> bit_j) & 1u;
}

void LCD_write_line (uint8_t *line_buf) {
  pthread_mutex_lock(&lcdPixelMutex);
  writeLineLocked(line_buf, true);
  pthread_mutex_unlock(&lcdPixelMutex);
}

void lcd_clear_buf () {
  if (!lcd_buffer) return;
  pthread_mutex_lock(&lcdPixelMutex);
  for (uint8_t row = 0; row < SCREEN_HEIGHT; row++) {
    uint8_t *line_buf = lcd_buffer + 52 * row;
    for (uint8_t c = 2; c < 52; c++) {
      line_buf[c] = 0u; // 0 = BACKGROUND
    }
    line_buf[1] = SCREEN_HEIGHT - row - 1;
    line_buf[0] = 1u; 
    writeLineLocked(line_buf, true);
  }
  pthread_mutex_unlock(&lcdPixelMutex);
}

void lcd_refresh () {
  if(!lcd_buffer) {
    return;
  }
  pthread_mutex_lock(&lcdPixelMutex);
  for(uint8_t row = 0; row < SCREEN_HEIGHT; row++) {
    if(lcd_buffer[52 * row]) {
      writeLineLocked(&lcd_buffer[52 * row], true);
    }
  }
  pthread_mutex_unlock(&lcdPixelMutex);
}

void lcd_refresh_lines (uint8_t ln, uint8_t cnt) {
  if(!lcd_buffer || ln >= SCREEN_HEIGHT || cnt == 0) {
    return;
  }
  uint32_t endRow = (uint32_t)ln + (uint32_t)cnt;
  if(endRow > SCREEN_HEIGHT) {
    endRow = SCREEN_HEIGHT;
  }
  pthread_mutex_lock(&lcdPixelMutex);
  for(uint32_t row = ln; row < endRow; row++) {
    writeLineLocked(&lcd_buffer[52 * row], true);
  }
  pthread_mutex_unlock(&lcdPixelMutex);
}

void bitblt24(uint32_t x, uint32_t dx, uint32_t y, uint32_t val, int blt_op, int fill) {
  if(!lcd_buffer) {
    return;
  }
  if(dx < 1 || dx > 24) {
    return;
  }
  if(x >= SCREEN_WIDTH || x + dx > SCREEN_WIDTH) {
    return;
  }
  x = SCREEN_WIDTH - dx - x;

  const uint32_t byte_i = x >> 3;
  const uint32_t bit_off = x & 7u;

  const uint32_t lowmask = (1u << dx) - 1u;
  const uint32_t bytes_needed = (bit_off + dx + 7) / 8; // Actual bytes to write (1-4), prevents overflow at line end

  const uint32_t srcbits = (val & lowmask) << bit_off;
  const uint32_t fillbits = (fill == BLT_SET) ? lowmask << bit_off : 0u; // BLT_SET: the dx columns are cleared before BLT_OR and set before BLT_ANDN
  uint8_t srcbytes[4] = {
    (uint8_t)(srcbits >> 0),
    (uint8_t)(srcbits >> 8),
    (uint8_t)(srcbits >> 16),
    (uint8_t)(srcbits >> 24),
  };
  uint8_t fillbytes[4] = {
    (uint8_t)(fillbits >> 0),
    (uint8_t)(fillbits >> 8),
    (uint8_t)(fillbits >> 16),
    (uint8_t)(fillbits >> 24),
  };
  uint8_t *j = &lcd_buffer[y * 52 + byte_i + 2];
  switch(blt_op) {
    case BLT_OR:
      for(uint32_t i = 0; i < bytes_needed; i++) {
        j[i] = (j[i] & ~fillbytes[i]) | srcbytes[i];
      }
      break;

    case BLT_XOR:
      for(uint32_t i = 0; i < bytes_needed; i++) {
        j[i] ^= srcbytes[i];
      }
      break;

    case BLT_ANDN:
      for(uint32_t i = 0; i < bytes_needed; i++) {
        j[i] = (j[i] | fillbytes[i]) & ~srcbytes[i];
      }
      break;

    default:
      return;
  }
  lcd_buffer[y * 52] = 1u;
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

static int bufCleared = 0;

/**
 * Rasterises dirty 1-bpp lcd_buffer scanlines into the ARGB8888 screenData frame for the UI refresh cycle.
 */
void lcd_refresh_dma(void) {
  lcd_refresh();
}

/**
 * Rasterises dirty 1-bpp lcd_buffer scanlines into the ARGB8888 screenData frame for the UI refresh cycle.
 */
void lcd_refresh_wait(void) {
  lcd_refresh();
}

/**
 * Re-renders all 1-bpp lcd_buffer scanlines into screenData regardless of per-row dirty flags.
 */
void lcd_forced_refresh(void) {
  if(!lcd_buffer) {
    return;
  }
  pthread_mutex_lock(&lcdPixelMutex);
  for(uint8_t row = 0; row < SCREEN_HEIGHT; row++) {
    writeLineLocked(&lcd_buffer[52 * row], true);
  }
  pthread_mutex_unlock(&lcdPixelMutex);
}

/**
 * Records whether the 1-bpp lcd_buffer has been cleared.
 */
void lcd_set_buf_cleared(int val) {
  bufCleared = val;
}

/**
 * Returns the recorded cleared state of the 1-bpp lcd_buffer.
 */
int lcd_get_buf_cleared(void) {
  return bufCleared;
}

/**
 * Triggers a BMP screen capture via fnScreenDump.
 */
int create_screenshot(int report_error) {
  (void)report_error;
  fnScreenDump(NOPARAM);
  return 0;
}

void _lcdBandRefresh(uint32_t y, uint32_t dy) { (void)y; (void)dy; lcd_refresh(); }
void refresh_gui(void) {}

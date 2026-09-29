// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors

/**
 * \file stubs.c
 * DMCP entry points implemented as code inside the emulated machine.
 *
 * A call answered here costs nothing on the host: the emulated processor runs these instructions and never leaves the JIT. A call answered in hostfn.py costs a
 * crossing into Python, many times what the instructions here take, and bitblt24 is most of the calls a boot makes.
 *
 * Nothing here needs the host, because everything these entry points touch is already in emulated memory: the LCD buffer, the key ring and the clock cell all sit
 * in the context block below, which the emulator fills in before the run.
 *
 * bitblt24 and lcd_fill_rect write the buffer as DMCP does with LCD_INVERT_XAXIS and LCD_INVERT_DATA, 0 for a black pixel: their captures match those of a run with
 * --dmcp, where DMCP5's own code draws.
 */

#define LCD_X            400
#define LCD_Y            240
#define LCD_LINE_SIZE     50
#define LCD_LINE_STRIDE   52
#define LCD_EMPTY_VALUE 0xFF

#define BLT_OR    0
#define BLT_ANDN  1
#define BLT_XOR   2
#define BLT_SET   1

#define KEY_RING_SIZE 64

#define FR_OK 0

// The two fields of the FIL that the firmware uses directly, at the offsets the structure in ff_ifc.h gives them. The one in emulated memory is where the file's
// position is kept, so the stub advances it and the host takes it back at the next call that reaches Python.
#define FIL_OBJSIZE(fp) (*(volatile uint32_t *)((uint8_t *)(fp) + 12))
#define FIL_FPTR(fp)    (*(volatile uint32_t *)((uint8_t *)(fp) + 24))

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

// Everything the stubs take from the host, written before the run. One global per field rather than one structure, so nm reports the address of each and no description
// of the layout has to be repeated on the Python side.
uint8_t *pgemu_lcd_base;                                                              // the LCD line buffers, LCD_Y lines of LCD_LINE_STRIDE bytes
uint32_t pgemu_ring_head;
uint32_t pgemu_ring_tail;
int pgemu_keys[KEY_RING_SIZE];

// A file opened for reading, put in emulated memory whole so that a read never leaves the machine. The firmware loads a state file one byte at a time, which is
// 46 423 calls for R47auto.sav, and a call answered in Python costs about ten microseconds against nothing at all here.
uint8_t *pgemu_win;                                                                   // where the file is
void *pgemu_win_fp;                                                                   // the FIL it belongs to, and zero where there is none
uint32_t pgemu_win_len;                                                               // how much of the file is there

// Emitted by stubs.py as an svc, so a read the window cannot answer reaches hostfs.py with the arguments it arrived with.
extern int pgemu_hostcall_f_read(void *fp, void *buf, unsigned int count, unsigned int *read);


/**
 * Take bytes from the window, or hand the call to the host where the window does not contain them.
 *
 * The tail call is deliberate: r0 to r3 still contain the original arguments and lr still points at the firmware, so the svc behind it returns straight there.
 */
int pgemu_f_read(void *fp, void *buf, unsigned int count, unsigned int *read) {
  if(fp != pgemu_win_fp) {
    return pgemu_hostcall_f_read(fp, buf, count, read);
  }
  const uint32_t position = FIL_FPTR(fp);
  if(position > pgemu_win_len) {
    return pgemu_hostcall_f_read(fp, buf, count, read);
  }
  uint32_t span = pgemu_win_len - position;
  if(span > count) {
    span = count;
  }
  const uint8_t *from = pgemu_win + position;
  uint8_t *to = (uint8_t *)buf;
  for(uint32_t i = 0; i < span; i++) {
    to[i] = from[i];
  }
  if(read) {
    *read = span;
  }
  FIL_FPTR(fp) = position + span;
  return FR_OK;
}

/**
 * Place up to 24 bits into one line of the LCD buffer.
 *
 * x is mirrored here for LCD_INVERT_XAXIS, so the buffer matches what the panel takes. BLT_OR clears bits and BLT_ANDN sets them, because LCD_INVERT_DATA makes a
 * set bit an unlit pixel. Byte 0 of the line is the dirty flag, which a refresh tests and clears.
 */
void pgemu_bitblt24(uint32_t x, uint32_t dx, uint32_t y, uint32_t val, int blt_op, int fill) {
  if(dx < 1 || dx > 24 || y >= LCD_Y || x >= LCD_X || x + dx > LCD_X) {
    return;
  }
  x = LCD_X - dx - x;

  const uint32_t byte_i = x >> 3;
  const uint32_t bit_off = x & 7u;
  const uint32_t lowmask = (1u << dx) - 1u;
  const uint32_t bytes_needed = (bit_off + dx + 7) / 8;

  const uint32_t srcbits = (val & lowmask) << bit_off;
  const uint32_t fillbits = (fill == BLT_SET) ? lowmask << bit_off : 0u;  // BLT_SET: the dx columns are set white before BLT_OR and black before BLT_ANDN

  uint8_t *line = pgemu_lcd_base + y * LCD_LINE_STRIDE;
  uint8_t *j = line + 2 + byte_i;
  for(uint32_t i = 0; i < bytes_needed; i++) {
    const uint8_t source = (uint8_t)(srcbits >> (8 * i));
    const uint8_t filled = (uint8_t)(fillbits >> (8 * i));
    if(blt_op == BLT_OR) {
      j[i] = (uint8_t)((j[i] | filled) & ~source);
    }
    else if(blt_op == BLT_XOR) {
      j[i] ^= source;
    }
    else if(blt_op == BLT_ANDN) {
      j[i] = (uint8_t)((j[i] & ~filled) | source);
    }
    else {
      return;
    }
  }
  line[0] = 1u;
}


void pgemu_lcd_fill_rect(uint32_t x, uint32_t y, uint32_t dx, uint32_t dy, int val) {
  if(x + dx > LCD_X || y + dy > LCD_Y) {
    return;
  }
  const int blt_op = val ? BLT_OR : BLT_ANDN;
  for(uint32_t col = x; col < x + dx; col += 24) {
    uint32_t cols = x + dx - col;
    if(cols > 24) {
      cols = 24;
    }
    for(uint32_t line = y; line < y + dy; line++) {
      pgemu_bitblt24(col, cols, line, 0xFFFFFFu, blt_op, 0);
    }
  }
}


uint8_t *pgemu_lcd_line_addr(int y) {
  uint8_t *line = pgemu_lcd_base + y * LCD_LINE_STRIDE;
  line[0] = 1u;
  return line + 2;
}


void pgemu_lcd_clear_buf(void) {
  for(uint32_t row = 0; row < LCD_Y; row++) {
    uint8_t *line = pgemu_lcd_base + row * LCD_LINE_STRIDE;
    line[0] = 1u;
    line[1] = (uint8_t)(LCD_Y - row);                       // the panel addresses its lines 1 to 240, which is what DMCP's own lcd_clear_buf writes here
    for(uint32_t c = 0; c < LCD_LINE_SIZE; c++) {
      line[2 + c] = LCD_EMPTY_VALUE;
    }
  }
}


int pgemu_key_empty(void) {
  return pgemu_ring_head == pgemu_ring_tail;
}


int pgemu_key_pop(void) {
  if(pgemu_ring_head == pgemu_ring_tail) {
    return -1;
  }
  const int key = pgemu_keys[pgemu_ring_head];
  pgemu_ring_head = (pgemu_ring_head + 1) % KEY_RING_SIZE;
  return key;
}


int pgemu_key_tail(void) {
  if(pgemu_ring_head == pgemu_ring_tail) {
    return -1;
  }
  return pgemu_keys[pgemu_ring_head];
}


void pgemu_key_pop_all(void) {
  pgemu_ring_head = pgemu_ring_tail;
}


void pgemu_noop(void) {
}


int pgemu_zero(void) {
  return 0;
}

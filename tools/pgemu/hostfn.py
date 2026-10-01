#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# Host implementations of the DMCP library table.
#
# One function here answers one entry of the table parsed by lft.py. A call arrives with the ARM procedure call standard in force, so the first four words are in
# r0 to r3 and any further ones are on the stack, and an integer result goes back in r0. Every implementation takes the emulator and returns either an integer or None,
# and None leaves r0 untouched, which is what a void function needs.
#
# An entry with no implementation returns zero and is counted in emu.missing, so a run reports exactly which part of DMCP the firmware reached and the next implementation
# to write is never a guess.

import queue
import struct
import sys
import time
from pathlib import Path

import picker
from target import LCD_X, LCD_Y, LCD_LINE_SIZE, LCD_LINE_STRIDE, LCD_EMPTY_VALUE

# What the emulator paints where the platform takes the screen for itself: vertical bars three pixels wide with three between them, on every line. A set bit is an
# unlit pixel, so the bars are the zero bits, and the value of a byte follows from its position along the line alone. Nothing a firmware screen contains looks like
# it: an empty line is LCD_EMPTY_VALUE, which no byte of this is, and a line drawn again is written across its whole width, so a byte still equal to the pattern is
# a byte the firmware never wrote. Covering every line rather than one in four is what makes a patch of any size visible, in the report and to the person watching.
PATTERN_LINE = bytes(sum(1 << bit for bit in range(8) if ((byte_i * 8 + bit) // 3) % 2) for byte_i in range(LCD_LINE_SIZE))

HANDLERS = {}

def dmcp(*names):
  """Register one implementation against one or more table entry names."""
  def register(fn):
    for name in names:
      HANDLERS[name] = fn
    return fn
  return register


class Allocator:
  """First fit allocator over one span of emulated memory.

  The firmware reaches malloc, free, calloc and realloc through the library table, because the DMCP build wraps _malloc_r, so every allocation the program makes arrives
  here and the emulator accounts for all of it. Block sizes are recorded in a Python dictionary rather than in a header in emulated memory, which keeps the arena byte for
  byte comparable with the calculator's own.
  """

  def __init__(self, base, size):
    self.base = base
    self.size = size
    self.free = [(base, size)]                                # sorted, disjoint (address, size) spans
    self.busy = {}                                            # address to size
    self.taken = 0                                            # the sum of busy, kept as it changes rather than added up on every allocation
    self.peak = 0
    self.high = base                                          # the highest address ever handed out, which is the floor a stack reading may not go below

  def malloc(self, size):
    if size <= 0:
      return 0
    size = (size + 7) & ~7
    for index, (addr, span) in enumerate(self.free):
      if span >= size:
        if span == size:
          del self.free[index]
        else:
          self.free[index] = (addr + size, span - size)
        self.busy[addr] = size
        self.taken += size
        self.high = max(self.high, addr + size)
        self.peak = max(self.peak, self.taken)
        return addr
    return 0

  def free_block(self, addr):
    if addr == 0:
      return
    size = self.busy.pop(addr, None)
    if size is None:
      raise ValueError('free of 0x%08x, which the host allocator never returned' % addr)
    self.taken -= size
    self.free.append((addr, size))
    self.free.sort()
    merged = []
    for span in self.free:
      if merged and merged[-1][0] + merged[-1][1] == span[0]:
        merged[-1] = (merged[-1][0], merged[-1][1] + span[1])
      else:
        merged.append(span)
    self.free = merged

  def largest_free(self):
    return max((span for _, span in self.free), default=0)

  def total_free(self):
    return sum(span for _, span in self.free)

  @property
  def limit(self):
    return self.base + self.size                              # the lowest address a stack may reach before the arena


class PlatformHeap:
  """The platform's own allocator, run from the platform image, as the calculator runs it.

  DMCP5 keeps its heap in a ThreadX byte pool, a 52 byte control block and a chain of blocks through the arena, each with an 8 byte header: the address of the next block,
  then 0xFFFFEEEE for a free block or the pool for one handed out. The last block points back to the first. DMCP for the DM42 has FreeRTOS heap_4, whose free blocks form
  a list from xStart, each headed by the next free block and its size, to the end block at the top of the arena. The platform does the allocating; this records each block
  it hands the firmware, which the page, the report and the pool lookup take from here as from the host's own allocator.
  """

  BLOCK_FREE = 0xFFFFEEEE

  def __init__(self, emu, base, size, control=None, start=None, limit=None):
    self.emu = emu
    self.base = base
    self.size = size
    self.control = control                                    # ThreadX: the pool's control block, where the platform keeps the free byte count and the chain
    self.start = start                                        # heap_4: xStart, the head of the free list
    self.limit = limit if limit is not None else base + size  # the lowest address a stack may reach without writing into the allocator
    self.kind = 'a ThreadX byte pool' if control is not None else 'FreeRTOS heap_4'
    self.busy = {}                                            # address to the size asked for
    self.taken = 0
    self.peak = 0
    self.high = base                                          # the highest address ever handed out, the lowest a stack reading may go

  def record(self, addr, size):
    if addr:
      self.busy[addr] = size
      self.taken += size
      self.peak = max(self.peak, self.taken)
      self.high = max(self.high, addr + size)

  def forget(self, addr):
    self.taken -= self.busy.pop(addr, 0)

  def _heap4_free(self):
    """The sizes of heap_4's free blocks, header included, following the list from xStart to the end block."""
    end = (self.base + self.size - 8) & ~7
    block, sizes = self.emu._u32(self.start), []
    while block != end and self.base <= block < end and len(sizes) < 1 << 16:
      sizes.append(self.emu._u32(block + 4))
      block = self.emu._u32(block)
    return sizes

  def total_free(self):
    if self.control is None:
      return sum(self._heap4_free())
    return self.emu._u32(self.control + 8)                     # tx_byte_pool_available

  def largest_free(self):
    if self.control is None:
      return max((size - 8 for size in self._heap4_free()), default=0)
    block = first = self.emu._u32(self.control + 16)           # tx_byte_pool_list
    largest = 0
    for _ in range(1 << 20):
      following = self.emu._u32(block)
      if self.emu._u32(block + 4) == self.BLOCK_FREE:
        largest = max(largest, following - block - 8)
      if following <= block or following == first:
        break
      block = following
    return largest


class Framebuffer:
  """The LCD line buffers, and the image that a refresh has sent to the panel.

  Two pieces of memory, and the difference between them is the reason for taking the display through the emulator at all. The buffer sits in emulated memory and the
  firmware draws into it through bitblt24 and lcd_fill_rect, exactly as on hardware. The pushed image is separate, and one line of it changes only when a refresh
  entry point sends that line. A capture is taken from the pushed image, never from the buffer, so drawing that reaches no refresh produces no picture here either.
  A native build that takes its screenshot from the buffer cannot show that class of defect.

  The blitter follows src/c47-gtk/hal/lcd.c, which is the tree's own statement of what DMCP does: x is mirrored inside the blitter for LCD_INVERT_XAXIS, BLT_OR clears
  bits and BLT_ANDN sets them because LCD_INVERT_DATA makes a set bit an unlit pixel, and every write marks its line dirty in byte 0.
  """

  BLT_OR = 0
  BLT_ANDN = 1
  BLT_XOR = 2
  BLT_SET = 1

  def __init__(self, emu, base, dirty_map=None):
    self.emu = emu
    self.base = base
    self.dirty_map = dirty_map                                # where a mapped platform keeps its one bit per line, or None when this emulator marks the line itself
    self.refreshes = 0
    self.pushes = 0
    self.cleared = 0
    self.painted = 0                                          # times the screen was taken for the platform and the firmware was left to draw it again
    self.watching = False                                     # whether a settled screen is still to be counted after one of those
    self.left_over = (0, 0, 0, 0)                             # the most of the pattern found on a settled screen, which is the worst the firmware left
    self.painted_where = ''                                   # the entry that last took the screen, for the message
    self.screen = bytearray([LCD_EMPTY_VALUE]) * (LCD_Y * LCD_LINE_SIZE)

  def line_addr(self, y):
    self.emu.uc.mem_write(self.base + y * LCD_LINE_STRIDE, b'\x01')
    return self.base + y * LCD_LINE_STRIDE + 2

  def clear(self):
    for row in range(LCD_Y):
      self.emu.uc.mem_write(self.base + row * LCD_LINE_STRIDE, bytes([1, LCD_Y - row]) + bytes([LCD_EMPTY_VALUE]) * LCD_LINE_SIZE)

  def _mark(self, row):
    """Record that a line has been drawn on, in whichever of the two places the refresh takes that from."""
    if self.dirty_map is None:
      self.emu.uc.mem_write(self.base + row * LCD_LINE_STRIDE, b'\x01')
      return
    numbered = LCD_Y - row
    at = self.dirty_map + (numbered >> 3)
    self.emu.uc.mem_write(at, bytes([self.emu.uc.mem_read(at, 1)[0] | (1 << (numbered & 7))]))

  def paint_over(self, where, redrawn=True, wait='brief'):
    """Fill the buffer with the pattern that marks a screen the platform has taken for itself, and send it.

    OFF is the one route with no firmware after it, so it paints with redrawn false and is left out of the count of what was left over.

    On hardware DMCP draws the file chooser, the disk information and the display after OFF over whatever the firmware had there, and the firmware has to draw the
    whole screen again to get it back. The emulator answers those entries without drawing anything, so a firmware that draws back too little leaves no trace. The
    bars of PATTERN_LINE go on every line, so whatever is left of them afterwards is the part never drawn again, down to the eight pixels one byte covers.
    """
    for row in range(LCD_Y):
      self.emu.uc.mem_write(self.base + row * LCD_LINE_STRIDE, bytes([1, LCD_Y - row]) + PATTERN_LINE)
      self._mark(row)
    self.painted += redrawn
    self.watching = bool(redrawn)
    self.painted_where = where
    self.push()
    self.emu.on_refresh()
    self.emu.hold_platform_screen(wait)

  def settled(self):
    """Count what is left of the pattern now the firmware waits for a key, which is the point its drawing is finished.

    A run that never reaches this point after the platform took the screen is counted at the end instead. The message names the entry, because the one that took the
    screen last is where a reader should look.
    """
    if not self.watching:
      return
    self.watching = False
    left = self.painted_left()
    self.left_over = max(self.left_over, left)
    if left[0]:
      print('%d bytes of the pattern the platform left on the screen at %s were never drawn again, on %d lines between %d and %d'
            % (left[0], self.painted_where, left[3], left[1], left[2]), file=sys.stderr, flush=True)

  def painted_left(self):
    """What is left of the pattern on the pushed screen: the bytes that still contain it, the first and last line any of them is on, and how many lines that is.

    The count is taken from the pushed image rather than from the buffer, so drawing that reaches no refresh counts as not drawn. A byte counts where it is still exactly
    what the pattern put there and the byte beside it is too, and anything above zero is that much of the display the firmware never wrote, sixteen pixels of one line
    being the least it reports. A neighbour is required because the three values the bars take are three set bits in a row, which a glyph stroke produces often enough to
    leave single bytes scattered over a screen that was drawn again in full.
    """
    rows, left = [], 0
    for row in range(LCD_Y):
      at = row * LCD_LINE_SIZE
      same = [self.screen[at + i] == PATTERN_LINE[i] for i in range(LCD_LINE_SIZE)]
      kept = sum(1 for i in range(LCD_LINE_SIZE) if same[i] and ((i > 0 and same[i - 1]) or (i + 1 < LCD_LINE_SIZE and same[i + 1])))
      if kept:
        rows.append(row)
        left += kept
    return (left, rows[0] if rows else 0, rows[-1] if rows else 0, len(rows))

  def blit(self, x, dx, y, val, blt_op, fill):
    if dx < 1 or dx > 24 or y >= LCD_Y or x >= LCD_X or x + dx > LCD_X:
      return
    x = LCD_X - dx - x                                        # LCD_INVERT_XAXIS, applied here so the buffer matches what the panel takes
    byte_i = x >> 3
    bit_off = x & 7
    lowmask = (1 << dx) - 1
    needed = (bit_off + dx + 7) // 8
    if fill == self.BLT_SET and blt_op != self.BLT_XOR:
      srcbits = (lowmask << bit_off) if blt_op == self.BLT_ANDN else 0
    else:
      srcbits = (val & lowmask) << bit_off
    line = self.base + y * LCD_LINE_STRIDE
    target = line + 2 + byte_i
    current = bytearray(self.emu.uc.mem_read(target, needed))
    for i in range(needed):
      source = (srcbits >> (8 * i)) & 0xFF
      if blt_op == self.BLT_OR:
        current[i] &= ~source & 0xFF
      elif blt_op == self.BLT_XOR:
        current[i] ^= source
      elif blt_op == self.BLT_ANDN:
        current[i] |= source
      else:
        return
    self.emu.uc.mem_write(target, bytes(current))
    self.emu.uc.mem_write(line, b'\x01')

  def fill_rect(self, x, y, dx, dy, val):
    if x + dx > LCD_X or y + dy > LCD_Y:
      return
    blt_op = self.BLT_OR if val else self.BLT_ANDN
    for col in range(x, x + dx, 24):
      cols = min(24, x + dx - col)
      for line in range(y, y + dy):
        self.blit(col, cols, line, 0xFFFFFF, blt_op, 0)

  def _drawn_on(self, row, line):
    """Whether this line has been drawn on since the last refresh, and take the mark off it.

    Two places keep that, and which one is right depends on who did the drawing. This emulator marks byte 0 of the line, as the firmware's own simulator HAL does.
    A mapped platform marks a bit per line in a bitmap of its own instead, keyed by the line's panel number, so where the platform draws that is the one to read.
    """
    if self.dirty_map is None:
      if self.emu.uc.mem_read(line, 1)[0] == 0:
        return False
      self.emu.uc.mem_write(line, b'\x00')
      return True
    numbered = LCD_Y - row                                    # the panel number written in this line, which is how the platform keys its bitmap
    at = self.dirty_map + (numbered >> 3)
    byte = self.emu.uc.mem_read(at, 1)[0]
    if not byte & (1 << (numbered & 7)):
      return False
    self.emu.uc.mem_write(at, bytes([byte & ~(1 << (numbered & 7))]))
    return True

  def push(self, first=0, count=LCD_Y, dirty_only=True):
    """Send lines to the pushed image, which is what a capture reports."""
    for row in range(first, min(first + count, LCD_Y)):
      line = self.base + row * LCD_LINE_STRIDE
      if dirty_only and not self._drawn_on(row, line):
        continue
      self.screen[row * LCD_LINE_SIZE:(row + 1) * LCD_LINE_SIZE] = self.emu.uc.mem_read(line + 2, LCD_LINE_SIZE)
      self.pushes += 1

  def capture(self, path):
    with open(path, 'wb') as out:
      out.write(self.bmp())
    return path

  def bmp(self):
    """The pushed image as a two colour BMP.

    Byte order within a line is reversed and the bit order within a byte is left alone: the blitter has already mirrored x, so data byte i of a line covers pixels
    399-8i down to 392-8i, with the most significant bit at the leftmost of those.
    """
    row_bytes = (LCD_LINE_SIZE + 3) & ~3                      # BMP pads every row to four bytes
    pixels = bytearray()
    for y in range(LCD_Y - 1, -1, -1):                        # BMP rows are bottom upwards
      line = self.screen[y * LCD_LINE_SIZE:(y + 1) * LCD_LINE_SIZE]
      pixels += bytes(reversed(line)) + bytes(row_bytes - LCD_LINE_SIZE)
    header = struct.pack('<2sIHHI', b'BM', 14 + 40 + 8 + len(pixels), 0, 0, 14 + 40 + 8)
    info = struct.pack('<IiiHHIIiiII', 40, LCD_X, LCD_Y, 1, 1, 0, len(pixels), 2835, 2835, 2, 2)
    palette = struct.pack('<II', 0x00000000, 0x00FFFFFF)      # a clear bit is a lit pixel, because LCD_INVERT_DATA is defined
    return header + info + palette + bytes(pixels)


# ----------------------------------
# Memory
# ----------------------------------

@dmcp('__sysfn_malloc')
def _malloc(emu):
  size = emu.arg(0)
  if emu.refuse_allocation(size):
    return 0
  if emu.platform_heap:
    return emu.call_heap('malloc', size)
  return emu.alloc.malloc(size)


@dmcp('__sysfn_free')
def _free(emu):
  if emu.platform_heap:
    emu.alloc.forget(emu.arg(0))
    return emu.call_heap('free')
  emu.alloc.free_block(emu.arg(0))
  return None


@dmcp('__sysfn_calloc')
def _calloc(emu):
  size = emu.arg(0) * emu.arg(1)
  if emu.refuse_allocation(size):
    return 0
  if emu.platform_heap:
    return emu.call_heap('calloc', size)
  addr = emu.alloc.malloc(size)
  if addr:
    emu.uc.mem_write(addr, bytes(size))
  return addr


@dmcp('__sysfn_realloc')
def _realloc(emu):
  old, size = emu.arg(0), emu.arg(1)
  if size and emu.refuse_allocation(size):
    return 0                                                  # a refused realloc leaves the old block where it was, as C's does
  if emu.platform_heap:
    return emu.call_heap('realloc', size, old)
  addr = emu.alloc.malloc(size)
  if addr and old:
    keep = min(size, emu.alloc.busy.get(old, 0))
    emu.uc.mem_write(addr, bytes(emu.uc.mem_read(old, keep)))
    emu.alloc.free_block(old)
  return addr


@dmcp('sys_reset')
def _sys_reset(emu):
  """Restart the calculator, which ends the run. Nothing follows it on the screen, so the screen at that moment is written as sys_reset.bmp."""
  if emu.capturing:
    emu.capture('sys_reset')
  emu.stop('the firmware called sys_reset, which restarts the calculator')
  return None


@dmcp('sys_free_mem')
def _sys_free_mem(emu):
  return emu.alloc.total_free()


@dmcp('sys_largest_free_mem')
def _sys_largest_free_mem(emu):
  return emu.alloc.largest_free()


# ----------------------------------
# Console
# ----------------------------------

@dmcp('__sysfn__write')
def _write(emu):
  count = emu.arg(2)
  emu.console += bytes(emu.uc.mem_read(emu.arg(1), count)) if count else b''
  return count


# ----------------------------------
# LCD
# ----------------------------------

@dmcp('lcd_line_addr')
def _lcd_line_addr(emu):
  return emu.lcd.line_addr(emu.arg(0))


@dmcp('lcd_clear_buf', 'LCD_clear')
def _lcd_clear_buf(emu):
  emu.lcd.clear()
  emu.lcd.cleared = 1
  return None


@dmcp('lcd_set_buf_cleared')
def _lcd_set_buf_cleared(emu):
  emu.lcd.cleared = emu.arg(0)
  return None


@dmcp('lcd_get_buf_cleared')
def _lcd_get_buf_cleared(emu):
  return emu.lcd.cleared


@dmcp('bitblt24')
def _bitblt24(emu):
  emu.lcd.blit(emu.arg(0), emu.arg(1), emu.arg(2), emu.arg(3), emu.arg(4), emu.arg(5))
  return None


@dmcp('lcd_fill_rect')
def _lcd_fill_rect(emu):
  emu.lcd.fill_rect(emu.arg(0), emu.arg(1), emu.arg(2), emu.arg(3), emu.arg(4))
  return None


@dmcp('LCD_write_line')
def _lcd_write_line(emu):
  """Send one line the firmware hands over itself.

  Byte 1 is the line's address on the panel, not an index into the buffer. DMCP's own LCD_write_line takes nothing from it: it raises a chip select and sends all
  54 bytes over SPI, so the number is part of the panel's wire protocol, and the panel counts its lines 1 to 240. DMCP's lcd_clear_buf writes LCD_Y less the buffer
  index there, which this emulator now does too, so the row a written line lands on is LCD_Y less the number. Taking the byte as the row instead puts anything sent
  this way upside down, and only this way, because a refresh steps through the buffer by index and never uses that byte.
  """
  addr = emu.arg(0)
  numbered = emu.uc.mem_read(addr + 1, 1)[0]
  if 1 <= numbered <= LCD_Y:
    row = LCD_Y - numbered
    emu.lcd.screen[row * LCD_LINE_SIZE:(row + 1) * LCD_LINE_SIZE] = emu.uc.mem_read(addr + 2, LCD_LINE_SIZE)
    emu.lcd.pushes += 1
  return None


@dmcp('lcd_refresh', 'lcd_forced_refresh', 'lcd_refresh_dma')
def _lcd_refresh(emu):
  """Send the lines that were drawn on since the last refresh.

  The platform keeps that set as a bitmap of one bit per line, thirty bytes of it right after the line buffers, set by its drawing routines and tested and cleared
  by the refresh. This emulator marks byte 0 of the line instead, which is what the firmware's own simulator HAL in src/c47-gtk/hal/lcd.c does, and the two agree
  on which lines get sent. Byte 0 is not that flag to the platform: its sender writes 1 there as the panel's command byte immediately before the line goes out.
  """
  emu.lcd.refreshes += 1
  emu.lcd.push()
  emu.on_refresh()
  return None


@dmcp('lcd_refresh_lines')
def _lcd_refresh_lines(emu):
  emu.lcd.refreshes += 1
  emu.lcd.push(first=emu.arg(0), count=emu.arg(1), dirty_only=False)
  emu.on_refresh()
  return None


@dmcp('lcd_refresh_wait', 'LCD_power_on', 'LCD_power_off')
def _lcd_noop(emu):
  return None


@dmcp('draw_power_off_image')
def _draw_power_off_image(emu):
  """DMCP puts its own image on the panel at power off. The buffer is taken as it is, so the capture shows what the firmware left there."""
  emu.lcd.push(dirty_only=False)
  emu.on_refresh()
  return None


# ----------------------------------
# Keyboard
# ----------------------------------

@dmcp('key_pop', 'sys_last_key', 'key_tail', 'sys_last_scan', 'wait_for_key_press', 'runner_get_key_delay')
def _key_pop(emu):
  return emu.key_take()


@dmcp('runner_get_key')
def _runner_get_key(emu):
  return emu.wait_key()


@dmcp('key_empty')
def _key_empty(emu):
  if emu.serve is not None:
    emu.take_page_keys()                                      # a served run asks here while it computes, see cross_on_key_empty
  return 0 if emu.keys_pending() else 1


@dmcp('key_push', 'key_pop_all', 'wait_for_key_release', 'reset_auto_off', 'runner_key_tout_init')
def _key_noop(emu):
  return None


@dmcp('sys_timer_disable', 'sys_critical_start', 'sys_critical_end', 'sys_delay')
def _sys_noop(emu):
  return None


@dmcp('sys_timer_start')
def _sys_timer_start(emu):
  """Take the wake the firmware arms, which is how long the sleep that follows it lasts.

  src/c47/c47.c arms TIMER_IDX_REFRESH_SLEEP immediately before every sys_sleep, for SCREEN_REFRESH_PERIOD or the shortest of its own timeouts, whichever is less.
  The second argument is that figure in milliseconds.
  """
  emu.sleep_ms = emu.arg(1)
  return None


@dmcp('sys_sleep')
def _sys_sleep(emu):
  """Wait the sleep the firmware armed, and end the run where it waits for a key and none is left.

  The wait is where the emulated clock and the host clock part. On hardware the processor stops until the timer or a key wakes it; here a whole sleep takes microseconds,
  so a run that waits through a hundred rounds advances the clock by a millisecond or two where the calculator would have advanced it by seconds, and every timeout the
  firmware has armed against that clock goes off late or not at all. A softkey is the clearest case: with FLAG_G_DOUBLETAP set the release does not execute the item, it
  starts TO_FN_EXEC for the 150 ms of TIME_FN_DOUBLE_RELEASE in src/c47/c47Extensions/keyboardTweak.c and execFnTimeout() runs it when that expires, so a scripted F1 to
  F6 was taken and then dropped. What the host really spent in here counts towards the sleep and the remainder is added to the clock, which leaves a served run, where a
  person really does take seconds, measuring the time that person took.
  """
  began = time.monotonic()
  answer = _waited(emu)
  emu.idle_for(emu.sleep_ms - (time.monotonic() - began) * 1000)
  return answer


def _waited(emu):
  """Wait for a key, and report whether the run continues.

  On hardware a key or a timer wakes the processor. A headless run has nothing to wake it with, so the run would otherwise spend its whole instruction budget in
  the idle iteration. Stopping here ends a run at the point the screen is settled, which is also where a capture is worth taking.
  """
  emu.lcd.settled()                                           # the firmware has finished drawing by the time it waits here
  if emu.program_ending():
    emu.lcd.paint_over('OFF', redrawn=False)                  # OFF hands the screen to the platform, which puts its display up and no firmware follows it
    emu.stop('the firmware set STAT_PGM_END, which is what OFF does')
    return None
  if emu.serve is None or emu.script_left():                  # a served run takes its script first, as a run without a page does
    if emu.keys_pending() or emu.advance_script():
      return None
  if emu.serve is not None:
    emu.serve_idle()                                          # a served run waits here for a key from the page instead of ending
    return None
  if emu.idle_budget > 0:
    emu.idle_budget -= 1
    return None
  emu.stop('the firmware reached sys_sleep with no key left to take')
  return None


@dmcp('sys_auto_off_cnt', 'is_menu_auto_off')
def _auto_off(emu):
  return 0


# ----------------------------------
# Time
# ----------------------------------

@dmcp('sys_current_ms', 'sys_tick_count', 'get_rtc_ticks')
def _sys_current_ms(emu):
  """Milliseconds since the run started.

  Everything the firmware times arrives here: TICKS# and the stopwatch through getUptimeMs in src/c47/timer.c, and every timeout beside them. So what comes back has
  to advance with the time a person waits. A count of the calls does not, because a whole start makes six of them, which is why tools/bench measured nothing and the
  stopwatch moved in whole seconds. The sleeps the run did not really wait out are added on top, which sys_sleep accounts for. --ms-per-tick puts the counted clock back
  where a run has to repeat exactly, and a stopwatch means nothing under it.
  """
  if emu.ms_per_tick:
    emu.ms_calls += 1
    return (emu.ms_calls // emu.ms_per_tick) & 0xFFFFFFFF
  return int((time.monotonic() - emu.started) * 1000 + emu.idle_ms) & 0xFFFFFFFF


@dmcp('rtc_read')
def _rtc_read(emu):
  """Fill the platform's time and date structures from the host clock.

  The fourth byte of tm_t is csec, the hundredths, and _currentTime in src/c47/timer.c builds the whole stopwatch reading out of this one call. A zero there is a
  stopwatch that moves in whole seconds however often it is read.
  """
  now = emu.calendar_time()
  parts = time.localtime(now)
  emu.uc.mem_write(emu.arg(0), struct.pack('<BBBBB', parts.tm_hour, parts.tm_min, parts.tm_sec, int(now % 1 * 100), parts.tm_wday))
  emu.uc.mem_write(emu.arg(1), struct.pack('<HBB', parts.tm_year, parts.tm_mon, parts.tm_mday))
  return None


@dmcp('rtc_read_min')
def _rtc_read_min(emu):
  return time.localtime(emu.calendar_time()).tm_min


@dmcp('rtc_read_sec')
def _rtc_read_sec(emu):
  return time.localtime(emu.calendar_time()).tm_sec


# ----------------------------------
# File system
# ----------------------------------
# Answered against a directory on the host by hostfs.py. The FIL the firmware names is a plain address, so a file opened on the frame works the same as the ppgm_fp
# from the system data block.

@dmcp('f_open')
def _f_open(emu):
  return emu.fs.f_open(emu.arg(0), emu.arg(1), emu.arg(2))


@dmcp('f_close')
def _f_close(emu):
  return emu.fs.f_close(emu.arg(0))


@dmcp('f_read')
def _f_read(emu):
  return emu.fs.f_read(emu.arg(0), emu.arg(1), emu.arg(2), emu.arg(3))


@dmcp('f_write')
def _f_write(emu):
  return emu.fs.f_write(emu.arg(0), emu.arg(1), emu.arg(2), emu.arg(3))


@dmcp('f_lseek')
def _f_lseek(emu):
  return emu.fs.f_lseek(emu.arg(0), emu.arg(1))


@dmcp('f_unlink')
def _f_unlink(emu):
  return emu.fs.f_unlink(emu.arg(0))


@dmcp('f_rename')
def _f_rename(emu):
  return emu.fs.f_rename(emu.arg(0), emu.arg(1))


@dmcp('file_size')
def _file_size(emu):
  return emu.fs.file_size(emu.arg(0)) & 0xFFFFFFFF


@dmcp('check_create_dir')
def _check_create_dir(emu):
  return emu.fs.check_create_dir(emu.arg(0))


@dmcp('make_date_filename')
def _make_date_filename(emu):
  return emu.fs.make_date_filename(emu.arg(0), emu.arg(1), emu.arg(2))


@dmcp('sys_disk_write_enable')
def _sys_disk_write_enable(emu):
  """DMCP takes the disk away from the USB host while the firmware writes. Nothing arbitrates for it here, so the state is kept and reported and nothing else."""
  emu.fs.write_enabled = emu.arg(0)
  return 0


@dmcp('sys_is_disk_write_enable')
def _sys_is_disk_write_enable(emu):
  return emu.fs.write_enabled


@dmcp('run_menu_item_sys')
def _run_menu_item_sys(emu):
  """Take the screen the way a platform menu item does, and report that nothing was chosen.

  activateUSBdisk and the DMCP menu item in src/c47/config.c both reach the platform through this entry, and both draw the screen again through clearScreen after
  it returns.
  """
  emu.lcd.paint_over('a platform menu item', wait='key')
  return 0


@dmcp('disp_disk_info')
def _disp_disk_info(emu):
  """Take the screen the way the platform's disk information page does, and give it back with nothing read."""
  emu.lcd.paint_over('the disk information page', wait='key')
  return None


PAGE_CHOOSER_WAIT = 600                                       # seconds the page is given to answer, so a browser that was closed cannot stall a run for ever
MRET_EXIT = -2                                                # dmcp.h, and the only value _file_selection_helper in src/c47-dmcp5/hal/io.c tests for
SELECTION_PATH = 256                                          # where the name starts in the text a selection function is handed, after the path


@dmcp('file_selection_screen')
def _file_selection_screen(emu):
  """Put the list up, and hand what is picked to the firmware's own selection function, which is what DMCP does.

  The value the function returns selects what follows: zero puts the list back, MRET_EXIT leaves it, and anything else ends the entry with that value. The functions
  in src/c47-dmcp5/hal/io.c are not all alike. load_programfile and save_datafile copy the path into the buffer handed to them as data and return, but load_statefile
  displays a warning that the current state will be lost and waits for ENTER or EXIT before it copies anything, and both state functions record the file for DMCP to load
  after a reset. So the function is run in the emulated machine rather than its effect written in its place. The arguments are kept here, because the function leaves r0
  to r3 changed by the time its result comes back.
  """
  emu.selection = {
    'title': emu.fs.cstring(emu.arg(0)),
    'named': emu.fs.cstring(emu.arg(1)),
    'extension': emu.fs.cstring(emu.arg(2)),
    'function': emu.arg(3),
    'saving': bool(emu.arg(4)),
    'data': emu.arg(6),
    'caller': emu.lr(),
  }
  emu.selection['suggested'] = _suggestion(emu.fs.cstring(emu.arg(6)), emu.selection['extension']) if emu.selection['saving'] else ''
  return _offer_selection(emu)


def _offer_selection(emu):
  """Put the list up and hand what is picked to the selection function, or end the entry with MRET_EXIT where nothing is."""
  emu.lcd.paint_over('the file chooser', wait='none' if emu.pick else 'brief')   # a chooser of its own is the pause, so none is added in front of it
  picked = emu.chosen_now() or _from_picker(emu)
  if picked is None:
    return emu.return_to(emu.selection['caller'], MRET_EXIT)
  emu.fs.touched[(picked, 'chosen at the file selection screen')] = 1
  name = picked.replace('/', '\\').rsplit('\\', 1)[-1]
  emu.uc.mem_write(emu.selection_text, picked.encode('utf-8') + b'\0')
  emu.uc.mem_write(emu.selection_text + SELECTION_PATH, name.encode('utf-8') + b'\0')
  emu.call_selection(emu.selection['function'], emu.selection_text, emu.selection_text + SELECTION_PATH, emu.selection['data'])
  return None


@dmcp('pgemu_heap_done')
def _heap_done(emu):
  """Record what the platform's allocator returned, which arrives here through the heap shim, and go back to the firmware's caller with it."""
  result = emu.arg(0)
  kind, size, old, caller = emu.heap_call
  emu.heap_call = None
  if kind == 'realloc' and (result or not size):
    emu.alloc.forget(old)                                     # moved, or freed by a zero size
  if kind != 'free':
    emu.alloc.record(result, size)
  return emu.return_to(caller, result)


@dmcp('pgemu_selection_done')
def _selection_done(emu):
  """Act on what the selection function returned, which arrives here through the shim.

  Zero asks for the list again. A run given its file with --choose has nobody to pick another, so there the list is left instead, which is what a person at the calculator
  does next.
  """
  result = struct.unpack('<i', struct.pack('<I', emu.arg(0)))[0]
  if result == 0:
    if emu.chosen_now():
      return emu.return_to(emu.selection['caller'], MRET_EXIT)
    return _offer_selection(emu)
  return emu.return_to(emu.selection['caller'], result)


def _suggestion(buffered, extension):
  """A name to offer for a save, out of whatever the firmware left in the buffer.

  What is there is the last path used, directories and all, and a chooser takes a name rather than a path. The last component of it without its extension is the
  part worth offering, and where there is nothing usable the name is untitled.
  """
  name = buffered.replace('\\', '/').rsplit('/', 1)[-1].rsplit('.', 1)[0]
  return (name or 'untitled') + extension


def _from_page(emu, title, named, folder, extension, saving, suggested):
  """Ask the page which file, and wait inside the entry the firmware called until it answers.

  Only the files the firmware asked for are offered, because the calculator can name nothing outside its own root, so the question the page puts is narrower than
  any chooser the desktop offers and needs no part of one. Saving offers the same list, where choosing a file names it and the write goes over that one, which is
  how the calculator does it.
  """
  files = sorted(p for p in folder.iterdir() if p.is_file() and p.suffix.lower() == extension.lower()) if folder.is_dir() else []
  emu.serve.ask({'title': title, 'folder': named or '\\', 'ext': extension, 'saving': saving, 'suggested': suggested,
                 'files': [{'name': p.name, 'size': p.stat().st_size} for p in files]})
  try:
    name = emu.serve.answer.get(timeout=PAGE_CHOOSER_WAIT)
  except queue.Empty:
    name = ''
  finally:
    emu.serve.ask(None)
  if not name.strip():
    return None
  name = name.strip()
  if saving and not name.lower().endswith(extension.lower()):
    name += extension
  return ('%s\\%s' % (named.rstrip('\\'), name)) if named else name


def _from_picker(emu):
  """Put the firmware's own question to the host's file chooser, and give back the answer as calculator path text.

  The arguments are the ones DMCP declares, as the entry kept them: the title to display, the folder to start in, the extension, and the flag that distinguishes naming a
  file to write from picking one to read. A file outside the mapped root is refused, because the calculator has no way to name it, and a name given without the extension
  for a save takes the one the firmware asked for, which is what src/c47-gtk/hal/io.c does with the same answer.
  """
  if not emu.pick:
    return None
  asked = emu.selection
  title, named, extension, saving, suggested = asked['title'], asked['named'], asked['extension'], asked['saving'], asked['suggested']
  folder = emu.fs.resolve(named)
  folder = folder if folder and folder.is_dir() else emu.fs.root
  if emu.serve is not None:
    return _from_page(emu, title, named, folder, extension, saving, suggested)
  chosen = picker.choose(title, str(folder), extension, saving, suggested)
  if chosen is None:
    return None
  if saving and not chosen.lower().endswith(extension.lower()):
    chosen += extension
  try:
    inside = Path(chosen).resolve().relative_to(emu.fs.root.resolve())
  except ValueError:
    print('%s is outside %s, which is the whole of the file system the calculator can name. Copy it in, or map that directory with --fs.'
          % (chosen, emu.fs.root), file=sys.stderr, flush=True)
    return None
  return str(inside).replace('/', '\\')


@dmcp('create_screenshot')
def _create_screenshot(emu):
  """Write the sent image into the host directory under the dated name DMCP gives it."""
  if emu.fs.read_only:
    return 0
  directory = emu.fs.root / 'DATA'
  directory.mkdir(parents=True, exist_ok=True)
  name = time.strftime('%Y%m%d-%H%M%S00.bmp')
  emu.lcd.capture(directory / name)
  emu.fs.touched[('DATA/' + name, 'screen written')] = 1
  return 1


# ----------------------------------
# Power, sound and identity
# ----------------------------------

@dmcp('get_vbat', 'read_power_voltage')
def _get_vbat(emu):
  return 3000                                                 # millivolts, above the BAT_MINIMUM of 2100 in defines.h


@dmcp('get_lowbat_state', 'usb_powered', 'get_beep_volume')
def _zero(emu):
  return 0


# The two scratch buffers DMCP owns on hardware. C47 takes tmpString from the first and errorMessage from the second in config.c, so both are live for the whole run
# and the emulator provides them once at the sizes dmcp.h states: AUX_BUF_SIZE is 5*512 and the write buffer is 4096.
AUX_BUF_SIZE = 5 * 512
WRITE_BUF_SIZE = 4096


@dmcp('aux_buf_ptr')
def _aux_buf_ptr(emu):
  return emu.aux_buf


@dmcp('write_buf_ptr')
def _write_buf_ptr(emu):
  return emu.write_buf


@dmcp('write_buf_size')
def _write_buf_size(emu):
  return WRITE_BUF_SIZE


@dmcp('sys_write_buf_used', 'sys_clear_write_buf_used')
def _write_buf_used(emu):
  return 0


@dmcp('get_hw_id')
def _get_hw_id(emu):
  return emu.target.hw_id

#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# Memory maps for the two DMCP targets.
#
# Every address below comes from a file in the tree, never from a datasheet. The flash origin and the RAM regions are the MEMORY block of the matching linker script,
# src/c47-dmcp*/stm32_program.ld, the library table base is LIBRARY_FN_BASE in the matching dep/DMCP*_SDK/dmcp/lft_ifc.h, and the sdb address is the "#define sdb" in the
# same SDK. Change a linker script and this file has to change with it.

import re
from pathlib import Path
from collections import namedtuple

# One mapped span of the emulated address space. Permissions are informational: the emulator maps everything readable and writable so that a wild store is caught by
# the region check in pgemu.py with a name attached, instead of faulting inside Unicorn with an address and nothing else.
Region = namedtuple('Region', 'name addr size')

# LCD geometry. LCD_X, LCD_Y and LCD_LINE_SIZE come from dmcp.h. The stride between two lines is 52 and not the LCD_LINE_BUF_SIZE of 54 that dmcp.h defines: the two
# trailing bytes are transmit padding and are not part of the buffer, which src/c47/screen.c states at the memcpy in the band refresh and src/c47-gtk/hal/lcd.c uses
# throughout. A line is one dirty flag byte, one line number byte and fifty data bytes, and lcd_line_addr returns the address of the data.
LCD_X = 400
LCD_Y = 240
LCD_LINE_SIZE = 50
LCD_LINE_STRIDE = 2 + LCD_LINE_SIZE
LCD_EMPTY_VALUE = 0xFF                                        # LCD_INVERT_DATA is defined, so a cleared buffer is all ones

# One address the emulator invents, outside every region the firmware uses. The library table needs no invented address because it is code in place, written over
# the mapped DMCP region at LIBRARY_FN_BASE.
SCRATCH_BASE = 0x0F100000                                     # structures DMCP owns on hardware and the emulator provides here: the FIL, the disp_stat_t set, the timer counters
SCRATCH_SIZE = 0x8000

# Where a file opened for reading is put, so that the f_read stub takes bytes from it without leaving the emulated machine. One megabyte covers every file the firmware
# opens for reading: the largest is a state file, and R47auto.sav is 46 kB.
WINDOW_BASE = 0x0F200000
WINDOW_SIZE = 1024 * 1024


class Target:
  def __init__(self, name, cpu_model, sdk_dir, image_suffix, flash_origin, sdb_addr, stack_top, heap, hw_id, regions, platform=None):
    self.name = name
    self.cpu_model = cpu_model
    self.sdk_dir = sdk_dir
    self.image_suffix = image_suffix
    self.flash_origin = flash_origin
    self.sdb_addr = sdb_addr
    self.stack_top = stack_top
    self.heap = heap                                          # (address, size) of the span the host allocator hands out
    self.hw_id = hw_id                                        # what get_hw_id returns, matching the HWM_ values in src/c47/defines.h
    self.regions = regions
    self.platform = platform                                  # a glob for the platform image kept in a checkout, taken when none is given

  def find_sdk(self, image):
    """Take the SDK from the checkout the image was built in, the first folder above it with one in it, or from the working folder."""
    for folder in list(Path(image).resolve().parents) + [Path.cwd()]:
      if (folder / self.sdk_dir / 'dmcp' / 'dmcp.h').is_file():
        self.sdk_dir = str(folder / self.sdk_dir)
        return

  def find_platform(self, image):
    """The platform image kept in the checkout of the image, the newest one matching the target's glob, or None where there is none."""
    if self.platform is None:
      return None
    for folder in list(Path(image).resolve().parents) + [Path.cwd()]:
      found = sorted(folder.glob(self.platform))
      if found:
        return found[-1]
    return None

  def lft_header(self):
    return '%s/dmcp/lft_ifc.h' % self.sdk_dir

  def dmcp_header(self):
    return '%s/dmcp/dmcp.h' % self.sdk_dir

  def interface(self):
    """The platform interface the SDK in the tree was written for, as the pair PLATFORM_IFC_CNR and PLATFORM_IFC_VER in its dmcp.h."""
    with open(self.dmcp_header(), encoding='utf-8', errors='replace') as header:
      text = header.read()
    found = []
    for name in ('PLATFORM_IFC_CNR', 'PLATFORM_IFC_VER'):
      match = re.search(r'^#define\s+%s\s+(\d+)' % name, text, re.M)
      if match is None:
        raise ValueError('%s defines no %s' % (self.dmcp_header(), name))
      found.append(int(match.group(1)))
    return tuple(found)

  def sdb_layout(self):
    """The offset of every member of sys_sdb_t, and the size of the whole, laid out from the struct in the SDK's dmcp.h.

    Every member there is a pointer or a uint32_t, four bytes each on this ABI with nothing to align, so a member's offset is its position times four. A member of
    any other type is refused by name rather than sized by a guess, because a wrong offset puts the display states and the file pointer into the wrong fields and
    nothing in a run shows it.
    """
    with open(self.dmcp_header(), encoding='utf-8', errors='replace') as header:
      text = header.read()
    match = re.search(r'typedef struct \{([^{}]*)\}\s*sys_sdb_t;', text)
    if match is None:
      raise ValueError('%s declares no sys_sdb_t' % self.dmcp_header())
    body = re.sub(r'//[^\n]*|/\*.*?\*/', '', match.group(1), flags=re.S)
    offsets = {}
    for position, declaration in enumerate(d.strip() for d in body.split(';') if d.strip()):
      words = declaration.replace('*', ' * ').split()
      plain = [w for w in words if w not in ('volatile', 'const')]
      if '[' in declaration or not ('*' in plain or plain[:1] in (['uint32_t'], ['int32_t'])):
        raise ValueError('sys_sdb_t in %s has a member the emulator cannot size: %s' % (self.dmcp_header(), declaration))
      offsets[plain[-1]] = 4 * position
    return offsets, 4 * len(offsets)


# STM32U575, the R47 target. Regions from src/c47-dmcp5/stm32_program.ld, which widens the program flash to 1408K, the whole 2048K less the 640K DMCP5 occupies. Both
# figures are read out of the platform image in res/combo, because src/c47-dmcp5/stm32_program.ld places only the program's own data, in SRAM4, and leaves the stack and
# the arena to DMCP. Its vector table starts the stack at 0x20040000, and the call at 0x080212c6 creates the heap at 0x20040000 with 0x70000 bytes. So the two meet back
# to back at the foot of SRAM3: the stack grows down through SRAM2 and the arena grows up through SRAM3, and neither shares with the other.
DMCP5 = Target(
  name         = 'dmcp5',
  cpu_model    = 'cortex-m33',
  sdk_dir      = 'dep/DMCP5_SDK',
  image_suffix = '.pg5',
  flash_origin = 0x080A0000,
  sdb_addr     = 0x20000000,
  stack_top    = 0x20040000,
  heap         = (0x20040000, 448 * 1024),
  platform     = 'res/combo/DMCP5_flash_*.bin',
  hw_id        = 3,
  regions      = [
    Region('DMCP5',   0x08000000, 640 * 1024),
    Region('FLASH',   0x080A0000, 1408 * 1024),
    Region('SRAM1',   0x20000000, 192 * 1024),
    Region('SRAM2',   0x20030000, 64 * 1024),
    Region('SRAM3',   0x20040000, 512 * 1024),
    Region('SRAM4',   0x28000000, 16 * 1024),
    Region('QSPI',    0x90000000, 2048 * 1024),
  ],
)

# STM32L476, the DM42 target. Regions from src/c47-dmcp/stm32_program.ld. The system data block is at 0x10002000, which dep/DMCP_SDK/dmcp/dmcp.h states and the ASSERT
# at the end of that linker script enforces from the other side: the program's static data starts at 0x10000000 and must not reach the block.
#
# The stack and the arena come from DMCP_flash_3.29.bin, because that script places neither: the vector table starts the stack at 0x20017ff0 and the code at 0x08018c88
# gives the arena 0x16000 bytes from 0x20000048. Both sit in SRAM1, so the stack grows down toward the allocator's own globals at 0x2001604c, which leaves 8104 bytes to
# the end of the arena and a little less to those globals, and an overrun writes into what the allocator has handed out.
DMCP = Target(
  name         = 'dmcp',
  cpu_model    = 'cortex-m4',
  sdk_dir      = 'dep/DMCP_SDK',
  image_suffix = '.pgm',
  flash_origin = 0x08050000,
  sdb_addr     = 0x10002000,
  stack_top    = 0x20017ff0,
  heap         = (0x20000048, 88 * 1024),
  hw_id        = 1,
  regions      = [
    Region('DMCP',    0x08000000, 320 * 1024),
    Region('FLASH',   0x08050000, 704 * 1024),
    Region('SRAM2',   0x10000000, 32 * 1024),
    Region('SRAM1',   0x20000000, 96 * 1024),
    Region('QSPI',    0x90000000, 2048 * 1024),
  ],
)

BY_NAME = {t.name: t for t in (DMCP5, DMCP)}


def for_image(path):
  """Return the target whose image suffix matches the given file name."""
  for target in BY_NAME.values():
    if str(path).endswith(target.image_suffix):
      return target
  raise ValueError('no target matches %s, expected one of %s' % (path, [t.image_suffix for t in BY_NAME.values()]))

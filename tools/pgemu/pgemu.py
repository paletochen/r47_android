#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# Run a built .pg5 or .pgm under an emulated Cortex-M, with the DMCP library table answered on the host.
#
# The firmware never touches a peripheral register: a search of src/c47, src/c47-dmcp and src/c47-dmcp5 for __disable_irq, SCB->, NVIC_, SysTick and 0xE000E returns
# nothing, so the whole hardware interface is the 195 function pointers at LIBRARY_FN_BASE. That makes the emulator a CPU, a memory map and one trap table, with no timer,
# interrupt or DMA model anywhere in it.
#
# The library table is code, not pointers. The macro in lft_ifc.h is (*(typeof(fn)*)(LIBRARY_FN_BASE+offset)), and dereferencing a function pointer type yields the
# function itself, so no load takes place: a call branches straight to LIBRARY_FN_BASE+offset, and the odd base has the Thumb bit set. Each entry is therefore four
# bytes of instruction, which is what a b.w to the real implementation takes on hardware.
#
# The trap follows from that. Four bytes of svc followed by bx lr are written at every entry: the svc reaches an interrupt hook, which runs the host implementation
# from hostfn.py, and the bx lr then returns to the caller. The entry is taken from the program counter, since every entry is at a fixed address.
#
# The mechanism is chosen for its cost, measured on this machine with a two instruction branch to itself: no hook 488 M instructions/s, an interrupt hook 489 M/s, and a
# code hook over a range the branch never enters 26 M/s. Registering any code hook takes the fast block chaining away from every block in the image, so a code hook over
# the table would cost a factor of nineteen for a trap taken a few million times in a whole run.
#
# Usage:
#   pip install unicorn
#   python3 tools/pgemu/pgemu.py build.dmcp5/src/c47-dmcp5/R47.pg5

import argparse
import bisect
import hashlib
import os
import queue
import signal
import struct
import subprocess
import sys
import threading
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent                      # fs and out sit beside the script, so a copy outside the tree keeps its own
sys.path.insert(0, str(HERE))

import elfsym
import hostfn
import hostfs
import keyscript
import lft
import profiler
import serve as servelib
import stubs as stublib
import target as targets
from target import LCD_Y, LCD_LINE_SIZE, LCD_LINE_STRIDE, SCRATCH_BASE, SCRATCH_SIZE, WINDOW_BASE, WINDOW_SIZE

from unicorn import Uc, UcError, UC_ARCH_ARM, UC_MODE_THUMB, UC_MODE_MCLASS, UC_HOOK_CODE, UC_HOOK_INTR, UC_HOOK_MEM_INVALID, UC_HOOK_MEM_WRITE, UC_PROT_ALL
from unicorn.arm_const import UC_CPU_ARM_CORTEX_M4, UC_CPU_ARM_CORTEX_M33
from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC

CPU_MODELS = {'cortex-m4': UC_CPU_ARM_CORTEX_M4, 'cortex-m33': UC_CPU_ARM_CORTEX_M33}

PROG_INFO_MAGIC = 0xD377C0DE
PROG_INFO_FORMAT = '<7I16s16sI'                               # prog_info_t in dmcp.h: magic, size, entry, ifc_cnr, ifc_ver, qspi_size, qspi_crc, name, version, keymap id
PLATFORM_SCREEN_MS = 400                                      # how long a screen the platform takes and gives straight back stays up, against a page that asks every 120 ms
PAGE_IDLE_MS = 200                                            # the longest a served run waits in one sleep, so a firmware that arms a wake seconds away leaves the page its frames
PLATFORM_SCREEN_WAIT = 60                                     # seconds a screen that waits for a key stays up, so a page nobody is at cannot stall a run for ever
INTERRUPT_GRACE = 1.0                                         # seconds a ctrl-c gives the run to stop and report before the process ends without a report
DEFAULT_BUDGET = 200_000_000                                  # what a run that ends by itself is allowed, where a served run is driven by hand and is not held to it
RETURN_SENTINEL = 0x0FF00000                                  # left unmapped, so a return from program_main, which Program_Entry never takes, is reported as a fetch outside every region

# The members of sys_sdb_t the emulator fills or takes from, laid out by Target.sdb_layout from the struct in the SDK's own dmcp.h, so a member added to it moves them.
SDB_MEMBERS = ('calc_state', 'ppgm_fp', 'pds_t20', 'pds_t24', 'pds_fReg', 'timer2_counter', 'timer3_counter')

# The stack reading. A span below the entry frame is written with this value and the deepest word that no longer contains it is how far the run went, which is the same
# method as the firmware's own STACK_WATERMARK in src/c47/memory.c and needs neither an instrumented build nor a calculator. The value is deliberately not that one's:
# where a build has the firmware's marking too, the two write over one stretch, and sharing a value would let each take the other's marking for ground nobody had touched.
# On a DM42 that hid 2360 bytes of a PLOT from this reading.
STACK_PATTERN = 0x5A3C5A3C
FIRMWARE_PATTERN = 0xA5C3A5C3                                 # what src/c47/memory.c writes where a build has STACK_WATERMARK, which spoils the reading above

STAT_PGM_END = 1 << 9                                         # calc_state bit the firmware sets to ask DMCP to end the program, from dmcp.h

KEY_RING_SIZE = 64                                            # the ring in stubs.c, which the host fills before a run

FIL_SIZE = 1024                                               # the FIL in ff_ifc.h is about 576 bytes, rounded up
DISP_STAT_SIZE = 64                                           # disp_stat_t in dmcp.h is 28 bytes, rounded up
POOL_BLOCK = 4                                                # BYTES_PER_BLOCK in src/c47/defines.h, the unit of C47's own block pool
MAX_POOL_REGIONS = 4096                                       # more free regions than this is a pointer read before the pool was set up, not a list
SYSTEM_CONTROL = 0xE000E000                                   # the Cortex-M system control space; FreeRTOS loads ICSR from it on entering a critical section
OVERFLOW_WATCH = 4096                                         # bytes below the allocator's variables a write watch covers, where the platform's heap meets the stack
KEY_EXIT = 33                                                 # dmcp.h
SELECTION_TEXT = 320                                          # the path and the name a selection function is handed, one after the other


class ProgInfo:
  def __init__(self, blob):
    fields = struct.unpack_from(PROG_INFO_FORMAT, blob, 0)
    self.magic, self.size, self.entry, self.ifc_cnr, self.ifc_ver, self.qspi_size, self.qspi_crc = fields[:7]
    self.name = fields[7].split(b'\0')[0].decode('ascii', 'replace')
    self.version = fields[8].split(b'\0')[0].decode('ascii', 'replace')
    self.keymap_id = fields[9]

  def __str__(self):
    return '%s %s, %d bytes, entry 0x%08x, interface %d.%d' % (self.name, self.version, self.size, self.entry, self.ifc_cnr, self.ifc_ver)


def branch_target(image, origin, address, link_only=False):
  """The destination of the b.w or bl at address in a Thumb image loaded at origin, or None where there is neither, or no bl when link_only is given."""
  at = address - origin
  if not 0 <= at <= len(image) - 4:
    return None
  first, second = struct.unpack_from('<HH', image, at)
  if first & 0xF800 != 0xF000 or second & 0xD000 not in ((0xD000,) if link_only else (0x9000, 0xD000)):
    return None
  sign, j1, j2 = (first >> 10) & 1, (second >> 13) & 1, (second >> 11) & 1
  offset = sign << 24 | (1 - (j1 ^ sign)) << 23 | (1 - (j2 ^ sign)) << 22 | (first & 0x3FF) << 12 | (second & 0x7FF) << 1
  return address + 4 + offset - (sign << 25)


def thumb_expand_imm(imm12):
  """The constant of a Thumb-2 modified immediate, as the architecture manual's ThumbExpandImm gives it."""
  if imm12 >> 10 == 0:
    byte = imm12 & 0xFF
    return (byte, byte << 16 | byte, byte << 24 | byte << 8, byte * 0x01010101)[(imm12 >> 8) & 3]
  unrotated, rotation = 0x80 | (imm12 & 0x7F), imm12 >> 7
  return (unrotated >> rotation | unrotated << (32 - rotation)) & 0xFFFFFFFF


def find_heap_creation(image, origin, base, size):
  """The function the platform image calls to create its heap: a bl after r1 is set to the arena's size and r0 loaded with its base from a literal pool."""
  for at in range(0, len(image) - 4, 2):
    first, second = struct.unpack_from('<HH', image, at)
    if first & 0xFBEF == 0xF04F and second & 0x8F00 == 0x0100:                                       # mov.w r1, #constant
      value = thumb_expand_imm(((first >> 10) & 1) << 11 | ((second >> 12) & 7) << 8 | (second & 0xFF))
    elif first & 0xFBF0 == 0xF240 and second & 0x8F00 == 0x0100:                                     # movw r1, #constant
      value = (first & 0xF) << 12 | ((first >> 10) & 1) << 11 | ((second >> 12) & 7) << 8 | (second & 0xFF)
    else:
      continue
    if value != size:
      continue
    for step in range(max(at - 8, 0), at + 16, 2):
      load = struct.unpack_from('<H', image, step)[0]
      literal = ((origin + step + 4) & ~3) + (load & 0xFF) * 4 - origin
      if load & 0xFF00 == 0x4800 and literal <= len(image) - 4 and struct.unpack_from('<I', image, literal)[0] == base:  # ldr r0, [pc, #n]
        for call in range(at + 4, at + 20, 2):
          target = branch_target(image, origin, origin + call, link_only=True)
          if target is not None:
            return target
  return None


class Emulator:
  def __init__(self, image_path, target, out_dir, fs_root, qspi=None, ms_per_tick=0, idle_budget=0, stack_watermark=False, chosen=None, key_gap=1, no_stubs=False, read_only=False, trace=False, dmcp=None, pick=False,
               host_heap=False, platform=None):
    self.image_path = Path(image_path)
    self.target = target
    self.out_dir = Path(out_dir)
    self.trace = trace
    self.table = lft.parse(target.lft_header())
    self.console = b''
    self.keys = []
    self.calls = {}                                           # entry name to call count
    self.missing = {}                                         # entry name to call count, for entries with no host implementation
    self.captures = []
    self.capture_all = False
    self.capturing = True                                     # whether a refresh writes a file at all, which a served run turns off
    self.last_image = None
    self.started = time.monotonic()
    self.stop_reason = None
    self.sdb_installed = False
    self.stubs = None
    self.hostcalls = {}
    self.returned = False
    self.text_states = {}                                     # t20 and t24 as the platform starts them, where a platform image is mapped
    self.selection = None                                     # the file_selection_screen call in progress, kept while its selection function runs
    self.no_stubs = no_stubs
    self.ms_per_tick = ms_per_tick                            # calls that advance the clock by one millisecond, or zero for the time the run has really taken
    self.ms_calls = 0
    self.sleep_ms = 0                                         # the wake the firmware last armed, which is how long the sleep after it lasts on hardware
    self.idle_ms = 0                                          # emulated milliseconds for the part of those sleeps the host did not really wait out
    self.idle_budget = idle_budget
    self.stack_watermark = stack_watermark
    self.script = []
    self.chosen = chosen                                      # what the file selection screen answers with
    self.page_took_over = False                               # set where a served run first waits for the page, which is where --choose stops answering
    self.pick = pick                                          # whether that screen instead opens the host's own chooser, for a run driven by hand
    self.serve = None                                         # the page a hand driven run is watched and typed on
    self.pool = None                                          # addresses of C47's block pool variables, where the ELF names them
    self.pool_peak = 0                                        # the most bytes of the pool an allocation asked to have taken
    self.last_frame = None
    self.script_at = 0
    self.script_wait = 0
    self.held_until = None                                    # where the firmware's clock lets the key of a hold go, once the hold has started
    self.marked_at = None
    self.key_gap = key_gap
    self.gap_left = 0
    self.stack_floor = 0
    self.deepest_sp = None                                    # the lowest stack pointer seen at a DMCP call, which is an independent reading to check the other against
    self.sp_now = target.stack_top                            # where the stack was at the last DMCP call, which the page displays beside the measured depth
    self.pattern_laid = stack_watermark                       # whether the depth comes from the pattern, which a served run turns on so its figure is the true one
    self.entries = {}                                         # --entry-sp: function name to its lowest stack pointer at entry, None until entered, and the calls
    self.arguments = {}                                       # --calls: function name to {argument value: calls}
    self.failed = {}                                          # --fail: function name to the calls answered with zero
    self.malloc_fail = set()                                  # --malloc-fail: the heap allocations, numbered from the last mark, that are refused
    self.malloc_fail_above = None                             # --malloc-fail-above: the size in bytes above which every heap allocation is refused
    self.malloc_armed = True                                  # false until the first mark of a script that has one, so the boot allocates as usual
    self.allocations = 0                                      # malloc, calloc and realloc calls since the last mark
    self.allocation_largest = 0
    self.malloc_refused = []                                  # (number, size, return address) of each allocation refused since the last mark
    self.elf_path = None                                      # --elf, for naming a caller in the report
    self.function_starts = None                               # sorted (address, name) of the firmware functions, read from the ELF when first asked
    self.profile = None                                       # --profile: the instruction counts, by stretch between marks
    self.frame_digests = None                                 # --frames: one digest of the pushed image per refresh
    self.clock_base = None                                    # --clock: the calendar time at the start of the run, in seconds since the epoch, or None for the host's

    self.uc = Uc(UC_ARCH_ARM, UC_MODE_THUMB | UC_MODE_MCLASS)
    self.uc.ctl_set_cpu_model(CPU_MODELS[target.cpu_model])
    self._map_memory()
    self.prog_info = self._load_image()
    self.sdb, self.sdb_size = target.sdb_layout()
    missing = [name for name in SDB_MEMBERS if name not in self.sdb]
    if missing:
      raise ValueError('sys_sdb_t in %s has no %s' % (target.dmcp_header(), ', '.join(missing)))
    self.interface = target.interface()
    if self.interface_differs():
      print(self.interface_warning(), file=sys.stderr, flush=True)   # at start as well as in the report, because a served run can go on for hours
    self.layout = keyscript.BY_NAME.get(self.prog_info.name, keyscript.C47)  # the name in the image picks the keyboard, and --layout overrides it
    self.qspi_path = self._load_qspi(qspi)
    self.platform_path = self._load_platform(dmcp or platform)
    self.dmcp_path = self.platform_path if dmcp else None       # the drawing entries run the platform's code only with --dmcp, the allocator whenever it is mapped
    self.fs = hostfs.HostFs(self, fs_root, read_only=read_only)
    self.alloc = hostfn.Allocator(*target.heap)
    self.platform_heap = False                                # whether malloc and its kin run the platform's allocator from the image
    self.heap_call = None                                     # the allocator call the heap shim is running: kind, size, old block, caller
    self.native_entries = set()                               # table entries left as the image has them, beside DMCP_DRAWS
    self.overflow = None                                      # the stack's first write into the platform's heap, its address and pc, and the lowest sp of any
    if self.dmcp_path:
      base = self._dmcp_lcd_base()
      self.lcd = hostfn.Framebuffer(self, base, dirty_map=base + LCD_Y * LCD_LINE_STRIDE + 2)   # the platform's bitmap sits right behind the lines it describes
      self.text_states = self._dmcp_text_states()               # read before the table is replaced, because it runs the platform's own font entry
    else:
      self.lcd = hostfn.Framebuffer(self, self._scratch(LCD_Y * LCD_LINE_STRIDE))
    if self.platform_path and not host_heap:
      self._start_platform_heap()
    self.aux_buf = self._scratch(hostfn.AUX_BUF_SIZE)
    self.write_buf = self._scratch(hostfn.WRITE_BUF_SIZE)
    self.fil = self._scratch(FIL_SIZE)
    self._install_trampolines()
    self._install_selection_shim()
    self._install_hooks()

  # ----- setup -----

  def _map_memory(self):
    for region in self.target.regions:
      self.uc.mem_map(region.addr, region.size, UC_PROT_ALL)
    self.uc.mem_map(SCRATCH_BASE, SCRATCH_SIZE, UC_PROT_ALL)
    self.uc.mem_map(WINDOW_BASE, WINDOW_SIZE, UC_PROT_ALL)
    self._scratch_next = SCRATCH_BASE

  def _scratch(self, size):
    """Reserve a span of the scratch region for a structure DMCP owns on hardware."""
    addr = self._scratch_next
    self._scratch_next = (addr + size + 7) & ~7
    if self._scratch_next > SCRATCH_BASE + SCRATCH_SIZE:
      raise MemoryError('the scratch region is full')
    self.uc.mem_write(addr, bytes(size))
    return addr

  def interface_differs(self):
    return (self.prog_info.ifc_cnr, self.prog_info.ifc_ver) != self.interface

  def interface_warning(self):
    """State what an image built for another platform interface means for the run.

    The library table the emulator puts down is the one in the SDK in the tree, and the image calls the table of the SDK it was built with. The two are the same only
    where the interface numbers agree: an image newer than the header can call entries the table does not have, and a changed first number can mean the same index is a
    different entry.
    """
    return ('the image was built for platform interface %d.%d and %s is %d.%d, so the library table the emulator puts down may not be the one the image calls'
            % (self.prog_info.ifc_cnr, self.prog_info.ifc_ver, self.target.dmcp_header(), self.interface[0], self.interface[1]))

  def _load_image(self):
    blob = self.image_path.read_bytes()
    info = ProgInfo(blob)
    if info.magic != PROG_INFO_MAGIC:
      raise ValueError('%s starts with 0x%08x, not the prog_info magic 0x%08x' % (self.image_path, info.magic, PROG_INFO_MAGIC))
    if info.size > len(blob):
      raise ValueError('%s declares %d bytes and contains %d' % (self.image_path, info.size, len(blob)))
    self.uc.mem_write(self.target.flash_origin, blob[:info.size])
    return info

  def _load_qspi(self, given):
    """Map the QSPI image, which the build writes beside the program as <name>_qspi.bin.

    Everything the linker script sends to the .qspi output section is in that file and nothing of it is in the program image, so a target whose fonts and cold tables sit
    in QSPI finds zeros there without it. The DM42 build puts about 1.6 Mb there and stops at the first font read when the region is empty.
    """
    path = Path(given) if given else self.image_path.with_name(self.image_path.stem + '_qspi.bin')
    if not path.exists():
      return None
    blob = path.read_bytes()
    region = next(r for r in self.target.regions if r.name == 'QSPI')
    if len(blob) > region.size:
      raise ValueError('%s is %d bytes and the QSPI region is %d' % (path, len(blob), region.size))
    self.uc.mem_write(region.addr, blob)
    return path

  DMCP_DRAWS = ('bitblt24', 'lcd_fill_rect', 'lcd_line_addr', 'lcd_clear_buf',
                'lcd_set_buf_cleared', 'lcd_get_buf_cleared',                # the flag lcd_clear_buf sets, which has to be read back from the same owner
                'lcd_writeText', 'lcd_setLine')                               # lcd_puts and the line it goes on, the only text entries the firmware calls

  def _load_platform(self, given):
    """Map a DMCP platform image under the program, so that the entries which only compute run the real thing instead of a rewrite of it.

    res/combo/DMCP5_flash_3.57.bin is in the tree because the release packaging combines it with the program. Following its call graph shows the drawing entries reach no
    peripheral register at all: bitblt24 is 89 instructions, lcd_fill_rect 126, lcd_line_addr 47 and lcd_clear_buf 27, none of them with an indirect branch. Everything
    that sends a line to the panel does touch one, so those keep their trap and the host answers them as before.
    """
    if not given:
      return None
    path = Path(given)
    blob = path.read_bytes()
    region = next((r for r in self.target.regions if r.name.startswith('DMCP')), None)
    if region is None:
      raise ValueError('%s has no region for a platform image' % self.target.name)
    if len(blob) > region.size:
      raise ValueError('%s is %d bytes and the %s region is %d' % (path, len(blob), region.name, region.size))
    self.uc.mem_write(region.addr, blob)
    return path

  def _dmcp_lcd_base(self):
    """Ask the mapped platform where its line buffer is, by running its own lcd_line_addr for line 0 and stepping back over the two byte header."""
    self.uc.reg_write(UC_ARM_REG_SP, self.target.stack_top)
    self.uc.reg_write(UC_ARM_REG_LR, RETURN_SENTINEL | 1)
    self.uc.reg_write(UC_ARM_REG_R0, 0)
    try:
      self.uc.emu_start(self.table.table_addr + self.table.by_name['lcd_line_addr'] * 4 | 1, 0, 0, 2000)
    except UcError:
      pass                                                    # the return to the unmapped sentinel is how the call ends
    return self.uc.reg_read(UC_ARM_REG_R0) - (LCD_LINE_STRIDE - LCD_LINE_SIZE)   # the entry gives the data, the buffer starts at the header before it

  HEAP_SHIM = b'\x98\x47\x00\xdf\x70\x47'                        # blx r3, then svc, then bx lr: the platform's allocator, called from a trap

  def _start_platform_heap(self):
    """Create the platform's heap as its own start-up does, so malloc and its kin can run the platform's code instead of the host's.

    The emulator never runs DMCP's start. DMCP5 creates its heap with an explicit call, the one that hands a function the arena of target.py, its size in r1 and its base
    in r0 from a literal: on 3.57 the bl at 0x080212cc to 0x08021290, which creates a ThreadX byte pool. DMCP for the DM42 has FreeRTOS heap_4, which creates its heap on
    the first allocation. So the call is made where it is found, and a trial allocation, which has to land in the arena, completes the heap either way. The four allocator
    entries still trap, so the host can refuse an allocation and record each one, and the trap runs the platform's code through a shim; the two free memory queries keep
    the image's own code. Where the trial fails, or neither allocator is recognised afterwards, the host's stays.
    """
    base, size = self.target.heap
    region = next(r for r in self.target.regions if r.name.startswith('DMCP'))
    image = bytes(self.uc.mem_read(region.addr, region.size))
    entries = {name: branch_target(image, region.addr, self.table.table_addr + self.table.by_name[name] * 4)
               for name in ('__sysfn_malloc', '__sysfn_free', '__sysfn_calloc', '__sysfn_realloc')}
    if None in entries.values():
      print('pgemu: the library table of %s does not branch to an allocator, so the host allocator answers malloc' % self.platform_path, file=sys.stderr)
      return
    self.uc.mem_map(SYSTEM_CONTROL, 0x1000, UC_PROT_ALL)       # read as zero, no exception active, which is what vPortEnterCritical checks for
    create = find_heap_creation(image, region.addr, base, size)
    if create is not None:
      self._run_at(create, base, size)
    trial = self._run_at(entries['__sysfn_malloc'], 16)
    if not base <= trial < base + size:
      print('pgemu: the heap of %s did not answer a trial allocation, so the host allocator answers malloc' % self.platform_path, file=sys.stderr)
      return
    self._run_at(entries['__sysfn_free'], trial)
    control = self._find_pool_control(base, size)
    start = None if control is not None else self._find_heap4_start(base, size)
    if control is None and start is None:
      print('pgemu: %s has an allocator this emulator cannot read, so the host allocator answers malloc' % self.platform_path, file=sys.stderr)
      return
    self.heap_entries = entries
    self.alloc = hostfn.PlatformHeap(self, base, size, control=control, start=start, limit=self._heap4_limit(start) if start else base + size)
    self.platform_heap = True
    self.native_entries = {'sys_free_mem', 'sys_largest_free_mem'}

  def _find_heap4_start(self, base, size):
    """FreeRTOS heap_4's xStart: the list head whose next block is the arena's aligned start and whose size is zero, and from which the chain reaches the end.

    Right after the trial allocation is freed the whole arena is one free block again, so xStart contains that pair. The candidate nearest above the arena is taken,
    where DMCP 3.29 and 3.31 have it, at 0x20016054, beside the allocator's other variables.
    """
    aligned, end = (base + 7) & ~7, (base + size - 8) & ~7
    candidates = []
    for region in self.target.regions:
      if region.name.startswith(('DMCP', 'FLASH', 'QSPI')):
        continue
      data = bytes(self.uc.mem_read(region.addr, region.size))
      at = data.find(struct.pack('<II', aligned, 0))
      while at >= 0:
        address = region.addr + at
        if not base <= address < base + size and at % 4 == 0 and self._u32(aligned) == end:
          candidates.append(address)
        at = data.find(struct.pack('<II', aligned, 0), at + 4)
    above = [a for a in candidates if a >= base + size]
    return min(above) if above else (candidates[0] if candidates else None)

  def _heap4_limit(self, start):
    """The end of heap_4's variables beside xStart, the lowest address the stack may reach without writing into the allocator: past xStart and the words beside it that
    the trial allocation left non-zero, the allocated bit, the free byte counts and the allocation counts of newer FreeRTOS."""
    limit = start + 8
    while limit < start + 64 and self._u32(limit) != 0:
      limit += 4
    return limit

  def _find_pool_control(self, base, size):
    """The ThreadX byte pool control block of the arena: the id 'BYTE', and the arena's start and size at offsets 24 and 28, anywhere outside the arena."""
    pattern = struct.pack('<I', 0x42595445)
    for region in self.target.regions:
      if region.name.startswith(('DMCP', 'FLASH', 'QSPI')):
        continue
      data = bytes(self.uc.mem_read(region.addr, region.size))
      at = data.find(pattern)
      while at >= 0:
        if data[at + 24:at + 32] == struct.pack('<II', base, size):
          return region.addr + at
        at = data.find(pattern, at + 4)
    return None

  def _run_at(self, address, *args):
    """Run the platform code at address to its return, before the run starts, and give back r0."""
    self.uc.reg_write(UC_ARM_REG_SP, self.target.stack_top)
    self.uc.reg_write(UC_ARM_REG_LR, RETURN_SENTINEL | 1)
    for register, value in zip((UC_ARM_REG_R0, UC_ARM_REG_R1), args):
      self.uc.reg_write(register, value & 0xFFFFFFFF)
    try:
      self.uc.emu_start(address | 1, 0, 0, 200000)
    except UcError:
      pass                                                    # the return to the unmapped sentinel is how the call ends
    return self.uc.reg_read(UC_ARM_REG_R0)

  def call_heap(self, kind, size=0, old=0):
    """Leave an allocator trap for the platform's own code, with the arguments still in r0 and r1, so its result comes back as the heap shim's trap."""
    self.heap_call = (kind, size, old, self.lr())
    self.uc.reg_write(UC_ARM_REG_R3, self.heap_entries['__sysfn_' + kind] | 1)
    self.uc.reg_write(UC_ARM_REG_LR, self.heap_shim | 1)
    return None

  def _run_platform(self, name, *args):
    """Run one entry of the mapped platform image to its return, before the table is replaced with traps, and give back r0."""
    self.uc.reg_write(UC_ARM_REG_SP, self.target.stack_top)
    self.uc.reg_write(UC_ARM_REG_LR, RETURN_SENTINEL | 1)
    for register, value in zip((UC_ARM_REG_R0, UC_ARM_REG_R1), args):
      self.uc.reg_write(register, value & 0xFFFFFFFF)
    try:
      self.uc.emu_start(self.table.table_addr + self.table.by_name[name] * 4 | 1, 0, 0, 20000)
    except UcError:
      pass                                                    # the return to the unmapped sentinel is how the call ends
    return self.uc.reg_read(UC_ARM_REG_R0)

  def _dmcp_text_states(self):
    """The display states DMCP starts t20 and t24 with, taken out of the platform image.

    A text entry draws with the font and the line settings its state names, and DMCP fills both states at start-up, which the emulator never runs. Its values are
    records in the image with the layout of disp_stat_t: a font pointer, the four positions at zero, and both line fill and new line set. The font of each is the one the
    platform's own lcd_switchFont gives for its number, 0 for t20 and 2 for t24, whose cells are 21 and 24 lines high, and the record is the first with that pointer. On
    DMCP5 3.57 they are at 0x08058d34 and 0x08058cd8. A state with no record found stays zero and its text is not drawn.
    """
    region = next(r for r in self.target.regions if r.name.startswith('DMCP'))
    image = bytes(self.uc.mem_read(region.addr, region.size))
    probe = self._scratch(DISP_STAT_SIZE)
    states = {}
    for name, number in (('t20', 0), ('t24', 2)):
      self.uc.mem_write(probe, bytes(DISP_STAT_SIZE))
      self._run_platform('lcd_switchFont', probe, number)
      font = self.uc.mem_read(probe, 4)
      at = image.find(bytes(font))
      while at >= 0:
        record = image[at:at + 28]
        if record[4:12] == bytes(8) and record[17] == 0 and record[19] == 1 and record[20] == 1:   # positions zero, not inverted, line fill and new line
          states[name] = record
          break
        at = image.find(bytes(font), at + 1)
    return states

  SELECTION_SHIM = b'\x98\x47\x00\xdf\x70\x47'                   # blx r3, then svc, then bx lr

  def _install_selection_shim(self):
    """Put down the three instructions a DMCP entry calls a firmware callback through.

    A trap cannot run emulated code from inside itself, so an entry that takes a callback is answered in two parts. The first leaves the trap with the callback in r3 and
    lr pointing here, so the entry's own bx lr arrives at the blx, which calls it with this as its return. Its result comes back through the svc as a second trap, whose
    handler either sends lr back here to call again or to the entry's caller, through the bx lr after the svc.
    """
    self.shim = self._scratch(len(self.SELECTION_SHIM))
    self.uc.mem_write(self.shim, self.SELECTION_SHIM)
    self.hostcalls[self.shim + 2] = 'pgemu_selection_done'
    self.heap_shim = self._scratch(len(self.HEAP_SHIM))
    self.uc.mem_write(self.heap_shim, self.HEAP_SHIM)
    self.hostcalls[self.heap_shim + 2] = 'pgemu_heap_done'
    self.selection_text = self._scratch(SELECTION_TEXT)

  def _install_trampolines(self):
    """Install the library table, with the entries a stub answers branching to it and every other entry trapping to the host.

    A stub answers inside the emulated machine and costs no host crossing at all. That matters because a boot makes tens of thousands of DMCP calls, most of them
    bitblt24, and a call answered in Python costs many times what a stub does. Where the firmware toolchain is absent and no prebuilt stubs match, every entry keeps
    its svc and hostfn.py answers it.
    """
    if self.dmcp_path is not None:
      self.stubs = None
      for name, index in self.table.by_name.items():
        if name not in self.DMCP_DRAWS and name not in self.native_entries:
          self.uc.mem_write(self.table.table_addr + index * 4, b'\x00\xdf\x70\x47')
      return
    platform_slots = {self.table.table_addr + self.table.by_name[name] * 4: bytes(self.uc.mem_read(self.table.table_addr + self.table.by_name[name] * 4, 4))
                      for name in self.native_entries}               # the image's own branches, put back over whatever the stubs lay down
    self.stubs = None if self.no_stubs else stublib.build(self.table, self.target.cpu_model)
    if self.stubs is None:
      self.uc.mem_write(self.table.table_addr, b'\x00\xdf\x70\x47' * self.table.count)
      for slot, word in platform_slots.items():
        self.uc.mem_write(slot, word)
      return
    self.uc.mem_write(self.stubs.base, self.stubs.blob)
    for slot, word in platform_slots.items():
      self.uc.mem_write(slot, word)
    self._write_symbol('pgemu_lcd_base', self.lcd.base)
    self._write_symbol('pgemu_win', WINDOW_BASE)
    self.hostcalls = {self.stubs.symbols['pgemu_hostcall_' + name]: name for name in stublib.HOSTCALLS}

  def _write_symbol(self, name, value):
    self.uc.mem_write(self.stubs.symbols[name], struct.pack('<I', value & 0xFFFFFFFF))

  def _read_symbol(self, name):
    return struct.unpack('<I', self.uc.mem_read(self.stubs.symbols[name], 4))[0]

  # ----- keys, which a stub takes from a ring in emulated memory and the host otherwise takes from a list -----

  def _expand_keys(self):
    """Turn the key codes given into the press and release pairs the firmware takes.

    src/c47/c47.c states the convention at the key_pop call: above zero is a press, zero is a release and below zero is no event. A key that is pressed and never
    released leaves the firmware waiting, which is why a release follows every press here. Pressing the yellow shift and releasing it before EXIT is what turns the
    calculator off, and that is --keys 28,33.
    """
    queue = []
    for key in self.keys:
      queue.append(key)
      if key != 0:
        queue.append(0)
    self.keys = queue

  def _load_keys(self):
    self._expand_keys()
    if self.stubs is None:
      return
    ring = self.stubs.symbols['pgemu_keys']
    for index, key in enumerate(self.keys[:KEY_RING_SIZE]):
      self.uc.mem_write(ring + index * 4, struct.pack('<i', key))
    self._write_symbol('pgemu_ring_head', 0)
    self._write_symbol('pgemu_ring_tail', min(len(self.keys), KEY_RING_SIZE))

  def _push_key(self, code):
    """Put one press and its release where the firmware takes them from."""
    self._push_raw(code)
    self._push_raw(0)

  def _push_raw(self, *codes):
    """Put key events through as they are, so the page can send the press of a key now and its release when the key comes up.

    src/c47/c47.c states the convention at its key_pop call: above zero is a press and zero is a release. A function name overlay or a shifted preview appears between the
    two, so a page that sends both at once can never show one.
    """
    if self.stubs is None:
      self.keys += list(codes)
      return
    tail = self._read_symbol('pgemu_ring_tail')
    for value in codes:
      self.uc.mem_write(self.stubs.symbols['pgemu_keys'] + tail * 4, struct.pack('<i', value))
      tail = (tail + 1) % KEY_RING_SIZE
    self._write_symbol('pgemu_ring_tail', tail)

  def advance_script(self):
    """Take the next step at the point the firmware has nothing left to do, and report whether the run continues.

    A press goes in every --key-gap idle rounds, one by default, which is how a person uses the calculator: the firmware takes a key, acts on it, finds the buffer
    empty and comes back here for the next. One round has been enough for everything driven so far, alpha name entry included; the option is there for a sequence
    that turns out to need longer. A wait lasts that many rounds on top, for a step the firmware acts on after the last key. A hold keeps its key down until the
    firmware's own clock has advanced by its milliseconds, so a longpress lands on the same stage with --ms-per-tick or without it.
    """
    while self.script_at < len(self.script):
      kind, value = self.script[self.script_at]
      if kind == keyscript.PRESS:
        if self.gap_left > 0:
          self.gap_left -= 1                                  # let the screen settle before the next key, which alpha entry needs to take every letter
          return True
        self.script_at += 1
        self.gap_left = self.key_gap
        self._push_key(value)
        return True
      if kind == keyscript.DOWN:
        if self.gap_left > 0:
          self.gap_left -= 1                                  # let the screen settle before the key goes down, as a press does
          return True
        self.script_at += 1
        self.gap_left = self.key_gap
        self._push_raw(value)                                 # the press with no release, so the key stays down through the hold that follows
        return True
      if kind == keyscript.HOLD:
        if self.held_until is None:
          self.held_until = self.run_ms() + value              # counted from the first round after the key went down, as the simulator counts from its press
        if self.run_ms() < self.held_until:
          return True
        self.held_until = None
        self.script_at += 1
        continue
      if kind == keyscript.UP:
        self.script_at += 1
        self._push_raw(0)                                     # the release that ends a hold, which a longpress runs its staged key on
        return True
      if kind == keyscript.WAIT:
        if self.script_wait == 0:
          self.script_wait = value
        self.script_wait -= 1
        if self.script_wait == 0:
          self.script_at += 1
        return True
      self.script_at += 1
      if kind == keyscript.MARK:
        self._lay_stack_pattern()                             # from here down the stack reading is of what follows, not of the whole run
        self._forget_watched()
        self.malloc_armed = True
        self.marked_at = value or 'mark'
        if self.profile is not None:
          self.profile.mark(self.marked_at)
      else:
        self.capture(value)                                   # a snap, taken where the sequence asks for it and not at every refresh
    return False

  def script_left(self):
    """Whether the key script has a step left, which a served run takes before the page's keys, as a run without a page does."""
    return self.script_at < len(self.script)

  def chosen_now(self):
    """The path --choose answers a file selection with: in every selection of a run without a page, and in a served run only until the page takes over.

    The page takes over where the run first waits for it, after the script's last key has been acted on, so a script that ends on the ENTER of READP still gets its file.
    From then on a selection is the person's, and --choose would answer a save with the file the script loaded and write over it.
    """
    return self.chosen if self.serve is None or not self.page_took_over else None

  def capture(self, name=None):
    self.out_dir.mkdir(parents=True, exist_ok=True)
    path = self.out_dir / ('%s.bmp' % (name if name else 'screen-%03d' % len(self.captures)))
    self.captures.append(self.lcd.capture(path))
    return path

  def keys_pending(self):
    if self.stubs is None:
      return bool(self.keys)
    return self._read_symbol('pgemu_ring_head') != self._read_symbol('pgemu_ring_tail')

  def key_take(self):
    """Take the next key, from wherever the keys are kept in this run."""
    if self.stubs is None:
      return self.keys.pop(0) if self.keys else -1
    head = self._read_symbol('pgemu_ring_head')
    if head == self._read_symbol('pgemu_ring_tail'):
      return -1
    key = struct.unpack('<i', self.uc.mem_read(self.stubs.symbols['pgemu_keys'] + head * 4, 4))[0]
    self._write_symbol('pgemu_ring_head', (head + 1) % KEY_RING_SIZE)
    return key

  def _install_sdb(self):
    """Fill the system data block fields DMCP owns, at the address the SDK names.

    This runs at the first DMCP call and not at load, because the two targets place the block differently. On DMCP5 it is at 0x20000000, clear of everything the
    image touches. On DMCP it is at the base of SRAM2, which is where the linker script puts .data.sdb, so Program_Entry copies over it during startup and anything
    written before that copy is lost. The first DMCP call comes from program_main, after the copy on either target.
    """
    self.sdb_installed = True
    self.uc.mem_write(self.target.sdb_addr, bytes(self.sdb_size))
    provided = {
      self.sdb['ppgm_fp']:        self.fil,
      self.sdb['pds_t20']:        self._scratch(DISP_STAT_SIZE),
      self.sdb['pds_t24']:        self._scratch(DISP_STAT_SIZE),
      self.sdb['pds_fReg']:       self._scratch(DISP_STAT_SIZE),
      self.sdb['timer2_counter']: self._scratch(4),
      self.sdb['timer3_counter']: self._scratch(4),
    }
    for offset, addr in provided.items():
      self.uc.mem_write(self.target.sdb_addr + offset, struct.pack('<I', addr))
    for name, offset in (('t20', self.sdb['pds_t20']), ('t24', self.sdb['pds_t24'])):
      if name in self.text_states:
        self.uc.mem_write(provided[offset], self.text_states[name])

  def _install_hooks(self):
    self.uc.hook_add(UC_HOOK_INTR, self._on_trampoline)
    self.uc.hook_add(UC_HOOK_MEM_INVALID, self._on_invalid)
    if self.platform_heap and self.alloc.base < self.target.stack_top:
      self.uc.hook_add(UC_HOOK_MEM_WRITE, self._on_heap_write, None, self.alloc.limit - OVERFLOW_WATCH, self.alloc.limit - 1)

  def _on_heap_write(self, uc, access, address, size, value, user_data):
    """Catch the stack writing into the platform allocator's variables or the top of its arena, which it does only with the stack pointer below them.

    The allocator writes the same words on every malloc and free, but then from a stack pointer well above them. On the DM42 a write like this lands in the heap
    on the calculator as well: the run goes on as it would there, and the report names the first such write and how deep the stack went.
    """
    sp = uc.reg_read(UC_ARM_REG_SP)
    if sp < self.alloc.limit:
      first = self.overflow
      if first is None:
        self.overflow = (address, uc.reg_read(UC_ARM_REG_PC), sp)
      elif sp < first[2]:
        self.overflow = (first[0], first[1], sp)              # replaced whole, so the page's thread never takes half an update

  def watch_functions(self, elf=None, entry_sp=(), calls=(), fail=()):
    """Hook firmware functions by name, taken from the ELF beside the image unless another is given, and return the ELF that was read.

    Each hook is a code hook on the function's first instruction. The branch to itself measured at the top of this file loses a factor of nineteen to any code hook, but a
    firmware run does not show it: OVER1 took 15.8 and 15.6 s without a hook and 14.0 and 15.6 s with one on calcDerivOfOrder.
    """
    elf = Path(elf) if elf else self.image_path.with_suffix('.elf')
    table = elfsym.functions(elf)
    roles = {}                                                # address to what its one hook does
    def role(name):
      return roles.setdefault(elfsym.address(table, name, elf), {'name': name, 'entry': False, 'argument': None, 'fail': None})
    for name in entry_sp:
      role(name)['entry'] = True
      self.entries[name] = (None, 0)
    for spec in calls:
      name, _, index = spec.partition(':')
      role(name)['argument'] = int(index) if index else 0
      self.arguments[name] = {}
    for spec in fail:
      name, _, value = spec.partition('=')
      if not value:
        raise ValueError('--fail %s names no value, as allocC47Blocks=345 does' % spec)
      role(name)['fail'] = int(value, 0)
      self.failed[name] = 0
    for address, what in roles.items():
      self.uc.hook_add(UC_HOOK_CODE, self._on_watched, what, address, address)
    return elf

  def watch_pool(self, elf=None):
    """Find C47's own block pool through the ELF, so the page can show it apart from every other allocation, and hook the two entries that take blocks from it.

    src/c47/config.c takes the whole pool with one malloc at start, and allocC47Blocks hands out blocks inside it. The host allocator counts that malloc as taken from
    the start, so RAM is full in the pool never shows in the heap. A peak read by the page every 250 ms would miss the working copies a matrix inverse frees before it
    returns, which is why the entries are hooked. Without the ELF or one of the names the page keeps the heap as one bar.
    """
    path = Path(elf) if elf else self.image_path.with_suffix('.elf')
    try:
      variables, functions = elfsym.objects(path), elfsym.functions(path)
      pool = {name: elfsym.address(variables, name, path) for name in ('ram', 'numberOfFreeMemoryRegions', 'freeMemoryRegions')}
      taking = elfsym.address(functions, 'allocC47Blocks', path), elfsym.address(functions, 'reallocC47Blocks', path)
    except (OSError, ValueError):
      return
    self.pool = pool
    self.uc.hook_add(UC_HOOK_CODE, self._on_pool_taken, False, taking[0], taking[0])
    self.uc.hook_add(UC_HOOK_CODE, self._on_pool_taken, True, taking[1], taking[1])

  def pool_state(self):
    """The size of C47's block pool and the bytes taken in it, program memory included, or None before the firmware has set it up."""
    if self.pool is None:
      return None
    try:
      size = self.alloc.busy.get(self._u32(self.pool['ram']))
      count = self._u32(self.pool['numberOfFreeMemoryRegions'])
      regions = self.pool['freeMemoryRegions']
      if self._u32(regions) in self.alloc.busy:
        regions = self._u32(regions)                          # a pointer to a malloc on DMCP5, where src/c47/c47.c keeps the array itself on the DM42
      if not size or count > MAX_POOL_REGIONS:
        return None
      sizes = struct.unpack('<%dH' % (2 * count), self.uc.mem_read(regions, 4 * count))[1::2] if count else ()
    except UcError:
      return None                                             # a figure the page can do without, never a reason to end the run
    return size, size - POOL_BLOCK * sum(sizes)

  def _on_pool_taken(self, uc, address, size, resizing):
    state = self.pool_state()
    if state is not None:
      wanted = self.arg(2) - self.arg(1) if resizing else self.arg(0)     # reallocC47Blocks(pointer, old, new) and allocC47Blocks(blocks)
      self.pool_peak = max(self.pool_peak, state[1] + POOL_BLOCK * wanted)

  def forget_heap(self):
    """Start the heap and pool peaks again from what is taken now, so one operation can be read on its own."""
    self.alloc.peak = self.alloc.taken
    self.pool_peak = 0

  def _u32(self, address):
    return struct.unpack('<I', self.uc.mem_read(address, 4))[0]

  def _on_watched(self, uc, address, size, what):
    name = what['name']
    if what['entry']:
      sp = uc.reg_read(UC_ARM_REG_SP)
      lowest, count = self.entries[name]
      self.entries[name] = (sp if lowest is None else min(lowest, sp), count + 1)
    if what['argument'] is not None:
      counts = self.arguments[name]
      value = self.arg(what['argument'])
      counts[value] = counts.get(value, 0) + 1
    if what['fail'] is not None and uc.reg_read(UC_ARM_REG_R0) == what['fail']:
      self.failed[name] += 1
      uc.reg_write(UC_ARM_REG_R0, 0)
      uc.reg_write(UC_ARM_REG_PC, uc.reg_read(UC_ARM_REG_LR))  # back to the caller before the function's first instruction runs

  def _forget_watched(self):
    """Start the --entry-sp, --calls, --fail, malloc and overflow records again, at a mark or the page's reset, as the stack reading does."""
    self.overflow = None
    for name in self.entries:
      self.entries[name] = (None, 0)
    for name in self.arguments:
      self.arguments[name] = {}
    for name in self.failed:
      self.failed[name] = 0
    self.allocations = 0
    self.allocation_largest = 0
    self.malloc_refused = []

  # ----- the trap -----

  def _on_trampoline(self, uc, intno, user_data):
    """One DMCP call. The svc has already advanced the program counter past itself, so the entry is two bytes back from where the trap arrives."""
    address = uc.reg_read(UC_ARM_REG_PC) - 2
    span = 4 * self.table.count
    if self.table.table_addr <= address < self.table.table_addr + span:
      name = self.table.name((address - self.table.table_addr) // 4)
    elif address in self.hostcalls:
      name = self.hostcalls[address]                          # a stub handing its call back, so hostfn.py answers it as though it had arrived at the entry
    else:
      self.stop('an svc at 0x%08x, outside the library table' % address)
      return
    here = uc.reg_read(UC_ARM_REG_SP)                          # one register read on a route that already costs a host crossing, so the page has live numbers
    self.sp_now = here
    self.deepest_sp = here if self.deepest_sp is None else min(self.deepest_sp, here)
    if not self.sdb_installed:
      self._install_sdb()
    self.calls[name] = self.calls.get(name, 0) + 1
    handler = hostfn.HANDLERS.get(name)
    if self.trace:
      print('  %-28s r0=0x%08x r1=0x%08x%s' % (name, self.arg(0), self.arg(1), '' if handler else '   [no implementation]'))
    if handler is None:
      self.missing[name] = self.missing.get(name, 0) + 1
      uc.reg_write(UC_ARM_REG_R0, 0)
      return
    result = handler(self)
    if result is not None:
      uc.reg_write(UC_ARM_REG_R0, result & 0xFFFFFFFF)

  def _on_invalid(self, uc, access, address, size, value, user_data):
    if address == RETURN_SENTINEL or address == (RETURN_SENTINEL | 1):
      self.returned = True                                    # the program returned to the sentinel, which is how a run that finishes ends
      self.stop_reason = 'the program returned'
      uc.emu_stop()
      return False
    self.stop_reason = 'access to 0x%08x, outside every mapped region (%s)' % (address, self._nearest_region(address))
    uc.emu_stop()
    return False

  def _nearest_region(self, address):
    for region in self.target.regions:
      if region.addr <= address < region.addr + region.size:
        return 'inside %s' % region.name
    below = [r for r in self.target.regions if r.addr + r.size <= address]
    return 'above %s' % max(below, key=lambda r: r.addr).name if below else 'below every region'

  # ----- the interface hostfn.py uses -----

  def arg(self, n):
    """Read argument n under the ARM procedure call standard."""
    if n < 4:
      return self.uc.reg_read([UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3][n])
    sp = self.uc.reg_read(UC_ARM_REG_SP)
    return struct.unpack('<I', self.uc.mem_read(sp + (n - 4) * 4, 4))[0]

  def program_ending(self):
    """Whether the firmware has asked to end, which is what fnOff does on DMCP: src/c47/calcMode.c sets STAT_PGM_END and nothing else."""
    if not self.sdb_installed:
      return False
    return bool(struct.unpack('<I', self.uc.mem_read(self.target.sdb_addr + self.sdb['calc_state'], 4))[0] & STAT_PGM_END)

  def refuse_allocation(self, size):
    """Count one heap allocation and report whether --malloc-fail or --malloc-fail-above refuses it, so that malloc returns NULL to the firmware."""
    if not self.malloc_armed:
      return False
    self.allocations += 1
    self.allocation_largest = max(self.allocation_largest, size)
    if self.allocations in self.malloc_fail or (self.malloc_fail_above is not None and size > self.malloc_fail_above):
      self.malloc_refused.append((self.allocations, size, self.lr() & ~1))
      return True
    return False

  def function_at(self, address):
    """The firmware function an address is in, as name+offset from the ELF beside the image, or the bare address where no ELF can be read."""
    if self.function_starts is None:
      try:
        table = elfsym.functions(Path(self.elf_path) if self.elf_path else self.image_path.with_suffix('.elf'))
      except (OSError, ValueError):
        table = {}
      self.function_starts = sorted((start, name) for name, starts in table.items() for start in starts)
    index = bisect.bisect_right(self.function_starts, (address, chr(0x10FFFF))) - 1
    if index < 0:
      return '0x%08x' % address
    start, name = self.function_starts[index]
    return '%s+0x%x' % (name, address - start)

  def lr(self):
    return self.uc.reg_read(UC_ARM_REG_LR)

  def call_selection(self, function, *args):
    """Leave the trap for a firmware callback, with its arguments in r0 to r2, so that its result comes back as the shim's trap."""
    for register, value in zip((UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2), args):
      self.uc.reg_write(register, value & 0xFFFFFFFF)
    self.uc.reg_write(UC_ARM_REG_R3, function | 1)
    self.uc.reg_write(UC_ARM_REG_LR, self.shim | 1)

  def return_to(self, caller, value):
    """End the entry with value, back at the caller it was called from, which the bx lr of either trap then takes."""
    self.uc.reg_write(UC_ARM_REG_LR, caller)
    return value

  def wait_key(self):
    """Wait for a key to go down and give back its code, which is what DMCP's runner_get_key does.

    A firmware callback relies on that: load_statefile and show_warning in src/c47-dmcp5/hal/io.c repeat it until ENTER or EXIT. The other key entries answer at
    once with -1 on an empty buffer, and a callback repeating one of those never gets back to sys_sleep, which is where a served run takes its keys from the page
    and a scripted run takes its next step. So both are taken here. A script with no press left gets EXIT, so a callback asking for more than the script gives is
    left rather than waited on for ever, and a served run goes to the page once its script has run out.
    """
    while True:
      code = self.key_take()
      if code > 0:
        return code
      if code == 0:
        continue                                              # a release, and the runner waits for a key to go down
      if (self.serve is None or self.script_left()) and self.advance_script():
        continue
      if self.serve is None:
        return KEY_EXIT
      self.serve_idle()

  def stop(self, reason):
    """End the run from inside a host implementation."""
    self.stop_reason = reason
    self.uc.emu_stop()

  def run_ms(self):
    """The milliseconds the run has taken on the firmware's own clock, which sys_current_ms gives it, without advancing a counted clock."""
    if self.ms_per_tick:
      return self.ms_calls // self.ms_per_tick
    return (time.monotonic() - self.started) * 1000 + self.idle_ms

  def calendar_time(self):
    """The date and time the firmware is given, in seconds since the epoch: --clock plus the run's own milliseconds, or else the host's time.

    With --clock and --ms-per-tick together every reading follows from the calls alone, so two runs draw the same date and time in the status bar.
    """
    if self.clock_base is None:
      return time.time() + self.idle_ms / 1000                 # ahead of the host by the sleeps the run did not wait out
    return self.clock_base + self.run_ms() / 1000

  def idle_for(self, ms):
    """Add what is left of a sleep the host went through faster than the calculator would, so the clock the firmware is given advances by the sleep it armed."""
    if ms > 0:
      self.idle_ms += ms

  def serve_idle(self):
    """Where a served run waits: show what the firmware has drawn, take whatever the page has sent, and give the processor back.

    This never stops the run. The wait lasts until a key arrives or until the wake the firmware armed, which is what the processor does on hardware, so the clock the
    firmware is given is the time the person at the page really took. A wait of a few milliseconds instead would return before the armed time and be counted as a sleep
    the host went through faster, and the clock would then run at several times real time: the shift ladder of src/c47/c47Extensions/keyboardTweak.c steps from f to g
    after 560 ms, and a single press would reach g while the finger is still on the key.
    """
    self.page_took_over = True
    self.publish()
    if self.take_page_keys():
      return
    try:
      self._push_raw(self.serve.keys.get(timeout=min(max(self.sleep_ms, 1), PAGE_IDLE_MS) / 1000))
    except queue.Empty:
      pass

  def take_page_keys(self):
    """Put what the page has sent into the ring, and restart here where the page asked for it. Reports whether a key came."""
    if self.serve.restarting:
      self.restart()
    taken = False
    while True:
      try:
        self._push_raw(self.serve.keys.get_nowait())
        taken = True
      except queue.Empty:
        return taken

  def cross_on_key_empty(self):
    """Let key_empty trap to the host in a served run, so that EXIT pressed during a long computation reaches the firmware.

    A computation that shows its progress tests for EXIT through exitKeyWaiting in src/c47/c47Extensions/addons.c, which asks key_empty and never sleeps. With the
    stub answering, the ring then stays empty however many keys the page has sent, because only a sleep moved them over. The host answer moves them over first.
    """
    if self.stubs is None or 'key_empty' not in self.stubs.answered:
      return
    self.uc.mem_write(self.table.table_addr + self.table.by_name['key_empty'] * 4, b'\x00\xdf\x70\x47')
    self.stubs.answered.discard('key_empty')

  def restart(self):
    """Start the run again from the beginning, with the image taken from disk as it is now.

    A new build lands in the same file, and what a person wants after building is this run replaced by one of the new image. The process replaces itself rather than
    building the machine again in place: the mapped memory, the stubs, the file system and the page's own server are all set up once at start-up out of the command
    line, so re-running that line is the whole of it. The exec closes the listening socket, and the page finds the new server on the same port as soon as it is up.
    """
    print('restarting: %s' % ' '.join(sys.argv), file=sys.stderr, flush=True)
    os.execv(sys.executable, [sys.executable] + sys.argv)

  def hold_platform_screen(self, wait):
    """Keep the screen the platform took up long enough for a person at the page to see it.

    A served run sends a frame only where it differs from the one before, and the page asks for one every 120 ms, so a pattern the firmware draws over within a few
    milliseconds reaches nobody. The disk information page and the platform menu are left with a key on hardware, fnDiskInfo through wait_for_key_press and the menu
    through EXIT, so those wait for one here too and the key is consumed rather than handed to the firmware. A run without a page needs none of this, because the capture
    is taken as the frame is pushed, and nor does a served run while its script has steps left, since the script supplies that key.
    """
    if self.serve is None or wait == 'none' or self.script_left():
      return
    if wait == 'brief':
      time.sleep(PLATFORM_SCREEN_MS / 1000)
      return
    deadline = time.monotonic() + PLATFORM_SCREEN_WAIT
    while time.monotonic() < deadline:
      try:
        code = self.serve.keys.get(timeout=0.05)
      except queue.Empty:
        continue
      if code:
        break
    until = time.monotonic() + 0.3                            # the page sends a release after every press, and that one belongs to the screen that was left
    while time.monotonic() < until:
      try:
        self.serve.keys.get(timeout=0.05)
      except queue.Empty:
        pass

  def publish(self):
    """Put the pushed image on the page, and only where it has changed."""
    if self.serve is None:
      return
    frame = bytes(self.lcd.screen)
    if frame != self.last_frame:
      self.last_frame = frame
      self.serve.put(self.lcd.bmp())

  def on_refresh(self):
    """Capture the pushed image, by default only where it differs from the one captured before, so a run of refreshes that change nothing produces one file."""
    self.publish()
    if self.frame_digests is not None:
      self.frame_digests.append(hashlib.sha1(bytes(self.lcd.screen)).hexdigest())
    if self.script or not self.capturing:
      return                                                  # a script names where a capture belongs, and a served run is watched rather than read back later
    image = bytes(self.lcd.screen)
    if self.capture_all or image != self.last_image:
      self.last_image = image
      self.capture()

  # ----- the run -----

  def _stack_region(self):
    """The mapped region the stack grows down through."""
    return next(r for r in self.target.regions if r.addr < self.target.stack_top <= r.addr + r.size)

  def _pattern_floor(self):
    """The lowest address the pattern may be written to.

    The block pool and the stack share a region on the DM42, so the pattern stops above every address the allocator has handed out. Writing below that would put
    the pattern over the calculator's own registers and its loaded program, which at the start of a run costs nothing because nothing is allocated yet, and at a
    mark in the middle of one destroys the state the case under test needs.
    """
    region = self._stack_region()
    floor = region.addr + self.sdb_size if self.target.sdb_addr == region.addr else region.addr
    if self.alloc.base < self.target.stack_top:
      floor = max(floor, self.alloc.limit if self.platform_heap else self.alloc.high)    # the platform's own heap keeps headers and variables up to its limit
    return floor

  def _lay_stack_pattern(self):
    """Write the pattern from the floor up to whatever is live now, which at the start of a run is the whole span and at a mark is everything below the frame."""
    self.stack_floor = self._pattern_floor()
    live = self.uc.reg_read(UC_ARM_REG_SP)
    top = self.target.stack_top if not self.sdb_installed else min(live, self.target.stack_top)
    self.uc.mem_write(self.stack_floor, struct.pack('<I', STACK_PATTERN) * ((top - self.stack_floor) // 4))

  def _read_stack_depth(self):
    """Find the deepest word the run disturbed, ignoring anything the allocator handed out below it.

    On this target the block pool and the stack can share a region, which is the collision src/c47/memory.c describes on the DM42, so a reading is only a depth while it
    stays above every address the allocator ever gave away. Reaching that address makes the figure a floor and not a measurement, exactly as STCKST 1 reports it on the
    calculator.
    """
    floor = max(self.stack_floor, self._pattern_floor())
    span = self.target.stack_top - floor
    words = self.uc.mem_read(floor, span - span % 4)
    marker = struct.pack('<I', STACK_PATTERN)
    for offset in range(0, len(words) - 3, 4):
      if words[offset:offset + 4] != marker:
        deepest = floor + offset
        return deepest, self.target.stack_top - deepest, deepest == floor
    return self.target.stack_top, 0, False

  def live_depth(self):
    """How deep the stack has been, found from the pattern rather than from what a DMCP call happened to catch.

    Reading the stack pointer at each call to the host misses the deepest point of any computation that never crosses, and on the DM42 that gives 3728 bytes for
    a PLOT the firmware's own marking measures at 6696. The pattern keeps no such secret: what overwrote it stays overwritten whenever it happened.
    """
    if not self.pattern_laid:
      return self.target.stack_top - (self.deepest_sp if self.deepest_sp is not None else self.target.stack_top), False
    floor = max(self.stack_floor, self._pattern_floor())
    span = self.target.stack_top - floor
    live = bytes(self.uc.mem_read(floor, span - span % 4))
    theirs = struct.pack('<I', FIRMWARE_PATTERN) in live       # the build marks the same stretch, so what is left of this pattern is their span and not a depth
    return self._read_stack_depth()[1], theirs

  def forget_depth(self):
    """Lay the pattern again, so what follows is measured on its own."""
    self.deepest_sp = None
    self._forget_watched()
    if self.pattern_laid:
      self._lay_stack_pattern()

  def run(self, max_instructions):
    self._load_keys()
    self.malloc_armed = not any(kind == keyscript.MARK for kind, _ in self.script)    # a script with a mark refuses nothing before it
    if self.pattern_laid:
      self._lay_stack_pattern()
    self.uc.reg_write(UC_ARM_REG_SP, self.target.stack_top)
    self.uc.reg_write(UC_ARM_REG_LR, RETURN_SENTINEL | 1)                          # unmapped, so a return from program_main is reported instead of running on
    entry = self.prog_info.entry & ~1
    try:
      self.uc.emu_start(entry | 1, 0, 0, max_instructions)                          # a count of zero is Unicorn's own way of saying no limit
      if self.stop_reason is None:
        self.stop_reason = ('the run ended with no reason recorded' if not max_instructions
                            else 'the instruction limit of %d was reached%s' % (max_instructions, self._spin_note()))
    except UcError as error:
      pc = self.uc.reg_read(UC_ARM_REG_PC)
      if self.stop_reason is None:
        self.stop_reason = '%s at pc 0x%08x (%s)' % (error, pc, self._nearest_region(pc))

  def stop_on_interrupt(self):
    """End the run with its report on ctrl-c.

    Python runs a signal handler only between its own instructions, which a computation that never crosses to the host does not reach, and a KeyboardInterrupt raised in
    a hook that waits on the page's key queue leaves the queue's lock half released, which Unicorn then raises as a RuntimeError. So the handler does nothing, the signal
    arrives as a byte on a pipe, and a thread of its own stops the emulated processor. A run still going after INTERRUPT_GRACE, in a dialog that waits for the page, ends
    there without a report.
    """
    self.ended = threading.Event()
    read_end, write_end = os.pipe()
    os.set_blocking(write_end, False)
    signal.set_wakeup_fd(write_end)
    signal.signal(signal.SIGINT, lambda number, frame: None)

    def watch():
      os.read(read_end, 1)
      self.stop_reason = 'ctrl-c at the terminal'
      self.uc.emu_stop()
      if not self.ended.wait(INTERRUPT_GRACE):
        print('pgemu: ended by ctrl-c while the run was waiting, so there is no report', file=sys.stderr, flush=True)
        os._exit(130)
    threading.Thread(target=watch, daemon=True).start()

  def _spin_note(self):
    """Name a branch to itself where the budget ran out, which is how FreeRTOS's configASSERT stops the platform with interrupts masked, a hung calculator."""
    pc = self.uc.reg_read(UC_ARM_REG_PC) & ~1
    try:
      halfword = struct.unpack('<H', bytes(self.uc.mem_read(pc, 2)))[0]
    except UcError:
      return ''
    return ', in a branch to itself at 0x%08x, which is how the platform stops on a failed assertion' % pc if halfword == 0xE7FE else ''

  def run_selftest(self, max_instructions):
    """Run the file system self test inside the emulated machine, so the whole route is exercised: the trap, the arguments and the FIL in emulated memory."""
    program = stublib.build_selftest(self.table, self.target.cpu_model)
    if program is None:
      print('the self test needs the firmware toolchain, which is absent')
      return 1
    self.uc.mem_write(program.base, program.blob)
    self.sdb_installed = True
    if self.stack_watermark:
      self._lay_stack_pattern()
    self.uc.reg_write(UC_ARM_REG_SP, self.target.stack_top)
    self.uc.reg_write(UC_ARM_REG_LR, RETURN_SENTINEL | 1)
    self.uc.reg_write(UC_ARM_REG_R0, self.fil)
    try:
      self.uc.emu_start(program.symbols['pgemu_selftest'] | 1, 0, 0, max_instructions)
    except UcError:
      pass                                                    # the return to the unmapped sentinel is how the program ends
    print('file system self test against %s' % self.fs.root)
    for (text, what), count in self.fs.touched.items():
      print('  %-40s %s%s' % (text, what, '' if count == 1 else ', %d times' % count))
    if not self.returned:
      print('  the test did not finish: %s' % self.stop_reason)
      return 1
    if self.stack_watermark:
      deepest, depth, _ = self._read_stack_depth()
      print('  stack %d bytes below the entry frame, deepest 0x%08x' % (depth, deepest))
    failed = self.uc.reg_read(UC_ARM_REG_R0)
    print('  result 0x%x, %s' % (failed, 'every check passed' if failed == 0 else 'a check failed'))
    return 0 if failed == 0 else 1

  def _built_from(self):
    """Name the commit the image was built at, where this repository has it.

    prog_info contains what the build wrote into pgm_ver, which for this tree is the abbreviated commit and a -mod where the working tree had changes. Printing the
    subject beside it shows at a glance which branch's work is in the image, which the hash alone does not.
    """
    version = self.prog_info.version
    short = version.split('-mod')[0]
    if not short:
      return version
    try:
      out = subprocess.run(['git', 'log', '-1', '--date=short', '--pretty=%ad  %s', short],
                           capture_output=True, text=True, timeout=5)
    except OSError:
      return version
    if out.returncode != 0:
      return '%s, which this repository does not contain' % version
    return '%s = %s%s' % (version, out.stdout.strip()[:96], '' if '-mod' not in version else '  (plus uncommitted changes)')

  def report(self):
    print('image      %s' % self.image_path)
    print('target     %s, %s, library table at 0x%08x with %d entries' % (self.target.name, self.target.cpu_model, self.table.table_addr, self.table.count))
    print('prog_info  %s, key layout %s' % (self.prog_info, self.layout.name))
    if self.interface_differs():
      print('interface  %s' % self.interface_warning())
    print('built from %s' % self._built_from())
    print('files      %s%s' % (self.fs.root, ', read only' if self.fs.read_only else ''))
    print('qspi       %s' % (self.qspi_path if self.qspi_path else 'not loaded, no image beside the program'))
    print('stopped    %s' % self.stop_reason)
    print('pc         0x%08x' % self.uc.reg_read(UC_ARM_REG_PC))
    print('heap       %d bytes taken at peak, %d free, largest free span %d%s' % (self.alloc.peak, self.alloc.total_free(), self.alloc.largest_free(),
                                                                                ', in the platform\'s own allocator, %s' % self.alloc.kind if self.platform_heap else ''))
    if self.platform_path:
      uses = ('the drawing entries and the allocator' if self.dmcp_path and self.platform_heap else 'the drawing entries' if self.dmcp_path
              else 'the allocator' if self.platform_heap else 'nothing, the host answered every entry')
      print('platform   %s, for %s' % (self.platform_path, uses))
    if self.overflow is not None:
      address, pc, lowest = self.overflow
      print('overflow   the stack wrote to 0x%08x, in the platform allocator\'s memory below 0x%08x, from %s; its pointer went down to 0x%08x, '
            '%d bytes below the entry frame%s' % (address, self.alloc.limit, self.function_at(pc), lowest, self.target.stack_top - lowest,
                                                   ' since %s' % self.marked_at if self.marked_at else ''))
    refusing = self.malloc_fail or self.malloc_fail_above is not None
    print('malloc     %d allocations%s, the largest %d bytes%s' % (self.allocations, ' since %s' % self.marked_at if self.marked_at else '',
                                                                     self.allocation_largest, ', %d refused' % len(self.malloc_refused) if refusing else ''))
    for number, size, caller in self.malloc_refused[:20]:
      print('           refused number %d, %d bytes, for %s' % (number, size, self.function_at(caller)))
    print('sp         0x%08x, %d bytes below the top' % (self.uc.reg_read(UC_ARM_REG_SP), self.target.stack_top - self.uc.reg_read(UC_ARM_REG_SP)))
    print('stubs      %s' % ('%d entries, built at 0x%08x' % (len(self.stubs.answered), self.stubs.base) if self.stubs
                              else 'none, --dmcp runs the platform\'s drawing entries' if self.dmcp_path
                              else 'none, the firmware toolchain is absent and every entry traps to the host'))
    if self.stack_watermark:
      deepest, depth, at_floor = self._read_stack_depth()
      heap_top = self.alloc.high if self.alloc.base < self.target.stack_top else None   # the highest address the heap has handed out, to the pool or to anything else
      print('stack      %d bytes below the entry frame%s, deepest 0x%08x%s' % (depth, ' since %s' % self.marked_at if self.marked_at else '', deepest, ', which is the floor, so the run went at least that far' if at_floor else ''))
      if self.live_depth()[1]:
        print('           the image marks the stack itself with STACK_WATERMARK, so this is the span that marking covers and not a depth')
      if self.deepest_sp is not None:
        print('           lowest stack pointer seen at a DMCP call 0x%08x, %d bytes below the entry frame' % (self.deepest_sp, self.target.stack_top - self.deepest_sp))
      if heap_top is not None:
        print('           the heap reaches 0x%08x, leaving %d bytes between it and the deepest word' % (heap_top, deepest - heap_top))
    since = ' since %s' % self.marked_at if self.marked_at else ''
    for name, (lowest, count) in self.entries.items():
      if lowest is None:
        print('entry      %s never entered%s' % (name, since))
      else:
        print('entry      %s at most %d bytes below the entry frame, %d calls%s' % (name, self.target.stack_top - lowest, count, since))
    for name, counts in self.arguments.items():
      shown = sorted(counts, key=lambda value: (-counts[value], value))
      print('calls      %s %d%s%s' % (name, sum(counts.values()), since, ''.join(', %d with %d' % (counts[value], value) for value in shown[:16])
                                      + (', and %d other values' % (len(shown) - 16) if len(shown) > 16 else '')))
    for name, count in self.failed.items():
      print('failed     %s %d calls answered with zero%s' % (name, count, since))
    print('refreshes  %d, %d lines pushed, %d captures' % (self.lcd.refreshes, self.lcd.pushes, len(self.captures)))
    if self.frame_digests is not None:
      print('frames     %d digests, one a line in %s' % (len(self.frame_digests), self.frames_path))
    if self.lcd.painted:
      left = max(self.lcd.left_over, self.lcd.painted_left() if self.lcd.watching else (0, 0, 0, 0))
      print('screen     taken for the platform %d times, %s' % (self.lcd.painted,
            'drawn again in full' if not left[0] else 'and %d bytes of the pattern were never drawn again, on %d lines between %d and %d'
            % (left[0], left[3], left[1], left[2])))
    for path in self.captures:
      print('           %s' % path)
    if self.fs.touched:
      print('file calls')
      for (text, what), count in self.fs.touched.items():
        print('           %-40s %s%s' % (text, what, '' if count == 1 else ', %d times' % count))
    if self.console:
      print('console    %s' % self.console.decode('utf-8', 'replace').strip())
    print()
    if self.stubs is not None:
      print('answered in the emulated machine, at no host crossing: %s' % ', '.join(sorted(self.stubs.answered)))
      print()
    print('DMCP entries reached on the host: %d of %d' % (len(self.calls), self.table.count))
    for name in sorted(self.calls, key=lambda k: -self.calls[k]):
      mark = '  no implementation' if name in self.missing else ''
      print('  %5d  %-28s%s' % (self.calls[name], name, mark))
    if self.profile is not None:
      print()
      self.profile.report()


def parse_clock(text):
  """Seconds since the epoch for a local date and time written YYYY-MM-DD HH:MM, with or without :SS, or a date alone for midnight."""
  for form in ('%Y-%m-%d %H:%M:%S', '%Y-%m-%d %H:%M', '%Y-%m-%d'):
    try:
      return time.mktime(time.strptime(text, form))
    except ValueError:
      continue
  raise argparse.ArgumentTypeError('%s is not a date and time written YYYY-MM-DD HH:MM' % text)


def main():
  parser = argparse.ArgumentParser(description='Run a built C47 or R47 firmware image under an emulated Cortex-M.')
  parser.add_argument('image', help='the .pg5 or .pgm to run')
  parser.add_argument('--target', choices=sorted(targets.BY_NAME), help='override the target the file suffix selects')
  parser.add_argument('--out', default=str(HERE / 'out'), help='where screen captures are written')
  parser.add_argument('--serve', nargs='?', type=int, const=8731, metavar='PORT',
                      help='put the screen on a local page and take keys from it, so the run is driven by hand; the port is 8731 unless one is given')
  parser.add_argument('--selftest', action='store_true', help='run the file system self test inside the emulated machine instead of the firmware')
  parser.add_argument('--fs', default=str(HERE / 'fs'), help="the directory the calculator's FAT root is mapped onto; pass . for the working folder")
  parser.add_argument('--read-only', action='store_true', help='refuse every write, so a run cannot change the directory')
  parser.add_argument('--qspi', help='the QSPI image, by default <name>_qspi.bin beside the program')
  parser.add_argument('--platform', metavar='FILE',
                      help='the DMCP platform image whose allocator malloc and its kin run; res/combo/DMCP5_flash_*.bin is found for the R47 when none is given')
  parser.add_argument('--host-heap', action='store_true', help='answer malloc and its kin on the host instead of in the platform\'s allocator')
  parser.add_argument('--dmcp', nargs='?', const='', metavar='FILE',
                      help='run the platform\'s drawing entries as well, from FILE or the image --platform names or finds')
  parser.add_argument('--max-instructions', type=int, default=None,
                      help='the budget a run is allowed, %d by default. A served run is driven by hand and has no budget unless one is given here' % DEFAULT_BUDGET)
  parser.add_argument('--layout', choices=sorted(keyscript.BY_NAME), help='override the keyboard the name in the image selects, for reading a C47 script on an R47 or the other way about')
  parser.add_argument('--script', help='a key script file, or - to read one from stdin')
  parser.add_argument('--press', help='a key script given on the command line')
  parser.add_argument('--keys', default='', help='comma separated key codes, each pressed and released in turn; 28,33 is the yellow shift then EXIT that turns the calculator off')
  parser.add_argument('--no-stubs', action='store_true', help='answer every entry on the host, for comparing against the stubs')
  parser.add_argument('--choose', help='the path the file selection screen answers with, for a function that opens the DMCP chooser; in a served run only while '
                                       'the script runs')
  parser.add_argument('--pick', action=argparse.BooleanOptionalAction, default=None,
                      help="open the host's own file chooser at that screen, the way the simulator does. A served run has a person at the keyboard and does this "
                           "unless --no-pick is given; --choose still wins over both")
  parser.add_argument('--stack-watermark', action='store_true', help='report how deep the run took the stack, and how close that came to the block pool')
  parser.add_argument('--key-gap', type=int, default=1, help='idle rounds between one key and the next, where a firmware needs longer than one to finish with a key')
  parser.add_argument('--idle', type=int, default=0, help='idle rounds allowed before a run ends, for a session that has to continue past a settled screen')
  parser.add_argument('--ms-per-tick', type=int, default=0,
                      help='count the clock in calls rather than in time, one millisecond per this many, for a run that has to repeat exactly')
  parser.add_argument('--clock', type=parse_clock, metavar="'YYYY-MM-DD HH:MM'",
                      help='start the calendar clock at this local time instead of the host\'s, so the status bar shows the same date and time in every run')
  parser.add_argument('--frames', metavar='FILE', help='write a digest of the pushed image at every refresh to FILE, one a line, for comparing two builds frame by frame')
  parser.add_argument('--profile', nargs='?', type=int, const=12, metavar='N',
                      help='count every instruction the run takes, in stretches that each mark starts, and list the N functions and DMCP entries that take most, 12 by default')
  parser.add_argument('--capture', action=argparse.BooleanOptionalAction, default=None,
                      help='write the screen to --out as it changes. A served run is watched in the page instead and does this only where asked')
  parser.add_argument('--capture-all', action='store_true', help='capture at every refresh, not only where the pushed image differs from the one before')
  parser.add_argument('--trace', action='store_true', help='print every DMCP call as it arrives')
  parser.add_argument('--elf', help='the ELF that --entry-sp, --calls, --fail and --profile take function names from, by default <name>.elf beside the program')
  parser.add_argument('--entry-sp', action='append', default=[], metavar='FUNCTION',
                      help='report how deep the stack was at the deepest entry to this firmware function, and how often it was entered; repeat for more than one')
  parser.add_argument('--calls', action='append', default=[], metavar='FUNCTION[:N]',
                      help='count the calls to this firmware function by the value of argument N, the first when no N is given; repeat for more than one')
  parser.add_argument('--fail', action='append', default=[], metavar='FUNCTION=VALUE',
                      help='answer a call to this firmware function whose first argument is VALUE with zero at once, which reaches a RAM full route without filling RAM')
  parser.add_argument('--malloc-fail', action='append', type=int, default=[], metavar='N',
                      help='refuse the Nth heap allocation, malloc, calloc and realloc numbered together from the last mark, so malloc returns NULL; repeat for more')
  parser.add_argument('--malloc-fail-above', type=int, metavar='BYTES',
                      help='refuse every heap allocation larger than BYTES, from the last mark, or from the start in a run whose script has no mark')
  args = parser.parse_args()

  target = targets.BY_NAME[args.target] if args.target else targets.for_image(args.image)
  target.find_sdk(args.image)
  platform = args.dmcp or args.platform
  if args.dmcp and args.platform and Path(args.dmcp).resolve() != Path(args.platform).resolve():
    print('pgemu: --dmcp %s and --platform %s name two platform images' % (args.dmcp, args.platform), file=sys.stderr)
    return 2
  if platform is None and (args.dmcp is not None or not args.host_heap):
    platform = target.find_platform(args.image)
  if args.dmcp is not None and platform is None:
    print('pgemu: --dmcp needs a platform image, and none is named or found for %s' % target.name, file=sys.stderr)
    return 2
  budget = args.max_instructions if args.max_instructions is not None else (0 if args.serve is not None else DEFAULT_BUDGET)
  pick = args.pick if args.pick is not None else args.serve is not None      # a served run has a person at the keyboard, so a dialog there can be answered
  emulator = Emulator(args.image, target, args.out, args.fs, qspi=args.qspi, read_only=args.read_only, ms_per_tick=args.ms_per_tick, idle_budget=args.idle,
                      stack_watermark=args.stack_watermark, chosen=args.choose, pick=pick, key_gap=args.key_gap, no_stubs=args.no_stubs, trace=args.trace,
                      dmcp=platform if args.dmcp is not None else None, host_heap=args.host_heap, platform=platform)
  emulator.capture_all = args.capture_all
  emulator.pattern_laid = args.stack_watermark or args.serve is not None   # the page reports a depth, and only the pattern gives a true one
  emulator.capturing = args.capture if args.capture is not None else args.capture_all or args.serve is None
  emulator.clock_base = args.clock
  emulator.elf_path = args.elf
  emulator.malloc_fail = set(args.malloc_fail)
  emulator.malloc_fail_above = args.malloc_fail_above
  if args.frames:
    emulator.frame_digests = []
    emulator.frames_path = Path(args.frames)
  if args.profile is not None:
    emulator.profile = profiler.Profile(emulator, Path(args.elf) if args.elf else Path(args.image).with_suffix('.elf'), args.profile)
  if args.entry_sp or args.calls or args.fail:
    try:
      emulator.watch_functions(args.elf, args.entry_sp, args.calls, args.fail)
    except (OSError, ValueError) as error:
      print('pgemu: %s' % error, file=sys.stderr)
      return 2
  if args.selftest:
    return emulator.run_selftest(budget)
  if args.layout:
    emulator.layout = keyscript.BY_NAME[args.layout]
  if args.serve is not None:
    emulator.serve, port = servelib.start(emulator, emulator.layout, args.serve)
    emulator.cross_on_key_empty()
    emulator.watch_pool(args.elf)
    print('drive it at http://127.0.0.1:%d/ , and stop with ctrl-c' % port, flush=True)
  keys = [(keyscript.PRESS, int(k)) for k in args.keys.split(',') if k.strip()]   # through the script, which feeds the ring of stubs.c one press at a time
  text = sys.stdin.read() if args.script == '-' else Path(args.script).read_text() if args.script else args.press
  emulator.script = keys + (keyscript.parse(text, emulator.layout) if text else [])
  emulator.stop_on_interrupt()
  emulator.run(budget)
  emulator.ended.set()
  if emulator.frame_digests is not None:
    emulator.frames_path.write_text(''.join(digest + '\n' for digest in emulator.frame_digests))
  emulator.report()
  return 0


if __name__ == '__main__':
  sys.exit(main())

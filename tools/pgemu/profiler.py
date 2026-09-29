# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# Instruction counts for --profile: every instruction the emulated processor runs, charged to a firmware function or a DMCP entry, in stretches that each mark in
# the key script starts.
#
# A block hook is called before every basic block runs, with its address and its length in bytes. A Thumb block is counted halfword by halfword, and a halfword whose
# top five bits are 11101, 11110 or 11111 starts a 32 bit instruction. The owner of a block is the firmware function whose symbol range contains its address, from
# the ELF beside the image. Code outside every firmware function belongs to DMCP: the library table, the stubs of stubs.c or, with --dmcp, the platform's own code.
# It is charged to the entry the firmware last branched to, so the work bitblt24 does shows under its own name. An entry answered on the host runs in Python and
# adds no instruction, which is why a count meant to match the calculator is taken with --dmcp.
#
# The hook calls into Python once per block, so a profiled run takes several times as long as one without; the counts themselves do not depend on that.

import bisect
import collections

from unicorn import UC_HOOK_BLOCK

import elfsym
from target import SCRATCH_BASE, SCRATCH_SIZE

LONG_PREFIXES = (0x1D, 0x1E, 0x1F)                              # the top five bits of the first halfword of a 32 bit Thumb instruction


class Stretch:
  """What one stretch of the run took, from a mark, or from the start, to the next mark or the end."""

  def __init__(self, name, refreshes, pushes):
    self.name = name
    self.instructions = 0
    self.owners = collections.Counter()                       # function name, or DMCP and the entry, to instructions
    self.calls = collections.Counter()                        # DMCP entry to the branches into its table slot
    self.first_refresh = refreshes
    self.first_push = pushes
    self.refreshes = 0
    self.pushes = 0


class Profile:
  def __init__(self, emu, elf, top):
    self.emu = emu
    self.top = top
    self.note = None
    try:
      self.ranges = elfsym.function_ranges(elf)
    except (OSError, ValueError) as error:
      self.ranges = []
      self.note = 'no function names, %s, so the firmware counts as one' % error
    self.starts = [start for start, _, _ in self.ranges]
    platform = next(r for r in emu.target.regions if r.name.startswith('DMCP'))
    self.platform = (platform.addr, platform.addr + platform.size)     # the table, the stubs and a mapped platform image all sit in this region
    self.table_start = emu.table.table_addr
    self.table_end = emu.table.table_addr + 4 * emu.table.count
    self.blocks = {}                                          # (address, size) to (instructions, owning function or None, table entry or None)
    self.entry = None                                         # the DMCP entry the firmware last branched to
    self.done = []
    self.current = Stretch('start', emu.lcd.refreshes, emu.lcd.pushes)
    emu.uc.hook_add(UC_HOOK_BLOCK, self._on_block)

  def _describe(self, uc, address, size):
    code = bytes(uc.mem_read(address, size))
    count = at = 0
    while at + 1 < len(code):
      halfword = code[at] | code[at + 1] << 8
      at += 4 if halfword >> 11 in LONG_PREFIXES else 2
      count += 1
    entry = None
    if self.table_start <= address < self.table_end and (address - self.table_start) % 4 == 0:
      entry = self.emu.table.name((address - self.table_start) // 4)
    if self.platform[0] <= address < self.platform[1] or SCRATCH_BASE <= address < SCRATCH_BASE + SCRATCH_SIZE:
      return count, None, entry                               # DMCP, charged at run time to the entry last branched to
    owner = 'firmware'
    if self.ranges:
      index = bisect.bisect_right(self.starts, address) - 1
      owner = self.ranges[index][2] if index >= 0 and address < self.ranges[index][1] else 'firmware outside every symbol'
    return count, owner, entry

  def _on_block(self, uc, address, size, user_data):
    known = self.blocks.get((address, size))
    if known is None:
      known = self._describe(uc, address, size)
      self.blocks[(address, size)] = known
    count, owner, entry = known
    stretch = self.current
    stretch.instructions += count
    if entry is not None:
      self.entry = entry
      stretch.calls[entry] += 1
    stretch.owners[owner if owner is not None else 'DMCP %s' % (self.entry or 'code before the first entry')] += count

  def _close(self):
    stretch = self.current
    stretch.refreshes = self.emu.lcd.refreshes - stretch.first_refresh
    stretch.pushes = self.emu.lcd.pushes - stretch.first_push
    self.done.append(stretch)

  def mark(self, name):
    """End the stretch in progress at a mark in the script and start one under the mark's name."""
    self._close()
    self.current = Stretch(name, self.emu.lcd.refreshes, self.emu.lcd.pushes)

  def report(self):
    self._close()
    if self.note:
      print('profile    %s' % self.note)
    for stretch in self.done:
      print('profile    %s: %d instructions, %d refreshes, %d lines pushed' % (stretch.name, stretch.instructions, stretch.refreshes, stretch.pushes))
      for owner, count in stretch.owners.most_common(self.top):
        share = 100.0 * count / stretch.instructions if stretch.instructions else 0.0
        calls = ', %d calls' % stretch.calls[owner[5:]] if owner.startswith('DMCP ') and owner[5:] in stretch.calls else ''
        print('           %11d  %5.1f %%  %s%s' % (count, share, owner, calls))

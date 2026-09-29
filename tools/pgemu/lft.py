#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# The DMCP library function table, parsed out of the SDK header rather than copied.
#
# Every DMCP entry point reaches the program as one macro in dep/DMCP*_SDK/dmcp/lft_ifc.h:
#
#   #define lcd_refresh (*(typeof(lcd_refresh)*)(LIBRARY_FN_BASE+48))
#
# so the whole interface is an array of function pointers at a fixed address, and the byte offset in the macro is the index times four. Parsing the header keeps the
# emulator in step with an SDK update: a new entry appears in the table with no edit here, and a reordered table cannot go unnoticed.

import re
from pathlib import Path

BASE_RE = re.compile(r'^#define\s+LIBRARY_FN_BASE\s+(0x[0-9a-fA-F]+)')
ENTRY_RE = re.compile(r'^#define\s+([A-Za-z_][A-Za-z0-9_]*)\s+\(\*\(typeof\([^)]*\)\s*\*\)\(LIBRARY_FN_BASE\+(\d+)\)\)')


class FunctionTable:
  """The parsed contents of one lft_ifc.h."""

  def __init__(self, base, by_offset):
    self.base = base                                          # the thumb-tagged address the header names, so the table itself starts one byte lower
    self.table_addr = base & ~1
    self.by_offset = by_offset                                # {byte offset: name}
    self.by_index = {off // 4: name for off, name in by_offset.items()}
    self.by_name = {name: off // 4 for off, name in by_offset.items()}
    self.count = max(by_offset) // 4 + 1 if by_offset else 0

  def name(self, index):
    return self.by_index.get(index, '<unassigned %d>' % index)


def parse(header_path):
  """Return the FunctionTable described by one lft_ifc.h."""
  base = None
  by_offset = {}
  for line in Path(header_path).read_text(encoding='utf-8', errors='replace').splitlines():
    match = BASE_RE.match(line.strip())
    if match:
      base = int(match.group(1), 16)
      continue
    match = ENTRY_RE.match(line.strip())
    if match:
      by_offset[int(match.group(2))] = match.group(1)
  if base is None:
    raise ValueError('%s defines no LIBRARY_FN_BASE' % header_path)
  if not by_offset:
    raise ValueError('%s defines no table entries' % header_path)
  return FunctionTable(base, by_offset)


if __name__ == '__main__':
  import sys
  table = parse(sys.argv[1])
  print('base 0x%08x, %d entries' % (table.table_addr, table.count))
  for index in sorted(table.by_index):
    print('%4d  +%-5d %s' % (index, index * 4, table.by_index[index]))

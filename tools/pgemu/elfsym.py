# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# The function and variable symbols of the ELF a build leaves beside its image, so a run can be told a firmware function or variable by name. C47.pgm has C47.elf beside
# it and R47.pg5 has R47.elf, and the addresses in either are the addresses the image runs at. Read here rather than through arm-none-eabi-nm so a run needs no toolchain.

import struct
from pathlib import Path

SHT_SYMTAB = 2
STT_OBJECT = 1
STT_FUNC = 2


def functions(path):
  """Map every function name in a 32 bit little endian ELF to its addresses, the Thumb bit cleared.

  A name maps to a list, because a static function can share its name with one in another file, and a caller that is given such a name has to refuse it rather than
  pick one.
  """
  return _symbols(path, STT_FUNC)


def objects(path):
  """Map every variable name in the ELF to its addresses, in the same form as functions."""
  return _symbols(path, STT_OBJECT)


def function_ranges(path):
  """Every function in the ELF as (start, end, name), sorted by start, for finding which function an address belongs to.

  A static function that shares its name with another keeps its own range here, since it is the range and not the name that separates them. A symbol of size zero, which
  hand written assembly can leave, is given one halfword.
  """
  ranges = {(value & ~1, (value & ~1) + max(size, 2), text) for text, value, size in _entries(path, STT_FUNC)}
  return sorted(ranges)


def _symbols(path, kind_wanted):
  found = {}
  for text, value, _ in _entries(path, kind_wanted):
    addresses = found.setdefault(text, [])
    if value & ~1 not in addresses:
      addresses.append(value & ~1)
  return found


def _entries(path, kind_wanted):
  """Each symbol of one kind as (name, value, size), from every symbol table in the ELF, leaving out those at address zero."""
  data = Path(path).read_bytes()
  if data[:4] != b'\x7fELF' or data[4] != 1 or data[5] != 1:
    raise ValueError('%s is not a 32 bit little endian ELF' % path)
  section_offset = struct.unpack_from('<I', data, 0x20)[0]
  entry_size, count = struct.unpack_from('<HH', data, 0x2E)
  sections = [struct.unpack_from('<10I', data, section_offset + n * entry_size) for n in range(count)]
  for _, kind, _, _, offset, size, link, _, _, symbol_size in sections:
    if kind != SHT_SYMTAB:
      continue
    names = sections[link][4]
    for at in range(offset, offset + size, symbol_size):
      name, value, length, info, _, _ = struct.unpack_from('<IIIBBH', data, at)
      if info & 0xF != kind_wanted or value == 0:
        continue
      yield data[names + name:data.index(b'\0', names + name)].decode('ascii', 'replace'), value, length


def address(table, name, elf):
  """The one address of name, or a ValueError that states why there is none."""
  addresses = table.get(name, [])
  if not addresses:
    raise ValueError('%s has no function named %s, or the compiler inlined every call to it' % (elf, name))
  if len(addresses) > 1:
    raise ValueError('%s has %d functions named %s, at %s' % (elf, len(addresses), name, ', '.join('0x%08x' % a for a in addresses)))
  return addresses[0]

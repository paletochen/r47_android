#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# Build the library table and the stubs that answer inside the emulated machine.
#
# The whole table is emitted as assembly and handed to arm-none-eabi-gcc, so no instruction is encoded here. An entry with a stub becomes a b.w to it and an entry without
# one keeps the svc that reaches hostfn.py. The table and the stubs are linked at the address the SDK header names, which puts entry zero exactly at LIBRARY_FN_BASE and
# lets the assembler resolve every branch.
#
# The toolchain is the one the firmware is built with. Where it is absent this module reports so and the emulator answers every entry in Python instead, which is
# slower and identical in behaviour.

import base64
import hashlib
import hostfs
import json
import re
import shutil
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent
CACHE = HERE / '.stubs-cache'
PREBUILT = HERE / 'prebuilt'
SOURCE = HERE / 'stubs.c'
SELFTEST = HERE / 'selftest.c'

# Where the self test program is linked, clear of the table and the stubs and inside the mapped DMCP region on either target.
SELFTEST_BASE_OFFSET = 0x2000

# Which DMCP entry each stub answers. bitblt24 and lcd_fill_rect are most of the calls a boot makes, f_read comes next while a state file is restored, and the others are
# the iteration the firmware repeats while it waits for a key. sys_sleep deliberately keeps its svc, because ending a run is the host's decision, and so do the two clock
# entries, because a boot asks the time six times and only the host has the time the run has taken. sys_timer_start keeps its svc for the same reason: the figure it arms
# is how long the sleep after it lasts, and the host clock is where that length has to be added.
STUBS = {
  'bitblt24':           'pgemu_bitblt24',
  'lcd_fill_rect':      'pgemu_lcd_fill_rect',
  'key_empty':          'pgemu_key_empty',
  'key_pop':            'pgemu_key_pop',
  'key_tail':           'pgemu_key_tail',
  'key_pop_all':        'pgemu_key_pop_all',
  'reset_auto_off':     'pgemu_noop',
  'sys_timer_disable':  'pgemu_noop',
  'sys_critical_start': 'pgemu_noop',
  'sys_critical_end':   'pgemu_noop',
  'lcd_refresh_wait':   'pgemu_noop',
  'f_read':             'pgemu_f_read',
}

# Entries a stub hands back to the host, reached through an svc the assembly below labels. The name is the DMCP entry hostfn.py answers.
HOSTCALLS = ('f_read',)

CPU_FLAGS = {
  'cortex-m33': ['-mthumb', '-march=armv8-m.main+dsp', '-mcpu=cortex-m33', '-mfloat-abi=hard', '-mfpu=fpv5-sp-d16'],
  'cortex-m4':  ['-mthumb', '-march=armv7e-m', '-mcpu=cortex-m4', '-mfloat-abi=hard', '-mfpu=fpv4-sp-d16'],
}

NM_RE = re.compile(r'^([0-9a-fA-F]+)\s+\S\s+(\S+)$')


class Stubs:
  """The linked blob, the address it goes at, and the symbols the emulator writes through."""

  def __init__(self, blob, base, symbols, answered):
    self.blob = blob
    self.base = base
    self.symbols = symbols
    self.answered = answered                                  # DMCP entry names a stub answers, which the host will not be asked for


def available():
  return shutil.which('arm-none-eabi-gcc') is not None


def _table_asm(table):
  """Emit the whole library table, one four byte entry per index, in the order the header states."""
  lines = ['  .syntax unified', '  .thumb', '  .section .lfttable,"ax",%progbits', '  .global pgemu_table', 'pgemu_table:']
  for index in range(table.count):
    name = table.by_index.get(index)
    stub = STUBS.get(name)
    if stub:
      lines.append('  b.w %s          @ %d %s' % (stub, index, name))
    else:
      lines.append('  svc 0')
      lines.append('  bx lr           @ %d %s' % (index, name or 'unassigned'))
  for name in HOSTCALLS:
    lines += ['  .global pgemu_hostcall_%s' % name, '  .thumb_func', '  .type pgemu_hostcall_%s, %%function' % name,
              'pgemu_hostcall_%s:' % name, '  svc 0', '  bx lr']
  return '\n'.join(lines) + '\n'


def _linker_script(base):
  return 'SECTIONS {\n  . = 0x%08x;\n  .lfttable : { KEEP(*(.lfttable)) }\n  .text : { *(.text*) *(.rodata*) }\n  .data : { *(.data*) }\n  .bss : { *(.bss*) *(COMMON) }\n}\n' % base


def build_selftest(table, cpu_model):
  """Compile the file system self test, which calls the table the way the firmware does, and link it clear of the table and the stubs."""
  if not available():
    return None
  base = table.table_addr + SELFTEST_BASE_OFFSET
  needed = ('f_open', 'f_close', 'f_read', 'f_write', 'f_lseek', 'check_create_dir')
  if any(name not in table.by_name for name in needed):
    return None
  defines = ['-DLIBRARY_FN_BASE=0x%08xu' % table.base, '-DREAD_BLOCK=%d' % hostfs.READ_BLOCK] + ['-DOFF_%s=%d' % (name.upper(), table.by_name[name] * 4) for name in needed]
  script = 'SECTIONS {\n  . = 0x%08x;\n  .text : { *(.text*) *(.rodata*) }\n  .data : { *(.data*) }\n  .bss : { *(.bss*) *(COMMON) }\n}\n' % base
  key = hashlib.sha256((script + cpu_model + ' '.join(defines) + SELFTEST.read_text()).encode()).hexdigest()[:16]
  out = CACHE / ('selftest-' + key)
  blob_path, nm_path = out / 'selftest.bin', out / 'selftest.nm'
  if not blob_path.exists():
    out.mkdir(parents=True, exist_ok=True)
    (out / 'selftest.ld').write_text(script)
    subprocess.run(['arm-none-eabi-gcc', *CPU_FLAGS[cpu_model], *defines, '-Os', '-ffreestanding', '-fno-builtin', '-nostdlib', '-Wall', '-Wextra', '-Werror',
                    '-Wl,-T', str(out / 'selftest.ld'), '-Wl,--build-id=none', '-Wl,--no-warn-rwx-segments', '-Wl,-e,pgemu_selftest',
                    '-o', str(out / 'selftest.elf'), str(SELFTEST)], check=True)
    subprocess.run(['arm-none-eabi-objcopy', '-O', 'binary', str(out / 'selftest.elf'), str(blob_path)], check=True)
    nm_path.write_text(subprocess.run(['arm-none-eabi-nm', str(out / 'selftest.elf')], check=True, capture_output=True, text=True).stdout)
  symbols = {}
  for line in nm_path.read_text().splitlines():
    match = NM_RE.match(line.strip())
    if match:
      symbols[match.group(2)] = int(match.group(1), 16)
  return Stubs(blob_path.read_bytes(), base, symbols, set())


def _key(asm, script, cpu_model):
  return hashlib.sha256((asm + script + cpu_model + SOURCE.read_text()).encode()).hexdigest()[:16]


def _prebuilt_path(cpu_model):
  return PREBUILT / ('%s.json' % cpu_model)


def _load_prebuilt(cpu_model, key, table):
  """Take the checked in blob where it was built from this source, this table and this processor."""
  path = _prebuilt_path(cpu_model)
  if not path.exists():
    return None
  held = json.loads(path.read_text())
  if held.get('key') != key:
    return None
  return Stubs(base64.b64decode(held['blob']), held['base'], {k: int(v) for k, v in held['symbols'].items()},
               {name for name in table.by_name if name in STUBS})


def emit(table, cpu_model):
  """Build the blob and check it in, so a run needs no ARM toolchain."""
  built = build(table, cpu_model, prebuilt=False)
  if built is None:
    raise SystemExit('the firmware toolchain is needed to emit the prebuilt stubs')
  PREBUILT.mkdir(parents=True, exist_ok=True)
  path = _prebuilt_path(cpu_model)
  path.write_text(json.dumps({
    'note': 'Built from stubs.c and the table of the matching SDK header. Regenerate with: python3 tools/pgemu/stubs.py --emit',
    'key': _key(_table_asm(table), _linker_script(table.table_addr), cpu_model),
    'base': built.base,
    'symbols': built.symbols,
    'blob': base64.b64encode(built.blob).decode(),
  }, indent=1, sort_keys=True) + '\n')
  return path


def build(table, cpu_model, prebuilt=True):
  """Give back the table and the stubs as one blob, with the symbol addresses the emulator writes through.

  The checked in blob under prebuilt/ is taken where it was built from this source, this table and this processor, so an ordinary run needs no ARM toolchain. Where
  it does not match, or is absent, the toolchain builds it and the result is cached against the same three things.
  """
  asm = _table_asm(table)
  script = _linker_script(table.table_addr)
  key = _key(asm, script, cpu_model)
  if prebuilt:
    held = _load_prebuilt(cpu_model, key, table)
    if held is not None:
      return held
  if not available():
    return None
  out = CACHE / key
  blob_path, nm_path = out / 'stubs.bin', out / 'stubs.nm'
  if not blob_path.exists():
    out.mkdir(parents=True, exist_ok=True)
    (out / 'table.s').write_text(asm)
    (out / 'stubs.ld').write_text(script)
    flags = CPU_FLAGS[cpu_model]
    subprocess.run(['arm-none-eabi-gcc', *flags, '-Os', '-ffreestanding', '-fno-builtin', '-nostdlib', '-Wall', '-Wextra', '-Werror',
                    '-Wl,-T', str(out / 'stubs.ld'), '-Wl,--build-id=none', '-Wl,--no-warn-rwx-segments', '-o', str(out / 'stubs.elf'), str(out / 'table.s'), str(SOURCE)], check=True)
    subprocess.run(['arm-none-eabi-objcopy', '-O', 'binary', str(out / 'stubs.elf'), str(blob_path)], check=True)
    nm_path.write_text(subprocess.run(['arm-none-eabi-nm', str(out / 'stubs.elf')], check=True, capture_output=True, text=True).stdout)
  symbols = {}
  for line in nm_path.read_text().splitlines():
    match = NM_RE.match(line.strip())
    if match:
      symbols[match.group(2)] = int(match.group(1), 16)
  answered = {name for name in table.by_name if name in STUBS}
  return Stubs(blob_path.read_bytes(), table.table_addr, symbols, answered)


if __name__ == '__main__':
  import sys
  import lft
  import target as targets
  if '--emit' not in sys.argv:
    raise SystemExit('usage: python3 tools/pgemu/stubs.py --emit')
  for t in targets.BY_NAME.values():
    print('wrote %s' % emit(lft.parse(t.lft_header()), t.cpu_model))

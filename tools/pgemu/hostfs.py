#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# The FatFS entry points, answered against a directory on the host.
#
# The firmware takes a FIL by address and uses two of its fields directly: f_size is (fp)->obj.objsize and f_tell is (fp)->fptr, both macros in ff_ifc.h over the
# structure in emulated memory. src/c47/c47Extensions/graphText.c uses f_size on a FIL of its own on the frame, so a handle is not always the ppgm_fp from the system data
# block and the two fields are written back after every operation.
#
# Path text arrives in the calculator's own form, which takes either separator: src/c47/saveRestoreCalcState.c has "SAVFILES\\C47auto.sav" and src/c47/config.c has
# "res/testPgms/testPgms.bin". Both are read here, a path that would leave the root is refused, and a name that does not match on a case sensitive host is looked up
# again without case, because FAT does not distinguish it.

import struct
import time
from pathlib import Path

from target import WINDOW_BASE, WINDOW_SIZE

FR_OK = 0
FR_DISK_ERR = 1
FR_NO_FILE = 4
FR_NO_PATH = 5
FR_INVALID_NAME = 6
FR_DENIED = 7
FR_EXIST = 8
FR_INVALID_OBJECT = 9
FR_WRITE_PROTECTED = 10

FA_READ = 0x01
FA_WRITE = 0x02
FA_CREATE_NEW = 0x04
FA_CREATE_ALWAYS = 0x08
FA_OPEN_ALWAYS = 0x10
FA_OPEN_APPEND = 0x30

# Field offsets in the FIL of ff_ifc.h. obj is an _FDID of twenty bytes with objsize twelve into it, then flag and err, then fptr at the next four byte boundary.
FIL_OBJSIZE = 12
FIL_FPTR = 24


# How much a read takes from the host at once. The firmware loads a state file a byte at a time, so restoring the 46 423 byte R47auto.sav arrives here as 46 423
# calls; without a block behind them every one of those is a read, a tell and two seeks on the host.
READ_BLOCK = 64 * 1024


class Handle:
  """One open file, with the block a read is served from and the size the FIL reports.

  The size is kept here rather than asked of the host at every operation, because f_size is a macro over the FIL in emulated memory and has to be right after every
  call, and seeking to the end to measure it would cost two seeks per read.
  """

  def __init__(self, path, stream, writable):
    self.path = path
    self.stream = stream
    self.writable = writable
    self.position = stream.tell()
    self.size = path.stat().st_size if path.is_file() else 0
    self.block = b''                                          # a span of the file, valid where nothing has been written since it was taken
    self.block_at = 0

  def read(self, count):
    """Take bytes from the block, refilling it from the host where the request falls outside."""
    if count <= 0 or self.position >= self.size:
      return b''
    count = min(count, self.size - self.position)
    if not (self.block_at <= self.position and self.position + count <= self.block_at + len(self.block)):
      self.stream.seek(self.position)
      self.block = self.stream.read(max(count, READ_BLOCK))
      self.block_at = self.position
    start = self.position - self.block_at
    self.position += count
    return self.block[start:start + count]

  def write(self, data):
    self.stream.seek(self.position)
    self.stream.write(data)
    self.stream.flush()
    self.position += len(data)
    self.size = max(self.size, self.position)
    self.block = b''                                          # the file has changed under it
    return len(data)

  def seek(self, offset):
    self.position = offset

  def close(self):
    self.stream.close()


class HostFs:
  """The calculator's FAT root, mapped onto one directory on the host."""

  def __init__(self, emu, root, read_only=False):
    self.emu = emu
    self.root = Path(root).resolve()
    self.read_only = read_only
    self.open_files = {}                                      # FIL address to Handle
    self.write_enabled = 0
    self.touched = {}                                         # (path, how) to the number of times, for the report
    self.windowed = 0                                         # the FIL whose file is in the window, and zero for none

  # ----- paths -----

  def cstring(self, addr, limit=512):
    """Read a NUL terminated string out of emulated memory."""
    out = bytearray()
    while len(out) < limit:
      byte = self.emu.uc.mem_read(addr + len(out), 1)[0]
      if byte == 0:
        break
      out.append(byte)
    return out.decode('utf-8', 'replace')

  def resolve(self, text):
    """Map calculator path text onto a host path under the root, or None where it would leave the root."""
    parts = [p for p in text.replace('\\', '/').split('/') if p not in ('', '.')]
    if any(p == '..' for p in parts):
      return None
    path = self.root.joinpath(*parts)
    try:
      path.resolve().relative_to(self.root)
    except ValueError:
      return None
    if path.exists() or not parts:
      return path
    return self._without_case(parts)

  def _without_case(self, parts):
    """Find the path again ignoring case, which is what FAT does and a case sensitive host does not."""
    current = self.root
    for index, part in enumerate(parts):
      if (current / part).exists():
        current = current / part
        continue
      match = next((c for c in current.iterdir() if c.name.lower() == part.lower()), None) if current.is_dir() else None
      if match is None:
        return current.joinpath(*parts[index:])               # the name as asked for, so a create lands where the firmware named it
      current = match
    return current

  # ----- the FIL in emulated memory -----

  # ----- the window the f_read stub takes bytes from -----

  def _window(self, addr, handle):
    """Put a file opened for reading into emulated memory, so the stub answers every read without reaching Python.

    Only a read-only handle takes a window, which removes every question of a write leaving it stale, and only where the file fits. The firmware loads a state file
    one byte at a time, so this is the difference between one call reaching Python and forty-six thousand.
    """
    self._clear_window()
    if self.emu.stubs is None or handle.writable or handle.size > WINDOW_SIZE:
      return
    handle.stream.seek(0)
    self.emu.uc.mem_write(WINDOW_BASE, handle.stream.read(handle.size))
    handle.stream.seek(handle.position)
    self.emu.uc.mem_write(self.emu.stubs.symbols['pgemu_win_len'], struct.pack('<I', handle.size))
    self.emu.uc.mem_write(self.emu.stubs.symbols['pgemu_win_fp'], struct.pack('<I', addr))
    self.windowed = addr

  def _clear_window(self):
    if self.emu.stubs is not None and self.windowed:
      self.emu.uc.mem_write(self.emu.stubs.symbols['pgemu_win_fp'], struct.pack('<I', 0))
    self.windowed = 0

  def _take_position(self, addr, handle):
    """Read the position back out of the FIL, because the stub advances it there without telling the host."""
    if self.windowed == addr:
      handle.position = struct.unpack('<I', self.emu.uc.mem_read(addr + FIL_FPTR, 4))[0]

  def _sync(self, addr, handle):
    """Write back the two fields the firmware uses directly, from what the handle keeps rather than from the host."""
    self.emu.uc.mem_write(addr + FIL_OBJSIZE, struct.pack('<I', handle.size))
    self.emu.uc.mem_write(addr + FIL_FPTR, struct.pack('<I', handle.position))

  # ----- the entry points -----

  def f_open(self, addr, path_addr, mode):
    text = self.cstring(path_addr)
    path = self.resolve(text)
    if path is None:
      return self._record(text, 'refused', FR_INVALID_NAME)
    wants_write = bool(mode & (FA_WRITE | FA_CREATE_NEW | FA_CREATE_ALWAYS | FA_OPEN_ALWAYS)) or (mode & FA_OPEN_APPEND) == FA_OPEN_APPEND
    if wants_write and self.read_only:
      return self._record(text, 'refused, read only', FR_WRITE_PROTECTED)
    if not path.parent.is_dir():
      return self._record(text, 'no such directory', FR_NO_PATH)
    exists = path.is_file()
    if (mode & FA_CREATE_NEW) and exists:
      return self._record(text, 'exists', FR_EXIST)
    if not exists and not (mode & (FA_CREATE_NEW | FA_CREATE_ALWAYS | FA_OPEN_ALWAYS)) and (mode & FA_OPEN_APPEND) != FA_OPEN_APPEND:
      return self._record(text, 'missing', FR_NO_FILE)
    try:
      if mode & FA_CREATE_ALWAYS or not exists:
        stream = open(path, 'w+b')
      else:
        stream = open(path, 'r+b' if wants_write else 'rb')
    except OSError:
      return self._record(text, 'refused by the host', FR_DENIED)
    self.f_close(addr)
    handle = Handle(path, stream, wants_write)
    if (mode & FA_OPEN_APPEND) == FA_OPEN_APPEND:
      handle.seek(handle.size)
    self.open_files[addr] = handle
    self._window(addr, handle)
    self._sync(addr, handle)
    return self._record(text, 'written' if wants_write else 'read', FR_OK)

  def f_close(self, addr):
    if self.windowed == addr:
      self._clear_window()
    handle = self.open_files.pop(addr, None)
    if handle is not None:
      handle.close()
    return FR_OK

  def f_read(self, addr, buffer, count, read_addr):
    handle = self.open_files.get(addr)
    if handle is None:
      return FR_INVALID_OBJECT
    self._take_position(addr, handle)
    data = handle.read(count)
    if data:
      self.emu.uc.mem_write(buffer, data)
    if read_addr:
      self.emu.uc.mem_write(read_addr, struct.pack('<I', len(data)))
    self._sync(addr, handle)
    return FR_OK

  def f_write(self, addr, buffer, count, written_addr):
    handle = self.open_files.get(addr)
    if handle is None:
      return FR_INVALID_OBJECT
    if not handle.writable:
      return FR_DENIED
    self._take_position(addr, handle)
    data = bytes(self.emu.uc.mem_read(buffer, count)) if count else b''
    handle.write(data)
    if written_addr:
      self.emu.uc.mem_write(written_addr, struct.pack('<I', len(data)))
    self._sync(addr, handle)
    return FR_OK

  def f_lseek(self, addr, offset):
    handle = self.open_files.get(addr)
    if handle is None:
      return FR_INVALID_OBJECT
    handle.seek(offset)
    self._sync(addr, handle)
    return FR_OK

  def f_unlink(self, path_addr):
    text = self.cstring(path_addr)
    path = self.resolve(text)
    if path is None:
      return self._record(text, 'refused', FR_INVALID_NAME)
    if self.read_only:
      return self._record(text, 'refused, read only', FR_WRITE_PROTECTED)
    if not path.exists():
      return self._record(text, 'missing', FR_NO_FILE)
    path.unlink()
    return self._record(text, 'deleted', FR_OK)

  def f_rename(self, old_addr, new_addr):
    old_text, new_text = self.cstring(old_addr), self.cstring(new_addr)
    old, new = self.resolve(old_text), self.resolve(new_text)
    if old is None or new is None:
      return self._record(old_text, 'refused', FR_INVALID_NAME)
    if self.read_only:
      return self._record(old_text, 'refused, read only', FR_WRITE_PROTECTED)
    if not old.exists():
      return self._record(old_text, 'missing', FR_NO_FILE)
    old.rename(new)
    return self._record('%s to %s' % (old_text, new_text), 'renamed', FR_OK)

  def file_size(self, path_addr):
    path = self.resolve(self.cstring(path_addr))
    return path.stat().st_size if path is not None and path.is_file() else -1

  def check_create_dir(self, path_addr):
    text = self.cstring(path_addr)
    path = self.resolve(text)
    if path is None:
      return FR_INVALID_NAME
    if path.is_dir():
      return FR_OK
    if self.read_only:
      return self._record(text, 'refused, read only', FR_WRITE_PROTECTED)
    path.mkdir(parents=True, exist_ok=True)
    return self._record(text, 'directory created', FR_OK)

  def make_date_filename(self, out_addr, dir_addr, ext_addr):
    """Build the dated name DMCP gives a capture, which is what the .bmp and .TSV files in the tree are named."""
    now = time.localtime()
    folder = self.cstring(dir_addr)
    separator = '' if folder.endswith(('/', '\\')) or not folder else '\\'
    name = '%s%s%04d%02d%02d-%02d%02d%02d00%s' % (folder, separator, now.tm_year, now.tm_mon, now.tm_mday,
                                                  now.tm_hour, now.tm_min, now.tm_sec, self.cstring(ext_addr))
    self.emu.uc.mem_write(out_addr, name.encode('utf-8') + b'\0')
    return None

  def _record(self, text, what, result):
    """Keep one line per path and how it was reached, with a count, because a program logging a row per step opens the same file hundreds of times."""
    key = (text, what)
    self.touched[key] = self.touched.get(key, 0) + 1
    return result

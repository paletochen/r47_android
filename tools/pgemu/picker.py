#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# The host's own file chooser, for the DMCP entry point the firmware opens whenever a function needs a file named: LOADST, SAVEST, LOADP, WRITEP, and the register
# import and export among them. file_selection_screen in src/c47-gtk/hal/io.c opens GtkFileChooserNative for the same purpose, with the same save or open action,
# the same starting folder and the same extension, so the simulator and the emulator put the same question to the same person.
#
# Nothing about the emulated file system changes. What comes back is one path under the mapped root, turned into the calculator's own DIR\NAME.EXT text, which is
# what --choose supplies by hand and what the firmware then opens through f_open.
#
# Each desktop has its own way of asking. macOS answers through osascript, which is always present; a Linux desktop through zenity or kdialog, whichever is installed;
# Windows through the dialog in System.Windows.Forms. Where none of them is present nothing is opened and the caller is told, so a run on a machine without a desktop
# behaves as it did before rather than stopping on a dialog nobody can answer.

import subprocess
import shutil
import sys


TIMEOUT = 300                                                 # a person is at the keyboard, so the wait is long, but a forgotten dialog does not stop a run for ever


def toolkit():
  """Name the chooser this machine offers, or None where it offers none."""
  if sys.platform == 'darwin':
    return 'osascript'
  if sys.platform == 'win32':
    return 'powershell'
  for name in ('zenity', 'kdialog'):
    if shutil.which(name):
      return name
  return None


def choose(title, folder, ext, saving, suggested):
  """Open the chooser and give back the absolute path chosen, or None where it was cancelled or no chooser is present.

  title is the firmware's own wording, folder the directory the firmware asks for, ext the extension with its dot, saving True for a name that need not exist yet
  and suggested the name to offer for that case.
  """
  which = toolkit()
  if which is None:
    return None
  build = {'osascript': _mac, 'zenity': _zenity, 'kdialog': _kdialog, 'powershell': _windows}[which]
  try:
    done = subprocess.run(build(title, folder, ext, saving, suggested), capture_output=True, text=True, timeout=TIMEOUT)
  except (OSError, subprocess.TimeoutExpired):
    return None
  path = done.stdout.strip()
  return path if done.returncode == 0 and path else None      # every one of the four exits non zero on cancel


def _quoted(text):
  """One AppleScript string literal."""
  return '"%s"' % text.replace('\\', '\\\\').replace('"', '\\"')


def _mac(title, folder, ext, saving, suggested):
  verb = 'choose file name' if saving else 'choose file'
  name = ' default name %s' % _quoted(suggested) if saving and suggested else ''
  script = 'POSIX path of (%s with prompt %s%s default location POSIX file %s)' % (verb, _quoted(title), name, _quoted(folder))
  # The dialog belongs to a process nobody is looking at, so it is put in front of the browser. A bare activate does that too and measures 2.1 s against 0.35 s for this
  # one, because it registers osascript itself as an application with a user interface.
  return ['osascript', '-e', 'tell application "System Events"', '-e', 'activate', '-e', script, '-e', 'end tell']


def _zenity(title, folder, ext, saving, suggested):
  start = '%s/%s' % (folder, suggested if saving else '')
  args = ['zenity', '--file-selection', '--title=%s' % title, '--filename=%s' % start, '--file-filter=*%s' % ext]
  return args + ['--save', '--confirm-overwrite'] if saving else args


def _kdialog(title, folder, ext, saving, suggested):
  where = '%s/%s' % (folder, suggested) if saving else folder
  return ['kdialog', '--getsavefilename' if saving else '--getopenfilename', where, '*%s' % ext, '--title', title]


def _windows(title, folder, ext, saving, suggested):
  kind = 'SaveFileDialog' if saving else 'OpenFileDialog'
  script = ('Add-Type -AssemblyName System.Windows.Forms;'
            '$d = New-Object System.Windows.Forms.%s;'
            '$d.Title = %s; $d.InitialDirectory = %s; $d.FileName = %s;'
            "$d.Filter = 'calculator files (*%s)|*%s|every file (*.*)|*.*';"
            "if($d.ShowDialog() -eq 'OK'){ Write-Output $d.FileName } else { exit 1 }"
            % (kind, _powershell(title), _powershell(folder), _powershell(suggested), ext, ext))
  return ['powershell', '-NoProfile', '-STA', '-Command', script]


def _powershell(text):
  return "'%s'" % text.replace("'", "''")

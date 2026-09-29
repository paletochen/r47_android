#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# The key script, written in the vocabulary res/SCRIPTS/cli_automation_examples.txt already uses for the simulator's press command.
#
# That file spells the keyboard path as F1..F6, @f and @g for the shifts, @k NN for a physical key and a single character for a typed one, and its worked examples
# are the tree's record of what drives the calculator. The same spelling is read here, so a sequence written for t47 runs unchanged.
#
# @k NN is the DMCP key code less one, which its own example states: "@k 05 = XEQ" against KEY_XEQ 6 in dmcp.h, and "21 8 14 = S I N" against the letters on the
# divide, roll down and change sign keys, which are 22, 9 and 15.
#
# Three commands have no counterpart there, because the simulator has no need of them. wait lets the firmware settle where it acts after the last key,
# which is what turning the calculator off needs; snap captures at that point in the sequence rather than at every refresh; and mark lays the stack pattern again, so what
# --stack-watermark reports is the depth of what follows and not of the whole run. mark is the STCKGO 1 of src/c47/memory.c, written from outside.

import re

# The two models put the letters on different keys, so a layout is chosen from the name in the image's own prog_info.
#
# C47 is the bezel: A on the sigma plus key through Z on the 3. Every figure here is verified by running it, XEQ XEQ C H S ENTER on a 3 giving -3 among them.
#
# R47 comes from convertKeyCode in src/c47/c47.c, which turns the DMCP code into the internal one, and from alpha_upper_transl above it, which names what each internal
# number does. Both were read wrongly at first, so every figure here was then run on R47.pg5 and the screen read: DMCP 1 turns 100 into 10000, 5 into 2 and 6 into 4.605,
# so those are x squared, LOG and LN; 10 tags the angle, so it is DRG; 11 opens the XEQ prompt; 12 and 28 raise the g and the f annunciator; 15 starts an exponent and 16
# negates, so 15 is EEX and 16 is CHS.
#
# The letters do not follow the functions. The same two keys give M on 15 and L on 16, which is what alpha_upper_transl takes straight from the DMCP code while the
# functions come from the converted one. Laid out on the bezel the letters are in order anyway, K then L then M across the row, because the two codes are what is
# exchanged and not the keys.
#
# The digits are the same on both, and so is ENTER, EXIT and the arithmetic.


class Layout:
  def __init__(self, name, letters, shift, xeq, up, down, chs, eex):
    self.name = name
    self.letters = dict(zip('ABCDEFGHIJKLMNOPQRSTUVWXYZ', letters))
    self.shift = shift
    self.named = dict(NAMED_COMMON, XEQ=xeq, UP=up, DOWN=down, CHS=chs, EEX=eex)


NAMED_COMMON = {
  'ENTER': 13,
  'R/S':   36,
  'EXIT':  33,
  'ON':    33,                                                # ON, EXIT and OFF are one key, and OFF is the shifted one
  'BSP':   17,
  'ADD':   37,
  'SUB':   32,
  'MUL':   27,
  'DIV':   22,
  'DOT':   35,
}

DIGITS = dict(zip('0123456789', (34, 29, 30, 31, 24, 25, 26, 19, 20, 21)))

C47 = Layout('C47', (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 16, 19, 20, 21, 22, 24, 25, 26, 27, 29, 30, 31),
             shift=28, xeq=6, up=18, down=23, chs=15, eex=16)
# A to Z. The three in the middle are K, M then L in alpha_upper_transl and not in alphabetical order, so L is on 16 and M on 15, which a run confirms.
R47 = Layout('R47', (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 14, 16, 15, 19, 20, 21, 22, 24, 25, 26, 27, 29, 30, 31, 32, 34),
             shift=28, xeq=11, up=18, down=23, chs=16, eex=15)

BY_NAME = {'C47': C47, 'R47': R47}

KEY_RE = re.compile(r'^@k\s*(\d+)$')

# A step is one of these. A key press, an idle wait, or a capture.
PRESS, WAIT, SNAP, MARK = 'press', 'wait', 'snap', 'mark'


class ScriptError(Exception):
  pass


def parse(text, layout=C47):
  """Read a script into steps. Commands are separated by a semicolon or a newline, and a hash starts a comment."""
  steps = []
  for line in text.replace(';', '\n').splitlines():
    word = line.split('#', 1)[0].strip()
    if not word:
      continue
    steps += _step(word, layout)
  return steps


def _step(word, layout):
  head, _, argument = word.partition(' ')
  argument = argument.strip()
  if head == 'wait':
    return [(WAIT, int(argument) if argument else 20)]
  if head == 'snap':
    return [(SNAP, argument or None)]
  if head == 'mark':
    return [(MARK, argument or None)]
  if head == 'press':
    word = argument
  return [(PRESS, key) for key in _keys(word, layout)]


def _keys(word, layout):
  """Turn one press argument into its DMCP codes."""
  if word == '@f':
    return [layout.shift]
  if word == '@g':
    return [layout.shift, layout.shift]                                     # the shift is one key that takes two presses to reach the second set
  match = KEY_RE.match(word)
  if match:
    code = int(match.group(1)) + 1                            # @k NN is the DMCP code less one
    if not 1 <= code <= 43:
      raise ScriptError('@k %s is outside the key table' % match.group(1))
    return [code]
  if re.fullmatch(r'F[1-6]', word):
    return [37 + int(word[1])]                                # F1 is 38
  if word.upper() in layout.named:
    return [layout.named[word.upper()]]
  if len(word) == 1 and word.upper() in layout.letters:
    return [layout.letters[word.upper()]] if word.isupper() else [layout.shift, layout.letters[word.upper()]]
  if len(word) == 1 and word in DIGITS:
    return [DIGITS[word]]
  if len(word) > 1 and all(c in DIGITS for c in word):
    return [DIGITS[c] for c in word]                          # a run of digits types itself, so 30 is two presses
  raise ScriptError('%s is not a key' % word)

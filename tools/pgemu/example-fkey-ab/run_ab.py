#!/usr/bin/env python3
# Worked example: an A/B of the f and g shift glyph on two DM42 images, the way patch 7be1f2250 was checked.
# Runs nine key sequences on each image with fixed clocks, then compares the frame digests and the final capture of every pair,
# checks the f capture against the g capture as a control, and prints the top-left corner of both f captures.
#
#   python3 tools/pgemu/example-fkey-ab/run_ab.py <pre>/C47.pgm <post>/C47.pgm [outdir]
#
# Each image needs its C47_qspi.bin beside it. outdir defaults to tools/pgemu/out-fkey-ab, which .gitignore leaves out.
# Run it outside any sandbox: Unicorn is killed inside one, exit 132.
import filecmp
import os
import re
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PGEMU = os.path.join(os.path.dirname(HERE), 'pgemu.py')

# name, key script. f EXIT is OFF on the C47, so that sequence ends before its snap and is compared on its frames alone.
SEQUENCES = [
  ('f',     'ENTER; wait 60; @f; wait; snap end'),
  ('g',     'ENTER; wait 60; @g; wait; snap end'),
  ('off',   'ENTER; wait 60; @f; @f; @f; wait; snap end'),
  ('ff',    'ENTER; wait 60; @f; wait; @f; wait; snap end'),
  ('tamf',  'ENTER; wait 60; @k 06; wait; @f; wait; snap end'),
  ('tamg',  'ENTER; wait 60; @k 06; wait; @g; wait; snap end'),
  ('typef', 'ENTER; wait 60; 1; 2; 3; @f; wait; snap end'),
  ('fexit', 'ENTER; wait 60; @f; wait; EXIT; wait; snap end'),
  ('fsin',  'ENTER; wait 60; 1; @f; @k 09; wait; snap end'),
]


def run(image, side, name, script, outdir):
  frames = os.path.join(outdir, 'frames-%s-%s.txt' % (side, name))
  capdir = os.path.join(outdir, 'out-%s-%s' % (side, name))
  report = os.path.join(outdir, 'report-%s-%s.txt' % (side, name))
  os.makedirs(capdir, exist_ok=True)
  args = [sys.executable, PGEMU, image, '--ms-per-tick', '1', '--clock', '2026-10-01 12:00', '--frames', frames, '--out', capdir, '--press', script]
  with open(report, 'w') as f:
    r = subprocess.run(args, stdout=f, stderr=subprocess.STDOUT)
  if r.returncode == 132:
    sys.exit('pgemu exit 132: Unicorn was killed, so this ran inside a sandbox. Run it outside one.')
  if r.returncode != 0:
    sys.exit('pgemu exit %d on %s %s, see %s' % (r.returncode, side, name, report))
  return frames, os.path.join(capdir, 'end.bmp'), report


def prog_info_bytes(report):
  m = re.search(r'^prog_info .*?, (\d+) bytes', open(report).read(), re.M)
  return int(m.group(1)) if m else None


def corner(path):
  b = open(path, 'rb').read()
  off = struct.unpack_from('<I', b, 10)[0]
  w, h = struct.unpack_from('<ii', b, 18)
  stride = ((w + 31) // 32) * 4
  ones = sum(bin(c).count('1') for c in b[off:off + stride * abs(h)])
  ink = 1 if ones < stride * 8 * abs(h) // 2 else 0  # a screen is mostly empty, so the minority bit is the ink; pgemu and t47 captures differ in polarity
  rows = []
  for y in range(16):
    base = off + (h - 1 - y) * stride
    rows.append(''.join('#' if (b[base + x // 8] >> (7 - x % 8)) & 1 == ink else '.' for x in range(24)))
  return rows


def main():
  if len(sys.argv) < 3:
    sys.exit(__doc__ or 'usage: run_ab.py <pre>/C47.pgm <post>/C47.pgm [outdir]')
  pre, post = sys.argv[1], sys.argv[2]
  outdir = sys.argv[3] if len(sys.argv) > 3 else os.path.join(os.path.dirname(HERE), 'out-fkey-ab')
  os.makedirs(outdir, exist_ok=True)

  results = {}
  for name, script in SEQUENCES:
    results[name] = (run(pre, 'pre', name, script, outdir), run(post, 'post', name, script, outdir))

  a = prog_info_bytes(results['f'][0][2])
  b = prog_info_bytes(results['f'][1][2])
  print('prog_info  pre %s bytes, post %s bytes' % (a, b))
  if a == b:
    print('the two images have the same size: check they are the two builds you meant before reading the lines below')

  bad = 0
  total = 0
  for name, _ in SEQUENCES:
    (fp, bp, _), (fq, bq, _) = results[name]
    n = sum(1 for _ in open(fp))
    total += n
    fr = 'same' if filecmp.cmp(fp, fq, shallow=False) else 'DIFF'
    if os.path.exists(bp) and os.path.exists(bq):
      cap = 'same' if filecmp.cmp(bp, bq, shallow=False) else 'DIFF'
    elif not os.path.exists(bp) and not os.path.exists(bq):
      cap = 'none in either run'
    else:
      cap = 'DIFF, one run only'
    if fr != 'same' or cap.startswith('DIFF'):
      bad = 1
    print('%-6s frames %-4s (%d)  capture %s' % (name, fr, n, cap))
  print('refreshes compared: %d' % total)

  fpre = results['f'][0][1]
  gpre = results['g'][0][1]
  ctl = not filecmp.cmp(fpre, gpre, shallow=False)
  print('control, the f and g captures differ: %s' % ('yes' if ctl else 'NO, the glyph never reached the panel'))
  for side, path in (('pre', fpre), ('post', results['f'][1][1])):
    print('top-left corner of the f capture, %s:' % side)
    print('\n'.join(corner(path)))
  print('pre and post agree on every sequence' if bad == 0 and ctl else 'DIFFERENCES or a failed control above')
  return 0 if bad == 0 and ctl else 1


if __name__ == '__main__':
  sys.exit(main())

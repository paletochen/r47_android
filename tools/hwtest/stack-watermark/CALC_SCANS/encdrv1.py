#!/usr/bin/env python3
# Encode FNKIND1.txt and patch in the steps rejig cannot write.
# rejig has no PGMDRV and no f', so the listing is encoded with PGMSLV and SOLVE, which take the same
# operand form, and the opcodes are put back afterwards. The file also carries genuine PGMSLV and SOLVE
# steps, case 37 among them, so the patch cannot simply replace every one: the listing is read in order
# to say which of them is a stand-in, and the byte stream is walked in the same order.
# Delete all of this the day rejig can encode PGMDRV and f' itself.
#
# The same method as _Project_Improvements/derivplot/encdrv.py, which builds AN0022/Deriv/func1.p47.

import pathlib
import subprocess

BASE  = pathlib.Path(__file__).resolve().parent
REPO  = BASE.parent.parent.parent.parent
REJIG = str(REPO / "rejig")

PGMSLV = [134, 11]    # item 1547, label operand
SOLVE  = [134, 72]    # item 1608, variable operand
PGMDRV = [139, 66]    # item 2882, label operand
F1DRV  = [139, 67]    # item 2883, variable operand
LBLK35 = [1, 253, 3, ord('K'), ord('3'), ord('5')]

src = (BASE / "FNKIND1.txt").read_text()

# Which of the four step words each line carries, in listing order, from LBL 'K35' on. A stand-in is a
# line the listing writes as PGMDRV or f'; the other two are the real thing and are left alone.
start = src.index("LBL 'K35'")
order = []
for line in src[start:].split("\n"):
  step = line.split(";")[0]
  for word, standin in (("PGMDRV '", True), ("f' '", True), ("PGMSLV '", False), ("SOLVE '", False)):
    if word in step:
      order.append((word, standin))

enc = src.replace("PGMDRV '", "PGMSLV '").replace("f' '", "SOLVE '")
if enc == src:
  raise SystemExit("nothing replaced: the listing does not carry the derivative steps")
(BASE / "_fnkind1_enc.txt").write_text(enc)

out = BASE / "PROGRAMS" / "FNKIND1.p47"
subprocess.run([REJIG, str(BASE / "_fnkind1_enc.txt"), "-o", str(out)], check=True)
(BASE / "_fnkind1_enc.txt").unlink()

lines = out.read_text().split("\n")
head  = lines[:6]
body  = [int(x) for x in lines[6:] if x.strip() != ""]

k35 = -1
for k in range(len(body) - len(LBLK35) + 1):
  if body[k:k + len(LBLK35)] == LBLK35:
    k35 = k
    break
if k35 < 0:
  raise SystemExit("LBL 'K35' not found")

seen = 0
n    = 0
k    = k35
while k < len(body) - 1:
  op = body[k:k + 2]
  if op == PGMSLV or op == SOLVE:
    if seen >= len(order):
      raise SystemExit("more opcodes in the file than steps in the listing")
    word, standin = order[seen]
    if standin:
      body[k:k + 2] = PGMDRV if op == PGMSLV else F1DRV
      n += 1
    seen += 1
  k += 1
print("LBL 'K35' at %d, %d steps in the listing, %d patched, %d bytes" % (k35, seen, n, len(body)))

if seen != len(order):
  raise SystemExit("listing has %d steps, file has %d" % (len(order), seen))
if len(body) != int(head[5]):
  raise SystemExit("byte count changed")
out.write_text("\n".join(head + [str(x) for x in body]) + "\n")

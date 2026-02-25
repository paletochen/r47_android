#!/usr/bin/env python

# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors

from math import lcm
from findiff import coefficients

max_order = 7
max_f_eval = 2 * max_order + 1

print('''
// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

#define MAX_ORDER   %d
#define MAX_F_EVAL  (2 * MAX_ORDER + 1)

typedef struct {
    const char *const desc;
    uint8_t n;      // Number of steps from the x, each step is by h in either direction
    uint8_t order;  // Order of the derivative (1 or 2 only at present)
    uint8_t denom; // Denominator of the finite different equation
    uint8_t coeff[/*2*n+1*/]; // Coefficients for each function evaluation point
} FINITE_DIFF_COEFF;

''' % max_order)

values = [ 0 ]
def getValIdx(n):
    try:
      pos = values.index(n)
    except ValueError:
      pos = len(values)
      values.append(n)
    return pos

masterTbl = []

def emitOne(order, offsets):
  if len(offsets) <= order:
    return False
  coeffs = coefficients(deriv=order, offsets=offsets, symbolic=True)['coefficients']
  d = [f.denominator for f in coeffs]
  denom = lcm(*d)
  n = max(*[abs(x) for x in offsets ])
  hasNegative = False
  hasPositive = False
  hasZero = False
  hasN = 0
  for i in range(len(offsets)):
    if coeffs[i] != 0:
      e = offsets[i]
      hasNegative = hasNegative or (e < 0)
      hasPositive = hasPositive or (e > 0)
      hasZero = hasZero or (e == 0)
      hasN = hasN + 1

  numOffsets = len(offsets)
  name = f"d{order}_{numOffsets}"
  priority = numOffsets * 10
  if hasNegative and hasPositive:
    priority = priority + 8
    name = name + "_central"
  else:
    if hasNegative:
      priority = priority + 3
      name = name + "_lower"
    if hasPositive:
      priority = priority + 2
      name = name + "_upper"
    if hasZero:
      priority = priority + 2
      name = name + "_middle"
  print("TO_QSPI static const FINITE_DIFF_COEFF %s = {" % name)
  print('  "%s", %d, %d, %d, { ' % (name, n, order, getValIdx(denom)), end='')
  sof = ''
  for i in range(-n, n+1):
    try:
      pos = offsets.index(i)
      val = coeffs[pos]
    except ValueError:
      val = 0
    print("%s%d" % ( sof, getValIdx(val * denom)), end='')
    sof = ', '
  print(" }\n};\n")
  return (priority, name)

def emit(name, order, tuples):
  res = []
  for tup in tuples:
    t = emitOne(order, tup)
    if t != False:
      res.append(t)
  return res

def emit_stencils(name, stents):
  n = f"{name}_derivatives"
  masterTbl.append(n)
  print("TO_QSPI static const FINITE_DIFF_COEFF *const %s[] = {" % n, end='')
  stents.sort(reverse=True, key=lambda z: z[0])
  for i in range(len(stents)):
    if (i % 4 == 0):
      print("\n  ", end='')
    s = stents[i]
    print('&%s, ' % s[1], end='')
  print("NULL\n};\n")

stencils = []
for i in range(1, max_order + 1):
    l = [0]
    for j in range(1, i+1):
      l.insert(0, -j)
      l.append(j)
    stencils.append(l)          # central
    stencils.append(l[:i+1])    # lower including centre
    stencils.append(l[i:])      # upper including centre
    stencils.append(l[:i])      # lower sans centre
    stencils.append(l[i+1:])    # upper sans centre

first_stencils = emit("first", 1, stencils)
second_stencils = emit("second", 2, stencils)

emit_stencils("d1", first_stencils)
emit_stencils("d2", second_stencils)

print("TO_QSPI static const FINITE_DIFF_COEFF *const *const finite_difference_table[] = {")
for t in masterTbl:
  print(f"  {t}, ")
print("};\n")

nVal = len(values)
print("TO_QSPI static const int32_t fdValues[%d] = {" % nVal, end='')
for v in range(nVal):
  if v % 8 == 0:
    print("\n", end='')
  print("%10d," % values[v], end='')
print("\n};\n")
  

#!/bin/sh
# Run the work steps of each case on its own in the simulator and print what it left in register X, so
# one failing case cannot hide the next. This is the check that a case computes, which a run without an
# error does not show. Expected values are in README.txt.
#
#   sh probe_cases.sh <folder holding t47 and FNKVFY.p47> [first case] [last case]
#
# The folder needs t47 and res beside it, and no backup.cfg, so the state is the factory one.
usage() {
  echo "usage: sh probe_cases.sh <folder holding t47 and FNKVFY.p47> [first] [last]" >&2
  exit 1
}
[ -n "$1" ] || usage
cd "$1" || usage
[ -x ./t47 ] || usage
[ -f FNKVFY.p47 ] || usage
first=${2:-1}
last=${3:-40}
n=$first
while [ "$n" -le "$last" ]; do
  nn=$(printf '%02d' "$n")
  out=$(./t47 --reset --exec "readp FNKVFY.p47; xeq VSET; xeq V$nn; puts \"RESULT X=[reg X] Y=[reg Y]\"" 2>&1 \
        | grep -v -i 'gdk\|xpc\|connection invalid\|hiservices')
  err=$(printf '%s\n' "$out" | grep -i -m1 'error\|not found\|cannot\|invalid\|too deep\|undefined\|no such\|non-programmable')
  res=$(printf '%s\n' "$out" | grep -m1 '^RESULT ')
  printf '%s  %-64.64s %s\n' "$nn" "$res" "$err"
  n=$((n + 1))
done

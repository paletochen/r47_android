#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# Generate PROGRAMS/BWALK.p47, the run-cycle program for the vbat sampling
# schedule hardware proof (MR !1611, see README.md). Reuses the encoder in
# tools/bench/genbench.py; byte encoding and .p47 container are identical to
# the benchmark programs.
#
# BWALK is one cycle of the proof: a tight DSZ/GTO loop long enough for the
# vbatSampleDelay schedule to saturate (>= ~7 s of dispatching), then STOP.
# The MONITOR_VBAT_SCHEDULE harness in the firmware injects R/S after each
# stop, which resumes at the GTO 'BWALK' step and starts the next cycle as a
# genuine top-level run. N below only needs "long enough"; hardware speed
# differences just stretch the saturated tail of each cycle.
#
# Usage:
#   python3 tools/hwtest/vbat-walk/genbwalk.py           regenerate PROGRAMS/
#   python3 tools/hwtest/vbat-walk/genbwalk.py --check   verify checked-in files match

import argparse
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parents[1] / "bench"))
from genbench import named, op, reg, lit_int, render  # noqa: E402

OUT_DIR = HERE / "PROGRAMS"

ITERATIONS = 10000  # ~14 s per cycle on a battery powered DM42n; anything past schedule saturation (~7 s) only adds saturated samples
COUNTER_REG = 99    # bench convention: R99 inner loop counter


def bwalk():
    b = []
    b += named("ITM_LBL", "BWALK")
    b += lit_int(ITERATIONS)
    b += reg("ITM_STO", COUNTER_REG)
    b += reg("ITM_LBL", 0)
    b += reg("ITM_DSZ", COUNTER_REG)
    b += reg("ITM_GTO", 0)
    b += op("ITM_STOP")
    b += named("ITM_GTO", "BWALK")
    b += op("ITM_END")
    return b


LISTING = """LBL 'BWALK'
  10000
  STO R99
  LBL 00
    DSZ R99
    GTO 00
  STOP
  GTO 'BWALK'
  END
"""


def main():
    parser = argparse.ArgumentParser(description="Generate tools/hwtest/vbat-walk/PROGRAMS/BWALK.p47")
    parser.add_argument("--check", action="store_true", help="verify checked-in files match the generator")
    args = parser.parse_args()

    files = {"BWALK.p47": render(bwalk()), "BWALK.txt": LISTING}
    if args.check:
        stale = [n for n, c in files.items() if not (OUT_DIR / n).exists() or (OUT_DIR / n).read_text() != c]
        if stale:
            print("stale or missing: " + " ".join(stale))
            print("run: python3 tools/hwtest/vbat-walk/genbwalk.py")
            return 1
        print("BWALK up to date")
        return 0

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for name, content in sorted(files.items()):
        (OUT_DIR / name).write_text(content)
        print(f"wrote {OUT_DIR / name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

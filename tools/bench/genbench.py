#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# Generate the benchmark suite programs in tools/bench/PROGRAMS/.
#
# Each benchmark isolates one cost axis of the program interpreter and times
# itself with a TICKS bracket (the pattern NQueens.p47 already uses), so the
# same program is measurable on the calculator (elapsed ticks in X and in
# R97) and in the t47 headless simulator (microsecond wall clock around xeq,
# ticks via "reg 97"). The iteration count N of every benchmark is FIXED
# here: hardware and simulator runs stay comparable only as long as N never
# changes, so treat the N values as frozen once a calibration file has been
# published (see tools/bench/README.md). The counts were sized from a real
# DM42n measurement (BMGTO: ~950 iterations/s on USB power) so that each
# benchmark runs long enough for <1% TICKS quantization (measured worst
# case 116 ticks); the whole suite measured about 3.5 minutes on USB power
# and 7.5 minutes on battery.
#
# Running state is visible on the calculator: every benchmark splits its
# loop into CHUNKS outer chunks and VIEWs the chunk countdown (R95) at each
# boundary, so the display changes steadily during a run; the BENCH driver
# shows each benchmark's name for a second before starting it, stores the
# index of the benchmark it is about to run in R89, and ends with BEEP and
# the string DONE in X. The driver also pre-fills every result register
# with the sentinel 999999: after an interrupted or failed run, registers
# still holding the sentinel name the benchmarks that never finished, and
# R89 names the one that was running. The chunk-boundary VIEW refreshes are
# inside the timed bracket (~1% of a hardware run) - identical structure on
# every platform, so comparisons stay honest.
#
# Harness register convention (documented in the README):
#   R99 inner loop counter   R98 start ticks   R97 elapsed ticks
#   R96 loop canary          R95 chunk countdown
#   R89 index of the benchmark the driver is running (1..8, 9 = done)
#   R80..R87 per-benchmark results stored by the BENCH driver program
#   R10..R14 benchmark operands
#
# Item numbers are parsed from src/c47/items.h at generation time, and the
# program byte encoding follows src/c47/programming/nextStep.c
# (countOpBytes) and the examples in src/testSuite/testSuite.c
# (covWriteAndLoadPgm): items < 128 are one byte, items >= 128 are
# (n >> 8) | 0x80 followed by n & 0xff; a numbered-register or numeric
# parameter is a single byte 0-99; a named label/variable parameter is
# 253, length, bytes; a literal is ITM_LITERAL, type, length, ASCII
# (type 8 = long integer, type 9 = real34, type 253 = alpha string).
#
# Usage:
#   python3 tools/bench/genbench.py           regenerate tools/bench/PROGRAMS/
#   python3 tools/bench/genbench.py --check   verify checked-in files match

import argparse
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
ITEMS_H = REPO / "src" / "c47" / "items.h"
OUT_DIR = REPO / "tools" / "bench" / "PROGRAMS"

STRING_LABEL_VARIABLE = 253
STRING_LONG_INTEGER = 8
STRING_REAL34 = 9

INNER_COUNTER_REG = 99
START_REG = 98
RESULT_REG = 97
CANARY_REG = 96
CHUNK_REG = 95
DRIVER_INDEX_REG = 89
DRIVER_RESULT_BASE = 80
DRIVER_SENTINEL = 999999
CHUNKS = 10
BANNER_TENTHS = 10  # driver shows each benchmark's name for 1.0 s


def load_items():
    items = {}
    for m in re.finditer(r"#define\s+(ITM_\w+)\s+(\d+)", ITEMS_H.read_text()):
        items[m.group(1)] = int(m.group(2))
    return items


ITEMS = load_items()


def op(name):
    n = ITEMS[name]
    return [n] if n < 128 else [(n >> 8) | 0x80, n & 0xff]


def named(opname, s):
    return op(opname) + [STRING_LABEL_VARIABLE, len(s)] + [ord(c) for c in s]


def reg(opname, r):
    return op(opname) + [r]


def lit_int(value):
    s = str(value)
    return op("ITM_LITERAL") + [STRING_LONG_INTEGER, len(s)] + [ord(c) for c in s]


def lit_real(s):
    return op("ITM_LITERAL") + [STRING_REAL34, len(s)] + [ord(c) for c in s]


def lit_str(s):
    return op("ITM_LITERAL") + [STRING_LABEL_VARIABLE, len(s)] + [ord(c) for c in s]


def harness(label, n, setup, body):
    # LBL "label" / setup / TICKS STO 98 /
    # CHUNKS STO 95 / LBL 01 / VIEW 95 / N/CHUNKS STO 99 / LBL 00 / body /
    # DSZ 99 GTO 00 / DSZ 95 GTO 01 / TICKS RCL 98 - STO 97 / RTN / END
    # The VIEW at each chunk boundary is the running-state indicator: the
    # display counts R95 down from CHUNKS to 1 while the benchmark runs.
    assert n % CHUNKS == 0, f"{label}: N={n} not divisible by {CHUNKS}"
    b = []
    b += named("ITM_LBL", label)
    b += setup
    b += op("ITM_TICKS") + reg("ITM_STO", START_REG)
    b += lit_int(CHUNKS) + reg("ITM_STO", CHUNK_REG)
    b += reg("ITM_LBL", 1)
    b += reg("ITM_VIEW", CHUNK_REG)
    b += lit_int(n // CHUNKS) + reg("ITM_STO", INNER_COUNTER_REG)
    b += reg("ITM_LBL", 0)
    b += body
    b += reg("ITM_DSZ", INNER_COUNTER_REG)
    b += reg("ITM_GTO", 0)
    b += reg("ITM_DSZ", CHUNK_REG)
    b += reg("ITM_GTO", 1)
    b += op("ITM_TICKS")
    b += reg("ITM_RCL", START_REG)
    b += op("ITM_SUB")
    b += reg("ITM_STO", RESULT_REG)
    b += op("ITM_RTN")
    b += op("ITM_END")
    return b


def repeated(body, times):
    return body * times


# Iteration counts: frozen once a calibration is published. Sized from the
# measured DM42n BMGTO rate (~950 it/s, USB 160 MHz) with per-step cost
# estimates for the others; all landed at 116 ticks or more on hardware,
# keeping TICKS quantization under 1%.
ITERATIONS = {
    "BMGTO": 20000,
    "BMREG": 10000,
    "BMVAR": 5000,
    "BMRSV": 10000,
    "BMARITH": 3000,
    "BMTRIG": 500,
    "BMNQN": 2,
    "BMDISP": 2000,
}

# The decoy variables allocated before the measured variable make the
# linear scan in findNamedVariable() realistic: the measured name is
# deliberately the last one allocated, so every lookup walks the whole list.
BMVAR_DECOYS = ["AA", "AB", "AC", "AD", "AE", "AF", "AG", "AH", "AI", "AJ"]


def bmgto():
    # Dispatch + local-label GTO scan; the ISZ doubles as the loop canary
    # (R96 == N after a run proves the loop executed N times).
    setup = lit_int(0) + reg("ITM_STO", CANARY_REG)
    body = reg("ITM_ISZ", CANARY_REG)
    return harness("BMGTO", ITERATIONS["BMGTO"], setup, body)


def bmreg():
    setup = lit_real("1.5") + reg("ITM_STO", 10)
    body = repeated(reg("ITM_RCL", 10) + reg("ITM_STO", 11), 5)
    return harness("BMREG", ITERATIONS["BMREG"], setup, body)


def bmvar():
    setup = lit_real("1.5")
    for name in BMVAR_DECOYS:
        setup += named("ITM_STO", name)
    setup += named("ITM_STO", "VV")
    body = repeated(named("ITM_RCL", "VV") + named("ITM_STO", "VV"), 5)
    return harness("BMVAR", ITERATIONS["BMVAR"], setup, body)


def bmrsv():
    # FV is a reserved variable resolved through the gperf hash
    # (reservedRegisterLookup.h) - the fast path BMVAR's user variable
    # cannot take, and the variable upstream used to benchmark issue #555.
    setup = lit_real("1.5") + named("ITM_STO", "FV")
    body = repeated(named("ITM_RCL", "FV") + named("ITM_STO", "FV"), 5)
    return harness("BMRSV", ITERATIONS["BMRSV"], setup, body)


def bmarith():
    # 1.5+2.5=4, 4*2.5=10, 10/2.5=4, sqrt(4)=2: value-stable every iteration.
    setup = lit_real("1.5") + reg("ITM_STO", 10) + lit_real("2.5") + reg("ITM_STO", 11)
    once = (
        reg("ITM_RCL", 10)
        + reg("ITM_RCL", 11)
        + op("ITM_ADD")
        + reg("ITM_RCL", 11)
        + op("ITM_MULT")
        + reg("ITM_RCL", 11)
        + op("ITM_DIV")
        + op("ITM_SQUAREROOTX")
    )
    return harness("BMARITH", ITERATIONS["BMARITH"], setup, repeated(once, 2))


def bmtrig():
    # sin -> ln -> exp of a small positive value: defined in any angle mode.
    setup = lit_real("0.7") + reg("ITM_STO", 12)
    once = reg("ITM_RCL", 12) + op("ITM_sin") + op("ITM_LN") + op("ITM_EXP")
    return harness("BMTRIG", ITERATIONS["BMTRIG"], setup, repeated(once, 2))


def bmnqn():
    # Macro benchmark: real instruction mix. Calls the stock NQueens program
    # (global label NQ, argument 8 on the stack), so NQueens.p47 must be
    # loaded alongside this file; the bench runner and the README both do so.
    # NQueens VIEWs its own progress, so this benchmark is chunked per call.
    body = lit_int(8) + named("ITM_XEQ", "NQ")
    n = ITERATIONS["BMNQN"]
    b = []
    b += named("ITM_LBL", "BMNQN")
    b += op("ITM_TICKS") + reg("ITM_STO", START_REG)
    b += lit_int(n) + reg("ITM_STO", CHUNK_REG)
    b += reg("ITM_LBL", 1)
    b += reg("ITM_VIEW", CHUNK_REG)
    b += body
    b += reg("ITM_DSZ", CHUNK_REG)
    b += reg("ITM_GTO", 1)
    b += op("ITM_TICKS")
    b += reg("ITM_RCL", START_REG)
    b += op("ITM_SUB")
    b += reg("ITM_STO", RESULT_REG)
    b += op("ITM_RTN")
    b += op("ITM_END")
    return b


def bmdisp():
    # Display-path op cost. The simulator number is measured but not
    # hardware-predictive (GTK redraw vs DMA refresh); see the README.
    setup = lit_int(100) + reg("ITM_STO", 13) + lit_int(150) + reg("ITM_STO", 14)
    body = repeated(reg("ITM_RCL", 13) + reg("ITM_RCL", 14) + op("ITM_PIXEL"), 5)
    return harness("BMDISP", ITERATIONS["BMDISP"], setup, body)


BENCHMARKS = ["BMGTO", "BMREG", "BMVAR", "BMRSV", "BMARITH", "BMTRIG", "BMNQN", "BMDISP"]
GENERATORS = {
    "BMGTO": bmgto,
    "BMREG": bmreg,
    "BMVAR": bmvar,
    "BMRSV": bmrsv,
    "BMARITH": bmarith,
    "BMTRIG": bmtrig,
    "BMNQN": bmnqn,
    "BMDISP": bmdisp,
}


def benchall():
    # One-shot driver for a hardware run. Status protocol:
    # - R80..R87 are pre-filled with the sentinel 999999; a register still
    #   holding it afterwards marks a benchmark that never finished.
    # - R89 holds the index (1..8) of the benchmark currently running,
    #   9 when the whole suite completed.
    # - each benchmark's name is shown for a second before it starts,
    #   and the suite ends with BEEP and the string DONE left in X.
    b = named("ITM_LBL", "BENCH")
    b += lit_int(DRIVER_SENTINEL)
    for i in range(len(BENCHMARKS)):
        b += reg("ITM_STO", DRIVER_RESULT_BASE + i)
    for i, bm in enumerate(BENCHMARKS):
        b += lit_int(i + 1) + reg("ITM_STO", DRIVER_INDEX_REG)
        b += lit_str(bm)
        b += reg("ITM_PAUSE", BANNER_TENTHS)
        b += named("ITM_XEQ", bm)
        b += reg("ITM_RCL", RESULT_REG)
        b += reg("ITM_STO", DRIVER_RESULT_BASE + i)
    b += lit_int(len(BENCHMARKS) + 1) + reg("ITM_STO", DRIVER_INDEX_REG)
    b += op("ITM_BEEP")
    b += lit_str("DONE")
    b += op("ITM_RTN")
    b += op("ITM_END")
    return b


def render(program_bytes):
    lines = ["PROGRAM_FILE_FORMAT", "0", "C47_program_file_version", "1", "PROGRAM", str(len(program_bytes))]
    lines += [str(byte) for byte in program_bytes]
    return "\n".join(lines) + "\n"


def generate():
    files = {name + ".p47": render(gen()) for name, gen in GENERATORS.items()}
    files["BENCHALL.p47"] = render(benchall())
    return files


def main():
    parser = argparse.ArgumentParser(description="Generate tools/bench/PROGRAMS/*.p47")
    parser.add_argument("--check", action="store_true", help="verify checked-in files match the generator")
    args = parser.parse_args()

    files = generate()
    if args.check:
        stale = []
        for name, content in sorted(files.items()):
            path = OUT_DIR / name
            if not path.exists() or path.read_text() != content:
                stale.append(name)
        if stale:
            print("stale or missing: " + " ".join(stale))
            print("run: python3 tools/bench/genbench.py")
            return 1
        print(f"{len(files)} benchmark programs up to date")
        return 0

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for name, content in sorted(files.items()):
        (OUT_DIR / name).write_text(content)
        print(f"wrote {OUT_DIR / name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# Analyze a VBATWALK.CSV captured by the MONITOR_VBAT_SCHEDULE harness
# (see README.md). Verifies, per run cycle, that the vbatSampleDelay table
# was walked entry by entry and saturated on the last entry, and prints an
# aggregate table across cycles. Exit code 0 = every cycle proves the walk,
# 1 = at least one violation.
#
# Usage: python3 analyze_vbatwalk.py VBATWALK.CSV

import sys
from statistics import median

# Must mirror vbatSampleDelay[] in src/c47/config.c.
TABLE = [0, 0, 0, 50, 50, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 1000, 2500]
SATURATED_IDX = len(TABLE) - 1  # 16


def parse(path):
    rows = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#") or line.startswith("t_ms"):
                continue
            t, event, idx, delta, vbat = line.split(",")
            rows.append((int(t), event, int(idx), int(delta), int(vbat)))
    return rows


def split_cycles(rows):
    # A cycle starts at each reset_runstart and holds everything up to the
    # next one, including the idle-gap samples after its run stopped.
    cycles, current, preamble = [], None, []
    for row in rows:
        if row[1] == "reset_runstart":
            if current is not None:
                cycles.append(current)
            current = []
        elif current is None:
            preamble.append(row)
        else:
            current.append(row)
    if current is not None:
        cycles.append(current)
    return preamble, cycles


def check_cycle(num, rows):
    problems = []
    samples = [r for r in rows if r[1] == "sample"]
    idx_seq = [r[2] for r in samples]
    expected = [min(n, SATURATED_IDX) for n in range(1, len(samples) + 1)]
    if idx_seq != expected[: len(idx_seq)]:
        problems.append(f"cycle {num}: index sequence {idx_seq[:20]}... does not march 1..{SATURATED_IDX} and hold")
    if len(samples) < len(TABLE) + 1:
        problems.append(f"cycle {num}: only {len(samples)} samples, walk did not reach saturation plus a repeat")
    for n, (t, event, idx, delta, vbat) in enumerate(samples, start=1):
        if n == 1:
            continue  # the delta of the first conversion spans the previous cycle; no gate applies
        gate = TABLE[n - 1] if n <= len(TABLE) else TABLE[-1]
        if delta < gate:
            problems.append(f"cycle {num}: sample {n} (idx {idx}) delta {delta} ms below its {gate} ms gate")
    return samples, problems


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__ or "usage: analyze_vbatwalk.py VBATWALK.CSV")
    rows = parse(sys.argv[1])
    preamble, cycles = split_cycles(rows)
    usb_resets = sum(1 for r in rows if r[1] == "reset_usb")
    minute = sum(1 for r in rows if r[1] == "sample_minute")

    all_problems = []
    per_position = {}  # position n -> list of deltas across cycles
    for num, cycle in enumerate(cycles, start=1):
        samples, problems = check_cycle(num, cycle)
        all_problems += problems
        for n, s in enumerate(samples, start=1):
            if n > 1:
                per_position.setdefault(n, []).append(s[3])

    print(f"rows={len(rows)} preamble={len(preamble)} cycles={len(cycles)} usb_resets={usb_resets} minute_samples={minute}")
    print()
    print("pos  gate_ms      min   median      max   n    (delta since previous conversion, across cycles)")
    for n in sorted(per_position):
        gate = TABLE[n - 1] if n <= len(TABLE) else TABLE[-1]
        d = per_position[n]
        tag = " saturated" if n > len(TABLE) else ""
        print(f"{n:3}  {gate:7}  {min(d):7}  {int(median(d)):7}  {max(d):7}  {len(d):3}{tag}")
    print()
    if all_problems:
        for p in all_problems:
            print("FAIL:", p)
        return 1
    print(f"PASS: all {len(cycles)} cycles walked the table entry by entry and saturated on {TABLE[-1]} ms")
    return 0


if __name__ == "__main__":
    sys.exit(main())

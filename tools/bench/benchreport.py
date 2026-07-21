#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# SPDX-FileCopyrightText: Copyright The C47 Authors
#
# Run the C47 benchmark suite in the optimized headless simulator and report
# the results, optionally predicting DM42n hardware times from a calibration
# file. See tools/bench/README.md for the full workflow.
#
# Usage:
#   python3 tools/bench/benchreport.py                 run and report
#   python3 tools/bench/benchreport.py --reps 3        fewer repetitions
#   python3 tools/bench/benchreport.py --calibrate "BMGTO=245,BMREG=180,..."
#       pair the given hardware tick readings (R80..R87 after XEQ BENCH on
#       the calculator) with a fresh local run and rewrite the calibration
#       file. Ticks are 0.1 s units, exactly as the programs store them.

import argparse
import csv
import json
import os
import statistics
import subprocess
import sys
from datetime import date
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
BENCH_BIN = REPO / "t47bench"
SCRIPT = REPO / "res" / "SCRIPTS" / "bench.t47"
CALIBRATION = Path(__file__).resolve().parent / "calibration-dm42n.json"
CSV_OUT = REPO / "bench-results.csv"

BENCHMARKS = ["BMGTO", "BMREG", "BMVAR", "BMRSV", "BMARITH", "BMTRIG", "BMNQN", "BMDISP"]
# Iterations fixed in tools/bench/genbench.py; kept here only to report it/s.
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
# The simulator's display path (GTK) does not model the hardware's DMA LCD
# writes, so no hardware prediction is offered for BMDISP.
HW_ONLY = {"BMDISP"}
SPREAD_WARN = 0.10


def run_suite(reps):
    if not BENCH_BIN.exists():
        sys.exit(f"{BENCH_BIN} not found - run 'make benchbin' first")
    env = dict(os.environ, BENCH_REPS=str(reps))
    cmd = [str(BENCH_BIN), "--headless", "--snapskiprefresh", "--reset", "--script", str(SCRIPT)]
    proc = subprocess.run(cmd, cwd=REPO, env=env, capture_output=True, text=True)
    lines = [l for l in proc.stdout.splitlines() if l.startswith("BENCH|")]
    if not lines or lines[-1] != "BENCH|END":
        sys.stderr.write(proc.stdout[-2000:] + proc.stderr[-2000:])
        sys.exit("benchmark run did not complete (no BENCH|END)")
    canary = None
    results = {}
    for line in lines[:-1]:
        parts = line.split("|")
        if parts[1] == "CANARY":
            canary = int(parts[2])
            continue
        name, _rep, us, ticks = parts[1], parts[2], int(parts[3]), int(parts[4])
        results.setdefault(name, []).append((us, ticks))
    if canary != ITERATIONS["BMGTO"]:
        sys.exit(f"BMGTO loop canary mismatch: R96={canary}, expected {ITERATIONS['BMGTO']}")
    return results


def summarize(results):
    rows = {}
    for name in BENCHMARKS:
        pairs = results.get(name)
        if not pairs:
            sys.exit(f"no results for {name}")
        us = [p[0] for p in pairs]
        med_s = statistics.median(us) / 1e6
        spread = (max(us) - min(us)) / statistics.median(us) if len(us) > 1 else 0.0
        ticks = pairs[-1][1]
        # The program's own TICKS bracket must agree with wall clock to
        # within its 0.1 s quantization (plus one tick of slack).
        if abs(ticks / 10.0 - med_s) > 0.25 and ticks / 10.0 > med_s:
            sys.exit(f"{name}: on-calc ticks {ticks} disagree with wall clock {med_s:.3f}s")
        rows[name] = {"median_s": med_s, "spread": spread, "ticks": ticks}
    return rows


def load_calibration():
    # The file holds one or more power profiles (e.g. usb-160mhz,
    # battery-80mhz), each pairing a hardware run with the local run it was
    # calibrated against. A pre-profile flat file is migrated on read.
    if not CALIBRATION.exists():
        return None
    data = json.loads(CALIBRATION.read_text())
    if "profiles" not in data and data.get("benchmarks"):
        data = {
            "device": data.get("device"),
            "firmware": data.get("firmware"),
            "local_host": data.get("local_host"),
            "profiles": {
                "usb-160mhz": {
                    "power": data.get("power"),
                    "date": data.get("date"),
                    "benchmarks": data["benchmarks"],
                },
            },
        }
    return data if data.get("profiles") else None


def profile_factors(profile):
    factors = {}
    for name, entry in profile["benchmarks"].items():
        if entry.get("hw_ticks") and entry.get("local_s"):
            factors[name] = (entry["hw_ticks"] / 10.0) / entry["local_s"]
    return factors


def report(rows, calib):
    profiles = list(calib["profiles"].items()) if calib else []
    factors = {pname: profile_factors(p) for pname, p in profiles}

    header = f"{'benchmark':<9} {'iters':>7} {'local s':>9} {'spread':>7} {'it/s':>12}"
    for pname, _ in profiles:
        header += f" {pname[:12]:>12}"
    if not profiles:
        header += f" {'DM42n s':>9}"
    print(header)
    print("-" * len(header))
    csv_rows = []
    for name in BENCHMARKS:
        r = rows[name]
        its = ITERATIONS[name] / r["median_s"]
        line = f"{name:<9} {ITERATIONS[name]:>7} {r['median_s']:>9.4f} {r['spread']:>6.1%} {its:>12.0f}"
        csv_row = {
            "benchmark": name,
            "iterations": ITERATIONS[name],
            "local_median_s": f"{r['median_s']:.6f}",
            "spread": f"{r['spread']:.4f}",
            "iterations_per_s": f"{its:.1f}",
        }
        if profiles:
            for pname, _ in profiles:
                if name in HW_ONLY:
                    pred = "hw-only"
                elif name in factors[pname]:
                    pred = f"{r['median_s'] * factors[pname][name]:12.1f}".strip()
                else:
                    pred = "uncal."
                line += f" {pred:>12}"
                csv_row[f"predicted_{pname}_s"] = pred
        else:
            line += f" {'uncal.':>9}"
            csv_row["predicted_dm42n_s"] = "uncal."
        warn = " !" if r["spread"] > SPREAD_WARN else ""
        print(line + warn)
        csv_rows.append(csv_row)
    if calib:
        for pname, p in profiles:
            f_span = sorted(profile_factors(p).values())
            span = f", factors {f_span[0]:.0f}x..{f_span[-1]:.0f}x" if len(f_span) > 1 else ""
            print(f"\n{pname}: {calib.get('device')} | {p.get('power')} | "
                  f"firmware {calib.get('firmware')} | {p.get('date')}{span}")
    else:
        print("\nuncalibrated: no DM42n predictions. Run the suite on hardware "
              "(XEQ BENCH, read R80..R87) and pass --calibrate.")
    if any(rows[n]["spread"] > SPREAD_WARN for n in BENCHMARKS):
        print("!  spread above 10% - rerun on AC power with the machine idle")

    with CSV_OUT.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=list(csv_rows[0].keys()))
        writer.writeheader()
        writer.writerows(csv_rows)
    print(f"\nwrote {CSV_OUT}")


def calibrate(spec, rows, profile, device, power, firmware):
    hw = {}
    for part in spec.split(","):
        name, _, ticks = part.partition("=")
        name = name.strip().upper()
        if name not in BENCHMARKS:
            sys.exit(f"--calibrate: unknown benchmark '{name}'")
        hw[name] = int(ticks)
    missing = [n for n in BENCHMARKS if n not in hw and n not in HW_ONLY]
    if missing:
        print(f"note: no hardware ticks given for {' '.join(missing)}; they stay uncalibrated")
    data = load_calibration() or {"profiles": {}}
    data["device"] = device
    data["firmware"] = firmware
    data["local_host"] = f"{os.uname().sysname} {os.uname().machine}, build.sim.t47.bench (-Os, LTO)"
    data["profiles"][profile] = {
        "power": power,
        "date": date.today().isoformat(),
        "benchmarks": {
            name: {"hw_ticks": hw[name], "local_s": round(rows[name]["median_s"], 6)}
            for name in hw
        },
    }
    CALIBRATION.write_text(json.dumps(data, indent=2) + "\n")
    print(f"wrote {CALIBRATION} (profile {profile})")


def main():
    parser = argparse.ArgumentParser(description="Run the C47 benchmark suite and report")
    parser.add_argument("--reps", type=int, default=5, help="repetitions per benchmark (default 5)")
    parser.add_argument("--calibrate", metavar="SPEC",
                        help='hardware ticks per benchmark, e.g. "BMGTO=245,BMREG=180"')
    parser.add_argument("--profile", default="battery-80mhz",
                        help="calibration profile name (e.g. usb-160mhz, battery-80mhz)")
    parser.add_argument("--device", default="DM42n (STM32U575)", help="calibration device label")
    parser.add_argument("--power", default="battery (80MHz)", help="calibration power state")
    parser.add_argument("--firmware", default="unspecified", help="calibration firmware version")
    args = parser.parse_args()

    rows = summarize(run_suite(args.reps))
    if args.calibrate:
        calibrate(args.calibrate, rows, args.profile, args.device, args.power, args.firmware)
    report(rows, load_calibration())


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Generate the function-kind hardware test listing FNKIND.txt, and the check listing FNKVFY.txt.

One case per kind of function. Each case is measured twice: once for the stack high-water figure of a
single call, and once for the time of N calls in a counted loop. Case 01 is empty, so its time is the
loop and call overhead every other case also pays.

Every case is written as work steps followed by clean-up steps. FNKIND runs both, so the stack comes
back as it was found. FNKVFY runs the work steps only and prints what each one left, which is how the
cases were shown to compute rather than merely to run without an error.

    python3 genfnkind.py
    ./rejig FNKIND.txt -o FNKIND.p47
    ./rejig FNKVFY.txt -o FNKVFY.p47

A literal with four or more decimals is read by rejig as an angle or a time: 2026.0729 encodes as
2026 deg 7 min 29 sec and 12.3456 as 12:34:56. Values of that shape are therefore built by division
in KSET and recalled, never written as literals.
"""

import argparse
import os
import sys

# Case table: (name, time window in tenths of a second, work steps, clean-up steps).
# The work is one or two functions of that kind, on operands that always succeed. Order is cheapest
# and safest first, so where the output stops is itself a result.
CASES = [
    ("NULL", 10, [], []),

    ("STK", 10, [
        "1 ENTER",
        "𝑥⇄𝑦 R↓",
    ], ["DROP𝑥 DROP𝑥"]),

    ("REG", 10, [
        "7 STO 10 DROP𝑥",
        "RCL 10",
    ], ["DROP𝑥"]),

    ("VAR", 10, [
        "7 STO 'kvv' DROP𝑥",
        "RCL 'kvv'",
    ], ["DROP𝑥"]),

    ("ARITH", 10, [
        "1.5 2.5 ×",
        "3.5 +",
    ], ["DROP𝑥"]),

    ("TRIG", 10, [
        "0.5 sin(𝑥)",
        "cos⁻¹(𝑥)",
    ], ["DROP𝑥"]),

    ("LOGEXP", 10, [
        "2.5 ln(𝑥)",
        "𝑒ˣ",
    ], ["DROP𝑥"]),

    ("POWER", 10, [
        "2.5 3.5 𝑦ˣ",
        "√𝑥",
    ], ["DROP𝑥"]),

    ("SPECIAL", 10, [
        "2.5 Γ(𝑥) DROP𝑥",
        "0.5 erf",
    ], ["DROP𝑥"]),

    ("CONST", 10, [
        "CNST 04 DROP𝑥",
        "𝜋",
    ], ["DROP𝑥"]),

    ("RANDOM", 10, [
        "RAN#",
    ], ["DROP𝑥"]),

    ("FLAGTEST", 10, [
        "SF 00",
        "FS? 00",
        "NOP",
        "CF 00",
        "1 2 𝑥<? Y",
        "NOP",
    ], ["DROP𝑥 DROP𝑥"]),

    ("DISPMODE", 10, [
        "SCI 04",
        "ALL 00",
        "FIX 00",
    ], []),

    ("ANGLE", 10, [
        "45 deg→rad",
        "rad→deg",
    ], ["DROP𝑥"]),

    ("UNITCONV", 10, [
        "10 ft→m",
        "m→ft",
    ], ["DROP𝑥"]),

    ("TIME", 10, [
        "RCL 23 HMS→HR",
        "HR→HMS",
    ], ["DROP𝑥"]),

    ("DATE", 10, [
        "RCL 22 𝑥→𝔻",
        "𝔻→J",
    ], ["DROP𝑥"]),

    ("COMBIN", 10, [
        "20 7 COMB DROP𝑥",
        "20 7 PERM",
    ], ["DROP𝑥"]),

    ("NUMTH", 10, [
        "123456 NEXTP DROP𝑥",
        "123456 78901 GCD",
    ], ["DROP𝑥"]),

    ("FRACT", 10, [
        "0.375 DECOMP",
    ], ["DROP𝑥 DROP𝑥"]),

    ("SHORTINT", 10, [
        "255 ShortInt",
        "170 ShortInt AND",
    ], ["DROP𝑥"]),

    ("BITS", 10, [
        "255 ShortInt MIRROR DROP𝑥",
        "255 ShortInt SLn 04",
    ], ["DROP𝑥"]),

    ("LONGINT", 10, [
        "123456789012345678901234567890 LongInt",
        "ENTER ×",
    ], ["DROP𝑥"]),

    ("CPX", 10, [
        "1 2 ℝ→ℂ",
        "ENTER ×",
    ], ["DROP𝑥"]),

    ("MATRIX", 10, [
        "SDIGS 06",
        "RCL 20 [𝑀]⁻¹",
    ], ["DROP𝑥", "SDIGS 00"]),

    ("STRING", 10, [
        "3 αLEFT 21",
    ], ["DROP𝑥"]),

    ("STAT", 10, [
        "CLΣ",
        "1 1 Σ+ DROP𝑥 DROP𝑥",
        "2 4 Σ+ DROP𝑥 DROP𝑥",
        "3 9 Σ+ DROP𝑥 DROP𝑥",
        "4 16 Σ+ DROP𝑥 DROP𝑥",
        "5 25 Σ+ DROP𝑥 DROP𝑥",
        "𝑥̄",
    ], ["DROP𝑥 DROP𝑥"]),

    # ResetF then BestF names the fit. Without it L.R. uses whatever the user last chose, and the same
    # points gave 1.000000 under AllF against 0.981105 under a linear fit. The model keys are toggles and
    # are not programmable: BestF takes the selection as a bit value, 1 linear, 2 exponential, 4 log.
    ("CURVEFIT", 10, [
        "ResetF",
        "BestF 01",
        "CLΣ",
        "1 1 Σ+ DROP𝑥 DROP𝑥",
        "2 4 Σ+ DROP𝑥 DROP𝑥",
        "3 9 Σ+ DROP𝑥 DROP𝑥",
        "4 16 Σ+ DROP𝑥 DROP𝑥",
        "5 25 Σ+ DROP𝑥 DROP𝑥",
        "L.R. DROP𝑥 DROP𝑥",
        "CORR",
    ], ["DROP𝑥"]),

    ("STATPLT", 10, [
        "CLΣ",
        "1 1 Σ+ DROP𝑥 DROP𝑥",
        "2 4 Σ+ DROP𝑥 DROP𝑥",
        "3 9 Σ+ DROP𝑥 DROP𝑥",
        "4 16 Σ+ DROP𝑥 DROP𝑥",
        "5 25 Σ+ DROP𝑥 DROP𝑥",
        "SDIGS 06",
        "SCATR",
    ], ["CLSTK", "SDIGS 00"]),

    ("PROBDIST", 10, [
        "1.5 Φ_ΦL DROP𝑥",
        "0.975 Φ⁻¹",
    ], ["DROP𝑥"]),

    ("FIN", 10, [
        "SDIGS 06",
        "10 STO 'NPPER' DROP𝑥",
        "5 STO 'I%/a' DROP𝑥",
        "1 STO 'PPER/a' DROP𝑥",
        "1 STO 'CPER/a' DROP𝑥",
        "1000 STO 'PV' DROP𝑥",
        "0 STO 'PMT' DROP𝑥",
        "FV",
    ], ["DROP𝑥", "SDIGS 00"]),

    ("GRAPHICS", 10, [
        "CLLCD",
        "50 100 PIXEL",
        "80 150 POINT",
    ], ["CLSTK"]),

    # Window 0 means one call and no more: the loop always runs once before it looks at the clock.
    # SNAP writes a capture file per call, so it is measured that way and its time is that one call.
    ("SNAP", 0, [
        "SNAP",
    ], []),

    ("EQN", 10, [
        "SDIGS 06",
        "'x^2+4' X.EDIT",
        "1 5",
        "cpxSlvˣʸ",
    ], ["DROP𝑥", "SDIGS 00"]),

    ("DERIV", 10, [
        "1 f'(𝑥) 'KFP'",
    ], ["DROP𝑥"]),

    ("INT", 10, [
        "PGMINT 'KFP'",
        "0 1",
        "∫^𝑥_𝑦 'x'",
    ], ["DROP𝑥"]),

    ("SOLVE", 10, [
        "SDIGS 06",
        "PGMSLV 'KF2'",
        "0 2",
        "SOLVE 'x'",
    ], ["DROP𝑥", "SDIGS 00"]),

    ("PLOT", 10, [
        "SDIGS 06",
        "PGMPLT 'KFP'",
        "-2 4",
        "PLTf 'x'",
    ], ["CLSTK", "SDIGS 00"]),

    # VECTOR is the one kind a hardware build may leave out: OPTION_VECTOR is undefined for the DM42.
    # Compiled out the functions are empty, so the case reports a time of nothing rather than stopping the
    # run, and it goes last where that cannot cost the cases behind it.
    ("VECTOR", 10, [
        "1 2 3 𝑧𝑦𝑥→𝑣̄₃",
        "ENTER DOT",
    ], ["DROP𝑥"]),

    ("HYPERB", 10, [
        "0.5 sinh(𝑥)",
        "sinh⁻¹(𝑥)",
    ], ["DROP𝑥"]),
]

# Cases after which the harness captures the screen, by case name. GRAPHICS is among them from the
# firmware change that gave SNAP the screenHoldsDrawnPixels flag: before it, SNAP refreshed first and
# repainted the stack over anything PIXEL or POINT had drawn, and the capture held the stack.
SNAP_AFTER = ("GRAPHICS", "STATPLT", "PLOT")

INTRO = [
    "Operating time and stack high-water of one case per kind of function, one or two functions each.",
    "No nesting: every case runs a single engine at most. Cheapest and safest case first, so where the",
    "output stops is itself a result. Needs a build with STACK_WATERMARK set in defines.h; on a build",
    "without it the run still completes and STCKST reads the 9 KSET stores, saying so, and only the times are real.",
    "The system flag PrintACT is clear unless set, so the print steps write to a dated DATA\\*.REGS.TSV.",
    "The program never stores STCKSPN. Each machine's own default stands: 8088 on a DM42, 16384 on a DM42n.",
    "A case that kills the calculator does not end the test: store the next case number in R94 and XEQ FNKFROM,",
    "which runs that case and every one after it. One case alone is XEQ M nn, the wrappers being M01 upward.",
    "Raising it is yours to do, before the run, and only on a machine where you know the depth is safe.",
    "Five lines per case: the stack figure, its STCKST, STCKSPU, how many calls fitted in the time",
    "window, and the tenths of a second that took. STCKST: 0 the figure is a depth reached, 1 the marker",
    "ran out so it is a floor, 2 nothing was disturbed, 3 no marker was laid. Only 0 is a measurement.",
    "The window is one second a case unless the table says otherwise, so nothing is sized in advance and",
    "the same program works on a machine eighty times slower than another. Time per call is the tenths",
    "divided by the count. Case 01 is empty, so its figures are the loop and call overhead every other",
    "case also pays: subtract it. Integration runs at ACC 1E-3 and the plot, matrix and solve cases at",
    "SDIGS 6, since the cost and the depth are under test and not the answer.",
    "Uses R95 to R98, R20 to R23, the named variable kvv, the STCK set, the TVM variables and the stat",
    "registers. Run it on a state you do not mind touching. Notes in README.txt.",
]

VERIFY_INTRO = [
    "The same cases with the clean-up steps left out, so each one prints what it computed. This is the",
    "check that a case does its work, which a run without an error does not show. Run one label at a",
    "time: XEQ VSET first, then XEQ V05 and read register X. Generated beside FNKIND.txt, never edited.",
]


def setup_lines(prefix):
    """State every case starts from. Same in both listings."""
    out = []
    out.append("LBL '%sSET'" % prefix)
    out.append("  # Fixed state for every case")
    out.append("  FIX 00")
    out.append("  RAD")
    out.append("  1E-3 STO 'ACC' DROP𝑥")
    out.append("  0 STO 'STCKHI' DROP𝑥")
    out.append("  9 STO 'STCKST' DROP𝑥")
    out.append("  0 STO 'STCKSPU' DROP𝑥")
    out.append("  0 STO 'STCKGO' DROP𝑥")
    out.append("  20260729 10000 ÷ STO 22 DROP𝑥")
    out.append("  123456 10000 ÷ STO 23 DROP𝑥")
    out.append("  'ABCDEF' STO 21 DROP𝑥")
    out.append("  3 3 M.DIM 20")
    out.append("  XEQ 'KMAT'")
    out.append("RTN")
    out.append("")
    return out


def matrix_lines():
    out = []
    out.append("NOP ; A well conditioned 3 by 3 in R20, so the matrix case inverts and never divides by zero.")
    out.append("")
    out.append("LBL 'KMAT'")
    out.append("  # Fill R20")
    out.append("  INDEX 20")
    for index, value in enumerate((4, 2, 1, 2, 5, 3, 1, 3, 6)):
        out.append("  %d %d STOIJ" % (index // 3 + 1, index % 3 + 1))
        out.append("  %d STOEL DROP𝑥" % value)
    out.append("RTN")
    out.append("")
    return out


def engine_lines():
    out = []
    out.append("NOP ; The functions the engine cases integrate, solve, plot and differentiate.")
    out.append("")
    out.append("LBL 'KFP'")
    out.append("  # 4 / (1 + x^2)")
    out.append("  MVAR 'x'")
    out.append("  4")
    out.append("  RCL 'x' ENTER ×")
    out.append("  1 +")
    out.append("  ÷")
    out.append("RTN")
    out.append("")
    out.append("LBL 'KF2'")
    out.append("  # x^2 - 2")
    out.append("  MVAR 'x'")
    out.append("  RCL 'x' ENTER ×")
    out.append("  2 -")
    out.append("RTN")
    out.append("")
    return out


def emit_measure(index, name, tenths):
    """The measuring wrapper for one case: the stack figure from one call, then calls counted into a
    fixed time window. The window is what makes the same program work on a machine eighty times slower
    than another: nothing is sized in advance and the count is the answer."""
    tag = "%02d %s" % (index, name)
    out = []
    out.append("LBL 'M%02d'" % index)
    out.append("  # %s" % tag)
    out.append("  1 STO 'STCKGO' DROP𝑥")
    out.append("  XEQ 'K%02d'" % index)
    out.append("  2 STO 'STCKGO' DROP𝑥")
    out.append("  CLSTK")
    out.append("  RCL 'STCKHI' '%s' 🖨xy DROP𝑥 DROP𝑥" % tag)
    out.append("  RCL 'STCKST' 's%02d' 🖨xy DROP𝑥 DROP𝑥" % index)
    out.append("  RCL 'STCKSPU' 'u%02d' 🖨xy DROP𝑥 DROP𝑥" % index)
    out.append("  %d STO 96 DROP𝑥" % tenths)
    out.append("  0 STO 97 DROP𝑥")
    out.append("  TICKS STO 98 DROP𝑥")
    out.append("  LBL %02d" % index)
    out.append("    XEQ 'K%02d'" % index)
    out.append("    1 STO+ 97 DROP𝑥")
    out.append("    TICKS RCL 98 -")
    out.append("  𝑥<? 96")
    out.append("  GTO %02d" % index)
    out.append("  STO 95 DROP𝑥")
    out.append("  CLSTK")
    out.append("  RCL 97 'c%02d' 🖨xy DROP𝑥 DROP𝑥" % index)
    out.append("  RCL 95 'e%02d' 🖨xy DROP𝑥 DROP𝑥" % index)
    if name in SNAP_AFTER:
        # The capture goes last and runs the work steps alone: the clean-up leaves the graph screen,
        # and a capture taken after it holds the cleared screen and not the drawing.
        out.append("  XEQ 'C%02d'" % index)
        out.append("  SNAP")
        out.append("  CLSTK")
    out.append("RTN")
    out.append("")
    return out


def emit_case(prefix, index, name, steps):
    out = []
    out.append("LBL '%s%02d'" % (prefix, index))
    out.append("  # %s" % name)
    for line in steps:
        out.append("  %s" % line)
    out.append("RTN")
    out.append("")
    return out


def generate_main():
    out = []
    for line in INTRO:
        out.append("NOP ; %s" % line)
    out.append("# Jaco Mostert  Function kinds 2026-07-29")
    out.append("")

    out.append("LBL 'FNKIND'")
    out.append("  # Entry point, runs every case")
    out.append("  XEQ 'KSET'")
    for index in range(1, len(CASES) + 1):
        out.append("  XEQ 'M%02d'" % index)
    out.append("  SNAP")
    out.append("RTN")
    out.append("")

    out.append("NOP ; Restart after a case that killed the calculator: put the next case number in R94 and")
    out.append("NOP ; run FNKFROM instead. It runs that case and every one after it. A single case on its own")
    out.append("NOP ; is XEQ M nn, the wrapper labels being M01 to M%02d." % len(CASES))
    out.append("")
    out.append("LBL 'FNKFROM'")
    out.append("  # Runs from the case in R94")
    out.append("  XEQ 'KSET'")
    for index in range(1, len(CASES) + 1):
        out.append("  RCL 94 %d 𝑥≥? Y" % index)
        out.append("  XEQ 'M%02d'" % index)
        out.append("  DROP𝑥 DROP𝑥")
    out.append("  SNAP")
    out.append("RTN")
    out.append("")

    out.extend(setup_lines("K"))
    out.extend(matrix_lines())

    out.append("NOP ; The measuring wrappers, one per case, in the order FNKIND runs them.")
    out.append("")
    for index, (name, tenths, _work, _clean) in enumerate(CASES, start=1):
        out.extend(emit_measure(index, name, tenths))

    out.append("NOP ; The cases themselves. One or two functions of one kind, then the steps that put the stack back.")
    out.append("")
    for index, (name, _tenths, work, clean) in enumerate(CASES, start=1):
        out.extend(emit_case("K", index, name, work + clean))

    out.append("NOP ; The work steps alone of the two cases that leave something on the screen, for the capture.")
    out.append("")
    for index, (name, _tenths, work, _clean) in enumerate(CASES, start=1):
        if name in SNAP_AFTER:
            out.extend(emit_case("C", index, name, work))

    out.extend(engine_lines())
    out.append("END")
    return "\n".join(out) + "\n"


def generate_verify():
    out = []
    for line in VERIFY_INTRO:
        out.append("NOP ; %s" % line)
    out.append("# Jaco Mostert  Function kinds check")
    out.append("")
    out.extend(setup_lines("V"))
    out.extend(matrix_lines())

    out.append("NOP ; The work steps of each case, with nothing dropped, so register X holds the result.")
    out.append("")
    for index, (name, _tenths, work, _clean) in enumerate(CASES, start=1):
        out.extend(emit_case("V", index, name, work))

    out.extend(engine_lines())
    out.append("END")
    return "\n".join(out) + "\n"


def write_or_check(path, text, check):
    if check:
        if not os.path.exists(path):
            print("missing: %s" % path)
            return 1
        with open(path, "r", encoding="utf-8") as handle:
            if handle.read() != text:
                print("differs: %s" % path)
                return 1
        print("matches: %s" % path)
        return 0
    with open(path, "w", encoding="utf-8") as handle:
        handle.write(text)
    print("wrote %s" % path)
    return 0


def apply_windows(path):
    """Override the time windows from a two column file: case name, tenths of a second."""
    counts = {}
    with open(path, "r", encoding="utf-8") as handle:
        for line in handle:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            name, count = line.split()
            counts[name] = int(count)
    for index, (name, tenths, work, clean) in enumerate(CASES):
        if name in counts:
            CASES[index] = (name, counts[name], work, clean)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dir", default=".", help="where to write the listings")
    parser.add_argument("--windows", help="two column file of case name and time window in tenths, overriding the table")
    parser.add_argument("--check", action="store_true", help="compare against the files instead of writing them")
    args = parser.parse_args()

    if args.windows:
        apply_windows(args.windows)

    status = write_or_check(os.path.join(args.dir, "FNKIND.txt"), generate_main(), args.check)
    status |= write_or_check(os.path.join(args.dir, "FNKVFY.txt"), generate_verify(), args.check)
    if not args.check:
        print("%d cases" % len(CASES))
    return status


if __name__ == "__main__":
    sys.exit(main())

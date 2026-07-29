# vbat sampling schedule hardware proof (MR !1611)

Proves on DM42/DM42n hardware that the `vbatSampleDelay[]` escalating
schedule (src/c47/config.c) is walked entry by entry: each ADC conversion is
gated by the table entry at the current schedule index, the index advances
per conversion and saturates on the last entry (2500 ms), and a top level
program run start resets the index so the first dispatches sample undelayed.

This code is `DMCP_BUILD` only: the simulator never compiles it, so hardware
is the only place it runs. The proof rig is a firmware patch plus a six step
program; the firmware drives the whole experiment itself by injecting R/S
key presses through the DMCP key buffer, so the injected presses traverse
the same physical key path as a finger.

## Contents

- `vbat-walk-monitor.patch` - the `MONITOR_VBAT_SCHEDULE` instrumentation:
  a 48 event capture buffer recording every ADC conversion (schedule index,
  ms since the previous conversion, vbat) and every schedule reset (tagged
  run start vs USB changeover vs minute pulse), a driver that injects R/S
  after each program stop (idle gaps cycling 0.5 s / 2 s / 5 s) for 20 run
  cycles, and a per stop append of the buffer to `/VBATWALK.CSV` on the FAT
  root. After cycle 20 the trailer line is written and "BWALK DONE" lands in
  X ("BWALK IOERR" if the final file open failed; failed opens are counted
  in the trailer and retried at the next stop).
- `genbwalk.py` - generates `PROGRAMS/BWALK.p47` (encoder reused from
  tools/bench/genbench.py). BWALK is one run cycle: a 10000 iteration
  DSZ/GTO loop (~9 s on a battery powered DM42, comfortably past the ~7 s
  the schedule needs to saturate), then STOP; the injected R/S resumes at
  the trailing GTO 'BWALK' so every cycle is a genuine top level run start
  through fnRunProgram() and the reset at the runProgram() entry.
- `analyze_vbatwalk.py` - verifies the CSV cycle by cycle (index marches
  1..16 and holds, every delta at or above its gate) and prints the
  aggregate per position table. Exit 0 only if every cycle proves the walk.
- `results/` - captured CSVs and their verdicts.

## Base

Written against MR !1611 head plus the BAT_MINIMUM sync commit
(`cb73d6c0a`, branch battery/per-dispatch-housekeeping). The patch touches
config.c, config.h, defines.h, c47.c and programming/lblGtoXeq.c and is
fully `#if` gated: with `MONITOR_VBAT_SCHEDULE` undefined the object code
is unchanged.

## Run it

```
git apply tools/hwtest/vbat-walk/vbat-walk-monitor.patch
export PATH="$HOME/.local/arm-gnu-toolchain-13.2.Rel1-darwin-arm64-arm-none-eabi/bin:$PATH"
make dmcp      # DM42:  build.dmcp.p4/src/c47-dmcp/C47.pgm + C47_qspi.bin
make dmcp5     # DM42n: build.dmcp5/src/c47-dmcp5/C47.pg5
```

1. Copy the firmware to the calculator (USB disk mode) in one folder with
   the canonical names and flash BOTH files: [4] QSPI, then [3] program.
   Never flash the pgm alone, even when no prompt asks for the QSPI: the
   QSPI image embeds function pointers into internal flash, so ANY code
   change alters its content while size and forced CRC stay identical - the
   loader cannot detect the stale pair and the mix hard faults at the first
   shifted dispatch. On the DM42n `C47.pg5` alone suffices (no QSPI split).
2. Copy `PROGRAMS/BWALK.p47` into the calculator's PROGRAMS directory and
   load it (LOADP). Run on battery power: on USB the CPU runs ~3.3x faster
   and each cycle stops before the 1000/2500 tail of the table can gate a
   conversion inside the run. Optional: start plugged into USB and unplug
   while idle before starting - the capture preamble then shows the USB
   changeover reset and an idle paced walk as well.
3. `XEQ 'BWALK'` once and walk away. The calculator runs 20 cycles by
   itself (~5 minutes) and shows "BWALK DONE" in X when finished.
4. USB disk mode again, copy `VBATWALK.CSV` from the FAT root, then:

```
python3 tools/hwtest/vbat-walk/analyze_vbatwalk.py VBATWALK.CSV
```

## Result (DM42, battery, 2026-07-24, results/vbatwalk-dm42-2026-07-24.csv)

417 events, 20/20 cycles PASS, zero drops, zero IO errors, one USB
changeover reset captured in the preamble:

```
pos  gate_ms      min   median      max   n
  2        0        0        1        1   20
  3        0        0        0       10   20
  4       50       50       50       55   20
  5       50       50       50       55   20
  6      100      100      100      104   20
  7      100      100      100      103   20
  8      100      100      100      104   20
  9..15   100      100      100      100   20   (all exactly 100)
 16     1000     1000     1000     1000   20
 17     2500     2500     2500     2500   20
 18     2500     2500     2500     2500   20   saturated
 19     2500     2527     2595     2695   13   saturated, idle gap
 20     2500     2542     2583     2583    6   saturated, idle gap
```

The delta column reproduces the table entry by entry, the index pegs at 16,
overshoot during a run is bounded by dispatch granularity (max 10 ms), and
idle gap samples ride the main loop wake cadence. The vbat column shows the
per cycle load sag and recovery (2708 -> 2644 -> 2684) that motivates the
undelayed first samples. The build carries the TO_QSPI table (0b88661d), so
the same data proves the QSPI resident table reads correctly.

## Hard won DM42 lessons baked into this rig

- SRAM2 has an undocumented ceiling below the linker script's 16 K: a
  5.6 KB capture buffer (.bss 13312/16384) hard faulted at boot before the
  first paint; at .bss ~8.1 KB the same code boots fine. Hence the 48 event
  buffer with a per stop flush instead of one big buffer.
- Bare string literals compile into `.rodata.str1.1`, which the DM42 ld
  script routes to QSPI - they shift the QSPI layout. All harness strings
  are named `static const char[]` arrays, which land in `.rodata.<name>`
  sections and fall through to internal flash.
- A program run executes entirely inside the key handler: the main loop
  never iterates while PGM_RUNNING (the same starvation that keeps
  checkBattery() out of a run - the finding behind this MR's items.c stop).
  A driver in the main loop can therefore never observe a running->stopped
  transition; this one counts run starts (via the reset hook on the real
  runProgram() path) against runs flushed while idle instead.
- The CSV path is rooted ("/VBATWALK.CSV"): file selection screens move the
  FatFS working directory, so a bare filename lands in the last browsed
  directory.
- If the calculator is still idle 10 s after an injected R/S the driver
  pushes the key again, turning a lost key into a hiccup instead of a
  stalled sequence.

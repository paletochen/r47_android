# pgemu, the firmware image emulator

pgemu runs a built `.pg5` or `.pgm` under an emulated Cortex-M and answers the DMCP library table on the host. What runs is the shipped artefact, byte for byte, in
the calculator's own memory map, so it answers what the GTK simulator cannot: what the DMCP build does, how deep the stack goes, how many instructions an operation
takes and what a refresh sends to the panel.

The figures below are examples. Figures measured with pgemu, each with its build and date, are in [MEASUREMENTS.md](MEASUREMENTS.md).

1. [Quick start](#quick-start)
2. [Why this and not the GTK simulator](#why-this-and-not-the-gtk-simulator)
3. [Running a firmware image](#running-a-firmware-image)
4. [Driving it by hand](#driving-it-by-hand)
5. [Checking screen hand-offs](#checking-screen-hand-offs)
6. [Comparing two builds](#comparing-two-builds)
7. [Counting instructions](#counting-instructions)
8. [Stack analysis](#stack-analysis)
9. [Watching firmware functions](#watching-firmware-functions)
10. [Stubs, --dmcp and the platform images](#stubs---dmcp-and-the-platform-images)
11. [Surviving a DMCP update](#surviving-a-dmcp-update)
12. [How it works](#how-it-works)

## Quick start

```
pip install unicorn
make dmcp5r47
python3 tools/pgemu/pgemu.py build.dmcp5/src/c47-dmcp5/R47.pg5 --press 'ENTER; 3; ENTER; 4; ADD; wait; snap add'
```

`make dmcp5r47` builds the R47 image, and `f=1` after it reuses the build directory, see `BUILD.md`. The last line boots that image, leaves the start notice with
ENTER, types 3 ENTER 4 +, lets the screen settle and writes `tools/pgemu/out/add.bmp` with 7 in X. No ARM toolchain is needed to run it.

Every run ends with a report, and the first lines name the image, here a build of 2026-09-12:

```
image      build.dmcp5/src/c47-dmcp5/R47.pg5
target     dmcp5, cortex-m33, library table at 0x08000300 with 195 entries
prog_info  R47 1f2f4076-mod, 1112192 bytes, entry 0x080ef6b5, interface 3.16, key layout R47
built from 1f2f4076-mod = 2026-09-12  Redraw the menu after leaving PEM with f PRGM  (plus uncommitted changes)
```

**Read the `built from` line before believing anything an image does.** It is looked up in this repository from `pgm_ver` in the image's own prog_info, and an
image left in a build directory can be weeks older than the branch.

To drive the calculator with the mouse instead, add `--serve` and open `http://127.0.0.1:8731/`, see [Driving it by hand](#driving-it-by-hand).

## Why this and not the GTK simulator

`t47` and the GTK simulator compile the C47 sources for the host, so several things they exercise are not what the calculator does:

- GMP is built with 64 bit limbs on the host and 32 bit limbs on the calculator, `GMP_LIMB_BITS` in the generated `gmp.h` of each build, so the limb boundaries and
  the `mpn_*` routines selected are different arithmetic.
- The block pool is 256 kB on the host and 64 kB on the DM42, `RAM_SIZE_IN_BLOCKS` in `src/c47/defines.h`, and the stack does not grow toward the pool.
- Every `#if defined(DMCP_BUILD)` route is compiled out.
- The screen capture is taken from `lcd_buffer`, so drawing that reaches no refresh still appears in a picture.
- Timing the simulator times host code: another instruction set and compiler, 64 bit GMP limbs, the GTK HAL's drawing instead of DMCP's, and the host's caches.
  `tools/bench` calibrates that against the calculator for arithmetic and leaves drawing out.

pgemu removes all five, the last through `--profile`, which counts the instructions of the shipped image, DMCP's allocator included where the platform image is mapped and
its drawing with `--dmcp`, the same in every run. It is not a replacement for `t47`: `t47` is for fast iteration on arithmetic and menus, pgemu for what the calculator
itself does.

## Running a firmware image

### The image and the files beside it

| What | Where it comes from |
|---|---|
| the target | the suffix, `.pg5` for DMCP5 (R47 hardware) and `.pgm` for DMCP (DM42); `--target` overrides it |
| the QSPI image | `<name>_qspi.bin` beside the program, or `--qspi`; the DM42 build needs it, because its fonts and cold tables are only there |
| function names | `<name>.elf` beside the program, or `--elf`; the options that watch functions and `--profile` use it |
| a DMCP platform image | `res/combo/DMCP5_flash_*.bin` for the R47, found as the SDK is, or the one `--platform` or `--dmcp` names; its allocator runs malloc, see [Stubs, --dmcp and the platform images](#stubs---dmcp-and-the-platform-images) |
| the keyboard | the model name in the image's prog_info; `--layout C47` or `--layout R47` overrides it |

### Keys and key scripts

`--press` takes a script on the command line and `--script` takes one from a file, or from stdin with `-`. The spelling is the one
`res/SCRIPTS/cli_automation_examples.txt` uses for the simulator's `press` command, so a sequence written for `t47` runs here unchanged.

| Step | Meaning |
|---|---|
| `3`, `A`, `a` | the key that types a digit or a letter; a lower case letter is the f shift first, and a run of digits types itself |
| `ENTER`, `EXIT`, `BSP`, `R/S`, `ADD`, `SUB`, `MUL`, `DIV`, `DOT` | named keys, the same code on both keyboards |
| `XEQ`, `UP`, `DOWN`, `CHS`, `EEX` | named keys whose code differs between the C47 and the R47 keyboard |
| `F1` to `F6` | the softkeys |
| `@f` | the f shift |
| `@g` | the yellow key twice, which is g on the C47; the R47 has a key of its own for g, `@k 11` |
| `@k NN` | a physical key, the DMCP code less one, as that file numbers them |
| `wait [N]` | N idle rounds, 20 without a number, for what the firmware does after the last key |
| `snap [name]` | a capture at this point, written as `name.bmp` in `--out` |
| `mark [name]` | lays the stack pattern again and starts the function records, the malloc counts, the overflow and the instruction counts again, so what follows is measured on its own |

Steps are separated by `;` or a new line, and `#` starts a comment. A press goes in with its release, one key per idle round; `--key-gap` adds rounds between keys.
`--keys` takes raw DMCP key codes separated by commas instead of a script: `--keys 28,33 --idle 60` is f then EXIT, which turns the calculator off.

| To | Script |
|---|---|
| run a function or a label by name | `XEQ; XEQ; C; H; S; ENTER` runs CHS. A name of seven letters runs as the seventh is typed: typing SINTINT runs the label SINTINT |
| open a menu and press a softkey | the I/O menu is `@k 11; 0` on the R47 and `@g; SUB` on the C47; then `F4` is LOADP, `@f; F1` READP and `@k 11; F1` XPORTP on the R47 |
| answer a Y/N confirmation | `F2` is YES and `F5` NO, as in `XEQ; XEQ; D; E; L; B; k; u; p; wait 60; F2` |

### Files

`--fs` maps the calculator's FAT root onto a host directory, `fs` beside `pgemu.py` by default, which `.gitignore` leaves out of the tree: the programs and state files a
run needs are put there, in `PROGRAMS`, `STATE` and `SAVFILES` as on the calculator. `--fs .` uses the working folder the way the GTK simulator does, and
`--read-only` refuses every write. Paths arrive in the calculator's own form with either separator, a name that does not match is looked up again without case, and
a path that would leave the root is refused.

A function that opens the DMCP file selection screen gets its answer from `--choose` where one is given, at every selection of a run without a page and in a served run
until the page takes over from the script, and otherwise from a dialog in the page in a served run. `--pick` opens the host's own file chooser for a headless run, and
`--no-pick` turns the page dialog off:

```
python3 tools/pgemu/pgemu.py build.dmcp5/src/c47-dmcp5/R47.pg5 --choose 'STATE\test.s47' \
  --press 'ENTER; wait 60; XEQ; XEQ; L; O; A; D; S; T; ENTER; wait 50; ENTER; wait 300; snap loaded'
```

LOADST's selection function displays a warning that the current state will be lost and waits for a key, which is why the script presses ENTER after LOADST; EXIT
there leaves the list without loading. READP, SAVEST and the register exports copy the path and return, and need no key of their own.

### A copy outside the tree, and a worktree

The folder can be copied out of the checkout, so that one copy runs the images of every branch. `fs` and `out` are taken beside `pgemu.py` wherever it is, and the
SDK from the first folder above the image that has `dep/DMCP5_SDK` or `dep/DMCP_SDK`, or from the working folder when none has, so the library table and the system
data block match the branch the image was built from:

```
cd c43-other-branch
python3 ../pgemu/pgemu.py build.dmcp5/src/c47-dmcp5/R47.pg5 --serve
```

The SDKs are git submodules, and a new worktree starts with them empty. `git submodule update --init dep/DMCP_SDK dep/DMCP5_SDK` fills them, or the run can be
started from a checkout that has them, which then serves as the working folder.

### How a run ends, and what it reports

A run stops at the first of these, and the `stopped` line of the report names it:

- `sys_sleep` with no key and no `--idle` round left, which is the settled screen;
- `STAT_PGM_END`, which OFF sets;
- `sys_reset`, which the firmware calls to restart the calculator, with the screen written as `sys_reset.bmp` where captures are on;
- the instruction budget, 200 000 000 unless `--max-instructions` gives another, where 0 is no budget and a served run has none; a run that spends it in a branch to
  itself, which is how DMCP stops on a failed assertion, is reported as one;
- an access outside every mapped region, reported with the region nearest to it.

**Give `--max-instructions 0` to any run that integrates, solves, plots or differentiates.** They go past the default budget.

Captures go to `--out`, `out` beside `pgemu.py` by default, one each time the pushed image changes. `--capture-all` writes one per refresh, `--no-capture` none, and a served
run writes them only where asked; a `snap` is written in every case. `--trace` prints every DMCP call with its first two arguments as it arrives.

The report ends with the file calls the run made and every DMCP entry it reached on the host, with a count each. An entry with no implementation is marked.

### The clock

The firmware's clock is host time since the run started, plus every sleep the firmware armed and the host did not wait out. TICKS, the stopwatch and every timeout
therefore behave as on the calculator, and a scripted `wait 400` advances the clock by what four hundred wakes take there. `--ms-per-tick N` counts it in calls
instead, one millisecond per N, for a run that has to repeat exactly; a stopwatch measures nothing then.

The date and time the status bar shows come from the host's calendar clock. `--clock '2026-09-16 12:00'` starts it at that local time instead and advances it with
the firmware's clock, so with `--ms-per-tick` as well every run of a script shows the same date and time.

## Driving it by hand

```
python3 tools/pgemu/pgemu.py build.dmcp5/src/c47-dmcp5/R47.pg5 --serve
```

Open `http://127.0.0.1:8731/`, or the port given after `--serve`, and click the page once so that it has the keyboard focus. The page is the pushed screen above a
keyboard of the model the image names, with the f legends in gold and the g legends in blue. Keys are clicked or typed on the computer keyboard, and a key stays
down while the button or the key is down, so the function name overlay and the shifted preview appear as they do on the calculator. A served run has no instruction
budget and ends with ctrl-c, which prints the report as any other run does. A script given with `--press` or `--script` runs first, as it does without a page, and
the page takes over where the script ends. `--choose` answers only until then, so a save from the page shows the dialog and never writes over the file the script loaded.

EXIT and R/S stop a long computation as on the calculator: a served run has `key_empty` answered on the host instead of by its stub, and that entry is how a
running program, SOLVE, INT, PLOT and the other operations that show their progress test for a key.

Under the calculator:

| Line | Shows |
|---|---|
| stack | gold is what is taken now, dark the room down to the deepest reading, blue the room beyond it, and the whole bar is red where the room is gone; `reset` starts the deepest reading again |
| overflow | in red, only where the stack has written into the platform's heap, see [An overflow on the DM42](#an-overflow-on-the-dm42): the address and the function of the first such write, while the stack bar takes the depth its pointer reached; the stack's `reset` starts it again |
| heap | the heap the same way, split where the ELF beside the image names C47's block pool: the pool, which C47 takes with one malloc at start, fills leftwards from the white line and every other malloc rightwards, and a side goes red when an allocation asked for more than that side has, which is where RAM is full comes from; `reset` starts both peaks again |
| run | `restart` runs the same command line again with the image read from disk, so a new build of the same file runs without leaving the page |

A function that opens the file selection screen shows a dialog listing the files of its folder, and a save accepts a new name. The screens DMCP draws for itself, the
file list, DISK, ActUSB and the DMCP menu, are shown as vertical bars, see the next section, and DISK, ActUSB and the DMCP menu wait for EXIT as on the calculator.

## Checking screen hand-offs

Where DMCP takes the screen, pgemu paints vertical bars over every line of the buffer and sends them, in place of the file list, the disk page, the menus and the
display after OFF. When the firmware next waits for a key, whatever is left of the bars is the part it never drew back, counted from the pushed image and not from the
buffer:

```
screen     taken for the platform 1 times, drawn again in full
screen     taken for the platform 1 times, and 818 bytes of the pattern were never drawn again, on 20 lines between 0 and 19
```

The second line is READP typed by name, `XEQ; XEQ; R; E; A; D; P; ENTER`, on a build before `51b3e9c8e`: lines 0 to 19 are the status bar, which that route left
undrawn while READP from the I/O menu redrew it in full. A byte counts only where the byte beside it is still pattern too, so the smallest patch reported is sixteen
pixels wide. The GTK simulator takes its capture from `lcd_buffer`, so this class of fault does not show there at all.

## Comparing two builds

`--frames FILE` writes a digest of the pushed image at every refresh, one a line, so the files of two runs compare with `cmp` or `diff`, and the first line that
differs is the refresh where the builds part. For the same script to give the same frames twice, fix both clocks with `--ms-per-tick 1` and `--clock`:

```
SCRIPT='ENTER; wait; mark typing; 1; 2; 3; ENTER; 4; 5; 6; wait; snap typing'
python3 tools/pgemu/pgemu.py old/R47.pg5 --dmcp res/combo/DMCP5_flash_3.57.bin --ms-per-tick 1 --clock '2026-09-16 12:00' --frames old.txt --press "$SCRIPT"
python3 tools/pgemu/pgemu.py new/R47.pg5 --dmcp res/combo/DMCP5_flash_3.57.bin --ms-per-tick 1 --clock '2026-09-16 12:00' --frames new.txt --press "$SCRIPT"
cmp old.txt new.txt
```

Two differences remain that are not the drawing's. The start screen shows the version with the commit and the build date, so a dozen frames of it differ between
builds of different commits or days; and a build from a tree with changes shows `-mod` after the commit, which is more glyphs to draw at start and so more
instructions. `--capture-all` writes a picture of every refresh where the digests alone do not show what changed.

## Counting instructions

`--profile` counts every instruction the emulated processor runs and charges it to the firmware function it belongs to, from the ELF beside the image, or to the DMCP
entry the firmware last branched to. Each mark in the script starts a new stretch, and the report lists every stretch with its instructions, refreshes and lines
pushed, and the functions and entries that took most, twelve unless a number follows `--profile`. Opening the I/O menu, `mark iomenu; @k 11; 0; wait`, on an R47
image of 2026-09-18:

```
profile    iomenu: 815829 instructions, 7 refreshes, 220 lines pushed
                224934   27.6 %  DMCP lcd_fill_rect, 128 calls
                211512   25.9 %  DMCP bitblt24, 2830 calls
                143876   17.6 %  showGlyphCode
                 70592    8.7 %  findGlyphExact
```

- Everything between two marks runs on the emulated processor and is counted: the key handling, the arithmetic, C47's drawing and DMCP's own code.
- An entry answered on the host runs in Python and counts nothing. malloc and its kin are the exception where the platform's allocator answers them: its instructions are
  counted and charged to the entry, as `DMCP __sysfn_realloc` for instance, see [The platform's allocator](#the-platforms-allocator).
- Without `--dmcp` the drawing entries run as the stubs of `stubs.c`, whose instructions are not DMCP's: typing six digits comes to about three times the count, most of
  it in the `lcd_fill_rect` stub. A count meant to match the calculator is taken with `--dmcp`.
- The time the panel takes to receive the lines is no instruction. An R47 on battery sends all 240 lines about twenty times a second, so every line pushed adds
  roughly 0.2 ms of wall time to the instructions of its stretch.
- Time asleep, waiting for a key or a timer, runs nothing and counts nothing.
- A count is instructions, not cycles. Flash wait states, QSPI accesses, divisions and interrupts cost more on the calculator than the count shows, so counts compare
  variants of the same code; absolute time is measured on the calculator or with `tools/bench`.

The count takes one call into Python per block, so a profiled run takes several times as long as one without; the counts do not depend on that. Fix the clocks as in
[Comparing two builds](#comparing-two-builds) where two counts are to be compared.

## Stack analysis

### Measuring a depth

`--stack-watermark` writes a pattern over the unused stack, and the deepest word that has lost it is how far the run went. `mark` in a script writes the pattern
again, so the reading is of what follows the mark alone. Load the program first, mark, then run it; `OVER2.p47` is one of the programs in
[MEASUREMENTS.md](MEASUREMENTS.md#programs-over-the-edge), made as [Writing test programs](#writing-test-programs) describes:

```
python3 tools/pgemu/pgemu.py build.dmcp5/src/c47-dmcp5/R47.pg5 --max-instructions 0 --stack-watermark --choose 'PROGRAMS\OVER2.p47' \
  --press 'ENTER; wait 60; XEQ; XEQ; R; E; A; D; P; ENTER; wait 200; mark over2; XEQ; XEQ; A; A; A; A; ENTER; wait 4000; snap over2'
```

```
stack      10096 bytes below the entry frame since over2, deepest 0x2003d890
           lowest stack pointer seen at a DMCP call 0x2003dac0, 9536 bytes below the entry frame
```

The second line is an independent reading, taken only where the firmware calls DMCP, so it comes out shallower by the frames that call nothing. A reading whose pattern is
gone all the way down is reported as a floor rather than a depth. The stack bar in the page is the same reading, deepened where the stack has written into the platform's
heap, see [An overflow on the DM42](#an-overflow-on-the-dm42).

**A depth read here is a few hundred bytes short of the calculator's**, measured against an R47 running the same build, so a margin taken from it should allow for that.
The DM42 has about 8 kB of stack with DMCP's heap directly below it, heap_4's own variables first, so an overrun there breaks the allocator before it reaches anything C47
has allocated, see [An overflow on the DM42](#an-overflow-on-the-dm42); the R47 has 64 kB. The figures and how they were found are in [MEASUREMENTS.md](MEASUREMENTS.md)
and `target.py`.

### The engine model

PLOT, INT, SOLVE, the derivatives f′ and f″, and Σn and Πn each run a program of the user's. Their depth follows one rule:

**depth(engine over g) = max(floor, depth(g) + k)**

depth(g) is what the program g takes run on its own, k is what the engine adds below its call to g, and the floor is the engine over a program that does nothing. k
is the engine over one program less that program alone, and it comes out the same to the byte whichever program is used. The floors and k of each engine, and the
combinations checked against the rule, are in [MEASUREMENTS.md](MEASUREMENTS.md#engine-depths).

### Writing test programs

A `.p47` is text, six header lines and then one decimal byte per line, and writing one directly is quicker than keying a program in.

```
PROGRAM_FILE_FORMAT
0
C47_program_file_version
1
PROGRAM
<the number of bytes that follow>
```

| Step | Bytes |
|---|---|
| an item below 128 | its number |
| any other item n | `128 + (n >> 8)`, then `n & 255` |
| a name or label parameter | `253`, the length, then the bytes |
| an integer literal | `114 8`, the length, then the digits |
| a real literal | `114 9`, the length, then the text, as `1E-3` |
| a string literal | `114 253`, the length, then the text |

Names are in the calculator's own encoding, so δ⒟ is `131 180 164 159`. Item numbers are the `/* n */` comments in `src/c47/items.c` and move between builds, so
take them from the build that runs; a program ends with END. Load the file with READP through `--choose`, give its first label four letters so that XEQ reaches it by
name, and put SHOW before the last RTN where a result has to be compared exactly: the capture then shows all 34 digits.

## Watching firmware functions

Three options take a firmware function by name, from the ELF beside the image or the one `--elf` names. Each may be given more than once, and a `mark` starts their
counts again as it does the stack reading.

| Option | Reports |
|---|---|
| `--entry-sp FUNCTION` | the deepest entry to the function, in bytes below the entry frame, and how often it was entered |
| `--calls FUNCTION[:N]` | the calls by the value of argument N, the first when no N is given |
| `--fail FUNCTION=VALUE` | a call whose first argument is VALUE returns zero at once, which reaches a RAM full route without filling RAM |

On OVER2, `--entry-sp calcDeriv --calls allocC47Blocks` adds, on a build of 2026-09-12:

```
entry      calcDeriv at most 7384 bytes below the entry frame, 236 calls since over2
calls      allocC47Blocks 28150 since over2, 24630 with 4, 3517 with 3, 3 with 2
```

`--fail` acts from the first instruction of the run, boot included, and a mark starts only its count again. So choose a value the case under test asks for and the
boot does not. On a program running `12 12 IDENT |M|`, `--calls allocC47Blocks` lists `2 with 4320`, and `--fail allocC47Blocks=4320` ends it on `RAM is full`. `--fail
allocC47Blocks=4` fails instead the first register allocation of the reset at boot, and the run stops on an access to address 0.

A name the compiler inlined everywhere has no symbol, and a name several static functions share cannot be told apart. Both are refused before the run starts:

```
pgemu: build.dmcp5/src/c47-dmcp5/R47.elf has 5 functions named setBlackPixel, at 0x08160310, 0x08155cf4, 0x081849cc, 0x0813dd8c, 0x08171470
pgemu: build.dmcp5/src/c47-dmcp5/R47.elf has no function named calcDerivOfOrder, or the compiler inlined every call to it
```

Each watched function is one code hook on one address, and a firmware run does not show its cost.

### Refusing heap allocations

`--fail` reaches C47's own block pool. Everything else the firmware allocates, the working reals of `REAL_T_ALLOC`, GMP's numbers and the buffers of a save, comes
from DMCP's heap through malloc, calloc and realloc, which trap to the host whichever allocator then answers them, and two options refuse those:

| Option | Refuses |
|---|---|
| `--malloc-fail N` | the Nth allocation, malloc, calloc and realloc numbered together; give it more than once for several |
| `--malloc-fail-above BYTES` | every allocation larger than BYTES |

Both count from the last mark, or from the start of a run whose script has no mark, so a mark after the boot keeps the block pool and the boot's own allocations out
of it. The report counts the allocations and names the caller of each one refused:

```
malloc     20 allocations since m, the largest 4104 bytes, 1 refused
           refused number 20, 4104 bytes, for allocGmp+0x16
```

That is NEXTP of 10^307 on a DM42 build with MR !1718, whose allocGmp stops a refused allocation with the bug screen and `sys_reset`. pgemu ends the run at
`sys_reset` and writes the screen as `sys_reset.bmp`. On master the same option refuses the 2500 bytes of `__gmp_randinit_mt_noseed` instead, GMP writes through the
null pointer, and the run stops on an access to address 0.

## Stubs, --dmcp and the platform images

`stubs.c` answers the busiest entries inside the emulated machine: `bitblt24`, `lcd_fill_rect`, `f_read` and the key entries. A call answered in Python costs many
times what those few instructions take, and a boot makes tens of thousands of such calls. The stubs are built with the ARM toolchain, and the copy checked in under
`prebuilt/` serves a run without it, see [How it works](#how-it-works).

`--dmcp` runs DMCP's own drawing entries from the platform image instead, the one it names or the one `--platform` names or finds, and every stub is off, which makes a
run somewhat slower. Captures of C47's own screens are byte for byte the same either way. Give `--dmcp`:

- where the screen shows DMCP's own text, the LOADST warning and `show_warning`, which only the platform draws; without it the key is waited for all the same;
- where the question is how DMCP marks lines for sending, since it then uses DMCP's own line bitmap;
- for instruction counts meant to match the calculator;
- where a stub run and the calculator disagree: the stubs are a rewrite of DMCP's drawing, and the platform image is the reference.

`res/combo/DMCP5_flash_3.57.bin` is in the tree, because the R47 release is packaged with it, `res/combo/R47_combo.py`. DMCP for the DM42 and the other DMCP5
builds are published by SwissMicros at https://technical.swissmicros.com/dmcp/ . `target.py` was read from DMCP 3.29 for the DM42 target; the DM42's allocator runs
from 3.29 and 3.31, and a DM42 run with `--dmcp` has not been checked.

### The platform's allocator

malloc, free, calloc and realloc run the platform's own allocator wherever a platform image is mapped, with or without `--dmcp`. That puts the allocator's cost in the
instruction counts, lays the heap out by the calculator's own allocator and, on the DM42, puts the allocator's variables where an overflowing stack reaches them:

| Start | Heap | Drawing |
|---|---|---|
| R47, no option | the platform's, from `res/combo` | stubs |
| R47, `--dmcp` | the platform's | the platform's |
| DM42, no option | the host's, no image being in the tree | stubs |
| DM42, `--platform DMCP_flash_3.31.bin` | the platform's | stubs |
| any, `--host-heap` | the host's | stubs, or the platform's with `--dmcp` |

pgemu never runs DMCP's start. DMCP5 creates its heap with an explicit call, and pgemu makes that call itself, found as the one that sets r1 to the arena's size from
`target.py` and loads r0 with its base: on 3.57 it creates a ThreadX byte pool. DMCP 3.29 and 3.31 for the DM42 have FreeRTOS heap_4, which creates its heap on the first
allocation and loads the ICSR of the system control space on entering a critical section, so pgemu maps that page as zero, no exception active. A trial allocation then
has to land in the arena, or the host's allocator answers, with a note on the console. The four entries still trap to the host, which is how `--malloc-fail` still refuses
an allocation and how the page still draws the heap bar, and the host runs the platform's code through a shim. `sys_free_mem` and `sys_largest_free_mem` keep the image's
own code. `--host-heap` asks for the host's allocator in any case. What DMCP allocates for itself is missing: DMCP 3.31 calls pvPortMalloc from five places near heap_4
besides the wrappers the library entries branch to, and pgemu runs none of them, so the calculator's heap contains blocks that pgemu's does not.

Measured on 2026-09-27 against an R47, with a test build that times 100 000 rounds of malloc and free on the calculator itself:

| | R47 | pgemu, the platform's allocator |
|---|---|---|
| malloc and free of one block, 16 to 8192 bytes | 1.81 to 1.83 us | 234 and 61 instructions |
| one call with 16 blocks live | 0.94 us | |

295 instructions in 1.82 us is one instruction every 6.2 ns, a cycle at 160 MHz. The same builds give identical screens and stack depths with either allocator,
twenty cases on the R47 and nineteen on the DM42.

### An overflow on the DM42

heap_4 keeps its variables right above the arena, xStart at 0x20016054 the highest, so on the DM42 a stack more than 8084 bytes deep writes into them. With the platform's
allocator the stack pattern is laid only above them, and a write watch over them and the top 4 kB of the arena catches the stack there instead, recognised by the stack
pointer being below them too. The run goes on as the calculator's would, and the report names the first write and how deep the stack went:

```
overflow   the stack wrote to 0x20015fd0, in the platform allocator's memory below 0x2001605c, from __gmpn_mul_1+0x0; its pointer went down to 0x20015fd0, 8224 bytes below the entry frame since m
stopped    access to 0x00000000, outside every mapped region (below every region)
```

That is NEXTP of 9E307 from a program on a DM42 build of master: with the host's allocator it finishes, 8224 bytes deep, and with DMCP 3.31's it stops on an access to
address 0 in `__gmpn_mul_1`, after the stack has written over heap_4's variables. PLOT over f′ over a function that shows x with VIEW goes 8144 bytes deep on an LTO build
and writes over xBlockAllocatedBit, and a later free stops in heap_4's assertion, a branch to itself with interrupts masked:

```
stopped    the instruction limit of 1000000000 was reached, in a branch to itself at 0x08018d0e, which is how the platform stops on a failed assertion
```

The depth on the overflow line is the lowest stack pointer until the run stops. A served run names the first write in red under the stack bar, and the bar takes that
depth where the pattern stops short of it. `--host-heap` lets the case finish instead, since the host keeps its allocator's state outside emulated memory, and the stack
line then gives the depth of the whole case. The watch slows a DM42 run by about a third even where it never fires, so it is set only where the heap shares the stack's
region.

## Surviving a DMCP update

pgemu emulates DMCP's interface, the library table and the system data block, and runs DMCP's own code only from a mapped platform image: its allocator, and with `--dmcp`
its drawing entries.

- **The table** is parsed at every start from `lft_ifc.h` in the SDK, so entries added to it are found, and an entry the firmware starts to call without an
  implementation is marked in the report.
- **The interface number** of the image is compared with `PLATFORM_IFC_CNR` and `PLATFORM_IFC_VER` in the SDK's `dmcp.h`, and a difference is printed at start and in
  the report:
  ```
  interface  the image was built for platform interface 3.17 and dep/DMCP5_SDK/dmcp/dmcp.h is 3.16, so the library table the emulator puts down may not be the one the image calls
  ```
- **The system data block** is laid out from `sys_sdb_t` in the same header. A member that is neither a pointer nor a 32 bit integer is refused by name, and a member
  pgemu needs that has gone stops the start.
- **The memory map** in `target.py` was read by hand out of one platform image per target, DMCP5 3.57 and DMCP 3.29. A DMCP that moves the stack or resizes the arena
  runs as before, but the stack and heap figures then describe the old machine.
- **The allocator** is started by the call the platform image makes to create its heap, where it makes one, and by a trial allocation. An allocator pgemu does not
  recognise, or an arena that has moved, leaves the host's allocator answering, with a note at start.
- **With `--dmcp`**, the drawing entries must stay free of peripheral access, and a new image that breaks that stops the run on an access outside every region.

## How it works

The firmware never touches a peripheral register: a search of `src/c47`, `src/c47-dmcp` and `src/c47-dmcp5` for `__disable_irq`, `SCB->`, `NVIC_`, `SysTick` and
`0xE000E` finds nothing, so the whole hardware interface is the library table at `LIBRARY_FN_BASE`, and there is no timer, interrupt or DMA model here.

The library table is code, not pointers: the macro in `lft_ifc.h` is `(*(typeof(fn)*)(LIBRARY_FN_BASE+offset))`, so a call branches straight to the entry. An entry with a
stub becomes a `b.w` to it, and where the platform's allocator runs, `sys_free_mem` and `sys_largest_free_mem` keep the image's own branch, as the drawing entries do with
`--dmcp`. Every other entry is an `svc` followed by `bx lr`, and the `svc` reaches the host through `UC_HOOK_INTR`, which costs nothing until it fires; a code hook would
take the fast block chaining away from the whole image. An entry that has to run emulated code before it can answer, the selection function of `file_selection_screen` or
the platform's allocator, is answered in two parts through three instructions in scratch memory, `blx r3; svc; bx lr`.

A file opened for reading is put into emulated memory whole, and the `f_read` stub takes bytes from there and advances the FIL's own `fptr`, so restoring a state
file reaches Python no times at all.

The stubs are checked in under `prebuilt/`, one file per processor, and taken only where they were built from the same `stubs.c`, the same table and the same
processor. Where none matches, the toolchain builds them into `.stubs-cache`, and without a toolchain every entry is answered in Python, which is slower and gives the
same result. Regenerate them after editing `stubs.c` or updating an SDK, which needs the ARM toolchain and the SDK submodules:

```
python3 tools/pgemu/stubs.py --emit
```

`--selftest` runs `selftest.c` inside the emulated machine instead of the firmware. It reaches `check_create_dir`, `f_open`, `f_write`, `f_lseek`, `f_read` and
`f_close` through the library table, checks the bytes and the FIL fields at every step, and reports one bit per failed check; it needs the toolchain as well.

| File | Contents |
|---|---|
| `pgemu.py` | the loader, the memory map, the trap, the options and the report |
| `target.py` | the two memory maps, where each target's platform image is kept, and the interface and `sys_sdb_t` layout read from the SDK |
| `lft.py` | the library table, parsed from the SDK header |
| `hostfn.py` | the host implementations, one per table entry, and the reading of the platform's heap |
| `hostfs.py` | the FatFS entries, against a directory on the host |
| `stubs.c`, `stubs.py` | the entries answered inside the emulated machine, and the table built around them |
| `profiler.py` | the instruction counts of `--profile` |
| `keyscript.py` | the key script |
| `serve.py` | the page of `--serve`, standard library only |
| `picker.py` | the host's own file chooser for `--pick` |
| `elfsym.py` | function names and ranges from the ELF, for the options that watch functions and for `--profile` |
| `selftest.c` | the file system self test |
| `MEASUREMENTS.md` | figures measured with pgemu, each with its build and date |

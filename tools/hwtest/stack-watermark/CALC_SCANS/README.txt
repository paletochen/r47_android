================================================================================================================
FUNCTION KIND TEST
================================================================================================================

Time per call and working memory depth for forty kinds of function, one or two functions each. No nesting: each
case runs one engine at most.

This sits beside the stack tool it measures with. The depth half is the firmware's STACK_WATERMARK, whose own
how-to is ../README.txt: read that for the marker, the anchor and the STCKST codes. This adds the time and the
coverage across kinds, and the runs from every calculator it has been on.


RUN
---

Calculator   Flash a build with STACK_WATERMARK enabled in defines.h. Without it you get the times and no
             depths, and every STCKST reads 9 to say so.
             Then READP PROGRAMS/FNKIND.p47, XEQ FNKIND, leave the keyboard alone. Copy the dated .REGS.TSV
             and the six .bmp out of DATA into one folder, a new folder per run.
Simulator    ./t47 --reset --exec 'readp FNKIND.p47; xeq FNKIND'      41 s, same files in the cwd

Fix the power state before you start and keep it: DM42n 160 MHz on USB, 80 on battery.
READP appends and the first copy of a label wins, so delete an old copy before loading a new one.


READ
----

One command, from anywhere, no arguments. It finds every run and writes both files:

    python3 checkfnkind.py

    report.txt   every run as a table, then what changed between each run and the one before it on the
                 same calculator. Runs from different calculators are never compared
    runs.xlsx    the same figures side by side, one block of columns per run, ms/call as a formula so
                 the sheet still adds up if a figure is corrected. Second sheet holds the captures

The rest is there when you want a single thing:

    python3 checkfnkind.py show            the newest run, on screen
    python3 checkfnkind.py compare         the newest two, older against newer
    python3 checkfnkind.py list            the runs it can see

Name a run when you want a particular one. A path or any part of the folder name will do, and a wrong name
prints the list rather than an error:

    python3 checkfnkind.py show    "2027-07-29_001 DM42n RC2/2"
    python3 checkfnkind.py compare "2027-07-29_001 DM42n RC2/1" "2027-07-29_001 DM42n RC2/2"

A run folder is any folder holding a .REGS.TSV, anywhere under this one. Make one per run and the tool does
the rest.

    stack    bytes used below the reference point
    status   0 means the figure is a depth reached. Anything else is not a measurement
    span     how far down was marked
    tenths   what the counted calls took
    calls    how many fitted
    ms/call  tenths over calls, less case 01. The answer

compare returns 1 if anything moved: depth in bytes, calls as a percentage, capture SHA-256 in order. Nothing
is stored anywhere. The runs are the record, so keep them.


STATUS CODES
------------

    0   depth reached, a measurement
    1   marker ran out, so the figure is a floor. Raising STCKSPN would measure it, but read the warning
    2   nothing disturbed. The case is shallower than the tool's own frame and cannot be resolved
    3   no marker laid, so the figure repeats the row above
    9   no stack tool in this build

Raising STCKSPN is not free. The marker is written to that depth whether or not it is your memory. Too far and
nothing fails at the time: the calculator hard faults on the next reset and needs reflashing. The defaults are
what each machine is known to survive. 8088 is the only depth ever run on a DM42, and 16384 the largest ever
confirmed written on a DM42n, where a run asking for 55000 crashed on the next reset. Raise it on one machine
at a time, in steps, and only where a case came back STCKST 1. Full detail in ../README.txt.

    20000 STO 'STCKSPN'     then XEQ FNKIND, the store being yours and not the program's


READING THE ODD ROW
-------------------

calls under 20            One call either way is over five per cent, so the checker reports rather than
                          compares. For SOLVE one call exceeds the window and that is the result.
output stops part way     A result. Cases run cheapest first, so where it stops is the first thing the machine
                          could not do. Every line written survived, the file is closed after each.
VECTOR near zero          A DM42 build leaves OPTION_VECTOR out. The functions are then empty, not absent, so
                          the case runs and measures nothing. It is the last case but one for that reason.
drawing capture is stack  Firmware predates the screenHoldsDrawnPixels change. Nothing else affected.
depth near the floor      There the figure is the tool's own frame, not the case, and it moves by up to 140
                          bytes between identical runs. The checker allows 160.


THE CASES
---------

  01 NULL       nothing, the control                     21 SHORTINT   integer mode, AND
  02 STK        ENTER, x<>y, roll down                    22 BITS       MIRROR, shift left
  03 REG        STO and RCL a numbered register           23 LONGINT    a 30 digit integer squared
  04 VAR        STO and RCL a named variable              24 CPX        a complex number squared
  05 ARITH      real multiply and add                     25 MATRIX     a 3 by 3 inverted
  06 TRIG       sine and arc cosine                       26 STRING     the left three characters of a string
  07 LOGEXP     natural log and exp                       27 STAT       five points summed, the mean
  08 POWER      y to the x, square root                   28 CURVEFIT   linear regression, correlation
  09 SPECIAL    gamma, the error function                 29 STATPLT    five points, a scatter plot
  10 CONST      a physical constant, pi                   30 PROBDIST   normal left tail and its inverse
  11 RANDOM     a random number                           31 FIN        a time value of money solve for FV
  12 FLAGTEST   set, test and clear a flag, compare       32 GRAPHICS   clear the screen, a pixel, a point
  13 DISPMODE   SCI, ALL, FIX                             33 SNAP       a screen capture
  14 ANGLE      degrees to radians and back               34 EQN        a formula solved over the complex field
  15 UNITCONV   feet to metres and back                   35 DERIV      the first derivative of a program
  16 TIME       hours minutes seconds to hours and back   36 INT        an integral
  17 DATE       a real to a date, the date to Julian      37 SOLVE      a root
  18 COMBIN     combinations and permutations             38 PLOT       a function plotted
  19 NUMTH      the next prime, a common divisor          39 VECTOR     a 3D vector, a dot product
  20 FRACT      a decimal decomposed into a fraction      40 HYPERB     a hyperbolic sine and its inverse

Captures, in order: scatter plot of 29, drawing of 32, stack twice from 33, plot of 38, screen at the end.

Writes R95 to R98, R20 to R23, the variable kvv, the STCK set, the TVM variables and the stat registers. About
9 kB of program memory. SAVEST first if the state matters.


METHOD
------

Depth from one call, marker laid before and read after. Time from as many calls as fit a one second window,
counted.

A window rather than a fixed call count is what lets one program serve the simulator, a DM42 and a DM42n. Those
differ by eighty times and then by three to five again, so no fixed count suits all three. A window costs the
same second everywhere and the count is what changes. Window zero means one call and no more; SNAP is set that
way since every call writes a file.

The program never stores STCKSPN, so each machine's own default stands: 8088 on a DM42, 16384 on a DM42n,
65536 in the simulator. Raising it is yours to do, before the run, and it is not a safe thing to do blind. See
the warning under STATUS CODES.

Case 01 is empty, so it carries the loop, XEQ, RTN, counter and clock read every case pays. The checker
subtracts it.

ACC 1E-3 on the integration, SDIGS 6 on the plot, matrix and solve cases. Cost and depth are under test, not the
answer.

Depth is the firmware's STACK_WATERMARK, documented in ../README.txt. This adds the time and the
coverage. Output goes to a dated DATA\*.REGS.TSV, five lines a case: the depth, s<nn> status,
u<nn> span, c<nn> calls, e<nn> tenths. A capture also appends the stack in a block with no quoted label, which
the checker skips.


LIMITS
------

- It does not say how much working memory there is. Nothing does: neither DMCP nor either linker script names
  it, and MR !1610 could not establish it. The figure is what was used against an unknown budget.
- Simulator depths carry an unknown constant, since the reference point there is wherever the first measurement
  landed rather than the head of program_main. They compare with each other only, never with a calculator.
- One or two functions stand for a kind, and a kind is not uniform. This finds the shape, not the worst case.
- Time is per call including the interpreter's per-step cost, which on hardware includes a keyboard poll the
  simulator does not pay. tools/bench separates those.


CHANGING A CASE
---------------

Edit the table at the top of genfnkind.py, then:

    python3 genfnkind.py
    ./rejig FNKIND.txt -o PROGRAMS/FNKIND.p47
    ./rejig FNKVFY.txt -o PROGRAMS/FNKVFY.p47
    sh probe_cases.sh <folder holding t47 and FNKVFY.p47>

The last step is the one that matters. A case running without an error has not been shown to compute anything:
several first drafts here ran clean and did nothing, and one inverted a matrix that had come out singular.
FNKVFY leaves each result in X so it can be read.

Put a new case where its cost puts it, cheapest first. Changing a window means older runs no longer compare.

Expected values, simulator, factory state, RAD, RC2. A dash means no result and only the absence of an error is
the check. The six SDIGS 6 cases print six digits by design.

  01 NULL       -                       21 SHORTINT   00000000-000000aa, 255 AND 170
  02 STK        1                       22 BITS       00000000-00000ff0, 255 shifted left four
  03 REG        7                       23 LONGINT    1524157875323883675049535156253619878750190519987501905...
  04 VAR        7                       24 CPX        -3 + ix4, the square of 1 + i2
  05 ARITH      7.25                    25 MATRIX     a matrix, printed as unsupported
  06 TRIG       1.0707963267948966...   26 STRING     ABC
  07 LOGEXP     2.5                     27 STAT       11, the mean of 1, 4, 9, 16 and 25
  08 POWER      4.9704420547940666...   28 CURVEFIT   0.9811049102515928..., the correlation
  09 SPECIAL    0.5204998778130465...   29 STATPLT    -
  10 CONST      3.1415926535897932...   30 PROBDIST   1.9599639845400542..., the 97.5 per cent point
  11 RANDOM     a number under one      31 FIN        -1628.89, 1000 at 5 per cent for 10 years
  12 FLAGTEST   2                       32 GRAPHICS   150 over 80, the coordinates POINT left
  13 DISPMODE   -                       33 SNAP       -
  14 ANGLE      45                      34 EQN        0 - ix2.00000, a root of x squared plus four
  15 UNITCONV   10                      35 DERIV      -2.0000000000000000..., the slope of 4/(1+x^2) at 1
  16 TIME       12.3456                 36 INT        3.1415962, pi to the accuracy the case sets
  17 DATE       2461251, a Julian day   37 SOLVE      1.41421, the root of x squared minus two
  18 COMBIN     390700800               38 PLOT       -
  19 NUMTH      1                       39 VECTOR     14, the dot product of (1,2,3) with itself
  20 FRACT      8, 0.375 being 3 over 8 40 HYPERB     0.5, sinh then its inverse

FILES
-----

  PROGRAMS/FNKIND.p47  the program, same one on calculator and simulator
  PROGRAMS/FNKVFY.p47  the check listing, for probe_cases.sh
  checkfnkind.py       show and compare
  genfnkind.py         writes both listings from the case table. --windows overrides the windows from a file,
                       --check verifies the checked in listings against the table
  FNKIND.txt           the listing as the calculator shows it
  FNKVFY.txt           the same cases with the clean-up left out
  probe_cases.sh       runs every case on its own and prints what it left
  bmprow.py            a band of rows of a capture as text, for softkey labels with no viewer
  bmpthumb.py          a whole capture, coarsely
  report.txt           written by checkfnkind.py: every run, and what changed between each and the one before
  runs.xlsx            written by checkfnkind.py: the same figures side by side, ms/call as a formula
  <date>_<nnn> <machine> <build>/<n>/
                       one folder per run, holding what came off the calculator: the dated .REGS.TSV and the
                       .bmp captures. A run finished in two goes leaves two TSVs in the one folder and the
                       tool merges them. The runs are the record, so keep them

No firmware is kept here. Build it with STACK_WATERMARK enabled in defines.h and flash from the build folder.


BUILD NOTES
-----------

- rejig reads a literal with four or more decimals as an angle or a time: 2026.0729 becomes 2026 deg 7 min 29
  sec, 12.3456 becomes 12:34:56. x->D then refuses it, the register carrying an angular tag. Such values are
  built by division in KSET and recalled. Reported, with files, under rejig-bugs.
- A hash at the start of a token starts a REM, so #B inline after other steps on one line is silently discarded
  by rejig. On its own line it encodes correctly. MIRROR stands in here. Same report.
- STOEL does not step the index and is not meant to; STOSEQ steps. Filling with STOEL needs a STOIJ per element
  or eight of nine stores land on one cell and the matrix comes out singular.
- TVM variables are NPPER, I%/a, PPER/a, CPER/a, PV, PMT, FV, not the catalogue names. A catalogue name creates
  an ordinary variable and the solve reads whatever was there before.
- LINE is not programmable, and Calc f was not when this was written. cpxSlv reaches the formula path, POINT the
  drawing.
- alpha LEFT takes the string from the register in its argument and the count from X.
- L.R. uses whatever fit the user last selected, so a case that does not set one is reading outside state:
  the same five points gave CORR 1.000000 under AllF against 0.981105 under a linear fit. The model keys are
  toggles and are no longer programmable. Set the fit with ResetF then BestF nn, the bit value of the model:
  1 linear, 2 exponential, 4 logarithmic and so on.
- Measuring the depth does not distort the time. Two simulator builds differing only in STACK_WATERMARK, same
  listing, same windows: ratio 0.967 to 1.038, median 1.000, which is the TICKS resolution. The tool's cost is
  at every keystroke outside a run, not inside one.

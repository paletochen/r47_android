================================================================================================================
STACK HIGH-WATER MEASUREMENT
================================================================================================================

Measures how much working memory an operation actually uses, on hardware or in the simulator.

The calculator gives each running operation a stretch of memory to work in. Every function call takes a slice of it, and calls inside calls take more, so a
nested INT inside a PLOT goes much further down than a bare addition. If an operation needs more than there is, nothing warns you. On the DM42 that has been
seen as a hang with the running annunciator on and no key answered, and the registers are in the same memory there, so an overrun writes into them.


HOW IT WORKS
------------

Before the case runs, the tool writes a known marker value into a stretch of that memory. The case then runs and overwrites the marker as far down as it needs
to go. Afterwards the tool looks for the lowest place the marker is gone. That place is how deep the case went, and the answer is how far it is from a fixed
reference point.

On a calculator that reference point is the start of the firmware's main function, so it is the shallowest the memory ever is. The simulator has no such
function, so it anchors wherever the first measurement happens to be. Simulator figures therefore carry an unknown constant and compare only with each other,
not with a calculator.

   TOP of the memory
   +------------------------------+  <-- fixed reference point, set at start-up
   |                              |
   |  marker overwritten:         |   the case worked down through here
   |  the case used this          |
   |                              |
   +------------------------------+  <-- lowest place the marker is gone. THE ANSWER is the
   |                              |       distance from here up to the reference point
   |  marker still there:         |
   |  the case never got this far |
   |                              |
   +------------------------------+  <-- bottom of the marked stretch, set by STCKSPN
   |  never marked, never checked |
   BOTTOM

The alternative would be to read the depth at some moment while the case runs. That misses the peak in between two readings, which is why the marker is used
instead.


THERE IS NO FIGURE TO COMPARE AGAINST
-------------------------------------

The operating system hands out that memory while the calculator runs and never says how much. Neither linker script reserves it or names it, and MR !1610 says
the same and could not pin it down. So the tool tells you how much was used. Nobody knows how much there is.


THE TOOL IS OFF BY DEFAULT
--------------------------

Enable it in defines.h:

    #define STACK_WATERMARK
    //#undef  STACK_WATERMARK

With it on, every keystroke outside a running program rewrites the whole marked stretch and then searches it again, so expect the calculator to feel slow.
Inside a running program that happens only where STCKGO asks for it.

A testSuite build switches it off again by itself. defines.h undefines it under TESTSUITE_BUILD, alongside the other diagnostics, because the tool creates
named variables and writes them at every dispatch, which would alter the register state the corpus compares against. The suite therefore cannot be measured
with this tool.


CONTENTS
--------

  PROGRAMS/stklog.txt   the supplied test in readable form: eight PLOT, INT and SOLVE combinations, shallowest first,
                        three lines of output each. Re-encode from the repository root with
                          ./rejig tools/hwtest/stack-watermark/PROGRAMS/stklog.txt -o tools/hwtest/stack-watermark/PROGRAMS/stklog.p47
  PROGRAMS/stklog.p47   the program itself. XEQ STKALL runs all eight in one go.
  results/              captured results, from the calculators and from the simulator.


THE VARIABLES
-------------

  STCKHI    the figure for the case just measured
  STCKST    what that figure is worth, see below
  STCKHWM   the deepest seen since you last cleared it; store 0 to start again
  STCKGO    store 1 to lay down a fresh marker, 2 to take a reading
  STCKSPN   how far down to mark
  STCKSPU   how far down was actually marked, after any raising of the floor

All are long integers. A real34 goes to the file through the display format, which turned 7080 into 7.08E+3 even under FIX 00.

STCKHI, STCKST, STCKHWM and STCKSPU are created by the tool itself at the first keystroke after switching on. They are ordinary named variables from then on:
they appear in the VAR menu, they go into state files, and they can be deleted.


STCKST: IS THE FIGURE REAL?
---------------------------

A number comes out every time, whether the tool measured anything or not. STCKST says which:

  0   the marker was gone part of the way down and still there below
      YES, this is the depth the case reached

  1   the marker was gone all the way to the bottom of the marked stretch
      no, the case went at least that far and possibly further. Mark deeper and run again

  2   the marker was untouched everywhere
      no, nothing wrote into the marked stretch at all. The figure is just where the tool itself was sitting

  3   no fresh marker was laid down before this case ran
      no, you are reading whatever the case before left behind

Read it on every line. Without it all four look like the same kind of number, and a column of identical figures reads as a result when it is really the tool
failing to measure.


MEASURING ONE CASE AT A TIME
----------------------------

STCKGO is what lets a single program run measure several cases separately, instead of only reporting the worst of the whole run:

    1 STO 'STCKGO' DROPX                             ; lay down a fresh marker
    XEQ 'MYCASE'                                     ; the operation under test
    2 STO 'STCKGO' DROPX                             ; take the reading
    CLSTK                                            ; the function that carries it out, see below
    RCL 'STCKHI'  'MY CASE'  🖨xy  DROPX DROPX       ; the figure for that case alone
    RCL 'STCKST'  's'        🖨xy  DROPX DROPX       ; whether it is real
    RCL 'STCKSPU' 'u'        🖨xy  DROPX DROPX       ; how far down was marked

A request is carried out by the next function the program runs, not by the store itself.

Without a request, a reading is taken where an operation ends and at the end of a top-level program run. It is not taken after every single step: each reading
searches the whole marked stretch, and doing that per step leaves the calculator crawling with nothing on screen.

CLSTK is used above because it also leaves the graph screen, which a plot case must do before anything can be written to a file. CLRMOD and EXIT leave the
graph too but both stop the running program, and EXITALL only closes menus.


HOW FAR DOWN TO MARK
--------------------

This cannot be worked out, so it has to be found by trying. The default is STACK_WATERMARK_SPAN in memory.c, set per target so the supplied eight cases need
nothing stored:

  target      default   why
  ---------   -------   ------------------------------------------------------------------------------
  DM42           8088   arbitrary, and the only depth ever run there; it covers every case that does not hang
  DM42n         16384   reaches its deepest case at 14244, and is the largest depth confirmed written on either calculator
  simulator     65536   clears the deepest simulator case ever captured, 19152

STCKSPN overrides it, to any depth from 1 up to 262144, above or below that target's default:

    25000 STO 'STCKSPN' DROPX     ; then run as usual

It is read where a program asks for a fresh marker with STCKGO 1, and held from there on. It is not looked up again each time a marker is laid, because that
also happens unasked at any step outside a run.

Raise it while cases keep coming back with STCKST 1. Go too far and the marker lands in memory that was never yours: nothing fails at the time, and the
calculator hard faults on the next reset and needs reflashing. That is the cost of finding the edge.

The largest depth confirmed as actually written is 16384, on the DM42n, where STCKSPU read back 16384 on all eight cases. A DM42n run asking for 55000 crashed on the next
reset, but no capture shows how far it reached, so it does not bound anything. An earlier run asking for 52500 completed, but its STCKST 1 lines read
8088, so that run marked 8088 and says nothing about 52500. Nothing bounds the DM42, which never got past case 6 at any setting.

On the DM42 the registers live in the same memory as the working stretch, below it, and the tool is written to stop short of them whatever STCKSPN says. That
has never actually happened in a run: in the simulator the registers are elsewhere, so the check is always false there and cannot be exercised.


RUNNING IT
----------

Build with STACK_WATERMARK enabled. Cases 6 to 8 put an engine inside a plot, which the DM42 refuses as shipped, so to run them there PLOT_NESTING_ALLOWED
must also be set to 1 for OLD_HW. That guard exists because case 6 runs the DM42 out of memory, so with it off that case will hang or fault there; a reset
recovers the calculator, and the hang is itself the result being recorded. The DM42n and the simulator already allow it.

The system flag 🖨ACT is clear unless you set it, so the output goes to a file without anything being enabled. Load the program, then XEQ STKALL.

Output is added to DATA\*.REGS.TSV, three lines per case: the figure, its STCKST, then STCKSPU. The file is opened, written and closed for
each line, so every case that finished survives a crash in a later one. With the cases ordered shallowest first, where the file stops is itself a result.

Expect the DM42 to stop at case 6, which hangs there. Cases 7 and 8 are not reached. Case 8 would be refused anyway on that machine, since it runs three
engines at once and the DM42 allows two: the screen reads "Nesting too deep" and the program stops. The DM42n allows three and runs all eight.


MEASURED
--------

DM42, taken twice by different means: T79 read after every step, T87 read on request with an STCKST on each line and nothing marked beyond 8088. Beside them
the simulator at STCKSPN 65536 for the same eight cases. T79 came from an earlier version that read after every dispatch; only the T87 and T88 columns can be
reproduced with the code as it stands.

The STCKST column is the figure beside it: 0 measured, 1 the mark ran out, 2 nothing disturbed, 3 stale. Only 0 is real. Full key under STCKST above.

  #   case                             DM42 T87   STCKST   DM42 T79      simulator
  --  -----------------------------    --------   ------   -----------   ---------
  1   BASE, no engine                      2240        0          2292        6688
  2   INT                                  5436        0          5388        8864
  3   SOLVE                                4644        0          4536        9216
  4   PLOT                                 6932        0          6932       10912
  5   INT nesting INT                      7684        0          7684       13200
  6   PLOT nesting INT                     hung        -          hung       14448
  7   PLOT nesting SOLVE            not reached        -   not reached       16192
  8   PLOT nesting INT nesting INT  not reached        -   not reached       18720

The two DM42 columns agree to the byte on the two deepest cases and within about 90 bytes elsewhere. Nothing came back STCKST 1, so 8088 already covered every
case that ran there and a larger STCKSPN would add nothing.

Set STCKSPN to 8088 in the simulator and most cases come back at exactly 8088 with STCKST 1, captured in results/sim-status-2026-07-27.tsv. That is the
pattern to watch for when the depth is too shallow, and it is what the DM42n produced on T80 and T81 before the defaults were set per target.

Read the two columns for shape, not ratio. The simulator is a 64-bit host build with its own call sizes, and it anchors lower, which together put its baseline
at 6688 against the calculator's 2292. The order differs too: on the DM42 SOLVE is cheaper than INT, on the host it is dearer.

What the DM42 columns establish: a plot on its own fits at 6932, and INT inside INT fits at 7684. Those are measurements, taken twice by different means.

Case 6 reports nothing on the DM42. A plot with an integral inside it hangs there: the running annunciator stays on, the iteration counter does not advance,
and no key is answered, repeatedly across about 70 runs. The DM42n measures that same case at 11932, far past the 7684 the DM42 is known to survive, so the
hang is the DM42 running out.

The deepest figure the DM42 is known to survive is 7684. Its actual limit is unknown, only that it is under what case 6 needs.

The simulator figures move a little between runs, because how many steps the integrator takes depends on the values the case before left behind. Cases 6 and 7
can swap order for the same reason. Compare like with like: same order, same build.

DM42n with T88, results/T88-dm42n-2026-07-27.tsv, at STCKSPN 16384. All eight are real, and every one confirms 16384 was marked:

  #   case                            DM42n   STCKST   STCKSPU
  --  -----------------------------   -----   ------   -------
  1   BASE, no engine                  2308        0    16384
  2   INT                              5356        0    16384
  3   SOLVE                            4840        0    16384
  4   PLOT                             9124        0    16384
  5   INT nesting INT                  7652        0    16384
  6   PLOT nesting INT                11932        0    16384
  7   PLOT nesting SOLVE              11916        0    16384
  8   PLOT nesting INT nesting INT    14244        0    16384

T87 gave the same figures within run-to-run variation: 2380, 5340, 4840, 9092, 7676, 12020, 11852, 14260.

Case 4 at 9124 is above the 8088 the DM42 uses, which is why runs before this one held PLOT at 8088 and looked constant.

The earlier T86 attempt at the same setting gave 8088 with STCKST 1 for cases 6, 7 and 8. An STCKST 1 figure is how far down the marker went, so that said only
8088 had been marked while 16384 was stored. STCKSPN was being looked up afresh each time a marker was laid, including at dispatches inside a running plot,
and those lookups sometimes did not find it. It is now taken once, where the program asks for a fresh marker, and held. STCKSPU reports what was marked, and
the run above shows 16384 on every case. Why the old lookups missed was never established; the code path that made them is gone. STCKGO is still looked up at
every dispatch inside a running program. If that lookup ever missed, the request would go unserved and the case would report STCKST 2 or 3 rather than a wrong
figure, so it would show rather than mislead. No run has shown it.


WRITING YOUR OWN CASE
---------------------

Follow the shape in stklog.txt. Keep ACC loose at 1E-4 if the case integrates, since only the depth is under test and not the answer. Put the shallowest case
first. ACC cannot be a program step, so set it by storing to the reserved variable, 1E-4 STO 'ACC'.


BASE
----

Written against 8d7a80a5d. The switch is STACK_WATERMARK in src/c47/defines.h, the reference point is set at the head of program_main in src/c47/c47.c, the
marking and the search are in src/c47/memory.c, and the two places they are called from are src/c47/items.c and src/c47/programming/lblGtoXeq.c.

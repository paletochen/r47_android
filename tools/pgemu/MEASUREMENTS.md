# pgemu measurements

Figures taken with pgemu, each with the build and the date it was measured on. They describe those builds: the firmware changes, and a figure here is a starting
point for a new measurement, not a reference to test against. How each kind is taken is in the [README](README.md).

1. [Stack depth against the calculator](#stack-depth-against-the-calculator)
2. [The stack room of each machine](#the-stack-room-of-each-machine)
3. [Engine depths](#engine-depths)
4. [Base functions](#base-functions)
5. [Programs over the edge](#programs-over-the-edge)
6. [Speed](#speed)
7. [Instruction counts](#instruction-counts)

## Stack depth against the calculator

On 2026-09-07 one `dmcp5r47` build with `STACK_WATERMARK` enabled was flashed to an R47 and given to pgemu, both ran `STKALL` from
`tools/hwtest/stack-watermark`, and both were read from the `DATA/*.REGS.TSV` the firmware writes itself. Only the machine differed.

| Case | R47 hardware | pgemu | Difference |
|---|---|---|---|
| INT | 3584 | 3352 | 232 |
| SOLVE | 2736 | 2600 | 136 |
| PLOT | 6900 | 6696 | 204 |
| INT inside INT | 5108 | 4776 | 332 |
| PLOT over INT | 6832 | 6696 | 136 |
| PLOT over SOLVE | 6948 | 6696 | 252 |
| PLOT over INT over INT | 7828 | 7488 | 340 |

pgemu came out a few hundred bytes short every time. The leading candidate is that nothing interrupts here: on hardware DMCP's timers fire, and an exception frame
with FP context is 104 bytes plus the handler's own, wherever the code happens to be.

## The stack room of each machine

| Machine | Stack | What is below it |
|---|---|---|
| DM42 | 8104 bytes, from 0x20017ff0 to the end of the arena at 0x20016048, of which heap_4's variables take the lowest 20, which leaves 8084 | DMCP's heap, heap_4's variables first, so an overrun breaks the allocator before it reaches anything C47 has allocated |
| R47 | 65536 bytes, SRAM2 to itself | SRAM1 with DMCP's own data; the arena is above the stack, in SRAM3 |

Both were read out of the platform images, DMCP 3.29 and DMCP5 3.57, because the linker scripts place neither; `target.py` states the addresses and how they were
found. The depth of a case does not depend on the machine: the same programs gave the same figures on the R47 and the DM42 images.

heap_4's variables, from xBlockAllocatedBit at 0x20016048 to the end of xStart at 0x2001605c, are at the same addresses in DMCP 3.29 and 3.31, found on 2026-09-27. The
calculator's heap contains blocks that pgemu's does not: DMCP 3.31 calls pvPortMalloc from five places near heap_4 besides the wrappers the library entries branch to, and
pgemu runs none of them, so C47's blocks presumably sit higher in the arena there than in pgemu. In pgemu, NEXTP of 9E307 on a DM42 build of master e2bfd63dc left the
highest address the heap handed out 21 336 bytes below the end of the arena with the host's allocator and 23 772 with DMCP 3.31's.

## Engine depths

The rule and its terms are in the README. Floors and k as measured:

| Engine | Floor | k | Build |
|---|---|---|---|
| SOLVE | 1992 | 1104 | master, 2026-09-10 |
| INT | 3400 | 1424 | master, 2026-09-10 |
| f′, f″ | 3592 | 2168 | master, 2026-09-10 |
| PLOT | 6744 | 2712 | master, 2026-09-10 |
| f′, f″ | 2096 | 672 | branch `deriv-frame-to-heap`, 2026-09-12 |
| Σn | 1872 | 768 | branch `deriv-frame-to-heap`, 2026-09-12 |
| Πn | 2008 | 768 | branch `deriv-frame-to-heap`, 2026-09-12 |

A floor depends on the trivial program chosen and k does not: the branch measured SOLVE, INT and PLOT over another trivial program, found other floors and the same
k. Every combination below was predicted by the rule first and measured afterwards, on master of 2026-09-10, and none differed:

| Combination | Predicted | Measured |
|---|---|---|
| INT inside INT | max(3400, 3400 + 1424) | 4824 |
| INT inside INT inside INT | max(3400, 4824 + 1424) | 6248 |
| f′ over Iγp | 4360 + 2168 | 6528 |
| PLOT over Iγp | 4360 + 2712 | 7072 |
| f′ over f′ over LN | 5312 + 2168 | 7480 |
| PLOT over f′ over LN | 5312 + 2712 | 8024 |
| f′ over f′ over Iγp | 6528 + 2168 | 8696 |
| PLOT over f′ over Iγp | 6528 + 2712 | 9240 |
| four f′ over f(x) = x, OVER2 | 3592, 5760, 7928, then 10096 | 10096 |
| PLOT over f′ over f′ over LN, OVER1 | 7480 + 2712 | 10192 |

On the branch OVER2 and OVER1 measured 4112 and 7200.

What that meant on the DM42 at the time: `engineNestingRefused` in `src/c47/solver/solve.c` allowed one engine at a time there, `MAX_ENGINE_NESTING_DEPTH` 1 under
`OLD_HW`, but it counted only PLOT, INT and SOLVE. The derivatives, Σn and Πn were not counted, so PLOT over f′ over Iγp ran with nothing to stop it: 9240 bytes,
the deepest word 1136 bytes inside the block pool's range.

Three things did not move the stack:

- **Program nesting.** A chain of twenty XEQ levels gave the same 1576 as one, because the interpreter keeps its return levels in the block pool.
- **Matrix size.** A 4×4 and a 12×12 determinant both gave 2924. A large matrix runs out of block pool first, `RAM is full` for a 24×24 on the DM42.
- **Equation nesting.** The parser has its own stacks of ten entries, `PARSER_OPERATOR_STACK_SIZE` in `src/c47/solver/equation.c`, and refuses a fifth level of
  parentheses with `This equation formula is too complex` long before the C stack is reached.

## Base functions

A program that takes its argument and calls one function, DM42 build of 2026-09-10. A program that only returns gave 1576, and building 3+4i with CC gave 2108:

| Function | Real | Complex |
|---|---|---|
| ζ | 3728 | 4864 |
| β | 2688 | 4720 |
| LNΓ | 3752 | 4608 |
| Iγp | 4360 | refused |
| AGM | 2864 | 4280 |
| Γ | 2376 | 4208 |
| erf | 3992 | refused |
| LN | 3144 | 3208 |
| SIN | 2312 | 3120 |
| EXP | 2576 | 2856 |
| √ | 2344 | 2640 |

A plotted program cannot build a complex this way: `fnKeyCC` acts only in `CM_NORMAL` and `CM_NIM`, and a plot samples in `CM_GRAPH`, so the two reals stay two reals.

## Programs over the edge

OVER1 and OVER2 are the two cases above that went past the DM42's 8104 bytes on master of 2026-09-10, one label per block:

```
LBL 'AAAA'                          LBL 'AAAA'
  1E-3 STO 'δ⒟' DROPx                  1E-3 STO 'δ⒟' DROPx
  PGMPLT 'DDDD' 1 4 PLTf 'z'          2 PGMDRV 'EEEE' f' 'w'
RTN                                 RTN
LBL 'DDDD'                          LBL 'EEEE'
  MVAR 'z' PGMDRV 'CCCC'              MVAR 'w' PGMDRV 'DDDD' RCL 'w' f' 'z'
  RCL 'z' f' 'y'                    RTN
RTN                                 LBL 'DDDD'
LBL 'CCCC'                            MVAR 'z' PGMDRV 'CCCC' RCL 'z' f' 'y'
  MVAR 'y' PGMDRV 'BBBB'            RTN
  RCL 'y' f' 'x'                    LBL 'CCCC'
RTN                                   MVAR 'y' PGMDRV 'BBBB' RCL 'y' f' 'x'
LBL 'BBBB'                          RTN
  MVAR 'x' RCL 'x' LN               LBL 'BBBB'
RTN                                   MVAR 'x' RCL 'x'
                                    RTN
```

`1E-3 STO 'δ⒟'` sets the step of the derivative. Without it f′ searches for a step a decade at a time, up to sixteen decades of a fifteen point stencil at each
level, and nesting multiplies that: three nested derivatives of LN did not finish in 900 seconds, and with δ⒟ set they took 15. ACC does the same for INT, and
`1E-4 STO 'ACC'` took a triple integral from past 900 seconds to under two minutes.

## Speed

Measured on the Mac pgemu is developed on, with the date each figure was written down.

| What | Figure | When |
|---|---|---|
| a two instruction branch to itself | no hook 488 M instructions/s, an interrupt hook 489 M/s, a code hook over a range the branch never enters 26 M/s | 2026-09-07 |
| one DMCP call answered in Python | about 16 µs | 2026-09-12 |
| a boot, every entry on the host against the stubs | 2.70 s against 0.40 s, when a boot made about 157 000 DMCP calls, almost all `bitblt24` | 2026-09-12 |
| the same boot | 0.91 s against 0.74 s, with about 22 000 DMCP calls: 15 900 `bitblt24`, 4 700 `f_read` restoring `R47auto.sav`, 1 500 `lcd_fill_rect` | d3d61cb8a, 2026-09-18 |
| `STKALL` | 1 min 47 s against about 1 min 30 s on the R47 | 2026-09-07 |
| boot, typing and the I/O menu with `--dmcp`, with `--profile` and without | 2.51 s against 0.38 s | d3d61cb8a, 2026-09-18 |
| OVER1 with and without a code hook on a derivative function | 14.0 and 15.6 s with, 15.8 and 15.6 s without | 2026-09-12 |

`--dmcp` against the stubs, best of three runs on an R47 image, 2026-09-12:

| Run | Without | With `--dmcp` |
|---|---|---|
| boot | 0.48 s | 0.59 s |
| LOADST, confirmed | 0.81 s | 1.56 s |
| 500 x! | 0.64 s | 1.03 s |
| a plot | 1.26 s | 1.35 s |

## Instruction counts

`--profile` with `--dmcp res/combo/DMCP5_flash_3.57.bin`, `--ms-per-tick 1` and a fixed clock, on R47 images built on 2026-09-18, before and after the
glyph row change: master 62f16af1d, and d3d61cb8a, which master took in as 3e0e0028d. Each figure is every instruction between two marks: the key handling, the
arithmetic, the drawing and DMCP's own code.

| Step | master 62f16af1d | d3d61cb8a |
|---|---|---|
| boot and ENTER | 19.9 M | 8.7 M |
| typing 1 2 3 ENTER 4 5 6 | 2.51 M | 1.16 M |
| MUL | 1.24 M | 0.47 M |
| opening the I/O menu | 3.20 M | 0.82 M |
| EXIT, then XEQ | 3.19 M | 0.89 M |

The same typing step on d3d61cb8a without `--dmcp` counted 3.72 M, of which 3.25 M in the `lcd_fill_rect` stub against 0.71 M in DMCP's own, which is why
counts are taken with `--dmcp`.

The counts above have malloc and its kin answered on the host, at no instruction. Since 2026-09-27 the platform's allocator answers them wherever the platform image is
mapped, and its instructions count: `450 x!` run from a program on an R47 build of master e2bfd63dc with one test item added counted 20.215 M with `--host-heap`
and 20.226 M without, which is 11 495 more, 0.06 %, for 20 allocations and their frees.

The wall time on the calculator falls by less than the instructions do, because the LCD transfer is left out of every count: an R47 on battery sends all 240 lines
about twenty times a second, about 0.2 ms a line, and opening the I/O menu sends 220 of them. On the R47 the menu came up about twice as fast.

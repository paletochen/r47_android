# Program label matrix on 67e0011

Every command that names a program, against every kind of label, by the program route. Measured on the
derivative branch commit `67e0011c0`, built from `git archive` under `_Project_Improvements/_scratch/bug5c`,
base and fix. Generator, listing and runner in `_scratch/bug5e`.

The listing is `MATRIX.txt`, encoded with the rejig of 2026-08-09 17:05. Every one of the twelve commands is
in the rejig op table, so nothing needed a hand built container or a patched opcode byte.

## The two axes

Twelve commands take a program label. Six carry it themselves and run the program; six hand it to an
executor that runs later.

| item | command | operand kind | executor that consumes it |
|---|---|---|---|
| 1547 | PGMSLV | label | SOLVE 1608 |
| 1546 | PGMINT | label | ∫f d 1700 and ∫ˣ_y 1690 |
| 2732 | PGMPLT | label | PLT f 2734 |
| 2882 | PGMDRV | label | f' 2883 and f" 2884 |
| 1672 | Σₙ | label | runs the program itself |
| 1671 | Πₙ | label | runs the program itself |
| 1755 | iΣₙ | label | runs the program itself, limits must be long integers |
| 1754 | iΠₙ | label | runs the program itself, limits must be long integers |
| 2755 | ∞Σₙ | label | runs the program itself |
| 1630 | VARMENU | label | opens the MVAR menu |

Six kinds of label:

| kind | written | keystroke code |
|---|---|---|
| lettered local | `H` | 107 |
| numeric local | `45` | 45 |
| named local | `:HS:` | 249 length name |
| global | `'GS'` | 253 length name |
| indirect to the numeric | `→01` with R01 = 45 | 254 register |
| indirect to the global | `→01` with R01 = 'GS' | 254 register |

Each command family has its own four targets carrying different constants, so the reading names the target
that ran rather than merely showing that something ran.

## Result, 66 cases

| command | lettered | numeric | named | global | ind numeric | ind global |
|---|---|---|---|---|---|---|
| PGMSLV then SOLVE | fail to pass | fail to pass | pass | pass | fail to pass | pass |
| PGMINT then ∫f d | fail to pass | fail to pass | pass | pass | fail to pass | pass |
| PGMINT then ∫ˣ_y | fail to pass | fail to pass | pass | pass | fail to pass | pass |
| PGMPLT then PLT f | fail to pass | fail to pass | pass | pass | fail to pass | pass |
| PGMDRV then f' | fail to pass | fail to pass | pass | pass | fail to pass | pass |
| PGMDRV then f" | fail to pass | fail to pass | pass | pass | fail to pass | pass |
| Σₙ | fail to pass | fail to pass | pass | pass | fail to pass | pass |
| Πₙ | fail to pass | fail to pass | pass | pass | fail to pass | pass |
| iΣₙ | fail to pass | fail to pass | pass | pass | fail to pass | pass |
| iΠₙ | fail to pass | fail to pass | pass | pass | fail to pass | pass |
| ∞Σₙ | fail to pass | fail to pass | pass | pass | fail to pass | pass |
| VARMENU | CRASH to pass | CRASH to pass | pass | pass | CRASH to pass | pass |

"fail" is the base build, "pass" is the same case with the patch. 33 of the 66 cases were broken, and all 66
are correct with the patch. The three VARMENU entries marked CRASH exited 139, a segmentation fault, and
printed nothing at all.

Readings that name the target, base then patched:

| case | command | kind | base | patched | expected |
|---|---|---|---|---|---|
| C1H | PGMSLV then SOLVE | lettered | 0, unexpected parameter 107 | 2.000000000000000000000000000000000 | 2 |
| C145 | PGMSLV then SOLVE | numeric | 0, unexpected parameter 45 | 3.000000000000000000000000000000000 | 3 |
| C1IN | PGMSLV then SOLVE | ind numeric | 0, unexpected parameter 45 | 3.000000000000000000000000000000000 | 3 |
| C2H | PGMINT then ∫f d | lettered | 0 | -3.66666622 | -3.666 |
| C3H | PGMINT then ∫ˣ_y | lettered | 0 | -3.66666622 | -3.666 |
| C4H | PGMPLT then PLT f | lettered | marker -1 | marker 9 | 9 |
| C5H | PGMDRV then f' | lettered | 0 | 6.0 | 6 |
| C6H | PGMDRV then f" | lettered | 0 | 2 | 2 |
| C7H | Σₙ | lettered | 0 | 10 | 10 |
| C8H | Πₙ | lettered | 0 | 24 | 24 |
| C9H | iΣₙ | lettered | 0 | 10 | 10 |
| CAH | iΠₙ | lettered | 0 | 24 | 24 |
| CBH | VARMENU | lettered | exit 139 | marker 9 | 9 |

The named, global and indirect to global columns read identically on both builds, which is what makes the
other columns a measurement rather than a coincidence.

∞Σₙ 2755 runs as its own six cases with from 1, to 1000000, step 1, the early stop deciding when to leave.
Base against patched: lettered 0 to 1.644933066848726436305748499979392, numeric 0 to
0.8224665334243632181528742499896959, indirect to the numeric the same, and the named local, the global and
the indirect to the global unchanged at 0.5483110222829088121019161666597972 and
0.4112332667121816090764371249948480.

## The trap that cost two runs on the sums

The sum and product machinery puts the counter in register X and fills the stack, so the term program reads
the STACK. It does not store the counter into an MVAR. A term program written as `MVAR 'x'` then `RCL 'x'`
reads an unset variable, so the term is one over zero and the answer comes back as Infinity, on every build.
The finite Σₙ over the same body is the control that shows it: 0.3559027777777777777777777777777778 with a
stack-reading body, Infinity with an MVAR-reading one. This is a property of the sums, not of the patch.

## Open items

- **The keyboard route is not done.** Everything above is the program route. The press route needs a key
  sequence per command and per kind and has not been built.
- **iΣₙ and iΠₙ need long integer limits**, X, Y and Z all of them, checked by `_checkRegisters` in
  isumprod.c. The matrix pushes `1 4 1`, which rejig encodes as long integers, so they run.

## Files

    _scratch/bug5e/gen_matrix.py      the generator
    _scratch/bug5e/MATRIX.txt         the listing it writes
    _scratch/bug5e/MATRIX.p47         the container
    _scratch/bug5e/run_matrix.sh      the runner, one t47 invocation per case
    _scratch/bug5e/MATRIX_CASES.tsv   the case table
    _scratch/bug5e/matrix_dbase.txt   readings on 67e0011 as it stands
    _scratch/bug5e/matrix_dfix.txt    readings with the patch
    _scratch/bug5e/INF.txt            the infinite sum attempt, unresolved

Two shell traps cost a run each and are worth recording: a description containing an apostrophe or a double
quote breaks the single quoted `--exec` string, and a case label that collides with a target label silently
runs the wrong program.

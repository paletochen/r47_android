# Building our rejig

rejig is the upstream RPN program encoder, kept in a Fossil repository at https://tangentsoft.com/rpn/. Our
build is upstream plus one changed file, `r47/rejig/program/ops.go`. That file adds the C47/R47 ops upstream
does not have: the FIN-12C package, FINISH (alias DIE), ATEXT, GRFNT# and GRFNT, x→POLY, V→Σ=0, ⎙XFN and
TICKS#. The STRUCT ops (IF, ELSE, ENDIF, WHILE, DO, FOR, NEXT, FORᵀᴼᴾ) are now in upstream itself and are not
ours to add.

Our delta against upstream base e8018447 is the patch beside this file,
`2026-10-03_c47-all-ops_rejig-trunk_e801844761.patch`. This note records how to rebuild from nothing, and how
to add the ops a new merge request introduces.

## What you need

- Go, the version named in `r47/rejig/go.mod`.
- `task` (go-task), the runner the Taskfile uses.
- `fossil`, to fetch the source and to stamp the build with its branch.

## Step 1 — collect the items from every open merge request

rejig mirrors the firmware item table in `src/c47/items.c`. Every programmable item needs an op in rejig. An
item marked `PTP_DISABLED`, a menu opener or a setting, does not.

- For a built firmware the item list is `./t47 --dslcommands`, which writes `t47-op-commands.txt`: item
  number, DSL name, catalogue name.
- For an item not yet merged, read that merge request's `src/c47/items.c` additions. Reach the file through
  the GitLab API with the token in `_Project_Improvements/gitlab-token.txt`, or open the merge request in Fork.
- For each new item record four things: the item number (the `/* NNNN */` comment), the catalogue name (the
  second name string, the spelling XPORTP prints), the parameter kind, and whether it is `PTP_DISABLED`. A
  `PTP_DISABLED` item is left out.

The parameter kind maps to a rejig argument type:

| items.c parameter | rejig argument |
|---|---|
| NOPARAM | ARG_NONE |
| a program label | ARG_LABEL |
| a register | ARG_REGISTER |
| an 8-bit value | ARG_VALUE_8 |

## Step 2 — add each item to ops.go

One file, `r47/rejig/program/ops.go`, in two places per item.

1. A constant in the opcode enum, at the slot whose value is the item number. A dense run is written
   `OP_X OpCode = <base> + iota` with `_` fillers for the gaps. A sparse block gives each constant its own
   number, `OP_X OpCode = 3248`, which is how the FIN-12C block is written. Put the constant at its item number.
2. A row in the op table: `{OP_X, "<catalogue name>", ARG_<kind>, nil},`. Place it among its neighbours; the
   table is grouped by name. An alternate accepted spelling goes in an `AliasMap`, as FINISH takes DIE:

```go
{OP_FINISH, "FINISH", ARG_LABEL, AliasMap{AltSpelling1: `DIE`}},
```

Then raise `MaxValidOpCode` to the highest item number you added.

Copy the glyphs of the catalogue name exactly. An ASCII substitute does not encode.

## Step 3 — fetch, patch, build, install

Fetch the source the first time:

```bash
fossil clone https://tangentsoft.com/rpn/ rpn.fossil
mkdir rpn
cd rpn
fossil open ../rpn.fossil
```

An existing checkout is brought up to date with `fossil update trunk` instead.

Apply our delta from the checkout root:

```bash
patch -p0 < <path-to>/tools/rejig/2026-10-03_c47-all-ops_rejig-trunk_e801844761.patch
```

Editing `ops.go` by hand as in step 2 reaches the same file. The patch is the recorded result of those edits
against upstream e8018447.

Build:

```bash
cd r47/rejig
make build
```

`make build` runs `task build`, which regenerates the parser and runs `go build ./cmd/rejig`. The binary is
`r47/rejig/rejig`.

Install over both live copies in the firmware tree:

```bash
cp r47/rejig/rejig ~/c43/rejig
cp r47/rejig/rejig ~/c43/tools/rejig_patched/rejig
```

Confirm the new ops are present:

```bash
~/c43/rejig --ops | grep <name>
```

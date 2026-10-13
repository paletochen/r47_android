// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file structured.h
 ***********************************************/

/*
 STRUCT memory. Byte counts are for the DM42, 32-bit pointers.

══════════════════════════════════════════════════════════════════════════════════
 1.  PROGRAM MEMORY          persistent, travels with the program
══════════════════════════════════════════════════════════════════════════════════

  IF ₀₁  ELSE ₀₁  ENDIF ₀₁        ┌──────┬──────┬──────┐
  DO ₀₂  WHILE ₀₂  ENDDO ₀₂       │ 139  │ 104  │  01  │   3 bytes each
  REPEAT ₀₃  UNTIL ₀₃             └──────┴──────┴──────┘
                                    op code       partner
                                    (2 bytes)     number  ← the number lives HERE,
                                                            the third byte, nowhere else

  FOR / NEXT, four operand forms  ┌──────┬──────┬──────┐
      FOR 01      numbered        │ 139  │ 111  │  01  │   3 bytes
      FOR I       lettered        └──────┴──────┴──────┘   3 bytes, every letter counts
      FOR →01     indirect        ┌──────┬──────┬──────┬──────┐
                                  │ 139  │ 111  │ 254  │  01  │   4 bytes
                                  └──────┴──────┴──────┴──────┘
      FOR 'CTR'   named           ┌──────┬──────┬──────┬──────┬───────────┐
                                  │ 139  │ 111  │ 253  │  3   │ C   T   R │   4 + name
                                  └──────┴──────┴──────┴──────┴───────────┘
                                                  tag    len
                                  no partner number byte on FOR or NEXT

  FORₓ 139 113   NEXTₓ 139 114    the not checked forms, same length and same operand,
                                  second op code byte only

  FORʸˣ 139 117  FORʸˣₓ 139 118   the FOR that works out its own step, and its not checked
                                  form, the same four operand forms and the same NEXT

  REPEAT 139 119  UNTIL 139 120    the loop that tests at the bottom, three bytes each with a partner
                                  number, the same as IF and DO

  FORᵀᴼᴾ 139 122  FORᵀᴼᴾₓ 139 123  the FOR that compares before the first pass, and its not
                                  checked form, the same operand forms and the same NEXT

  VALID 139 115   AUTOVALID 139 116  never enter a program: items.c sets doNotAddStep on
  CLSTRUC 139 121                 all three, so they cost no program memory

══════════════════════════════════════════════════════════════════════════════════
 2.  LABEL LIST               RAM, rebuilt by scanLabelsAndPrograms on every edit
══════════════════════════════════════════════════════════════════════════════════

  One labelList_t per jump target. Appended after the real labels, so label indices
  never move.

     entry ┌─────────┬─────────┬───────────────┬────────────────────┐
           │ program │  step   │ labelPointer  │ instructionPointer │
           │  int16  │  int32  │    pointer    │      pointer       │
           └─────────┴─────────┴───────────────┴────────────────────┘
              DM42 (32-bit) 16 bytes
              sim  (64-bit) 24 bytes, measured 20 entries = 480 bytes

     gets an entry:  ELSE  ENDIF  DO  ENDDO  REPEAT  UNTIL  FOR  NEXT  FORₓ  NEXTₓ  FORʸˣ  FORʸˣₓ
                     FORᵀᴼᴾ  FORᵀᴼᴾₓ
     gets nothing :  IF  WHILE  VALID  AVALID  CLSTRUC
     option off: no structure entries at all

══════════════════════════════════════════════════════════════════════════════════
 3.  VALID WALK ARRAYS        RAM, static, resident whenever the option is on
══════════════════════════════════════════════════════════════════════════════════

     STRUCT_MAX_NESTING = 63 open structures on the R47, the DM42n and the simulator, 10 on the DM42

                                       63      10
     structOpenedBy      × uint16     126      20
     structOpenStepNumber× uint16     126      20
     structOpenNumber    × uint16     126      20
     structOpenSawWhile  × bool        63      10
     structOpenCrossed   × bool        63      10
     structOpenForStep   × pointer    252      40
                                     ─────   ─────
                                       756     120 bytes, fixed, never grows

══════════════════════════════════════════════════════════════════════════════════
 4.  RUNNING FOR LOOP TABLE   RAM, static, one row per FOR running anywhere
══════════════════════════════════════════════════════════════════════════════════

     FOR_MAX_LOOPS = 18 loops running at one time, over every subroutine level, 4 on the DM42

     VALID counts the FOR structures written one inside another and refuses more than this.
     FORs reached through a series of XEQ commands can only be counted at run time, so the
     table can still run out at a FOR that VALID passed.

     forLoopTable[18] × forLoop_t     180   saved in backup.cfg, 40 on the DM42
        counterRegister   uint16   the register counted in
        localStepNumber   uint16   where the FOR stands in its program, 0 marks the row free
        localRegisterBase uint16   first of its two local registers, which hold the end value and the increment
        programNumber     uint16   the program the FOR is in
        stepDescends      uint8    two bits: NEXT subtracts the increment instead of adding it, and the FOR compared before the first pass
        subroutineLevel   uint8    the subroutine level it opened in

     sizeof(forLoop_t) is 10, measured: four uint16 and two uint8, which is why stepDescends cost nothing

     forLoopStep[18] × pointer         72   not saved, rebuilt as a loop opens, 16 on the DM42
                                     ────
                                      252 bytes, fixed, never grows, 56 on the DM42

══════════════════════════════════════════════════════════════════════════════════
 5.  A RUNNING FOR            RAM, local registers, allocated by FOR, freed by NEXT
══════════════════════════════════════════════════════════════════════════════════

     FOR takes  Z=start  Y=end  X=step, and keeps two of them. FORᵀᴼᴾ takes the same
     three and differs only in comparing before the first pass. FORʸˣ takes
     Y=start  X=end and makes the step itself, after which it is the same loop:

     local level ┌─────────────────┬─────────────────┐
        top ──►  │  base+0  END    │  base+1  STEP   │   the counter itself is an
                 └─────────────────┴─────────────────┘   ordinary register, no extra
                   4-byte header     4-byte header       cost

     value size follows the type handed to FOR, measured:
        real34 end and step        40 bytes
        small long integer         24 bytes

     freed by NEXT when past the end, and by RTN with the level
     ceiling 99 local registers per subroutine level → 49 FOR nested in one level,
     but the 18-row table binds first, so 18 is the depth a program reaches

     A running IF or WHILE costs nothing: no stack, no table, no allocation. They
     jump through the label list that already exists.

══════════════════════════════════════════════════════════════════════════════════
 6.  QSPI                     flash, not RAM
══════════════════════════════════════════════════════════════════════════════════

     structTest[62] × int16 = 124 bytes, the items VALID accepts as a condition

*/




#if !defined(STRUCTURED_H)
  #define STRUCTURED_H

#undef    OPTION_STRUCT_INDENT               // PEM and XPORTP indent a structure's body; off gives the columns of the build before STRUCT
#if defined(OPTION_STRUCTURED_PGM)
#if defined(OLD_HW)                          // the DM42 set: 176 bytes of static RAM, which is what its RAM has room for
  #define STRUCT_MAX_NUMBER               10 // the highest partner number a program may hold
  #define STRUCT_MAX_NESTING              10 // the deepest nesting VALID can hold open at once
  #define FOR_MAX_LOOPS                    4 // FOR structures that may run at one time, counted over every subroutine level together; 56 bytes
#else // OLD_HW                              // the R47, the DM42n and the simulator: 1008 bytes of static RAM
  #define STRUCT_MAX_NUMBER              255 // the highest partner number, which is what one operand byte holds
  #define STRUCT_MAX_NESTING              63 // the deepest nesting VALID can hold open at once
  #define FOR_MAX_LOOPS                   18 // FOR structures that may run at one time, counted over every subroutine level together; 252 bytes
#endif // OLD_HW
  #define FOR_LOCALS                       2 // the end value and the increment, the two values a running FOR holds for itself
  // The two bits of forLoop_t.stepDescends. They share the byte the descending flag already had, so a row is still 10 bytes and backup.cfg keeps its layout: a file
  // written before FORTOP holds 0 or 1 there and reads back as a bottom tested loop, which is what it was.
  #define FOR_STEP_DESCENDS             0x01 // NEXT subtracts the increment instead of adding it
  #define FOR_TOP_TESTED                0x02 // the FOR compared before the first pass, so a start past the end runs no pass and its NEXT leaves the counter past the end
  #define FOR_MAX_LOCALS                  99 // what allocateLocalRegisters() permits a subroutine level, so 49 FOR structures fit one level, above the 18 the table holds
  // Display
  #define OPTION_STRUCT_INDENT               // PEM and XPORTP indent a structure's body; off gives the columns of the build before STRUCT
  #define PEM_STRUCT_INDENT                2 // spaces a structure indents its body by in the editor
  #define PEM_MAX_STRUCT_LEVELS            4 // levels the editor indents; deeper structures all share that column, the 400 pixel screen being the limit
  #define XPORTP_STRUCT_INDENT             2 // spaces a structure indents its body by in an exported listing
  #define XPORTP_MAX_STRUCT_LEVELS        99 // levels an exported listing indents; a file has no width limit, so this is effectively no limit

  extern forLoop_t forLoopTable[FOR_MAX_LOOPS];
#endif // OPTION_STRUCTURED_PGM

  void     fnIf                      (uint16_t structureNumber);
  void     fnElse                    (uint16_t structureNumber);
  void     fnEndif                   (uint16_t structureNumber);
  void     fnDo                      (uint16_t structureNumber);
  void     fnWhile                   (uint16_t structureNumber);
  void     fnEnddo                   (uint16_t structureNumber);
  void     fnRepeat                  (uint16_t structureNumber);
  void     fnUntil                   (uint16_t structureNumber);
  void     fnFor                     (uint16_t regist);
  void     fnForYx                   (uint16_t regist);
  void     fnForTop                  (uint16_t regist);
  void     fnNext                    (uint16_t regist);
  void     fnValid                   (uint16_t unusedButMandatoryParameter);
  void     fnClearStructures         (uint16_t unusedButMandatoryParameter);
  uint16_t structFusedOp             (uint8_t *step);
  bool_t   structNoLegacySkip        (uint8_t *step);
  bool_t   structOpHasNumber         (uint16_t op);
  bool_t   structStepIsIfOrWhile     (uint8_t *step);
  bool_t   structStepIsJumpTarget    (uint8_t *step);
  bool_t   structDisplayOutdent      (uint8_t *step);
  bool_t   structStepOpensIndent     (uint8_t *step);
  bool_t   structStepClosesIndent    (uint8_t *step);
  bool_t   structStepOnOpenerColumn  (uint8_t *step);
  bool_t   structEndSplitsWell       (void);
  bool_t   structProgramHasUnnumbered(void);
  bool_t   structRangeHasNumbered    (uint8_t *from, uint8_t *to);
  void     structClearProgramNumbers (void);
  void     forClearChecked           (void);
  void     forClearLoops             (void);
  void     forAdjustCountersAfterVariableDelete(uint16_t deletedVariable);
  void     fnForNotChecked           (uint16_t regist);
  uint16_t structPlainOp             (uint16_t op);
  uint16_t structNotCheckedOp        (uint16_t op);
#endif // !STRUCTURED_H

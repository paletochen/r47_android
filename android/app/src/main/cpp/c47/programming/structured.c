// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file structured.c IF, ELSE, ENDIF, DO, WHILE, ENDDO, FOR, NEXT and VALID of the STRUCT programming set
 ***********************************************/

#include "c47.h"

// The op code of a step, 0 when the step cannot be read. Bounds checked: the step after a test is not reached yet, so the second byte of a two-byte op code can lie past
// the end of program memory.
static uint16_t structOpOfStep(uint8_t *step) {
  uint16_t stepOp;

  if(step == NULL || !programBytesAvailable(step, 1)) {
    return 0;
  }
  stepOp = *step;
  if(stepOp & 0x80) {
    if(!programBytesAvailable(step, 2)) {
      return 0;
    }
    stepOp = ((stepOp & 0x7f) << 8) | *(step + 1);
  }
  return stepOp;
}

// True for the STRUCT commands that carry a partner number. Their number reads as subscript digits joined to the name.
bool_t structOpHasNumber(uint16_t op) {
  return op == ITM_IF || op == ITM_ELSE || op == ITM_ENDIF || op == ITM_DO || op == ITM_WHILE || op == ITM_ENDDO
      || op == ITM_REPEAT || op == ITM_UNTIL;
}

// True when the step reads a test answer, that is IF, WHILE or UNTIL. SST on the test runs both the test and IF..., so BST steps back two steps
bool_t structStepIsIfOrWhile(uint8_t *step) {
  uint16_t op = structOpOfStep(step);

  return op == ITM_IF || op == ITM_WHILE || op == ITM_UNTIL;
}

// True for the STRUCT structure tokens. A test indents the step after it, and these take their own column instead.
bool_t structDisplayOutdent(uint8_t *step) {
  uint16_t op = structPlainOp(structOpOfStep(step));

  return structOpHasNumber(op) || op == ITM_FOR || op == ITM_NEXT;
}

// True for the step that opens a structure. A listing stands its body two columns in from here.
bool_t structStepOpensIndent(uint8_t *step) {
  uint16_t op = structPlainOp(structOpOfStep(step));

  return op == ITM_IF || op == ITM_DO || op == ITM_FOR || op == ITM_REPEAT;
}

// True for the step that closes a structure. It stands on the column its opener stands on, so the count falls before it is written.
bool_t structStepClosesIndent(uint8_t *step) {
  uint16_t op = structPlainOp(structOpOfStep(step));

  return op == ITM_ENDIF || op == ITM_ENDDO || op == ITM_NEXT || op == ITM_UNTIL;
}

// True for the step that stands on its opener's column without closing it, so the body below it stays indented. The exported listing takes this from the name table.
bool_t structStepOnOpenerColumn(uint8_t *step) {
  uint16_t op = structPlainOp(structOpOfStep(step));

  return op == ITM_ELSE || op == ITM_WHILE;
}

// True for the STRUCT steps a jump lands on: both branch ends of IF, both ends of the loop since ENDDO goes back to its DO, and FOR and NEXT in every form. The label
// scan records each one, so a STRUCT jump is the lookup a GTO to a local label makes.
bool_t structStepIsJumpTarget(uint8_t *step) {
  uint16_t op = structPlainOp(structOpOfStep(step));

  return op == ITM_ELSE || op == ITM_ENDIF || op == ITM_DO || op == ITM_ENDDO || op == ITM_FOR || op == ITM_NEXT
      || op == ITM_REPEAT || op == ITM_UNTIL;
}

// The plain form of a step VALID has marked as not checked, and the plain FOR of a FORyx. The forms differ only in the op code, so everything that reads a program asks
// this and sees the structure, while the run and the file writer test for the marked form and refuse it. FORyx differs from FOR only in where its step comes from, and
// once it is running it is a FOR, so every walk of a program treats the two alike and only the item table tells them apart.
uint16_t structPlainOp(uint16_t op) {
  if(op == ITM_FORx || op == ITM_FORYX || op == ITM_FORYXx || op == ITM_FORTOP || op == ITM_FORTOPx) {
    return ITM_FOR;
  }
  if(op == ITM_NEXTx) {
    return ITM_NEXT;
  }
  return op;
}

// The op code of the step after 'step' when that step is a complete IF or WHILE, 0 otherwise. The run loop then runs the test and that step as one action instead of taking
// the legacy skip. Compiled in every build: without the option a false test would skip the IF and run the branch it did not select. All three bytes must be present,
// since the caller reads the number as the third one and a program damaged there would read past the end.
uint16_t structFusedOp(uint8_t *step) {
  uint8_t *next = findNextStep(step);
  uint16_t op = structOpOfStep(next);

  if((op == ITM_IF || op == ITM_WHILE || op == ITM_UNTIL) && programBytesAvailable(next, 3)) {
    return op;
  }
  return 0;
}

// True when the step after 'step' must run whatever the test answered. The legacy rule skips the step after a false test, and skipping a structure token breaks the
// structure silently: a skipped ENDDO ends the loop after one pass, a skipped ELSE runs the branch it should have skipped.
bool_t structNoLegacySkip(uint8_t *step) {
  return structFusedOp(step) != 0 || structStepIsJumpTarget(findNextStep(step));
}

#if !defined(OPTION_STRUCTURED_PGM)

// The items exist in every build so programs stay portable. Without the option the commands only report that they cannot run on this hardware. IF and WHILE arrive with the
// test answer still pending, and that display would cover the message, so it is cleared first.
static void structNotHere(void) {
  temporaryInformation = TI_NO_INFO;
  displayCalcErrorMessage(ERROR_NOT_AVAILABLE_HERE, ERR_REGISTER_LINE);
}

void fnIf(uint16_t unusedButMandatoryParameter) {
  structNotHere();
}

void fnElse(uint16_t unusedButMandatoryParameter) {
  structNotHere();
}

void fnEndif(uint16_t unusedButMandatoryParameter) {
  structNotHere();
}

void fnDo(uint16_t unusedButMandatoryParameter) {
  structNotHere();
}

void fnWhile(uint16_t unusedButMandatoryParameter) {
  structNotHere();
}

void fnEnddo(uint16_t unusedButMandatoryParameter) {
  structNotHere();
}

void fnRepeat(uint16_t unusedButMandatoryParameter) {
  structNotHere();
}

void fnUntil(uint16_t unusedButMandatoryParameter) {
  structNotHere();
}

void fnFor(uint16_t regist) {
  structNotHere();
}

void fnForYx(uint16_t regist) {
  structNotHere();
}

void fnNext(uint16_t regist) {
  structNotHere();
}

void fnValid(uint16_t unusedButMandatoryParameter) {
  structNotHere();
}

void fnClearStructures(uint16_t unusedButMandatoryParameter) {
  structNotHere();
}

void fnForTop(uint16_t regist) {
  structNotHere();
}

// Without the option there is no VALID to write the numbers, so a program is never refused for carrying 0 and an edit has nothing to clear. The program editor calls all
// of these whatever the build.
bool_t structProgramHasUnnumbered(void) {
  return false;
}

bool_t structRangeHasNumbered(uint8_t *from, uint8_t *to) {
  return false;
}

void structClearProgramNumbers(void) {
}

void forClearChecked(void) {
}

void forAdjustCountersAfterVariableDelete(uint16_t deletedVariable) {
}

// No FOR runs on this hardware.
void forClearLoops(void) {
}

void fnForNotChecked(uint16_t regist) {
  structNotHere();
}

// Without the option there is no VALID, so a step keyed in enters as itself and a DM42 stores a program as it was loaded.
uint16_t structNotCheckedOp(uint16_t op) {
  return op;
}

#else // OPTION_STRUCTURED_PGM

// True when the step holds exactly this operation.
static bool_t structStepIs(uint8_t *step, uint16_t op) {
  return structOpOfStep(step) == op;
}

// The structures VALID has open at the step it is reading: what opened each one, whether its WHILE has been seen, the step number it opened at, the partner number it was
// given, whether a routine boundary crossed it, and, for a FOR, the step itself. An unclosed structure reports the step number.
static uint16_t structOpenedBy[STRUCT_MAX_NESTING];
static bool_t   structOpenSawWhile[STRUCT_MAX_NESTING];
static uint16_t structOpenStepNumber[STRUCT_MAX_NESTING];
static uint16_t structOpenNumber[STRUCT_MAX_NESTING];
static bool_t   structOpenCrossed[STRUCT_MAX_NESTING];
static uint8_t *structOpenForStep[STRUCT_MAX_NESTING];

// True when the two steps carry the same counter. Both are a FOR or a NEXT, so the operand runs from the third byte to the end of the step. The bytes are compared as
// they stand: the written form is what pairs them, and an indirect operand is not resolved.
static bool_t forStepsSameCounter(uint8_t *forStep, uint8_t *nextStep) {
  uint8_t *forEnd = findNextStep(forStep);
  uint8_t *nextEnd = findNextStep(nextStep);
  uint16_t byte;

  if(forEnd == NULL || nextEnd == NULL || (forEnd - forStep) != (nextEnd - nextStep)) {
    return false;
  }
  for(byte = 2; byte < (uint16_t)(forEnd - forStep); byte++) {
    if(*(forStep + byte) != *(nextStep + byte)) {
      return false;
    }
  }
  return true;
}

// A partner number runs from 1 to STRUCT_MAX_NUMBER, which is the whole range one operand byte holds. 0 is what an unnumbered step carries.
static bool_t structNumberIsValid(uint16_t structureNumber) {
  return structureNumber != 0 && structureNumber <= STRUCT_MAX_NUMBER;
}

// Running an unnumbered step stops the program.
static bool_t structNumberMissing(uint16_t structureNumber) {
  if(structNumberIsValid(structureNumber)) {
    return false;
  }
  temporaryInformation = TI_NO_INFO;
  displayCalcErrorMessage(ERROR_STRUCTURE_NOT_NUMBERED, ERR_REGISTER_LINE);
  return true;
}

// The nearest recorded structure step of this number, below the step in hand when 'below' is set and above it otherwise. Only the op codes named count, and 'secondOp' is
// 0 when one op ends the search. scanLabelsAndPrograms() holds them all in the label list, after the real labels, in program and step order, so the walk stops once it is
// past its target and program memory is never scanned. Nearest, and never wrapping, lets one number serve structures that follow one another rather than nest.
static uint16_t structFindPartner(uint16_t structureNumber, uint16_t firstOp, uint16_t secondOp, bool_t below) {
  const uint16_t lastLabel = numberOfLabels + numberOfStructureLabels;
  const uint16_t programStep = programList[currentProgramNumber - 1].step;
  uint16_t label, bestLabel = lastLabel;

  for(label = numberOfLabels; label < lastLabel; label++) {
    if(labelList[label].program > currentProgramNumber) {
      break;
    }
    if(labelList[label].program != currentProgramNumber) {
      continue;
    }
    uint16_t localStep = (-labelList[label].step) - programStep + 1;
    if(below ? (localStep <= currentLocalStepNumber) : (localStep >= currentLocalStepNumber)) {
      if(!below) {
        break; // the entries ascend, so everything from here on is below the step in hand
      }
      continue;
    }
    if(*(labelList[label].labelPointer) != structureNumber) {
      continue;
    }
    if(!structStepIs(labelList[label].labelPointer - 2, firstOp)
    && !(secondOp != 0 && structStepIs(labelList[label].labelPointer - 2, secondOp))) {
      continue;
    }
    if(below) {
      return label; // the first one below is the nearest
    }
    bestLabel = label; // above, the last one seen is the nearest
  }
  return bestLabel;
}

// Go to the step after the partner found. The label list holds that step, so the jump is the lookup a GTO to a local label makes.
static bool_t structJumpToLabel(uint16_t label) {
  if(label >= numberOfLabels + numberOfStructureLabels) {
    return false;
  }
  currentLocalStepNumber = (-labelList[label].step) - programList[currentProgramNumber - 1].step + 2; // the step after the one found
  currentStep = labelList[label].instructionPointer;
  return true;
}

static bool_t structJumpToPartner(uint16_t structureNumber, uint16_t firstOp, uint16_t secondOp, bool_t below) {
  return structJumpToLabel(structFindPartner(structureNumber, firstOp, secondOp, below));
}

void fnIf(uint16_t structureNumber) {
  if(programRunStop != PGM_RUNNING) {
    return;
  }
  if(structNumberMissing(structureNumber)) {
    return;
  }
  if(temporaryInformation != TI_TRUE && temporaryInformation != TI_FALSE) {
    displayCalcErrorMessage(ERROR_IF_WHILE_CONDITION_MISSING, ERR_REGISTER_LINE);
    return;
  }
  if(temporaryInformation == TI_TRUE) {
    temporaryInformation = TI_NO_INFO;
    fnSkip(0);
    return;
  }
  temporaryInformation = TI_NO_INFO;
  // A false IF ends its branch at either token, whichever comes first.
  if(!structJumpToPartner(structureNumber, ITM_ENDIF, ITM_ELSE, true)) {
    displayCalcErrorMessage(ERROR_STRUCTURE_INVALID, ERR_REGISTER_LINE); // Redundant message, cannot happen with validated program
  }
}

// Closes the true branch by jumping past its own ENDIF. An ELSE no IF opened is reached and jumps the same way. Only ENDIF ends the search: a second ELSE of the same
// structure belongs to the branch being skipped.
void fnElse(uint16_t structureNumber) {
  if(programRunStop != PGM_RUNNING) {
    return;
  }
  if(structNumberMissing(structureNumber)) {
    return;
  }
  if(!structJumpToPartner(structureNumber, ITM_ENDIF, 0, true)) {
    displayCalcErrorMessage(ERROR_STRUCTURE_INVALID, ERR_REGISTER_LINE); // Redundant message, cannot happen with validated program
  }
}

// Nothing to do. Both branches end here and the step after it is the one to run.
void fnEndif(uint16_t structureNumber) {
  if(programRunStop != PGM_RUNNING) {
    return;
  }
  if(structNumberMissing(structureNumber)) {
    return;
  }
  fnSkip(0);
}

// Nothing to do. The test follows it, and the ENDDO comes back to the step after this one.
void fnDo(uint16_t structureNumber) {
  if(programRunStop != PGM_RUNNING) {
    return;
  }
  if(structNumberMissing(structureNumber)) {
    return;
  }
  fnSkip(0);
}

// The test answer decides the loop: true runs the body, false leaves the structure by jumping past its own ENDDO.
void fnWhile(uint16_t structureNumber) {
  if(programRunStop != PGM_RUNNING) {
    return;
  }
  if(structNumberMissing(structureNumber)) {
    return;
  }
  if(temporaryInformation != TI_TRUE && temporaryInformation != TI_FALSE) {
    displayCalcErrorMessage(ERROR_IF_WHILE_CONDITION_MISSING, ERR_REGISTER_LINE);
    return;
  }
  if(temporaryInformation == TI_TRUE) {
    temporaryInformation = TI_NO_INFO;
    fnSkip(0);
    return;
  }
  temporaryInformation = TI_NO_INFO;
  if(!structJumpToPartner(structureNumber, ITM_ENDDO, 0, true)) {
    displayCalcErrorMessage(ERROR_STRUCTURE_INVALID, ERR_REGISTER_LINE); // Redundant message, cannot happen with validated program
  }
}

// Goes back to its own DO, landing on the step after it, which is the test. An ENDDO that no DO opened has nowhere to go and says so.
void fnEnddo(uint16_t structureNumber) {
  if(programRunStop != PGM_RUNNING) {
    return;
  }
  if(structNumberMissing(structureNumber)) {
    return;
  }
  // The nearest DO above is the loop this closes, unless an ENDDO of the same number lies between the two: that one closed it, so nothing is open here and the jump
  // would re-enter a finished loop.
  const uint16_t doLabel    = structFindPartner(structureNumber, ITM_DO, 0, false);
  const uint16_t enddoLabel = structFindPartner(structureNumber, ITM_ENDDO, 0, false);

  if(doLabel < numberOfLabels + numberOfStructureLabels
  && (enddoLabel >= numberOfLabels + numberOfStructureLabels || labelList[enddoLabel].step > labelList[doLabel].step)) {
    structJumpToLabel(doLabel);
    return;
  }
  displayCalcErrorMessage(ERROR_STRUCTURE_INVALID, ERR_REGISTER_LINE); // Redundant message, cannot happen with validated program
}

// Nothing to do. The body follows, and the UNTIL comes back to the step after this one.
void fnRepeat(uint16_t structureNumber) {
  if(programRunStop != PGM_RUNNING) {
    return;
  }
  if(structNumberMissing(structureNumber)) {
    return;
  }
  fnSkip(0);
}

// The test is at the end, not the top, so the body has already run: true ends the loop, false goes back to the step after its own REPEAT. A stray UNTIL says so.
void fnUntil(uint16_t structureNumber) {
  if(programRunStop != PGM_RUNNING) {
    return;
  }
  if(structNumberMissing(structureNumber)) {
    return;
  }
  if(temporaryInformation != TI_TRUE && temporaryInformation != TI_FALSE) {
    displayCalcErrorMessage(ERROR_IF_WHILE_CONDITION_MISSING, ERR_REGISTER_LINE);
    return;
  }
  if(temporaryInformation == TI_TRUE) {
    temporaryInformation = TI_NO_INFO;
    fnSkip(0);
    return;
  }
  temporaryInformation = TI_NO_INFO;
  // The nearest REPEAT above is the one this closes, unless an UNTIL of the same number lies between them: that one closed it and this jump would re-enter it.
  const uint16_t repeatLabel = structFindPartner(structureNumber, ITM_REPEAT, 0, false);
  const uint16_t untilLabel  = structFindPartner(structureNumber, ITM_UNTIL, 0, false);

  if(repeatLabel < numberOfLabels + numberOfStructureLabels
  && (untilLabel >= numberOfLabels + numberOfStructureLabels || labelList[untilLabel].step > labelList[repeatLabel].step)) {
    structJumpToLabel(repeatLabel);
    return;
  }
  displayCalcErrorMessage(ERROR_STRUCTURE_INVALID, ERR_REGISTER_LINE); // Redundant message, cannot happen with validated program
}

// The running FOR structures. A row is filled as a loop opens and its step set back to 0 as it closes. The row is what a NEXT trusts: it reads the two local registers only
// because a row says a loop of its counter runs at its level, and a program cannot write a row. The table is saved with the calculator, so a loop survives a power cycle.
forLoop_t forLoopTable[FOR_MAX_LOOPS];

// Where each running FOR stands in memory. Not part of the saved state: program memory moves, so this is set as a loop opens and tested before it is used.
static uint8_t *forLoopStep[FOR_MAX_LOOPS];

// True when the pointer the FOR left still reaches that FOR. Program memory moves when a step is inserted or deleted, so it is trusted only while it lies inside the open
// program and still reads as a FOR. A NEXT that cannot trust it counts steps from the top of the program instead, as every jump did before.
static bool_t forLoopStepPointerLive(uint16_t row) {
  uint8_t *step = forLoopStep[row];

  return step != NULL && step >= beginOfCurrentProgram && step < endOfCurrentProgram && structPlainOp(structOpOfStep(step)) == ITM_FOR;
}

// True when the two registers the row names are still there. LocR and PopLR set the count outright, so either can delete them under a running loop, and the row alone
// would then send the NEXT to whatever took their place.
static bool_t forLoopRegistersLive(uint16_t row) {
  return forLoopTable[row].localRegisterBase + FOR_LOCALS <= FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters;
}

// No FOR structure runs after a state file is loaded. The state file does not carry them.
void forClearLoops(void) {
  uint16_t row;

  for(row = 0; row < FOR_MAX_LOOPS; row++) {
    forLoopTable[row].localStepNumber = 0;
    forLoopStep[row] = NULL;
  }
}

// forLoopRegistersLive compares the row's first register number against the last local register the level now running owns. A row of this
// level failing that test has lost the two registers its NEXT reads, so its localStepNumber is set to 0 and another loop may take the row.
static void forDropDeadLoops(void) {
  uint16_t row;

  for(row = 0; row < FOR_MAX_LOOPS; row++) {
    if(forLoopTable[row].localStepNumber != 0 && (forLoopTable[row].subroutineLevel > currentSubroutineLevel
    || (forLoopTable[row].subroutineLevel == currentSubroutineLevel && (forLoopTable[row].programNumber != currentProgramNumber || !forLoopRegistersLive(row))))) {
      forLoopTable[row].localStepNumber = 0;
    }
  }
}

// Deleting a named variable compacts the table and moves every variable above it down one, so a running loop counting in one of those follows its counter. A loop
// counting in the variable deleted has lost what it counts in, and the number it holds now names whatever moves into that slot, so the loop ends here. The delete is
// the only place that is known without the name: a NEXT reading a register number alone cannot tell a variable from the one that replaced it.
void forAdjustCountersAfterVariableDelete(uint16_t deletedVariable) {
  uint16_t row;

  for(row = 0; row < FOR_MAX_LOOPS; row++) {
    if(forLoopTable[row].localStepNumber != 0 && forLoopTable[row].counterRegister == deletedVariable) {
      forLoopTable[row].localStepNumber = 0;
    }
    else if(forLoopTable[row].localStepNumber != 0 && forLoopTable[row].counterRegister > deletedVariable
    && forLoopTable[row].counterRegister <= LAST_NAMED_VARIABLE) {
      forLoopTable[row].counterRegister -= 1;
    }
  }
}

// The running loop counting in this register at this level, or FOR_MAX_LOOPS when there is none.
static uint16_t forLoopOfCounter(uint16_t counter) {
  uint16_t row;

  for(row = 0; row < FOR_MAX_LOOPS; row++) {
    if(forLoopTable[row].localStepNumber != 0 && forLoopTable[row].subroutineLevel == currentSubroutineLevel && forLoopTable[row].counterRegister == counter) {
      return row;
    }
  }
  return FOR_MAX_LOOPS;
}

// Prevent having the same variable in a second simultaneous FOR loop. A local register belongs to the subroutine level that declared it, so a loop of another level
// counting in one of the same number counts in a different register; every other register is one register whatever the level. A routine called from a body is a level
// down, which is why the level alone will not do.
static bool_t forCounterAlreadyCounted(uint16_t counter) {
  uint16_t row;

  for(row = 0; row < FOR_MAX_LOOPS; row++) {
    if(forLoopTable[row].localStepNumber != 0 && forLoopTable[row].counterRegister == counter
    && (forLoopTable[row].subroutineLevel == currentSubroutineLevel || counter < FIRST_LOCAL_REGISTER)) {
      return true;
    }
  }
  return false;
}

// The running loop that opened at this step and level: this FOR reached a second time with its loop still open. BST then SST does that, and so does a GTO onto it. The
// loop carries on rather than taking a second pair of registers.
static uint16_t forLoopOfStep(void) {
  uint16_t row;

  for(row = 0; row < FOR_MAX_LOOPS; row++) {
    if(forLoopTable[row].localStepNumber == currentLocalStepNumber && forLoopTable[row].subroutineLevel == currentSubroutineLevel) {
      return row;
    }
  }
  return FOR_MAX_LOOPS;
}

// A free row, or FOR_MAX_LOOPS when every row is running.
static uint16_t forFreeLoopRow(void) {
  uint16_t row;

  for(row = 0; row < FOR_MAX_LOOPS; row++) {
    if(forLoopTable[row].localStepNumber == 0) {
      return row;
    }
  }
  return FOR_MAX_LOOPS;
}

// True when a loop of this level opened at a later step than the row given, so it is inside that one and still running. Loops nest in the text, so the innermost is
// always the latest step and owns the pair at the top of the level. Closing an outer loop first is a crossed structure.
static bool_t forInnerLoopStillOpen(uint16_t row) {
  uint16_t other;

  for(other = 0; other < FOR_MAX_LOOPS; other++) {
    if(forLoopTable[other].localStepNumber != 0 && forLoopTable[other].subroutineLevel == currentSubroutineLevel
    && forLoopTable[other].localStepNumber > forLoopTable[row].localStepNumber) {
      return true;
    }
  }
  return false;
}

// True when a NEXT closes the FOR now running, somewhere below it in the same program. RTN does not end the search: an RTN inside the body is the early exit the
// specification allows, and stopping there would refuse a loop that is written correctly.
static uint8_t *forNextOfThisFor(uint16_t *stepsAhead) {
  uint8_t *step = findNextStep(currentStep);
  uint16_t depth = 0;
  uint16_t ahead = 1;

  while(step != NULL && !isAtEndOfProgram(step) && !isAtEndOfPrograms(step)) {
    if(structPlainOp(structOpOfStep(step)) == ITM_FOR) {
      depth++;
    }
    else if(structPlainOp(structOpOfStep(step)) == ITM_NEXT) {
      if(depth == 0) {
        if(stepsAhead != NULL) {
          *stepsAhead = ahead;
        }
        return step;
      }
      depth--;
    }
    step = findNextStep(step);
    ahead++;
  }
  return NULL;
}

static bool_t forHasNext(void) {
  return forNextOfThisFor(NULL) != NULL;
}

// True when the two registers hold the same value. Long and short integers compare exactly. A complex value is compared part by part, the ordinary compare having no
// meaning for it.
static bool_t forValuesEqual(uint16_t first, uint16_t second) {
  int8_t cmp;

  if(getRegisterDataType(first) == dtComplex34 || getRegisterDataType(second) == dtComplex34) {
    real_t firstRe, firstIm, secondRe, secondIm;
    bool_t cmplx;

    if(!getRegisterAsComplexOrAnyRealQuiet(first,  &firstRe,  &firstIm,  &cmplx)
    || !getRegisterAsComplexOrAnyRealQuiet(second, &secondRe, &secondIm, &cmplx)) {
      return false;
    }
    realSubtract(&firstRe, &secondRe, &firstRe, &ctxtReal39);
    realSubtract(&firstIm, &secondIm, &firstIm, &ctxtReal39);
    return realIsZero(&firstRe) && realIsZero(&firstIm);
  }
  return registerCmp(first, second, &cmp) && cmp == 0;
}

// True for a value a FOR can count with: a long or short integer, a real or a complex number. Anything else, a string or a matrix among them, can be neither stepped nor
// compared, and a loop given one would run its body once and say nothing.
static bool_t forValueCanCount(uint16_t regist) {
  return getRegisterDataType(regist) == dtLongInteger || getRegisterDataType(regist) == dtShortInteger
      || getRegisterDataType(regist) == dtReal34     || getRegisterDataType(regist) == dtComplex34;
}

// True when adding the step to the value leaves it where it was, so no number of passes reaches the end. A step of zero is the plain case; a step too small for the
// value's digits, and a value so large that the step rounds away, are the same fault and take the same message. The sum is worked in the stack the way NEXT works it, so
// the two agree digit for digit. X and Y are put back from a snapshot in memory: saving them into the two saved stack registers allocates, and an allocation while a loop
// is open takes the storage of the loop's own two local registers.
static bool_t forStepCannotMove(uint16_t value, uint16_t stepReg) {
  snap_t snapX, snapY;
  bool_t same;

  saveRegisterSnapshot(REGISTER_X, &snapX);
  saveRegisterSnapshot(REGISTER_Y, &snapY);
  copySourceRegisterToDestRegister(value, TEMP_REGISTER_1);
  copySourceRegisterToDestRegister(stepReg, REGISTER_X);
  copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_Y);
  addition[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();
  same = (lastErrorCode == ERROR_NONE) && forValuesEqual(REGISTER_X, TEMP_REGISTER_1);
  restoreRegisterSnapshot(REGISTER_X, &snapX);
  restoreRegisterSnapshot(REGISTER_Y, &snapY);
  return same;
}

// True when the counter has passed the end value, which ends the loop. A real or integer counter compares against the end directly, upwards for a positive step and
// downwards for a negative one or a descending row, in an arithmetic that keeps a long integer exact. A complex value in any of the three compares two lengths from the
// origin instead, the counter's against the end value's, squared so no root is needed and no answer changes.
static bool_t forCounterPastEnd(uint16_t counter, uint16_t endReg, uint16_t stepReg, bool_t descends) {
  int8_t cmp;

  if(getRegisterDataType(counter) == dtComplex34 || getRegisterDataType(endReg) == dtComplex34
  || getRegisterDataType(stepReg) == dtComplex34) {
    real_t counterRe, counterIm, endRe, endIm, reach, target, term;
    bool_t cmplx;

    if(!getRegisterAsComplexOrAnyRealQuiet(counter, &counterRe, &counterIm, &cmplx)
    || !getRegisterAsComplexOrAnyRealQuiet(endReg,  &endRe,     &endIm,     &cmplx)) {
      return true; // a value the arithmetic cannot read ends the loop instead of running for ever
    }
    realMultiply(&counterRe, &counterRe, &reach,  &ctxtReal39);
    realMultiply(&counterIm, &counterIm, &term,   &ctxtReal39);
    realAdd(&reach, &term, &reach, &ctxtReal39);
    realMultiply(&endRe, &endRe, &target, &ctxtReal39);
    realMultiply(&endIm, &endIm, &term,   &ctxtReal39);
    realAdd(&target, &term, &target, &ctxtReal39);
    realCompare(&reach, &target, &term, &ctxtReal39);
    return !realIsZero(&term) && realIsPositive(&term);
  }

  real_t step39;

  if(!registerCmp(counter, endReg, &cmp) || !getRegisterAsAnyRealQuiet(stepReg, &step39)) {
    return true;
  }
  return (descends || !realIsPositive(&step39)) ? (cmp < 0) : (cmp > 0);
}

// True when the step carried the counter the wrong way. A short integer does that when the addition passes the word size and wraps: the counter reappears at the far end
// and can never reach its end value. The loop ends there, holding the last value that did not wrap.
static bool_t forStepWentBackwards(uint16_t sum, uint16_t counter, uint16_t stepReg, bool_t descends) {
  int8_t cmp;
  real_t step39;

  if(getRegisterDataType(counter) == dtComplex34 || getRegisterDataType(sum) == dtComplex34) {
    return false; // a complex counter has no direction, and its own length test ends the loop
  }
  if(!registerCmp(sum, counter, &cmp) || !getRegisterAsAnyRealQuiet(stepReg, &step39)) {
    return false;
  }
  return (descends || !realIsPositive(&step39)) ? (cmp > 0) : (cmp < 0);
}

// The out of range refusal adjustResult makes for every other addition. adjustResult itself cannot be called here: it drops the stack and undoes the whole step,
// where the caller borrows X and Y and puts them back.
static void forRefuseInfinite(calcRegister_t regist, real34_t *value) {
  if(real34IsInfinite(value)) {
    displayCalcErrorMessage(real34IsPositive(value) ? ERROR_OVERFLOW_PLUS_INF : ERROR_OVERFLOW_MINUS_INF, ERR_REGISTER_LINE);
  }
}

// One step of the counter, and whether that step passed the end. The arithmetic runs in the stack the way an arithmetic store does, borrowing the same two saved
// registers, so a NEXT costs what a STO+ costs and leaves X and Y as it found them. A descending row subtracts the increment where the others add it.
static bool_t forStepAndTest(uint16_t counter, uint16_t endReg, uint16_t stepReg, bool_t descends, bool_t keepLastStep) {
  snap_t snapX, snapY;
  bool_t past, unusable, writeBack;

  saveRegisterSnapshot(REGISTER_X, &snapX);
  saveRegisterSnapshot(REGISTER_Y, &snapY);
  copySourceRegisterToDestRegister(counter, TEMP_REGISTER_1);
  copySourceRegisterToDestRegister(stepReg, REGISTER_X);
  copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_Y);
  if(getRegisterDataType(REGISTER_Y) == dtShortInteger) {
    *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)) &= shortIntegerMask;
  }
  if(descends) {
    subtraction[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();
  }
  else {
    addition[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();
  }
  if(!getSystemFlag(FLAG_SPCRES) && lastErrorCode == ERROR_NONE) { // out of range, which every other addition refuses under this flag
    if(getRegisterDataType(REGISTER_X) == dtReal34) {
      forRefuseInfinite(REGISTER_X, REGISTER_REAL34_DATA(REGISTER_X));
    }
    else if(getRegisterDataType(REGISTER_X) == dtComplex34) {
      forRefuseInfinite(REGISTER_X, REGISTER_REAL34_DATA(REGISTER_X));
      forRefuseInfinite(REGISTER_X, REGISTER_IMAG34_DATA(REGISTER_X));
    }
  }
  if(lastErrorCode == ERROR_NONE && forValuesEqual(REGISTER_X, TEMP_REGISTER_1)) { // the counter has grown until the step rounds away, so the end is now
    displayCalcErrorMessage(ERROR_STEP_OF_ZERO, ERR_REGISTER_LINE);    // unreachable whatever the FOR was given
  }
  unusable = (lastErrorCode != ERROR_NONE) || forStepWentBackwards(REGISTER_X, TEMP_REGISTER_1, stepReg, descends);
  past = unusable || forCounterPastEnd(REGISTER_X, endReg, stepReg, descends);
  // A bottom tested row keeps the value its last pass ran with, a top tested row the step that failed, one past the end, its comparison belonging at the FOR.
  // An unusable step is written for neither: X then contains a wrapped or errored value the loop never produced.
  writeBack = !past || (keepLastStep && !unusable);
  if(writeBack) {
    copySourceRegisterToDestRegister(REGISTER_X, TEMP_REGISTER_1);
  }
  restoreRegisterSnapshot(REGISTER_X, &snapX);
  restoreRegisterSnapshot(REGISTER_Y, &snapY);
  if(writeBack) {
    copySourceRegisterToDestRegister(TEMP_REGISTER_1, counter);
  }
  return past;
}

// FOR opens a counted structure. The start comes from Z, the end from Y and the step from X, the layout Σₙ uses, and all three leave the stack. The start goes to the
// counter; the end and the step go to two local registers of the structure's own, above whatever the routine declared. The body always runs once, because the comparison
// is at the NEXT.
void fnFor(uint16_t regist) {
  if(programRunStop != PGM_RUNNING) {
    return;
  }
  forDropDeadLoops();
  uint16_t row = forLoopOfStep();
  uint16_t base = currentNumberOfLocalRegisters;

  if(row < FOR_MAX_LOOPS && forLoopRegistersLive(row)) {
    return; // this FOR is running already, reached again by BST or by a GTO, so its loop carries on untouched and the stack is left alone
  }
  if(!isRegInRange(regist)) { // the quiet range test, so the counter's own message is the one that shows
    displayCalcErrorMessage(ERROR_INVALID_COUNTER_REGISTER, ERR_REGISTER_LINE);
    return;
  }
  if(!forValueCanCount(REGISTER_Z) || !forValueCanCount(REGISTER_Y) || !forValueCanCount(REGISTER_X)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    return;
  }
  if(forStepCannotMove(REGISTER_Z, REGISTER_X)) { // the start plus the step, against the start
    displayCalcErrorMessage(ERROR_STEP_OF_ZERO, ERR_REGISTER_LINE);
    return;
  }
  if(!forHasNext()) {
    displayCalcErrorMessage(ERROR_NEXT_NOT_FOUND, ERR_REGISTER_LINE);
    return;
  }
  if(row < FOR_MAX_LOOPS) { // the row outlived its registers, which a run that ended inside the loop does, so it is free again
    forLoopTable[row].localStepNumber = 0;
  }
  if(forCounterAlreadyCounted(regist)) { // a running loop already counts in it, so neither could ever end
    displayCalcErrorMessage(ERROR_INVALID_COUNTER_REGISTER, ERR_REGISTER_LINE);
    return;
  }
  row = forFreeLoopRow();
  if(row >= FOR_MAX_LOOPS || base + FOR_LOCALS > FOR_MAX_LOCALS) {
    displayCalcErrorMessage(ERROR_NESTING_TOO_DEEP, ERR_REGISTER_LINE);
    return;
  }
  allocateLocalRegisters(base + FOR_LOCALS);
  if(lastErrorCode != ERROR_NONE) {
    return;
  }
  forLoopTable[row].counterRegister   = regist;
  forLoopTable[row].localRegisterBase = FIRST_LOCAL_REGISTER + base; // where its two registers are, not merely that they are on top
  forLoopTable[row].subroutineLevel   = currentSubroutineLevel;
  forLoopTable[row].programNumber     = currentProgramNumber;
  forLoopTable[row].stepDescends      = 0; // both bits clear: FOR adds the step it was handed, whichever way it points, and compares at its NEXT
  forLoopStep[row]                 = currentStep; // where the FOR stands, so a pass returns to it without counting steps from the top of the program
  forLoopTable[row].localStepNumber   = currentLocalStepNumber; // last, since it is what marks the row taken

  copySourceRegisterToDestRegister(REGISTER_Y, FIRST_LOCAL_REGISTER + base);
  copySourceRegisterToDestRegister(REGISTER_X, FIRST_LOCAL_REGISTER + base + 1);
  fnDrop(NOPARAM);
  fnDrop(NOPARAM);
  copySourceRegisterToDestRegister(REGISTER_X, TEMP_REGISTER_1); // the start goes aside for the last drop and fills the counter after it: a counter in a stack register
  fnDrop(NOPARAM);                                               // would otherwise be written and then moved over by that drop
  copySourceRegisterToDestRegister(TEMP_REGISTER_1, regist);
}

// FORyx opens the same structure as FOR from the start in Y and the end in X. The step is a 1 in the start's type, so the counter holds one type throughout. An end
// below the start marks the row descending and NEXT subtracts that 1, so nothing negative is built and unsigned counts down. A complex value has no direction and counts
// up. The lift leaves the three values FOR reads.
void fnForYx(uint16_t regist) {
  if(programRunStop != PGM_RUNNING) {
    return;
  }
  forDropDeadLoops();
  const uint16_t running = forLoopOfStep();

  if(running < FOR_MAX_LOOPS && forLoopRegistersLive(running)) {
    return; // this FOR is running already, reached again by BST or by a GTO, so the test below it decides the pass and the stack is left alone
  }
  if(!forValueCanCount(REGISTER_Y) || !forValueCanCount(REGISTER_X)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    return;
  }
  const uint32_t startType = getRegisterDataType(REGISTER_Y);
  const uint32_t startBase = getRegisterShortIntegerBase(REGISTER_Y);
  int8_t cmp;
  bool_t down = false;

  if(startType != dtComplex34 && getRegisterDataType(REGISTER_X) != dtComplex34) {
    down = registerCmp(REGISTER_Y, REGISTER_X, &cmp) && cmp > 0; // the start stands above the end, so the count runs down to it
  }
  setSystemFlag(FLAG_ASLIFT); // the end value has to reach Y whatever the step before this one left behind, so the lift is not the conditional one
  liftStack();
  switch(startType) {
    case dtShortInteger: {
      convertUInt64ToShortIntegerRegister(0, 1, startBase, REGISTER_X);
      break;
    }
    case dtLongInteger: {
      longInteger_t stepValue;

      longIntegerInit(stepValue);
      int32ToLongInteger(1, stepValue);
      convertLongIntegerToLongIntegerRegister(stepValue, REGISTER_X);
      longIntegerFree(stepValue);
      break;
    }
    case dtComplex34: {
      reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
      real34Copy(const34_1, REGISTER_REAL34_DATA(REGISTER_X));
      real34Copy(const34_0, REGISTER_IMAG34_DATA(REGISTER_X));
      break;
    }
    default: { // liftStack() has made X a real34 already, so only the value is left to write
      int32ToReal34(1, REGISTER_REAL34_DATA(REGISTER_X));
      break;
    }
  }
  fnFor(regist);
  if(down && lastErrorCode == ERROR_NONE) { // the row at this step and level, which FOR has just filled or found already running
    const uint16_t row = forLoopOfStep();

    if(row < FOR_MAX_LOOPS) {
      forLoopTable[row].stepDescends |= FOR_STEP_DESCENDS;
    }
  }
}

// FORTOP opens the same structure as FOR from the same three values, and the one difference is where the first comparison happens. FOR compares at its NEXT, so a body
// always runs once. FORTOP compares the start against the end before any pass, which is where the HP-71B and ANSI BASIC put it, so a start already past the end runs no
// pass at all and the run carries on below the NEXT. Every later pass is decided at the NEXT exactly as before, so the two forms differ in that first comparison and in
// nothing else. The row is marked FOR_TOP_TESTED, which its NEXT uses to decide what the counter keeps on the way out: a top tested loop leaves it one step past the end.
void fnForTop(uint16_t regist) {
  if(programRunStop != PGM_RUNNING) {
    return;
  }
  forDropDeadLoops();
  const uint16_t entered = forLoopOfStep();
  const bool_t reEntered = entered < FOR_MAX_LOOPS && forLoopRegistersLive(entered); // BST or a GTO back to this step, so the loop is running and nothing is compared

  fnFor(regist);
  if(lastErrorCode != ERROR_NONE) {
    return;
  }
  const uint16_t row = forLoopOfStep();

  if(row >= FOR_MAX_LOOPS) {
    return;
  }
  forLoopTable[row].stepDescends |= FOR_TOP_TESTED;

  const uint16_t base = forLoopTable[row].localRegisterBase;

  if(!reEntered && forCounterPastEnd(regist, base, base + 1, (forLoopTable[row].stepDescends & FOR_STEP_DESCENDS) != 0)) {
    uint16_t ahead = 0;
    uint8_t *nextStep = forNextOfThisFor(&ahead);

    if(nextStep == NULL) { // fnFor refuses a FOR with no NEXT below it, so this cannot be reached from a program that got this far
      displayCalcErrorMessage(ERROR_NEXT_NOT_FOUND, ERR_REGISTER_LINE);
      return;
    }
    forLoopTable[row].localStepNumber = 0;                                                          // the row goes, no pass having been run
    allocateLocalRegisters(forLoopTable[row].localRegisterBase - FIRST_LOCAL_REGISTER);              // and the two registers with it
    currentLocalStepNumber += ahead + 1;
    currentStep = findNextStep(nextStep);
    return;
  }
  currentLocalStepNumber++; // this step places its own pointer, as NEXT does, so the body is entered from here
  currentStep = findNextStep(currentStep);
}

// NEXT steps the counter and decides the pass. A row for a running loop of its counter at its own subroutine level is what says the two local registers at the top of the
// level are its own: a stray NEXT, a body reached by a GTO, and a jump over the FOR all arrive with no such row. Past the end the registers and the row go and the run
// carries on below, otherwise it returns to the step after the FOR, which the row names.
void fnNext(uint16_t regist) {
  if(programRunStop != PGM_RUNNING) {
    return;
  }
  if(!isRegInRange(regist)) {
    displayCalcErrorMessage(ERROR_INVALID_COUNTER_REGISTER, ERR_REGISTER_LINE);
    return;
  }
  forDropDeadLoops();
  const uint16_t row = forLoopOfCounter(regist);

  // A loop of this level that opened at a later step is inside this one and still running, so this NEXT closes out of order. Registers that are no longer there are a
  // LocR or a PopLR that deleted them under the loop, and reading whatever took their place would answer wrongly and say nothing.
  if(row >= FOR_MAX_LOOPS || forInnerLoopStillOpen(row) || !forLoopRegistersLive(row)) {
    displayCalcErrorMessage(ERROR_NEXT_WITHOUT_FOR, ERR_REGISTER_LINE);
    return;
  }
  const uint16_t base = forLoopTable[row].localRegisterBase;

  // The body may write the counter and the loop's own two registers as freely as any other, so all three are tested here, where they are read. A type that cannot be
  // counted would otherwise end the loop without a word.
  if(!forValueCanCount(regist) || !forValueCanCount(base) || !forValueCanCount(base + 1)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "%s of this FOR", !forValueCanCount(regist) ? "the counter" : !forValueCanCount(base) ? "the end value" : "the step");
      moreInfoOnError("In function fnNext:", errorMessage, "holds a type the loop cannot count with", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  const bool_t past = forStepAndTest(regist, base, base + 1, (forLoopTable[row].stepDescends & FOR_STEP_DESCENDS) != 0, (forLoopTable[row].stepDescends & FOR_TOP_TESTED) != 0);

  if(lastErrorCode != ERROR_NONE) {
    return;
  }
  if(!past) {
    if(forLoopStepPointerLive(row)) {
      currentLocalStepNumber = forLoopTable[row].localStepNumber + 1; // the step after the FOR, from the row, so no search is made on a pass
      currentStep = findNextStep(forLoopStep[row]);
      return;
    }
    // No address to return to: a restore does not carry one, and an edit invalidates the one there was. The step number finds the FOR again and a fresh address is
    // taken, so only the first pass after that pays for the search. A step number that no longer holds a FOR is an edit that moved it, and the loop cannot go on.
    currentLocalStepNumber = forLoopTable[row].localStepNumber;
    defineCurrentStep();
    if(structPlainOp(structOpOfStep(currentStep)) != ITM_FOR) {
      displayCalcErrorMessage(ERROR_NEXT_WITHOUT_FOR, ERR_REGISTER_LINE);
      return;
    }
    forLoopStep[row] = currentStep;
    currentStep = findNextStep(currentStep);
    currentLocalStepNumber++;
    return;
  }
  forLoopTable[row].localStepNumber = 0;
  allocateLocalRegisters(forLoopTable[row].localRegisterBase - FIRST_LOCAL_REGISTER); // back to what the level held before this FOR grew it
  fnSkip(0);
}

// True when the byte range holds a STRUCT step that pairs: a numbered step, or a FOR or a NEXT. The editor asks before it inserts or deletes a range.
bool_t structRangeHasNumbered(uint8_t *from, uint8_t *to) {
  uint8_t *step = from;

  while(step != NULL && step < to) {
    const uint16_t op = structPlainOp(structOpOfStep(step));

    if(structOpHasNumber(op) || op == ITM_FOR || op == ITM_NEXT) { // a FOR or a NEXT leaving the program unpairs the rest as surely as a numbered step
      return true;
    }
    step = findNextStep(step);
  }
  return false;
}

// Set every partner number of the open program back to 0. An edit that adds or removes a numbered step can leave the rest paired wrongly, and the numbers are what the
// run and the file writer check, so they go and VALID puts them back.
void structClearProgramNumbers(void) {
  uint8_t *step = beginOfCurrentProgram;

  while(step != NULL && step < endOfCurrentProgram && !isAtEndOfProgram(step) && !isAtEndOfPrograms(step)) {
    uint8_t *nextStep = findNextStep(step);

    if(structOpHasNumber(structOpOfStep(step)) && nextStep != NULL && nextStep - step >= 3) {
      *(step + 2) = 0;
    }
    step = nextStep;
  }
}

// The marked form of a FOR or a NEXT. Only VALID writes it and only VALID takes it off. Reaching one in a run means VALID has not passed the structure since it was last
// edited, so the run stops with the message the numbered structures give for the same thing.
void fnForNotChecked(uint16_t regist) {
  temporaryInformation = TI_NO_INFO;
  displayCalcErrorMessage(ERROR_STRUCTURE_NOT_NUMBERED, ERR_REGISTER_LINE);
}

// The not checked form of a FOR or a NEXT. A new step enters the program in that form. Everything else is itself.
uint16_t structNotCheckedOp(uint16_t op) {
  if(op == ITM_FOR) {
    return ITM_FORx;
  }
  if(op == ITM_FORYX) {
    return ITM_FORYXx;
  }
  if(op == ITM_FORTOP) {
    return ITM_FORTOPx;
  }
  if(op == ITM_NEXT) {
    return ITM_NEXTx;
  }
  return op;
}

// Write the marked form of a FOR or a NEXT over the step given. Both forms are two op code bytes and carry the same counter, so only the second byte changes and no step
// moves.
static void structMarkNotChecked(uint8_t *step) {
  uint16_t op = structOpOfStep(step);

  if(op == ITM_FOR) {
    *(step + 1) = ITM_FORx & 0xff;
  }
  else if(op == ITM_FORYX) {
    *(step + 1) = ITM_FORYXx & 0xff;
  }
  else if(op == ITM_FORTOP) {
    *(step + 1) = ITM_FORTOPx & 0xff;
  }
  else if(op == ITM_NEXT) {
    *(step + 1) = ITM_NEXTx & 0xff;
  }
}

// Rewrite every FOR and NEXT of the open program from one form to the other. The two forms differ only in the second op code byte, so no step moves and the operand is
// untouched.
static void forRewriteMarks(uint16_t fromFor, uint16_t toFor, uint16_t fromNext, uint16_t toNext) {
  uint8_t *step = beginOfCurrentProgram;

  while(step != NULL && step < endOfCurrentProgram && !isAtEndOfProgram(step) && !isAtEndOfPrograms(step)) {
    uint16_t op = structOpOfStep(step);

    if(op == fromFor) {
      *(step + 1) = toFor & 0xff;
    }
    else if(op == fromNext) {
      *(step + 1) = toNext & 0xff;
    }
    step = findNextStep(step);
  }
}

// Take the mark off every FOR and NEXT of the open program. VALID does this only when the walk found no fault in them.
static void structClearMarks(void) {
  forRewriteMarks(ITM_FORx, ITM_FOR, ITM_NEXTx, ITM_NEXT);
  forRewriteMarks(ITM_FORYXx, ITM_FORYX, ITM_NEXTx, ITM_NEXT); // the second walk finds only the FORyx steps, the NEXT of the first walk being plain already
  forRewriteMarks(ITM_FORTOPx, ITM_FORTOP, ITM_NEXTx, ITM_NEXT); // and the third only the FORTOP steps, for the same reason
}

// Forget that VALID has checked the FOR structures of the open program: every FOR and NEXT goes back to the not checked form. An edit that adds or removes a FOR or a
// NEXT calls this, as it calls the number clearing above, and for the same reason: what is left may pair wrongly.
void forClearChecked(void) {
  forRewriteMarks(ITM_FOR, ITM_FORx, ITM_NEXT, ITM_NEXTx);
  forRewriteMarks(ITM_FORYX, ITM_FORYXx, ITM_NEXT, ITM_NEXTx); // the second walk finds only the FORyx steps, the NEXT of the first walk being marked already
  forRewriteMarks(ITM_FORTOP, ITM_FORTOPx, ITM_NEXT, ITM_NEXTx); // and the third only the FORTOP steps, for the same reason
}

// True when a structure step of the selected program carries no valid partner number, or is a FOR or a NEXT that VALID has not passed. Such a program can be neither
// stored nor run, so the file writer asks before it opens the file. The caller has already selected the program.
bool_t structProgramHasUnnumbered(void) {
  uint8_t *step = beginOfCurrentProgram;

  while(step != NULL && step < endOfCurrentProgram && !isAtEndOfProgram(step) && !isAtEndOfPrograms(step)) {
    uint8_t *nextStep = findNextStep(step);

    const uint16_t op = structOpOfStep(step);

    if(op == ITM_FORx || op == ITM_NEXTx || op == ITM_FORYXx || op == ITM_FORTOPx) { // VALID has not passed this structure, so the program can be neither stored nor run
      return true;
    }
    if(structOpHasNumber(op) && nextStep != NULL && nextStep - step >= 3 && !structNumberIsValid(*(step + 2))) {
      return true;
    }
    step = nextStep;
  }
  return false;
}

// A paused run owns the program pointer: it is where R/S resumes. VALID reads the program and must leave that alone, so it only moves the editor when no run is waiting.
static bool_t structMayMoveThePointer(void) {
  return programRunStop == PGM_STOPPED;
}

// Put the editor on this step, the same way goToGlobalStep() does, so the listing scrolls to it and the cursor lands on it rather than on the header.
static void structGoToLocalStep(uint16_t localStepNumber) {
  currentLocalStepNumber = localStepNumber;
  defineCurrentStep();
  firstDisplayedLocalStepNumber = (localStepNumber >= 3) ? localStepNumber - 3 : 0; // 0 is what keeps the program header line on screen, as GTO does
  defineFirstDisplayedStep();
  pemCursorIsZerothStep = false;
}

// The commands that leave a test answer for the step after them, which is what an IF or a WHILE reads. They are every item a program can hold whose function leaves
// temporaryInformation at TI_TRUE or TI_FALSE. A new test command has to be added here as well.
TO_QSPI const int16_t structTest[] = {
  ITM_ISE,        ITM_ISG,        ITM_ISZ,        ITM_DSE,        ITM_DSL,        ITM_DSZ,
  ITM_XEQU,       ITM_XNE,        ITM_XEQUP0,     ITM_XEQUM0,     ITM_XAEQU,      ITM_XLT,
  ITM_XLE,        ITM_XGE,        ITM_XGT,        ITM_XGEP0,      ITM_XLEM0,
  ITM_FC,         ITM_FS,         ITM_FCC,        ITM_FCS,        ITM_FCF,        ITM_FSC,
  ITM_FSS,        ITM_FSF,        ITM_BS,         ITM_BC,
  ITM_EVEN,       ITM_ODD,        ITM_FPQ,        ITM_INTQ,       ITM_LINTQ,      ITM_SINTQ,
  ITM_NANQ,       ITM_SPECQ,      ITM_INFQ,       ITM_PRIME,      ITM_NUMBRQ,     ITM_REALQ,
  ITM_CPXQ,       ITM_STRINGQ,    ITM_MATRIXQ,    ITM_REALMATQ,   ITM_COMPLEXMATQ,
  ITM_M_SQRQ,     ITM_M_FIND,     ITM_ISVECT2DQ,  ITM_ISVECT3DQ,
  ITM_ISREZQ,     ITM_ISIMZQ,     ITM_ISRENZQ,    ITM_ISIMNZQ,
  ITM_ANGLEQ,     ITM_DATEQ,      ITM_TIMEQ,      ITM_LEAPQ,      ITM_CONFIGQ,
  ITM_LBLQ,       ITM_TOP,        ITM_ENTRY,      ITM_KEYQ,       ITM_CONVG
};

// True when the item leaves a test answer.
static bool_t structOpIsTest(uint16_t op) {
  uint16_t i;

  for(i = 0; i < sizeof(structTest) / sizeof(structTest[0]); i++) {
    if(op == (uint16_t)structTest[i]) {
      return true;
    }
  }
  return false;
}

// Put the editor on the step VALID is reporting and raise the message, so the program opens where the fault is with that step in view.
static void structReportFault(uint16_t localStepNumber, uint16_t error) {
  if(structMayMoveThePointer()) {
    structGoToLocalStep(localStepNumber);
  }
  temporaryInformation = TI_NO_INFO; // a pending test display would otherwise cover the message
  displayCalcErrorMessage(error, ERR_REGISTER_LINE);
}

// One walk of the open program, from its first step to its END. It checks the structures and, on the second call, numbers them: every structure gets its own number, in
// the order the openers appear, IF and DO in two series of their own. A fault stops the walk with the editor on the offending step, and nothing is numbered, because
// the checking call always runs first. A stopAt walks to that step instead of to the END, which is where a keyed END would put one.
static bool_t structWalkProgram(bool_t numbering, const uint8_t *stopAt) {
  uint8_t *step = beginOfCurrentProgram;
  uint16_t localStepNumber = 1, depth = 0, forDepth = 0, previousOp = 0, previousStepOp = 0;
  uint16_t ifNext = 0, doNext = 0, repeatNext = 0;
  bool_t forSeen = false;                       // a FOR has taken local registers of its own in this routine, so no LocR may set the count after it
  uint8_t *firstForStep = NULL;                 // that first FOR, which is what a LocR after it would rob

  while(step != NULL && (stopAt == NULL || step < stopAt) && step < endOfCurrentProgram && !isAtEndOfProgram(step) && !isAtEndOfPrograms(step)) {
    uint16_t op = structPlainOp(structOpOfStep(step)); // a step VALID marked last time is checked again as the structure it is
    uint8_t *nextStep = findNextStep(step);

    // A label after a return starts the next routine, with any number of REMs between the two. A structure still open there would close in a routine of its own, which
    // no jump could pair with its opener, so it is flagged as crossed and refused when its closer is reached.
    // The counters run on: they never issue a number twice, so a routine's numbers are its own without a step at the boundary.
    if(op == ITM_LBL && (previousOp == ITM_RTN || previousOp == ITM_RTNP1)) {
      uint16_t open;

      forSeen = false; // the next routine declares its own local registers from the start
      for(open = 0; open < depth; open++) {
        structOpenCrossed[open] = true;
      }
    }

    // The step before an IF, a WHILE or an UNTIL has to leave a test answer. Nothing stands between the two, so this reads the step itself and not the last step that was
    // not a REM.
    if((op == ITM_IF || op == ITM_WHILE || op == ITM_UNTIL) && !structOpIsTest(previousStepOp)) {
      structClearProgramNumbers(); // the step keyed above the test is neither numbered nor a FOR, so nothing else takes the numbers off
      structReportFault(localStepNumber, ERROR_IF_WHILE_CONDITION_MISSING);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "step %" PRIu16 ":", localStepNumber);
        moreInfoOnError("In function structWalkProgram:", errorMessage, "the step above this IF, WHILE or UNTIL is not a test", NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }

    // A FOR takes two local registers at the top of its level and a NEXT reads them back. LocR and PopLR set the count outright, so either of them after the first FOR of
    // the routine would take that pair away. The routine declares what it needs before its first FOR, and the count is left alone after.
    if(op == ITM_FOR) {
      if(!forSeen) {
        firstForStep = step;
      }
      forSeen = true;
    }
    else if(forSeen && (op == ITM_LocR || op == ITM_POPLR)) {
      structMarkNotChecked(firstForStep); // the LocR is the offending step, but the FOR whose registers it would take is what must not run
      structReportFault(localStepNumber, ERROR_STRUCTURE_INVALID);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "step %" PRIu16 ":", localStepNumber);
        moreInfoOnError("In function structWalkProgram:", errorMessage,
                        "this LocR or PopLR comes below a FOR of the same routine, and a jump can put the run back inside that loop", NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }

    // FOR and NEXT go on the same stack as IF and DO, so a structure crossing another is caught wherever the crossing happens. They take no number: the counter pairs
    // them, and the counter written on the NEXT has to be the one written on the FOR it closes.
    if(op == ITM_FOR) {
      if(depth >= STRUCT_MAX_NESTING || forDepth >= FOR_MAX_LOOPS) { // more FOR structures inside one another than the table has rows could never all run
        structReportFault(localStepNumber, ERROR_NESTING_TOO_DEEP);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "step %" PRIu16 ":", localStepNumber);
          moreInfoOnError("In function structWalkProgram:", errorMessage,
                          depth >= STRUCT_MAX_NESTING ? "this FOR passes STRUCT_MAX_NESTING structures open at once"
                                                      : "this FOR passes FOR_MAX_LOOPS FOR structures inside one another", NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
      forDepth++;
      structOpenedBy[depth] = op;
      structOpenSawWhile[depth] = false;
      structOpenStepNumber[depth] = localStepNumber;
      structOpenCrossed[depth] = false;
      structOpenForStep[depth] = step;
      depth++;
    }
    else if(op == ITM_NEXT) {
      if(depth == 0 || structOpenedBy[depth - 1] != ITM_FOR) {
        structMarkNotChecked(step);
        structReportFault(localStepNumber, depth == 0 ? ERROR_NEXT_WITHOUT_FOR : ERROR_STRUCTURE_INVALID);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "step %" PRIu16 ":", localStepNumber);
          moreInfoOnError("In function structWalkProgram:", errorMessage, "this NEXT has no open FOR to close, and what is open here was opened by something else", NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
      if(!forStepsSameCounter(structOpenForStep[depth - 1], step)) {
        // a FOR and NEXT are paired by their common counter register. This NEXT's counter is different from the open FOR, so it closes nothing and the FOR has no NEXT.
        structMarkNotChecked(structOpenForStep[depth - 1]);
        structReportFault(structOpenStepNumber[depth - 1], ERROR_NEXT_NOT_FOUND);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "step %" PRIu16 ":", structOpenStepNumber[depth - 1]);
          moreInfoOnError("In function structWalkProgram:", errorMessage,
                          "the NEXT below this FOR counts in a different register, so this FOR has no matching NEXT", NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
      // the crossing test further down only reads steps with a partner number, and a FOR and a NEXT have none, so this pair is tested here
      if(structOpenCrossed[depth - 1]) {
        // Marked before the report: a boundary between a FOR and its NEXT is neither a numbered step nor a FOR, so nothing else takes the pair out of the passed form.
        structMarkNotChecked(structOpenForStep[depth - 1]);
        structMarkNotChecked(step);
        structReportFault(localStepNumber, ERROR_STRUCTURE_INVALID); // the NEXT is the offending step: the FOR above the boundary is written correctly
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "step %" PRIu16 ":", localStepNumber);
          moreInfoOnError("In function structWalkProgram:", errorMessage,"the FOR this NEXT closes is in another routine, so nothing can pair them at run time", NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
      forDepth--;
      depth--;
    }
    else if(op == ITM_IF || op == ITM_DO || op == ITM_REPEAT) {
      if(depth >= STRUCT_MAX_NESTING) {
        structReportFault(localStepNumber, ERROR_NESTING_TOO_DEEP);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "step %" PRIu16 ":", localStepNumber);
          moreInfoOnError("In function structWalkProgram:", errorMessage, "this opener passes STRUCT_MAX_NESTING structures open at once", NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
      if((op == ITM_IF ? ifNext : op == ITM_DO ? doNext : repeatNext) >= STRUCT_MAX_NUMBER) {
        structReportFault(localStepNumber, ERROR_STRUCTURE_INVALID);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "step %" PRIu16 ":", localStepNumber);
          moreInfoOnError("In function structWalkProgram:", errorMessage, "the partner numbers of this opener's family are used up at STRUCT_MAX_NUMBER", NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
      structOpenedBy[depth] = op;
      structOpenSawWhile[depth] = false;
      structOpenStepNumber[depth] = localStepNumber;
      structOpenCrossed[depth] = false;
      structOpenNumber[depth] = (op == ITM_IF) ? ++ifNext : (op == ITM_DO) ? ++doNext : ++repeatNext; // every structure takes a number of its own, never one already used
      depth++;
    }
    else if(structOpHasNumber(op)) {
      const uint16_t opener = (op == ITM_ELSE || op == ITM_ENDIF) ? ITM_IF : (op == ITM_UNTIL) ? ITM_REPEAT : ITM_DO;

      if(depth == 0) {
        structReportFault(localStepNumber, ERROR_STRUCTURE_INVALID);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "step %" PRIu16 ":", localStepNumber);
          moreInfoOnError("In function structWalkProgram:", errorMessage, "this closer has no structure open above it", NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
      if(structOpenedBy[depth - 1] != opener) {
        structReportFault(localStepNumber, ERROR_STRUCTURE_INVALID);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "step %" PRIu16 ":", localStepNumber);
          moreInfoOnError("In function structWalkProgram:", errorMessage, "this closer does not match the structure it would close", NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
      if(op == ITM_WHILE) {
        structOpenSawWhile[depth - 1] = true;
      }
      if(op == ITM_ENDDO && !structOpenSawWhile[depth - 1]) { // a loop with no test never ends, so it is a fault and not a run to be started
        structReportFault(structOpenStepNumber[depth - 1], ERROR_STRUCTURE_INVALID);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "step %" PRIu16 ":", structOpenStepNumber[depth - 1]);
          moreInfoOnError("In function structWalkProgram:", errorMessage, "this DO has no WHILE, so its loop could never end", NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
    }
    // A closer takes the number its opener was given. A routine boundary between the two leaves nothing that can pair them at run time, so the structure is refused and
    // the editor lands on the step below the boundary, which is the one written wrongly.
    if(structOpHasNumber(op) && depth > 0) {
      const uint16_t level = depth - 1;

      if(structOpenCrossed[level] && op != ITM_IF && op != ITM_DO && op != ITM_REPEAT) {
        structClearProgramNumbers(); // numbers left in place would leave the program storable, exportable and runnable
        structReportFault(localStepNumber, ERROR_STRUCTURE_INVALID); // this step is the offending one: the opener above the boundary is written correctly
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "step %" PRIu16 ":", localStepNumber);
          moreInfoOnError("In function structWalkProgram:", errorMessage, "the structure this step belongs to was opened in another routine", NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
      // The number is the step's third byte. A step damaged shorter than that is left alone: writing it would land on the step after, or on the END.
      if(numbering && nextStep != NULL && nextStep - step >= 3) {
        *(step + 2) = (uint8_t)structOpenNumber[level];
      }
    }
    if(op == ITM_ENDIF || op == ITM_ENDDO || op == ITM_UNTIL) {
      depth--;
    }
    previousStepOp = op;
    if(op != ITM_REM) { // a REM is transparent to the routine boundary, and to nothing else
      previousOp = op;
    }
    step = nextStep;
    localStepNumber++;
  }

  if(depth > 0) {
    // The innermost one still open. A FOR reaching the END with no NEXT is that message rather than the general one, and the cursor lands on the FOR.
    if(structOpenedBy[depth - 1] == ITM_FOR) {
      structMarkNotChecked(structOpenForStep[depth - 1]);
    }
    structReportFault(structOpenStepNumber[depth - 1], structOpenedBy[depth - 1] == ITM_FOR ? ERROR_NEXT_NOT_FOUND : ERROR_STRUCTURE_INVALID);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "step %" PRIu16 ":", structOpenStepNumber[depth - 1]);
      moreInfoOnError("In function structWalkProgram:", errorMessage, "this structure reaches the END of the program still open", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  if(numbering) { // the checking call passed, so nothing here is marked any more
    structClearMarks();
  }
  return true;
}

// Numbers the structures of the open program and reports the first fault. It is not programmable: it edits the program.
void fnValid(uint16_t unusedButMandatoryParameter) {
  defineCurrentProgramFromCurrentStep();
  if(!structWalkProgram(false, NULL)) {
    return;
  }
  const uint16_t editedStep   = currentLocalStepNumber;         // VALID numbers the program and leaves the editor exactly as it found it,
  const uint16_t editedFirst  = firstDisplayedLocalStepNumber;  // the step under the cursor and the step at the top of the window both
  const bool_t   editedZeroth = pemCursorIsZerothStep;

  (void)structWalkProgram(true, NULL); // the checking call above passed, so this one cannot fault
  if(structMayMoveThePointer()) {
    currentLocalStepNumber = editedStep;
    defineCurrentStep();
    firstDisplayedLocalStepNumber = editedFirst;
    defineFirstDisplayedStep();
    pemCursorIsZerothStep = editedZeroth;
  }
  temporaryInformation = TI_NO_INFO;
}

// True when the steps above the cursor check out as a program by themselves. An END keyed there splits the program in two and the editor follows the second half, so
// AVALID reaches that one on the way out; the first half is left behind and nothing else would ever check it.
bool_t structEndSplitsWell(void) {
  defineCurrentProgramFromCurrentStep();
  return structWalkProgram(false, currentStep);
}

// CLSTRUC leaves every structure that is open and every subroutine that has not returned, stops the run, and puts the step pointer at the top of the program. Nothing of
// the run is left: no loop holds a row or a pair of local registers, no level holds its locals, and the ! icon goes with the paused state. The partner numbers of that
// program go back to 0 and its FOR and NEXT steps back to the marked form, so VALID allocates the numbers again before the program can be stored or run.
void fnClearStructures(uint16_t unusedButMandatoryParameter) {
  const uint16_t theProgram = currentProgramNumber;

  forClearLoops();
  while(currentSubroutineLevel > 0) { // the same unwind fnClP makes before it deletes a program, and for the same reason
    fnReturn(0);
  }
  cleanLocalFlagsAndRegisters(); // the level the run started in gives up its locals, all fnReturn does at level 0 beyond a step position goToPgmStep sets below
  programRunStop = PGM_STOPPED; // a paused or running program is stopped, and the ! icon goes with PGM_WAITING
  goToPgmStep(theProgram, 1);
  pemCursorIsZerothStep = true; // the editor draws step 0000 from this, so the listing opens at the top by either route
  defineCurrentProgramFromCurrentStep(); // the two calls below read the open program, which the step above has just settled
  structClearProgramNumbers();
  forClearChecked();
  temporaryInformation = TI_NO_INFO;
}

#endif // OPTION_STRUCTURED_PGM

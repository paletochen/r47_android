// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file differentiate.c
 ***********************************************/

#include "c47.h"

#define DERIV_FIRST_SHIFT       1   // h starts at x/10, the coarsest step a 15 point stencil is worth taking
#define DERIV_LAST_SHIFT       16   // and stops at x*1e-16, the step this engine used for every stencil before the ladder
#define DERIV_TOLERANCE_DIGITS 32   // digits a sample carries, less one for the coefficient sum, which is what two estimates are compared against
#define DERIV_WORK_REALS       (8 + MAX_F_EVAL)   // calcDeriv's heap block: eight reals, then fx

_Static_assert(sizeof(real_t) == REAL_SIZE_IN_BYTES(75), "fx is indexed as a real_t array");


#if 0
// All of the above finite differences combined into a single array */
TO_QSPI static const FINITE_DIFF_COEFF *const all_first_derivatives[] = {
    &der_1_central_10,      &der_1_central_8,       &der_1_central_6,
    &der_1_central_4,       &der_1_central_2,
    &der_1_upper_5,         &der_1_upper_4,         &der_1_strict_upper_5,
    &der_1_upper_3,         &der_1_strict_upper_4,  &der_1_upper_2,
    &der_1_strict_upper_3,  &der_1_upper_1,         &der_1_strict_upper_2,
    &der_1_lower_5,         &der_1_lower_4,         &der_1_strict_lower_5,
    &der_1_lower_3,         &der_1_strict_lower_4,  &der_1_lower_2,
    &der_1_strict_lower_3,  &der_1_lower_1,         &der_1_strict_lower_2,
    NULL
};
TO_QSPI static const FINITE_DIFF_COEFF *const all_second_derivatives[] = {
    // &der_2_central_10
    &der_2_central_8,       &der_2_central_6,       &der_2_central_4,
    &der_2_central_2,
    &der_2_upper_5,         &der_2_upper_4,         &der_2_strict_upper_5,
    &der_2_upper_3,         &der_2_strict_upper_4,  &der_2_upper_2,
    &der_2_strict_upper_3,
    &der_2_lower_5,         &der_2_lower_4,         &der_2_strict_lower_5,
    &der_2_lower_3,         &der_2_strict_lower_4,  &der_2_lower_2,
    &der_2_strict_lower_3,
    NULL
};
#endif // 0

static void calcDeriv(calcRegister_t label, const FINITE_DIFF_COEFF *const *finDiff);
static calcRegister_t deriv_pgm_variable(calcRegister_t label, bool_t *usesDelta);

static void calcDerivOfOrder(uint16_t label, int order) {
  calcDeriv(label, finite_difference_table[order]);
}


// A program that declares MVARs has more than one thing it could be differentiated with respect to, and the answer depends on which, so the MVAR menu is opened for
// the user to say, the way the solver and the integrator do. The variable key stores the point and takes the selection, and f' on the last softkey runs it. Inside a
// running program, or under another engine, there is nobody to press a key: the derivative is taken there and then, with respect to the selected variable.
static bool_t deriv_open_mvar_menu(uint16_t label, uint16_t order, bool_t solving) {
  if(programRunStop == PGM_RUNNING || solving || getSystemFlag(FLAG_INTING)) {
    return false;
  }
  currentSolverProgram = label - FIRST_LABEL;
  currentMvarLabel = INVALID_VARIABLE;   // the menu builds from currentSolverProgram, and its variable key acts for the solver rather than for VARMNU
  currentSolverStatus &= ~SOLVER_STATUS_EQUATION_MODE;
  currentSolverStatus |= (order == DERIVATIVE_FIRST_CENTRAL ? SOLVER_STATUS_EQUATION_1ST_DERIVATIVE : SOLVER_STATUS_EQUATION_2ND_DERIVATIVE);
  currentSolverStatus |= SOLVER_STATUS_INTERACTIVE;
  showSoftmenu(-MNU_MVAR);
  return true;
}

static void derivativeEquation(uint16_t order, uint8_t ti) {
  // FLAG_SOLVING suppresses the per-item undo snapshot, so the one calcDeriv takes before sampling survives to be restored, and it is what lets execProgram run a
  // body at all.
  bool_t solving = getSystemFlag(FLAG_SOLVING);
  real34_t probeValue;   // only the test result is used, and this frame is under every sample
  snap_t savedRegister;
  bool_t restore;

  setSystemFlag(FLAG_SOLVING);
  if(!(currentSolverVariable >= FIRST_NAMED_VARIABLE && currentSolverVariable <= LAST_NAMED_VARIABLE)) {
    // Nothing selected. A formula with exactly one variable leaves no choice, so take it, as fnEqSolvGraph does. A program is excluded: its variables are its MVAR
    // declarations, and a formula in the pool is not one of them. Parsed into the tail of tmpString, as softmenus.c does, to leave aimBuffer alone.
    if(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) {
      parseEquation(currentFormula, EQUATION_PARSER_MVAR, tmpString + TMP_STR_LENGTH - AIM_BUFFER_LENGTH, tmpString);
      if(tmpString[0] != 0 && (getNthString((uint8_t *)tmpString, 1))[0] == 0) {
        currentSolverVariable = findOrAllocateNamedVariable(tmpString);
      }
    }
    if(!(currentSolverVariable >= FIRST_NAMED_VARIABLE && currentSolverVariable <= LAST_NAMED_VARIABLE)) {
      if(!solving) {
        clearSystemFlag(FLAG_SOLVING);
      }
      displayCalcErrorMessage(ERROR_VARIABLE_NOT_SELECTED, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function derivativeEquation:", "no variable selected for the derivative", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  //new method to maintain solver variable
  reallyRunFunction(ITM_RCL, currentSolverVariable);
  // The sampling stores each point in the variable, so its own value is kept here and put back after. A register cannot hold it: for a program the user's code runs
  // in between and reaches every temporary register, which is what used to hand the variable back holding a number out of that program.
  restore = (currentSolverVariable != INVALID_VARIABLE) && getRegisterAsReal34Quiet(currentSolverVariable, &probeValue);
  if(restore) {
    saveRegisterSnapshot(currentSolverVariable, &savedRegister);
  }
  if(!(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && currentSolverProgram < numberOfLabels) {
    calcDerivOfOrder(currentSolverProgram + FIRST_LABEL, order);   // the MVAR menu was opened on a program, so that is what this key differentiates
  }
  else {
    currentSolverStatus |= SOLVER_STATUS_USES_FORMULA;
    calcDerivOfOrder(INVALID_VARIABLE, order);
  }
  if(restore) {
    restoreRegisterSnapshot(currentSolverVariable, &savedRegister);
  }
  temporaryInformation = ti;
  if(!solving) {
    clearSystemFlag(FLAG_SOLVING);
  }
}

// PGMDRV names the program f' and f" differentiate, the way PGMSLV names the solver's and PGMPLT the plotter's. It is a slot of its own so that taking a derivative
// does not repoint what SOLVE, INT and PLOT will run next.
void fnPgmDrv(uint16_t label) {
  label = findProgramLabel(label, "In function fnPgmDrv:");
  if(label != INVALID_VARIABLE) {
    currentDerivProgram = label - FIRST_LABEL;
  }
}


// f' and f" in the SOLVE form: From the keyboard the operand is a program and the MVAR menu opens on it, so the variable is picked off a softkey.
// As a program step the operand is a variable, the program is the one PGMDRV named and the variable is the parameter.
static void derivativeVariable(uint16_t variable, uint16_t order, uint8_t ti) {
  bool_t solving;
  real34_t probeValue;   // only the test result is used, and this frame is under every sample
  snap_t savedRegister;
  bool_t restore;

  if((FIRST_UC_LOCAL_LABEL <= variable && variable <= LAST_LOCAL_LABEL) || (FIRST_LABEL <= variable && variable <= LAST_LABEL) || (REGISTER_X <= variable && variable <= REGISTER_T)) {
    currentSolverStatus &= ~SOLVER_STATUS_USES_FORMULA;   // a formula left in play would otherwise build the menu from its variables rather than the program's
    fnPgmDrv(variable);
    if(lastErrorCode == ERROR_NONE) {
      deriv_open_mvar_menu(currentDerivProgram + FIRST_LABEL, order, getSystemFlag(FLAG_SOLVING));
    }
    return;
  }
  if(!(FIRST_NAMED_VARIABLE <= variable && variable <= LAST_NAMED_VARIABLE)) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "unexpected parameter %u", variable);
      moreInfoOnError("In function derivativeVariable:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  if(currentDerivProgram >= numberOfLabels) {
    displayCalcErrorMessage(ERROR_NO_PROGRAM_SPECIFIED, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function derivativeVariable:", "no program named by PGMDRV", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  solving = getSystemFlag(FLAG_SOLVING);
  setSystemFlag(FLAG_SOLVING);
  currentSolverStatus &= ~SOLVER_STATUS_USES_FORMULA;
  currentSolverVariable = variable;
  reallyRunFunction(ITM_STO, currentSolverVariable);   // the point comes off the stack, as SOLVE takes its guesses and ∫ its limits, and calcDeriv reads it from X
  // The sampling stores each point in the variable, so the given point is kept here and put back after, leaving the variable on the value it was differentiated at.
  restore = getRegisterAsReal34Quiet(currentSolverVariable, &probeValue);
  if(restore) {
    saveRegisterSnapshot(currentSolverVariable, &savedRegister);
  }
  calcDerivOfOrder(currentDerivProgram + FIRST_LABEL, order);
  if(restore) {
    restoreRegisterSnapshot(currentSolverVariable, &savedRegister);
  }
  temporaryInformation = ti;
  if(!solving) {
    clearSystemFlag(FLAG_SOLVING);
  }
}

void fn1stDerivVar(uint16_t variable) {
  derivativeVariable(variable, DERIVATIVE_FIRST_CENTRAL, TI_1ST_DERIVATIVE);
}

void fn2ndDerivVar(uint16_t variable) {
  derivativeVariable(variable, DERIVATIVE_SECOND_CENTRAL, TI_2ND_DERIVATIVE);
}

void fn1stDerivEq(uint16_t unusedButMandatoryParameter) {
  derivativeEquation(DERIVATIVE_FIRST_CENTRAL, TI_1ST_DERIVATIVE);
}

void fn2ndDerivEq(uint16_t unusedButMandatoryParameter) {
  derivativeEquation(DERIVATIVE_SECOND_CENTRAL, TI_2ND_DERIVATIVE);
}

/* The following routines are ported from WP34s. */

// Is the step variable one of the current formula's own variables? A function that uses it writes to it while it is being sampled, so it cannot also be the step.
// The list searched here is the same parse the MVAR menu is built from. The program side of the same question is answered by deriv_pgm_variable, which reports a
// declaration of the name while it is already walking them.
static bool_t deriv_formula_uses_delta(void) {
  uint16_t i;

  parseEquation(currentFormula, EQUATION_PARSER_MVAR, tmpString + TMP_STR_LENGTH - AIM_BUFFER_LENGTH, tmpString);
  for(i = 0; (getNthString((uint8_t *)tmpString, i))[0] != 0; i++) {
    if(compareString((char *)getNthString((uint8_t *)tmpString, i), STD_delta STD_SUB_d, CMP_NAME) == 0) {
      return true;
    }
  }
  return false;
}


// The step the user set, which is the step variable. It is taken unchanged and the ladder is not walked at all. False means it was not there, h is then undefined,
// and the caller scales it per pass. Asked once per derivative, since none of this can change while the ladder runs.
static bool_t deriv_user_step(real_t *h, bool_t usesDelta) {
  calcRegister_t deltaX;

  if(!usesDelta && (deltaX = findNamedVariable(STD_delta STD_SUB_d)) != INVALID_VARIABLE &&
     getRegisterAsRealQuiet(deltaX, h) && !realIsZero(h) && !realIsSpecial(h)) {
    return true;
  }
  return false;
}


// Digits a sample is good for, and how far the step is worth shrinking. Normally 34. While a graph is drawn the calculator works to the SDIGS setting instead, so
// two estimates can never match to 34 digits, the step shrinks all the way down and the samples cancel to nothing. The solver reads the same setting for its own
// tolerance.
static int32_t deriv_tolerance_digits(void) {
  return graphAccActive ? max(significantDigitsForEqnGraphs - 2, 4) : DERIV_TOLERANCE_DIGITS;
}

static int32_t deriv_last_shift(void) {
  return graphAccActive ? deriv_tolerance_digits() / 2 : DERIV_LAST_SHIFT;
}


// Do two estimates of the same derivative, taken a factor of ten apart in h, agree to better than the cancellation the finer one suffers? A sample carries 34 digits
// and the points are h apart, so differencing them loses the digits they share and the error is about 1e-DERIV_TOLERANCE_DIGITS times ten to the shift, for each
// order of the derivative. Agreement means the truncation of the coarser estimate is already below that, so the coarser one is the better of the two. The gap
// between the pair is handed back for the caller to rank the pairs by, whether they agreed or not. tolerance is the caller's working space.
static bool_t deriv_agrees(const real_t *coarse, const real_t *fine, int shift, uint8_t order, real_t *difference, real_t *tolerance) {
  realSubtract(fine, coarse, difference, &ctxtReal39);
  if(realIsZero(difference)) {
    return true;
  }
  realCopyAbs(fine, tolerance);
  tolerance->exponent += shift * order - deriv_tolerance_digits();
  return realCompareAbsLessThan(difference, tolerance);
}


// A program that declares MVARs takes its argument from named storage (RCL 'x'), not from the stack, so the sample point has to be stored where the program will
// recall it. Return the variable to perturb, or INVALID_VARIABLE for a program that declares none and therefore reads the stack. Among several MVARs the caller's
// selection wins whenever the program declares it, matching what the MVAR softmenu and the equation derivative differentiate with respect to; otherwise the first
// declaration, which is the argument by convention and the leftmost key of the MVAR menu.
static calcRegister_t deriv_pgm_variable(calcRegister_t label, bool_t *usesDelta) {
  uint8_t *step;
  char name[MAX_LABEL_NAME_LENGTH + 1];
  calcRegister_t first = INVALID_VARIABLE;
  uint16_t declared;

  if(label < FIRST_LABEL || label > LAST_LABEL || (uint16_t)(label - FIRST_LABEL) >= numberOfLabels) {
    return INVALID_VARIABLE;
  }
  step = labelList[label - FIRST_LABEL].instructionPointer;

  for(declared = 0; declared < MAX_MVAR_DECLARATIONS; declared++) {
    while(checkOpCodeOfStep(step, ITM_REM)) {   // a REM ahead of an MVAR is transparent, as in the MVAR softmenu
      step = findNextStep(step);
    }
    if(!(checkOpCodeOfStep(step, ITM_MVAR) && *(step + 2) == STRING_LABEL_VARIABLE)) {
      break;
    }
    uint8_t nameLength = boundProgramNameLength(step + 4, *(step + 3));
    if(nameLength == 0 || nameLength > MAX_LABEL_NAME_LENGTH) {
      break;
    }
    xcopy(name, step + 4, nameLength);
    name[nameLength] = 0;
    if(usesDelta != NULL && compareString(name, STD_delta STD_SUB_d, CMP_NAME) == 0) {   // the program declares the step variable, so it writes it and it is no step
      *usesDelta = true;
    }
    calcRegister_t variable = findOrAllocateNamedVariable(name);
    if(variable != INVALID_VARIABLE) {
      if((uint16_t)variable == currentSolverVariable) {
        return variable;
      }
      if(first == INVALID_VARIABLE) {
        first = variable;
      }
    }
    step = findNextStep(step);
  }
  return first;
}


static void _differentiatorIteration(calcRegister_t label, calcRegister_t variable, real_t *r0) {
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  realToReal34(r0, REGISTER_REAL34_DATA(REGISTER_X));
  fnFillStack(NOPARAM);

  if(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) {
    reallyRunFunction(ITM_STO, currentSolverVariable);
    parseEquation(currentFormula, EQUATION_PARSER_XEQ, tmpString, tmpString + AIM_BUFFER_LENGTH);
    setRoundingMode(RM_ENGINE);  // a mode the function sets does not reach the engine
  }
  else {
    if(variable != INVALID_VARIABLE) {   // feed both channels: the stack for a program that consumes X, the variable for one that recalls its MVAR
      reallyRunFunction(ITM_STO, variable);
    }
    dynamicMenuItem = -1;
    execProgram(label);
    setRoundingMode(RM_ENGINE);  // a mode the function sets does not reach the engine
    fnToReal(NOPARAM);
  }

  if(lastErrorCode == ERROR_NONE && getRegisterDataType(REGISTER_X) == dtReal34) {
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), r0);
  }
  else {
    // The function is not defined at this point, maybe outside its domain, so the sample is made a NaN and the stencil that reads it is refused, which makes the
    // ladder take its points closer in. The error must be cleared: left standing, the next sample's fnExecute takes it for its own goto having failed, steps the
    // caller back onto this derivative and restarts it, endlessly.
    if(lastErrorCode != ERROR_SOLVER_ABORT) {   // an abort is the one error that stays: calcFuncValues reads it to stop the sampling and the caller to stop the run
      lastErrorCode = ERROR_NONE;
    }
    realSetNaN(r0);
  }
}

// Try to compute a single derivative estimate from a stencil. r takes the weighted sum as it builds, and t is the caller's working space.
static bool_t calcOneDeriv(const FINITE_DIFF_COEFF *stencil, const real_t fxIn[],
                           const real_t *h, real_t *r, real_t *t, realContext_t *realContext) {
  uint16_t i, maxi = 2*stencil->n+1;
  const real_t * const fx = fxIn + MAX_ORDER - stencil->n;

  // Check if all f(x) are defined or not
  for(i=0; i<maxi; i++) {
    if(stencil->coeff[i] != 0 && realIsSpecial(fx + i)) {
      return false;
    }
  }

  // All values are defined where required so calculate the weighted sum
  realSetZero(r);
  for(i=0; i<maxi; i++) {
    if(stencil->coeff[i] != 0) {
      int32ToReal(fdValues[stencil->coeff[i]], t);
      realFMA(fx+i, t, r, r, realContext);
    }
  }
  // Inefficiently factor in the derivative order
  // It's not a problem since the order can only be 1 or 2 currently
  // For larger orders we need to divide the result by h^order
  uInt32ToReal(fdValues[stencil->denom], t);
  for(i=0; i<stencil->order; i++) {
    realMultiply(t, h, t, realContext);
  }
  realDivide(r, t, r, realContext);
  return true;
}

// Compute the function values f(x + k h), k = -MAX_ORDER .. MAX_ORDER
static void calcFuncValues(calcRegister_t label, calcRegister_t variable, const real_t *x, real_t fx[MAX_F_EVAL], real_t *h, realContext_t *realContext) {
  int i;

  for(i=0; i < MAX_F_EVAL; i++) {
    if(lastErrorCode == ERROR_SOLVER_ABORT || programRunStop == PGM_WAITING || exitKeyWaiting()) {
      // calcOneDeriv rejects a stencil only on the samples that stencil reads, so a narrow one would still succeed on the points already taken and hand back a
      // value beside the abort. Poison them all to make the abort the only outcome.
      lastErrorCode = ERROR_SOLVER_ABORT;
      if(programRunStop == PGM_RUNNING) {   // halt the outer program too, as every other abort point does
        programRunStop = PGM_WAITING;
      }
      for(i = 0; i < MAX_F_EVAL; i++) {
        realSetNaN(fx + i);
      }
      return;
    }
    int32ToReal(i - MAX_ORDER, fx + i);   // the multiplier sits in the sample's own slot: FMA may write its result over an operand
    realFMA(fx + i, h, x, fx + i, realContext);
    _differentiatorIteration(label, variable, fx + i);
  }
}


// Evaluate the function at stencil points and compute "best" estimate
static void calcDeriv(calcRegister_t label, const FINITE_DIFF_COEFF *const *finDiff) {
  // One heap block for the working reals: this frame is on the stack under every sample and every nested derivative. Freed at freeWork.
  uint8_t *const work = allocC47Blocks(DERIV_WORK_REALS * REAL_SIZE_IN_BLOCKS(75));
  if(work == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    return;
  }
  const uint8_t userRoundingMode = roundingMode;
  setRoundingMode(RM_ENGINE);  // the iteration takes no mode from RM
  REAL_T_IN(work, 75, 0, x);
  REAL_T_IN(work, 75, 1, h);
  REAL_T_IN(work, 75, 2, scratch);   // the MVAR probe, then working space for calcOneDeriv and deriv_agrees
  REAL_T_IN(work, 75, 3, estimate);
  REAL_T_IN(work, 75, 4, coarse);
  REAL_T_IN(work, 75, 5, gap);
  REAL_T_IN(work, 75, 6, best);
  REAL_T_IN(work, 75, 7, bestGap);
  REAL_T_IN(work, 75, 8, fx);   // MAX_F_EVAL samples
  snap_t savedRegister;
  calcRegister_t variable = INVALID_VARIABLE;
  bool_t userStep = false, usesDelta = false;
  int i, shift, stencil, coarseStencil = -1, coarseShift = 0, bestShift = 0, lastShift = deriv_last_shift();

  if(!getRegisterAsReal(REGISTER_X, x)) {
    goto freeWork;
  }

  if(!realIsSpecial(x)) {
    if(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) {
      usesDelta = deriv_formula_uses_delta();
    }
    else {
      uint8_t probeError = lastErrorCode;   // an MVAR name the variable allocator rejects raises here, before any sampling the caller asked for

      lastErrorCode = ERROR_NONE;
      variable = deriv_pgm_variable(label, &usesDelta);
      if(lastErrorCode != ERROR_NONE) {   // no room for the MVAR: the user is told, rather than given the wrong answer a fall back to the stack would return
        goto freeWork;
      }
      lastErrorCode = probeError;
      if(variable != INVALID_VARIABLE && !getRegisterAsRealQuiet(variable, scratch)) {
        variable = INVALID_VARIABLE;   // differentiate only with respect to something numeric
      }
    }

#if 0
    // This block of code prints out all the function evaluations and
    // all the various derivative estimates.
    {
      char buf[1000];

      for(i=0; i<MAX_F_EVAL; i++) {
        printf("f[x+%dh] = %s\n", i - MAX_ORDER, decNumberToString(fx+i, buf));
      }
      for(i=0; finDiff[i] != NULL; i++) {
        if(calcOneDeriv(finDiff[i], fx, h, x, scratch, &ctxtReal39)) {
          printf("df/dx = %s\t(%s)\n", decNumberToString(x, buf), finDiff[i]->desc);
        }
      }
    }
#endif // 0
    userStep = deriv_user_step(h, usesDelta);

    // Walk the step down a decade at a time. Each step gives one estimate, and two estimates from the same stencil that agree say the coarser step's truncation is
    // already lost in the noise, so the coarser one is taken: it is the one that threw away the fewest digits. A step the user set is taken as it stands, so the
    // first pass is the only one.
    for(shift = DERIV_FIRST_SHIFT; shift <= lastShift; shift++) {
      if(variable != INVALID_VARIABLE) {
        // Kept here rather than in a register: the user program runs between the save and the restore and every temporary register is scratch to something it can
        // call, RCL of a stack register among them. The snapshot carries the type and the tag, so the value comes back as itself and not as the real34 the sampling
        // stored. It is taken again for each step, because the restore hands back the long integer it holds. getRegisterAsRealQuiet has already turned away
        // everything the snapshot does not cover.
        saveRegisterSnapshot(variable, &savedRegister);
      }
      if(!userStep) {
        realCopy(x, h);   // the step is relative to x, and at x = 0 it collapses and the weighted sum would be divided by zero
        if(realIsZero(h)) {
          realCopy(const_1, h);
        }
        h->exponent -= shift;
      }

      // Compute the function at the finite difference points
      saveForUndo();
      calcFuncValues(label, variable, x, fx, h, &ctxtReal39);
      undo();
      if(variable != INVALID_VARIABLE) {   // undo() rolls back the stack only, so the sampled variable is put back here
        restoreRegisterSnapshot(variable, &savedRegister);
      }
      if(lastErrorCode == ERROR_SOLVER_ABORT) {
        break;
      }

      // Try finite differences until we get a result
      stencil = -1;
      for(i=0; finDiff[i] != NULL; i++) {
        if(calcOneDeriv(finDiff[i], fx, h, estimate, scratch, &ctxtReal39)) {
          stencil = i;
          break;
        }
      }
      if(stencil < 0) {   // every stencil rejected this step's samples, so take the points closer in. A step the user set does not move, so there is nothing to retry
        if(userStep) {
          break;
        }
        continue;
      }
      if(userStep) {
        realCopy(estimate, x);
        goto found;
      }
      if(stencil == coarseStencil) {
        if(deriv_agrees(coarse, estimate, shift, finDiff[stencil]->order, gap, scratch)) {
          goto settled;
        }
        // The two are a decade apart, so the gap between them is smallest where the truncation of the coarser one and the cancellation of the finer one balance.
        // The coarser member of the closest pair is therefore the best the ladder saw, and it is what the answer falls back to when no pair ever agrees.
        if(bestShift == 0 || realCompareAbsLessThan(gap, bestGap)) {
          realCopy(coarse, best);
          realCopy(gap, bestGap);
          bestShift = coarseShift;
        }
      }
      realCopy(estimate, coarse);
      coarseStencil = stencil;
      coarseShift = shift;
    }
    if(coarseStencil < 0) {   // no step gave a usable set of samples
      goto noResult;
    }
    if(bestShift != 0) {   // the ladder ran out without a pair ever agreeing, so the closest pair is as near as this function gets
      realCopy(best, coarse);
      coarseShift = bestShift;
    }

settled:                      // the coarser of the two estimates is the answer, and its own step is what the display reports
    realCopy(x, h);
    if(realIsZero(h)) {
      realCopy(const_1, h);
    }
    h->exponent -= coarseShift;
    realCopy(coarse, x);
    goto found;
  }
  goto noResult;

found:
  {
    //Add string, for display at TI
    decContext c = ctxtReal4;
    c.digits = 2;
    realPlus(h, h, &c);   // h is not read again
    strcpy(errorMessage, STD_delta "=");
    decNumberToString(h, errorMessage + stringByteLength(errorMessage));
    strcat(errorMessage, "; ");
    goto finish;
  }

noResult:;
  // No estimate possible
  realSetNaN(x);
  //Add string, for display at TI
  errorMessage[0] = 0;

finish:
  setRoundingMode(userRoundingMode);  // the result is rounded once, by RM
  convertRealToResultRegister(x, REGISTER_X, amNone);

freeWork:
  setRoundingMode(userRoundingMode);
  freeC47Blocks(work, DERIV_WORK_REALS * REAL_SIZE_IN_BLOCKS(75));
}


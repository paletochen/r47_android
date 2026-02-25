// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file differentiate.c
 ***********************************************/

#include "c47.h"
#include "finite_differences.h"


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
#endif

static void calcDeriv(calcRegister_t label, const FINITE_DIFF_COEFF *const *finDiff);

static void calcDerivOfOrder(uint16_t label, int order) {
  calcDeriv(label, finite_difference_table[order]);
}

static void derivativeCommon(uint16_t label, uint16_t order, uint8_t ti) {
  currentSolverStatus &= ~SOLVER_STATUS_USES_FORMULA;
  bool_t solving = getSystemFlag(FLAG_SOLVING);
  char buf[2];

  setSystemFlag(FLAG_SOLVING);
  if(label >= FIRST_LABEL && label <= LAST_LABEL) {
    calcDerivOfOrder(label, order);
    temporaryInformation = ti;
  }
  else if(REGISTER_X <= label && label <= REGISTER_T) {
    // Interactive mode
    buf[0] = letteredRegisterName((calcRegister_t)label);
    buf[1] = 0;
    label = findNamedLabel(buf);
    if(label == INVALID_VARIABLE) {
      displayCalcErrorMessage(ERROR_LABEL_NOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "string '%s' is not a named label", buf);
        moreInfoOnError("In function derivativeCommon:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      calcDerivOfOrder(label, order);
      temporaryInformation = ti;
    }
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "unexpected parameter %u", label);
      moreInfoOnError("In function derivativeCommon:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  if(!solving) {
    clearSystemFlag(FLAG_SOLVING);
  }
}

void fn1stDeriv(uint16_t label) {
  derivativeCommon(label, DERIVATIVE_FIRST_CENTRAL, TI_1ST_DERIVATIVE);
}

void fn2ndDeriv(uint16_t label) {
  derivativeCommon(label, DERIVATIVE_SECOND_CENTRAL, TI_2ND_DERIVATIVE);
}

static void derivativeEquation(uint16_t order, uint8_t ti) {
    #if !defined(TESTSUITE_BUILD)
    //new method to maintain solver variable
      reallyRunFunction(ITM_RCL, currentSolverVariable);
      copySourceRegisterToDestRegister(REGISTER_X, TEMP_REGISTER_1);
    #endif // TESTSUITE_BUILD
  currentSolverStatus |= SOLVER_STATUS_USES_FORMULA;
  calcDerivOfOrder(INVALID_VARIABLE, order);
    #if !defined(TESTSUITE_BUILD)
      reallyRunFunction(ITM_RCL, TEMP_REGISTER_1);
      reallyRunFunction(ITM_STO, currentSolverVariable);
      fnDrop(NOPARAM);
    #endif // TESTSUITE_BUILD
  temporaryInformation = ti;
}

void fn1stDerivEq(uint16_t unusedButMandatoryParameter) {
  derivativeEquation(DERIVATIVE_FIRST_CENTRAL, TI_1ST_DERIVATIVE);
}

void fn2ndDerivEq(uint16_t unusedButMandatoryParameter) {
  derivativeEquation(DERIVATIVE_SECOND_CENTRAL, TI_2ND_DERIVATIVE);
}

/* The following routines are ported from WP34s. */

static void deriv_found_lbl(calcRegister_t deltaX, real_t *h) {
  execProgram(deltaX);
  fnToReal(NOPARAM);
  if(getRegisterDataType(REGISTER_X) == dtReal34) {
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), h);
  }
  else {
    lastErrorCode = ERROR_NONE;
    realCopy(const_1on10, h);
  }
}

static void deriv_default_h(real_t *h) {
  calcRegister_t deltaX;
  unsigned int i;
  TO_QSPI static const char *const lbls[] = {
    STD_delta "x",  STD_delta "X",
    STD_DELTA "x",  STD_DELTA "X",
  };

  saveForUndo();
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  realToReal34(h, REGISTER_REAL34_DATA(REGISTER_X));
  fnFillStack(NOPARAM);

  dynamicMenuItem = -1;
  for (i=0; i<nbrOfElements(lbls); i++)
    if ((deltaX = findNamedLabel(lbls[i])) != INVALID_VARIABLE) {
      deriv_found_lbl(deltaX, h);
      undo();
      return;
  }
  undo();
  realMultiply(h, const_1e_16, h, &ctxtReal39);
}


static void _differentiatorIteration(calcRegister_t label, real_t *r0) {
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  realToReal34(r0, REGISTER_REAL34_DATA(REGISTER_X));
  fnFillStack(NOPARAM);

  if(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) {
    #if !defined(TESTSUITE_BUILD)
      reallyRunFunction(ITM_STO, currentSolverVariable);
      parseEquation(currentFormula, EQUATION_PARSER_XEQ, tmpString, tmpString + AIM_BUFFER_LENGTH);
    #endif // TESTSUITE_BUILD
  }
  else {
    dynamicMenuItem = -1;
    execProgram(label);
    fnToReal(NOPARAM);
  }

  if(getRegisterDataType(REGISTER_X) == dtReal34) {
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), r0);
  }
  else {
    lastErrorCode = ERROR_NONE;
    realCopy(const_NaN, r0);
  }
}

// Try to compute a single derivative estimate from a stencil
static bool_t calcOneDeriv(const FINITE_DIFF_COEFF *stencil, const real_t fxIn[],
                           const real_t *h, real_t *r, realContext_t *realContext) {
  uint16_t i, maxi = 2*stencil->n+1;
  real_t t, s;
  const real_t *const fx = fxIn + MAX_ORDER - stencil->n;

  // Check if all f(x) are defined or not
  for (i=0; i<maxi; i++)
    if (stencil->coeff[i] != 0 && realIsSpecial(fx + i))
      return false;

  // All values are defined where required so calculate the weighted sum
  realZero(&s);
  for (i=0; i<maxi; i++)
    if (stencil->coeff[i] != 0) {
      int32ToReal(fdValues[stencil->coeff[i]], &t);
      realFMA(fx+i, &t, &s, &s, realContext);
    }
  // Inefficiently factor in the derivative order
  // It's not a problem since the order can only be 1 or 2 currently
  // For larger orders we need to divide the result by h^order
  uInt32ToReal(fdValues[stencil->denom], &t);
  for (i=0; i<stencil->order; i++)
    realMultiply(&t, h, &t, realContext);
  realDivide(&s, &t, r, realContext);
  return true;
}

// Compute the function values f(x + k h), k = -MAX_ORDER .. MAX_ORDER
static void calcFuncValues(calcRegister_t label, const real_t *x, real_t fx[MAX_F_EVAL],
                           real_t *h, realContext_t *realContext) {
    int i;
    real_t t;

    for (i=0; i < MAX_F_EVAL; i++) {
        int32ToReal(i - MAX_ORDER, &t);
        realFMA(&t, h, x, fx + i, realContext);
        _differentiatorIteration(label, fx + i);
    }
}


// Evaluate the function at stencil points and compute "best" estimate
static void calcDeriv(calcRegister_t label, const FINITE_DIFF_COEFF *const *finDiff) {
  real_t x, h, fx[MAX_F_EVAL];
  int i;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  if (!realIsSpecial(&x)) {
    realCopy(&x, &h);   // Pass X into the h determination code to allow relative steps
    deriv_default_h(&h);

    // Compute the function at the finite difference points
    saveForUndo();
    calcFuncValues(label, &x, fx, &h, &ctxtReal39);
    undo();

#if 0
    // This block of code prints out all the function evaluations and
    // all the various derivative estimates.
    {
        char buf[1000];

        for (i=0; i<MAX_F_EVAL; i++)
          printf("f[x+%dh] = %s\n", i - MAX_ORDER, decNumberToString(fx+i, buf));
        for (i=0; finDiff[i] != NULL; i++)
          if (calcOneDeriv(finDiff[i], fx, &h, &x, &ctxtReal39))
            printf("df/dx = %s\t(%s)\n", decNumberToString(&x, buf), finDiff[i]->desc);
    }
#endif
    // Try finite differences until we get a result
    for (i=0; finDiff[i] != NULL; i++)
      if (calcOneDeriv(finDiff[i], fx, &h, &x, &ctxtReal39)) {
        //Add string, for display at TI
        decContext c = ctxtReal4;
        c.digits = 2;
        real_t hh;
        realPlus(&h, &hh, &c);
        strcpy(errorMessage, STD_delta "=");
        decNumberToString(&hh, errorMessage + stringByteLength(errorMessage));
        strcat(errorMessage, "; ");
        goto finish;
      }
  }
  // No estimate possible
  realCopy(const_NaN, &x);
  //Add string, for display at TI
  errorMessage[0] = 0;

finish:
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}


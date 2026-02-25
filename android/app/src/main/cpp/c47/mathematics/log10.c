// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"


void realLog10(const real_t *x, real_t *res, realContext_t *realContext) {
  WP34S_Ln(x, res, realContext);
  realDivide(res, const_ln10, res, realContext);
}


void logxyReal(const real_t *denom) {
  real_t a, b;

  if (!getRegisterAsReal(REGISTER_X, &a))
    return;

  if(realIsZero(&a)) {
    if(getSystemFlag(FLAG_SPCRES))
      realCopy(const_minusInfinity, &a);
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function logxyReal:", "cannot calculate log2(0)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

  else if(realIsInfinite(&a)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function logxyReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of log2 when flag D is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    else if(getFlag(FLAG_CPXRES)) {
      if(real34IsPositive(REGISTER_REAL34_DATA(REGISTER_X)))
        realCopy(const_plusInfinity, &a);
      else {
        reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
        realDivide(const_pi, denom, &a, &ctxtReal39);
        convertComplexToResultRegister(const_plusInfinity, &a, REGISTER_X);
        return;
      }
    }
    else {
      realCopy(const_NaN, &a);
    }
  }

  else {
    if(realIsPositive(&a)) {
      WP34S_Ln(&a, &a, &ctxtReal39);
      realDivide(&a, denom, &a, &ctxtReal39);
    }
    else if(getFlag(FLAG_CPXRES)) {
      realSetPositiveSign(&a);
      WP34S_Ln(&a, &a, &ctxtReal39);
      realDivide(&a, denom, &a, &ctxtReal39);
      realDivide(const_pi, denom, &b, &ctxtReal39);
      convertComplexToResultRegister(&a, &b, REGISTER_X);
      return;
    }
    else if(getSystemFlag(FLAG_SPCRES)) {
      realCopy(const_NaN, &a);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function logxyReal:", "cannot calculate log2 of a negative number when CPXRES is not set!", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  convertRealToResultRegister(&a, REGISTER_X, amNone);
}



void logxyCplx(const real_t *denom) {
  real_t a, b;

  if (!getRegisterAsComplex(REGISTER_X, &a, &b))
      return;

  if(realIsZero(&a) && realIsZero(&b)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy(const_minusInfinity, &a);
      realZero(&b);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function logxyCplx:", "cannot calculate log2(0)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    realRectangularToPolar(&a, &b, &a, &b, &ctxtReal39);
    WP34S_Ln(&a, &a, &ctxtReal39);
    realDivide(&a, denom, &a, &ctxtReal39);
    realDivide(&b, denom, &b, &ctxtReal39);
  }
  convertComplexToResultRegister(&a, &b, REGISTER_X);
}


void logxyLonI(const real_t *denom) {
  real_t x;

  if (!getRegisterAsReal(REGISTER_X, &x))
    return;
  if (realIsNegative(&x) || realIsZero(&x)) {
    logxyReal(denom);
    return;
  }

  WP34S_Ln(&x, &x, &ctxtReal39);
  realDivide(&x, denom, &x, &ctxtReal34);   /* Round using the 34 digit context */
  if (!realIsAnInteger(&x))
    convertRealToResultRegister(&x, REGISTER_X, amNone);
  else
    convertRealToLongIntegerRegister(&x, REGISTER_X, DEC_ROUND_HALF_EVEN);
}


/**********************************************************************
 * In all the functions below:
 * if X is a number then X = a + ib
 * The variables a and b are used for intermediate calculations
 ***********************************************************************/

static void log10LonI(void) {
  logxyLonI(const_ln10);
}

static void log10ShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intLog10(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}

static void log10Real(void) {
  logxyReal(const_ln10);
}

static void log10Cplx(void) {
  logxyCplx(const_ln10);
}


/********************************************//**
 * \brief regX ==> regL and log10(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLog10(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&log10Real, &log10Cplx, &log10ShoI, &log10LonI);
}

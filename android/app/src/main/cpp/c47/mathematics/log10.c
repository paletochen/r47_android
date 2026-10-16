// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"


void realLog10(const real_t *x, real_t *res, realContext_t *realContext) {
  WP34S_Ln(x, res, realContext);
  realDivide(res, const39_ln10, res, realContext);
}


void logxyReal(const real_t *denom) {
  real_t a, b;

  if(!getRegisterAsReal(REGISTER_X, &a)) {
    return;
  }

  if(realIsZero(&a)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetMinusInfinity(&a);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function logxyReal:", "cannot calculate log2(0)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

  else if(realIsInfinite(&a)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function logxyReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of log2 when flag SPCRES is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    else if(realIsPositive(&a)) {
      realSetPlusInfinity(&a);
    }
    else if(getFlag(FLAG_CPXRES)) {
      reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
      realDivide(const39_pi, denom, &a, &ctxtReal39);
      convertComplexPartsToResultRegister(const_plusInfinity, &a, REGISTER_X);
      return;
    }
    else {
      realSetNaN(&a);
    }
  }

  else {
    if(realIsPositive(&a)) {
      real_t k, p;
      realCopy(&a, &b);
      WP34S_Ln(&a, &a, &ctxtReal39);
      realDivide(&a, denom, &a, &ctxtReal39);
      realRescale(&a, &k, const_0, &ctxtReal39);
      const int32_t n = realToInt32C47(&k, NULL);
      if(denom != const39_ln2 || (n >= -100 && n <= 112)) {   // 10ᵏ, and 2ᵏ in that range, are exact at 75 digits; the range takes every power of 2 of 34 digits
        (denom == const39_ln2 ? realPower2 : realPower10)(&k, &p, &ctxtReal75);
        if(realCompareEqual(&p, &b)) {                         // X an exact power of the base gives the integer
          realCopy(&k, &a);
        }
      }
    }
    else if(getFlag(FLAG_CPXRES)) {
      realSetPositiveSign(&a);
      WP34S_Ln(&a, &a, &ctxtReal39);
      realDivide(&a, denom, &a, &ctxtReal39);
      realDivide(const39_pi, denom, &b, &ctxtReal39);
      convertComplexPartsToResultRegister(&a, &b, REGISTER_X);
      return;
    }
    else if(getSystemFlag(FLAG_SPCRES)) {
      realSetNaN(&a);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function logxyReal:", "cannot calculate log2 of a negative number when CPXRES is not set!", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  convertRealToResultRegister(&a, REGISTER_X, amNone);
}



void logxyCplx(const real_t *denom) {
  cplx_t x;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  if(realIsZero(&x.Real) && realIsZero(&x.Imag)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetMinusInfinity(&x.Real);
      realSetZero(&x.Imag);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function logxyCplx:", "cannot calculate log2(0)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    realRectangularToPolar(&x, &x.Mag, &x.Arg, &ctxtReal39);
    WP34S_Ln(&x.Mag, &x.Real, &ctxtReal39);
    realDivide(&x.Real, denom, &x.Real, &ctxtReal39);
    realDivide(&x.Imag, denom, &x.Imag, &ctxtReal39);
  }
  convertComplexToResultRegister(&x, REGISTER_X);
}


void logxyLonI(const real_t *denom) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }
  if(realIsNegative(&x) || realIsZero(&x)) {
    logxyReal(denom);
    return;
  }

  real_t x34;
  realContext_t c = ctxtReal34;
  c.round = DEC_ROUND_HALF_EVEN;            // the test for an exact integer takes no mode from RM
  WP34S_Ln(&x, &x, &ctxtReal39);
  realDivide(&x, denom, &x, &ctxtReal39);
  realPlus(&x, &x34, &c);                   /* Round using the 34 digit context */
  if(!realIsAnInteger(&x34)) {
    convertRealToResultRegister(&x, REGISTER_X, amNone);
  }
  else {
    longInteger_t lgInt, power;
    int32_t k = realToInt32C47(&x34, NULL);

    getRegisterAsLongInt(REGISTER_X, lgInt, NULL);
    longIntegerInit(power);
    longIntegerPowerUIntUInt(denom == const39_ln2 ? 2 : 10, k, power);
    if(longIntegerCompare(power, lgInt) == 0) {   // an integer result only where X is the base to that power
      convertRealToLongIntegerRegister(&x34, REGISTER_X, DEC_ROUND_HALF_EVEN);
    }
    else {
      convertRealToResultRegister(&x, REGISTER_X, amNone);
    }
    longIntegerFree(power);
    longIntegerFree(lgInt);
  }
}


/**********************************************************************
 * In all the functions below:
 * if X is a number then X = a + ib
 * The variables a and b are used for intermediate calculations
 ***********************************************************************/

static void log10LonI(void) {
  logxyLonI(const39_ln10);
}

static void log10ShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intLog10(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}

static void log10Real(void) {
  logxyReal(const39_ln10);
}

// log10 and log2 of a complex differ only in the constant they divide by. logxyCplx above has no near unit branch, so it is a different routine and is
// left alone.
void logBaseCplx(const real_t *denom) {
  cplx_t x;
  cplx_t result;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  if(realIsZero(&x.Real) && realIsZero(&x.Imag)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetMinusInfinity(&result.Real);
      realSetZero(&result.Imag);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function logBaseCplx:", "cannot calculate the log of 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    realRectangularToPolar(&x, &result.Mag, &result.Arg, &ctxtReal39);
    if(realCompareGreaterThan(&result.Real, const_1on2) && realCompareLessThan(&result.Real, const_2)) {
      realContext_t nearUnitContext = ctxtReal75;
      // Factoring the component with greater magnitude keeps its offset from one in working precision, while the other component contributes its square directly.
      if(realCompareAbsLessThan(&x.Real, &x.Imag)) {
        realSubtract(&x.Imag, const_1, &result.Real, &nearUnitContext);
        realAdd(&x.Imag, const_1, &x.Imag, &nearUnitContext);
        realMultiply(&result.Real, &x.Imag, &result.Real, &nearUnitContext);
        realFMA(&x.Real, &x.Real, &result.Real, &result.Real, &nearUnitContext);
      }
      else {
        realSubtract(&x.Real, const_1, &result.Real, &nearUnitContext);
        realAdd(&x.Real, const_1, &x.Real, &nearUnitContext);
        realMultiply(&result.Real, &x.Real, &result.Real, &nearUnitContext);
        realFMA(&x.Imag, &x.Imag, &result.Real, &result.Real, &nearUnitContext);
      }
      WP34S_Ln1P(&result.Real, &result.Real, &nearUnitContext);
      realMultiply(&result.Real, const_1on2, &result.Real, &nearUnitContext);
      realDivide(&result.Real, denom, &result.Real, &nearUnitContext);
    }
    else {
      WP34S_Ln(&result.Real, &result.Real, &ctxtReal39);
      realDivide(&result.Real, denom, &result.Real, &ctxtReal39);
    }
    realDivide(&result.Imag, denom, &result.Imag, &ctxtReal39);
  }
  convertComplexToResultRegister(&result, REGISTER_X);
}


static void log10Cplx(void) {
  logBaseCplx(const39_ln10);
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

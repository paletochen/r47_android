// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arctanh.c
 ***********************************************/

#include "c47.h"

static void arctanhCplx(void) {
  real_t aAbs, denominator, numerator, temp;
  cplx_t x;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  if(realIsNaN(&x.Real) || realIsNaN(&x.Imag)) {
    realSetNaN(&numerator);
    realSetNaN(&denominator);
    convertComplexPartsToResultRegister(&numerator, &denominator, REGISTER_X);
    return;
  }

  if(realIsInfinite(&x.Real) || realIsInfinite(&x.Imag)) {
    realSetZero(&numerator);
    if(realIsNegative(&x.Real)) {
      realSetNegativeSign(&numerator);
    }
    realCopy(const39_piOn2, &denominator);
    if(realIsNegative(&x.Imag)) {
      realSetNegativeSign(&denominator);
    }
    convertComplexPartsToResultRegister(&numerator, &denominator, REGISTER_X);
    return;
  }

  if(realIsZero(&x.Imag) && realCompareAbsEqual(&x.Real, const_1)) {
    realCopy(realIsNegative(&x.Real) ? const_minusInfinity : const_plusInfinity, &numerator);
    realCopy(&x.Imag, &denominator);
    convertComplexPartsToResultRegister(&numerator, &denominator, REGISTER_X);
    return;
  }

  // Re(atanh(a+bi)) = sign(a)/4 * ln(1 + 4|a|/((1-|a|)²+b²)).
  realCopyAbs(&x.Real, &aAbs);
  if(realIsZero(&x.Real)) {
    realCopy(&x.Real, &numerator);
  } else {
    realSubtract(const_1, &aAbs, &denominator, &ctxtReal39);
    realMultiply(&denominator, &denominator, &denominator, &ctxtReal39);
    realFMA(&x.Imag, &x.Imag, &denominator, &denominator, &ctxtReal39);
    realMultiply(&aAbs, const_4, &numerator, &ctxtReal39);
    realDivide(&numerator, &denominator, &numerator, &ctxtReal39);
    WP34S_Ln1P(&numerator, &numerator, &ctxtReal39);
    realMultiply(&numerator, const_1on2, &numerator, &ctxtReal39);
    realMultiply(&numerator, const_1on2, &numerator, &ctxtReal39);
    if(realIsNegative(&x.Real)) {
      realSetNegativeSign(&numerator);
    }
  }

  // Im(atanh(a+bi)) = atan2(2b, (1-a)(1+a)-b²)/2.
  realSubtract(const_1, &x.Real, &denominator, &ctxtReal39);
  realAdd(const_1, &x.Real, &temp, &ctxtReal39);
  realMultiply(&denominator, &temp, &denominator, &ctxtReal39);
  realMinus(&x.Imag, &temp, &ctxtReal39);
  realFMA(&temp, &x.Imag, &denominator, &denominator, &ctxtReal39);
  realMultiply(&x.Imag, const_2, &temp, &ctxtReal39);
  C47_WP34S_Atan2(&temp, &denominator, &denominator, &ctxtReal39);
  realMultiply(&denominator, const_1on2, &denominator, &ctxtReal39);

  convertComplexPartsToResultRegister(&numerator, &denominator, REGISTER_X);
}


static void arctanhReal(void) {
  real_t x;
  const real_t *r = &x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsZero(&x)) {
    WP34S_ArcTanh(&x, &x, &ctxtReal39);
    r = &x;
  }
  else {
    if(realCompareEqual(&x, const_1)) {
      if(getSystemFlag(FLAG_SPCRES)) {
        r = const_plusInfinity;
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function arctanhReal:", "X = 1", "and DANGER flag is not set!", NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
       }
    }
    else if(realCompareEqual(&x, const__1)) {
      if(getSystemFlag(FLAG_SPCRES)) {
        r = const_minusInfinity;
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function arctanhReal:", "X = -1", "and DANGER flag is not set!", NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
       }
    }
    else {
      if(realCompareAbsGreaterThan(&x, const_1)) {
        if(getFlag(FLAG_CPXRES)) {
          arctanhCplx();
          return;
        }
        else if(getSystemFlag(FLAG_SPCRES)) {
          r = const_NaN;
        }
        else {
          displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function arctanhReal:", "|X| > 1", "and CPXRES is not set!", NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return;
        }
      }
      else if(realGetExponent(&x) < -18) {   // artanh x = x + x³/3 to 72 digits, above |x| by less than half a unit at 34 digits
        WP34S_TinyOddSeries(&x, const_3, true, &x, &ctxtReal75);
      }
      else {
        WP34S_ArcTanh(&x, &x, &ctxtReal39);
      }
    }
  }
  convertRealToResultRegister(r, REGISTER_X, amNone);
}


/********************************************//**
 * \brief regX ==> regL and arctanh(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnArctanh(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&arctanhReal, &arctanhCplx);
}

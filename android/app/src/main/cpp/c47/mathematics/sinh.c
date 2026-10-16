// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sinh.c
 ***********************************************/

#include "c47.h"

static void multiplyHyperbolicByTrig(const real_t *hyperbolic, const real_t *trigonometric, real_t *res) {
  if(!realIsInfinite(hyperbolic)) {
    realMultiply(hyperbolic, trigonometric, res, &ctxtReal39);
  }
  else if(realIsNaN(trigonometric)) {
    realSetNaN(res);
  }
  else if(realIsZero(trigonometric)) {
    realCopy(trigonometric, res);
    if(realIsNegative(hyperbolic)) {
      realChangeSign(res);
    }
  }
  else {
    realSetPlusInfinity(res);
    if(realIsNegative(hyperbolic) != realIsNegative(trigonometric)) {
      realSetNegativeSign(res);
    }
  }
}

void sinhCoshReal(trigType_t trigType) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function sinhCoshReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of sinh when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(trigType == trigSin && !realIsZero(&x) && !realIsSpecial(&x) && realGetExponent(&x) < -18) {   // sinh x = x + x³/6 to 72 digits, above |x| by less than half a unit at 34 digits
    WP34S_TinyOddSeries(&x, const_6, true, &x, &ctxtReal75);
  }
  else if(trigType == trigSin) {
    WP34S_SinhCosh(&x, &x,   NULL, &ctxtReal39);
  }
  else {
    WP34S_SinhCosh(&x, NULL, &x,   &ctxtReal39);
  }

  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



void sinhCoshCplx(trigType_t trigType) {
  real_t sinha, cosha, sinb, cosb;
  cplx_t x;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  if(realIsInfinite(&x.Real) && realIsSpecial(&x.Imag)) {
    const bool_t negative = realIsNegative(&x.Real);

    realSetPlusInfinity(&x.Real);
    if(trigType == trigSin && negative) {
      realSetNegativeSign(&x.Real);
    }
    realSetNaN(&x.Imag);
  }
  else if(realIsZero(&x.Real) && realIsSpecial(&x.Imag)) {
    if(trigType == trigSin) {
      realSetNaN(&x.Imag);
    }
    else {
      realCopy(&x.Real, &x.Imag);
      realSetNaN(&x.Real);
    }
  }
  else if(!(realIsNaN(&x.Real) && realIsZero(&x.Imag))) {
    WP34S_SinhCosh(&x.Real, &sinha, &cosha, &ctxtReal39);
    C47_WP34S_Cvt2RadSinCosTan(&x.Imag, amRadian, &sinb, &cosb, NULL, &ctxtReal39);

    if(trigType == trigSin) {
      // sinh(a + i b) = sinh(a) cos(b) + i cosh(a) sin(b)
      multiplyHyperbolicByTrig(&sinha, &cosb, &x.Real);
      multiplyHyperbolicByTrig(&cosha, &sinb, &x.Imag);
    }
    else {
      // cosh(a + i b) = cosh(a) cos(b) + i sinh(a) sin(b)
      multiplyHyperbolicByTrig(&cosha, &cosb, &x.Real);
      multiplyHyperbolicByTrig(&sinha, &sinb, &x.Imag);
    }
  }

  convertComplexToResultRegister(&x, REGISTER_X);
}



static void sinhReal(void) {
  sinhCoshReal(trigSin);
}



static void sinhCplx(void) {
  sinhCoshCplx(trigSin);
}



/********************************************//**
 * \brief regX ==> regL and sinh(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSinh(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&sinhReal, &sinhCplx);
}

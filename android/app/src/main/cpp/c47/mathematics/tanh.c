// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file tanh.c
 ***********************************************/

#include "c47.h"

static void tanhReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function tanhReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of tanh when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(!realIsZero(&x) && !realIsSpecial(&x) && realGetExponent(&x) < -18) {   // tanh x = x - x³/3 to 72 digits, below |x| by less than half a unit at 34 digits
    WP34S_TinyOddSeries(&x, const_3, false, &x, &ctxtReal75);
  }
  else {
    WP34S_Tanh(&x, &x, &ctxtReal39);
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static void tanhCplx(void) {
  cplx_t x;
  cplx_t r;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  TanhComplex(&x, &r, &ctxtReal39);

  convertComplexToResultRegister(&r, REGISTER_X);
}


uint8_t TanhComplex(const cplx_t *x, cplx_t *r, realContext_t *realContext) {
  cplx_t iz, tanIz;

  if(realIsNaN(&x->Real) && realIsZero(&x->Imag)) {
    realCopy(&x->Imag, &r->Imag);
    realSetNaN(&r->Real);
    return ERROR_NONE;
  }

  if(realIsInfinite(&x->Real) && realIsSpecial(&x->Imag)) {
    const bool_t negative = realIsNegative(&x->Real);

    realCopy(const_1, &r->Real);
    if(negative) {
      realSetNegativeSign(&r->Real);
    }
    realSetZero(&r->Imag);
    return ERROR_NONE;
  }

  // tanh(z) = -i tan(iz), with iz = -Im(z) + i Re(z).
  realCopy(&x->Imag, &iz.Real);
  realChangeSign(&iz.Real);
  realCopy(&x->Real, &iz.Imag);
  TanComplex(&iz, &tanIz, realContext);

  realCopy(&tanIz.Imag, &r->Real);
  realCopy(&tanIz.Real, &r->Imag);
  realChangeSign(&r->Imag);
  // TanComplex returns +0 as the real part of tan(-0 + iy), where tanh(x +- i0) is tanh(x) +- i0, so a zero Im(z) gives the sign of Im(tanh(z)) directly.
  if(realIsZero(&iz.Real)) {
    realCopy(&iz.Real, &r->Imag);
    realChangeSign(&r->Imag);
  }

  return ERROR_NONE;
}



/********************************************//**
 * \brief regX ==> regL and tanh(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnTanh(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&tanhReal, &tanhCplx);
}

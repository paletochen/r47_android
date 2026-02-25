// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file expMOne.c
 ***********************************************/
// Coded by JM, based on exp.c

#include "c47.h"

void expM1Complex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  real_t z2_real, z2_imag, e_real, e_imag;

  if(realIsZero(imag)) {
    if(realIsInfinite(real) && realIsNegative(real)) {
      realCopy(const__1, resReal);
      realZero(resImag);
      return;
    }
    realExpM1(real, resReal, realContext);
    realZero(resImag);
    return;
  }

  if(realIsSpecial(real) || realIsSpecial(imag)) {
    realCopy(const_NaN, resReal);
    realCopy(const_NaN, resImag);
    return;
  }

  /* Complex (e^z)-1.
   *
   * With a bit of algebra it can be shown that:
   *  e^z - 1 = -exp(z/2) * 2*i*sin(i*z/2)
   * which has no obvious accuracy issues inherent here.
   *
   * The negation and multiplication by i are simplified to negates and swaps.
   */
  realMultiply(real, const_1on2, &z2_real, realContext);
  realMultiply(imag, const_1on2, &z2_imag, realContext);
  expComplex(&z2_real, &z2_imag, &e_real, &e_imag, realContext);
  realChangeSign(&e_real);
  realAdd(&e_real, &e_real, &e_real, realContext);
  realAdd(&e_imag, &e_imag, &e_imag, realContext);
  /* sin(i * z/2) */
  realChangeSign(&z2_imag);
  sinComplex(&z2_imag, &z2_real, &z2_real, &z2_imag, realContext);
  mulComplexComplex(&z2_real, &z2_imag, &e_imag, &e_real, resReal, resImag, realContext);
}

void realExpM1(const real_t *x, real_t *res, realContext_t *realContext)
{
  WP34S_ExpM1(x, res, realContext);
}

static void expM1Real(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function expM1Real:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of exp when flag D is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  realExpM1(&x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static void expM1Cplx(void) {
  real_t zReal, zImag;

  if(!getRegisterAsComplex(REGISTER_X, &zReal, &zImag))
    return;

  expM1Complex(&zReal, &zImag, &zReal, &zImag, &ctxtReal75);
  convertComplexToResultRegister(&zReal, &zImag, REGISTER_X);
}


/********************************************//**
 * \brief regX ==> regL and expM1(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnExpM1(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&expM1Real, &expM1Cplx);
}




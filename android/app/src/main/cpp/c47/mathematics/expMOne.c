// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file expMOne.c
 ***********************************************/
// Coded by JM, based on exp.c

#include "c47.h"

void expM1Complex(const cplx_t *z, cplx_t *res, realContext_t *realContext) {
  cplx_t z2;
  cplx_t e;

  if(realIsZero(&z->Imag)) {
    if(realIsInfinite(&z->Real) && realIsNegative(&z->Real)) {
      realSetNegativeOne(&res->Real);
      realSetZero(&res->Imag);
      return;
    }
    realExpM1(&z->Real, &res->Real, realContext);
    realSetZero(&res->Imag);
    return;
  }

  if(realIsNaN(&z->Real) || realIsSpecial(&z->Imag)) {
    realSetNaN(&res->Real);
    realSetNaN(&res->Imag);
    return;
  }

  /* The split identity below has factors that approach zero and infinity for
   * a large negative real part. Direct subtraction is well-conditioned for
   * real <= -0.1 and avoids multiplying those factors.
   */
  if(realIsInfinite(&z->Real) || (realIsNegative(&z->Real) && !realCompareAbsLessThan(&z->Real, const_1on10))) {
    expComplex(z, res, realContext);
    realSubtract(&res->Real, const_1, &res->Real, realContext);
    return;
  }

  /* Complex (e^z)-1.
   *
   * With a bit of algebra it can be shown that:
   *  e^z - 1 = -exp(z/2) * 2*i*sin(i*z/2)
   * which avoids direct subtraction near the zeros of e^z - 1.
   *
   * The negation and multiplication by i are simplified to negates and swaps.
   */
  realMultiply(&z->Real, const_1on2, &z2.Real, realContext);
  realMultiply(&z->Imag, const_1on2, &z2.Imag, realContext);
  expComplex(&z2, &e, realContext);
  chsComplex(&e);
  mulComplexi(&e, &e);
  addComplex(&e, &e, &e, realContext);
  /* sin(i * z/2) */
  mulComplexi(&z2, &z2);
  sinComplex(&z2, &z2, realContext);
  mulComplexComplex(&z2, &e, res, realContext);
}

void realExpM1(const real_t *x, real_t *res, realContext_t *realContext)
{
  WP34S_ExpM1(x, res, realContext);
}

static void expM1Real(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function expM1Real:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of exp when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  realExpM1(&x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static void expM1Cplx(void) {
  cplx_t z;
  realContext_t *realContext = &ctxtReal75;

  if(!getRegisterAsComplex(REGISTER_X, &z)) {
    return;
  }

  if(realIsInfinite(&z.Real) || (realIsNegative(&z.Real) && !realCompareAbsLessThan(&z.Real, const_1on10))) {
    realContext = &ctxtReal39;
  }
  expM1Complex(&z, &z, realContext);
  convertComplexToResultRegister(&z, REGISTER_X);
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

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sincpi.c
 ***********************************************/
// Coded by JM, based on sin.c

#include "c47.h"

void sincpiComplex(const cplx_t *z, cplx_t *res, realContext_t *realContext) {
  // sin(a + ib) = sin(a)*cosh(b) + i*cos(a)*sinh(b)
  // sinc(a + ib) = sin(a + ib) / (a + ib), for the allowable conditions
  cplx_t x;
  const real_t *pi = realContext->digits > 39 ? const75_pi : const39_pi;

  realCopy(&z->Real, &x.Real);
  realCopy(&z->Imag, &x.Imag);

  if(realIsZero(&x.Real) && realIsZero(&x.Imag)) {
    realSetOne(&res->Real);
    realSetZero(&res->Imag);
  }
  else if(!realIsSpecial(&x.Real) && realIsAnInteger(&x.Real) && realIsZero(&x.Imag)) {
    realSetZero(&res->Real);
    realSetZero(&res->Imag);
  }
  else {
    realMultiply(&x.Real, pi, &x.Real, realContext);
    realMultiply(&x.Imag, pi, &x.Imag, realContext);
    sincComplex(&x, res, realContext);
  }
}


static void sincpiReal(void) {
  real_t x, sine;
  const real_t *r = &x;
  angularMode_t xAngularMode;
  const uint32_t type = getRegisterDataType(REGISTER_X);

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsInfinite(&x)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      r = const_0;
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function sincpiReal:", "cannot divide a real34 by " STD_PLUS_MINUS STD_INFINITY " when flag SPCRES is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    if(realIsZero(&x)) {
      r = const_1;
    }
    else if(type != dtReal34) {
      r = const_0;
    }
    else {
      xAngularMode = getRegisterAngularMode(REGISTER_X);
      if(xAngularMode != amNone) {
        convertAngleFromTo(&x, xAngularMode, amRadian, &ctxtReal75);
      }
      if(realIsAnInteger(&x)) {
        r = const_0;
      }
      else {
        realMultiply(&x, const75_pi, &x, &ctxtReal75);   //This pi is to convert sincpi to sinc for all input, regardless
        C47_WP34S_Cvt2RadSinCosTan(&x, amRadian, &sine, NULL, NULL, &ctxtReal75);
        realDivide(&sine, &x, &x, &ctxtReal75);
      }
    }
  }
  convertRealToResultRegister(r, REGISTER_X, amNone);
}


static void sincpiCplx(void) {
  cplx_t z;

  if(!getRegisterAsComplex(REGISTER_X, &z)) {
    return;
  }

  sincpiComplex(&z, &z, &ctxtReal75);

  convertComplexToResultRegister(&z, REGISTER_X);
}


/********************************************//**
 * \brief regX ==> regL and sincpi(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSincpi(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&sincpiReal, &sincpiCplx);

}

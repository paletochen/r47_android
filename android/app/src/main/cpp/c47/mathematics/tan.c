// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file tan.c
 ***********************************************/

#include "c47.h"

static void tanReal(void) {
  real_t sin, cos, tan;
  const real_t *r = &tan;
  angularMode_t xAngularMode;

  if(!getRegisterAsRealAngle(REGISTER_X, &tan, &xAngularMode))
    return;

  if(realIsSpecial(&tan))
    r = const_NaN;
  else {
    WP34S_Cvt2RadSinCosTan(&tan, xAngularMode, &sin, &cos, &tan, &ctxtReal75);
    if(realIsZero(&sin)) {
       realSetPositiveSign(&tan);
    }

    if(realIsZero(&cos) && !getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function tanReal:", "X = " STD_PLUS_MINUS "90" STD_DEGREE, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    else {
      if(realIsZero(&cos))
        r = const_NaN;
    }
  }
  convertRealToResultRegister(r, REGISTER_X, amNone);
}



static void tanCplx(void) {
  //                sin(a)*cosh(b) + i*cos(a)*sinh(b)
  // tan(a + ib) = -----------------------------------
  //                cos(a)*cosh(b) - i*sin(a)*sinh(b)

  real_t xReal, xImag;

  if(!getRegisterAsComplex(REGISTER_X, &xReal, &xImag))
    return;

  TanComplex(&xReal, &xImag, &xReal, &xImag, &ctxtReal51);

  convertComplexToResultRegister(&xReal, &xImag, REGISTER_X);
}

uint8_t TanComplex(const real_t *xReal, const real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  real_t sina, cosa, sinhb, coshb;
  real_t numerReal, denomReal;
  real_t numerImag, denomImag;

  WP34S_Cvt2RadSinCosTan(xReal, amRadian, &sina, &cosa, NULL, realContext);
  WP34S_SinhCosh(xImag, &sinhb, &coshb, realContext);

  realMultiply(&sina, &coshb, &numerReal, realContext);
  realMultiply(&cosa, &sinhb, &numerImag, realContext);

  realMultiply(&cosa, &coshb, &denomReal, realContext);
  realMultiply(&sina, &sinhb, &denomImag, realContext);
  realChangeSign(&denomImag);

  divComplexComplex(&numerReal, &numerImag, &denomReal, &denomImag, rReal, rImag, realContext);

  return ERROR_NONE;
}

/********************************************//**
 * \brief regX ==> regL and tan(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnTan(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&tanReal, &tanCplx);
}

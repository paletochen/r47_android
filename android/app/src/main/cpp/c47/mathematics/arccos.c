// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arccos.c
 ***********************************************/

#include "c47.h"

static void arccosCplx(void) {
  cplx_t x, r;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  ArcsinAcosComplex(&x, &r, true, &ctxtReal39);

  convertComplexToResultRegister(&r, REGISTER_X);
}



static void arccosReal(void) {
  real_t x;
  const real_t *r = &x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realCompareAbsGreaterThan(&x, const_1)) {
    if(getFlag(FLAG_CPXRES)) {
      cplx_t z;

      realCopy(&x, &z.Real);
      realSetZero(&z.Imag);
      if(!realIsNegative(&x)) {
        realSetNegativeSign(&z.Imag);
      }
      ArcsinAcosComplex(&z, &z, true, &ctxtReal39);
      convertComplexToResultRegister(&z, REGISTER_X);
      return;
    }
    else if(getSystemFlag(FLAG_SPCRES)) {
      r = const_NaN;
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function arccosReal:", "|X| > 1", "and CPXRES is not set!", NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else if(!exactArcSinCosAngle(&x, true, &x)) {
    if(currentAngularMode != amRadian && realCompareAbsLessThan(&x, const_1on2)) {   // 90° less arcsin x: the right angle is exact outside radians
      C47_WP34S_Asin(&x, &x, &ctxtReal39);
      rightAngleLessRadians(&x, false);
    }
    else {
      C47_WP34S_Acos(&x, &x, &ctxtReal39);
      convertAngleFromTo(&x, amRadian, currentAngularMode, &ctxtReal39);
    }
  }
  convertRealToResultRegister(r, REGISTER_X, currentAngularMode);
}



/********************************************//**
 * \brief regX ==> regL and arccos(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnArccos(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&arccosReal, &arccosCplx);
}

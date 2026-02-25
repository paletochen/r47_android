// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ortho_polynom.c
 ***********************************************/

#include "c47.h"

#if !defined(SAVE_SPACE_DM42_12ORTHO)
static bool_t getOrthoPolyParam(calcRegister_t regist, real_t *val, realContext_t *realContext) {
  if (!getRegisterAsReal(regist, val)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, regist);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Incompatible type for orthogonal polynomial.");
      moreInfoOnError("In function getOrthoPolyParam:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  return true;
}
#endif // !SAVE_SPACE_DM42_12ORTHO

void fnOrthoPoly(uint16_t kind) {
#if !defined(SAVE_SPACE_DM42_12ORTHO)
  real_t x, y, z, ans;

  if(!saveLastX()) {
    return;
  }
  if(getOrthoPolyParam(REGISTER_X, &x, &ctxtReal39) && getOrthoPolyParam(REGISTER_Y, &y, &ctxtReal39)) {
    realCopy(const_0, &z);
    if((kind != ORTHOPOLY_LAGUERRE_L_ALPHA) || getOrthoPolyParam(REGISTER_Z, &z, &ctxtReal39)) {
      if(realIsSpecial(&y) || realIsNegative(&y) || (!realIsAnInteger(&y)) || realCompareLessEqual(&z, const__1)) {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnOrthoPoly:", "Y must be a nonnegative integer.", kind == ORTHOPOLY_LAGUERRE_L_ALPHA ? "In addition, Z must be greater than -1." : NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      else {
        WP34S_OrthoPoly(kind, &x, &y, &z, &ans, &ctxtReal39);
        convertRealToResultRegister(&ans, REGISTER_X, amNone);
        if(kind == ORTHOPOLY_LAGUERRE_L_ALPHA) {
          fnDropY(NOPARAM);
        }
      }
    }
  }
  adjustResult(REGISTER_X, true, false, REGISTER_X, REGISTER_Y, -1);
#endif // !SAVE_SPACE_DM42_12ORTHO
}

void fnHermite(uint16_t unusedButMandatoryParameter) {
  fnOrthoPoly(ORTHOPOLY_HERMITE_HE);
}
void fnHermiteP(uint16_t unusedButMandatoryParameter) {
  fnOrthoPoly(ORTHOPOLY_HERMITE_H);
}
void fnLaguerre(uint16_t unusedButMandatoryParameter) {
  fnOrthoPoly(ORTHOPOLY_LAGUERRE_L);
}
void fnLaguerreAlpha(uint16_t unusedButMandatoryParameter) {
  fnOrthoPoly(ORTHOPOLY_LAGUERRE_L_ALPHA);
}
void fnLegendre(uint16_t unusedButMandatoryParameter) {
  fnOrthoPoly(ORTHOPOLY_LEGENDRE_P);
}
void fnChebyshevT(uint16_t unusedButMandatoryParameter) {
  fnOrthoPoly(ORTHOPOLY_CHEBYSHEV_T);
}
void fnChebyshevU(uint16_t unusedButMandatoryParameter) {
  fnOrthoPoly(ORTHOPOLY_CHEBYSHEV_U);
}

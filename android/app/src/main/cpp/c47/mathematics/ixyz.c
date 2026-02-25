// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ixyz.c
 ***********************************************/

#include "c47.h"

void fnIxyz(uint16_t unusedButMandatoryParameter) {
  real_t x, a, b, val;

  if(!saveLastX()) {
    return;
  }

  if(getRegisterAsReal(REGISTER_X, &x) && getRegisterAsReal(REGISTER_Y, &a) && getRegisterAsReal(REGISTER_Z, &b)) {
    if(realCompareGreaterEqual(&x, const_0) && realCompareLessEqual(&x, const_1) && realCompareGreaterThan(&a, const_0) && realCompareGreaterThan(&b, const_0)) {
      WP34S_betai(&b, &a, &x, &val, &ctxtReal39);
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      convertRealToReal34ResultRegister(&val, REGISTER_X);
      fnDropY(NOPARAM);
      if(lastErrorCode == ERROR_NONE) {
        fnDropY(NOPARAM);
      }
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not in 0 " STD_LESS_EQUAL " x " STD_LESS_EQUAL " 1, a > 0, b > 0");
        moreInfoOnError("In function fnIxyz:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Ixyz for %s, %s, %s",
        getRegisterDataTypeName(REGISTER_X, true, false),
        getRegisterDataTypeName(REGISTER_Y, true, false),
        getRegisterDataTypeName(REGISTER_Z, true, false));
      moreInfoOnError("In function fnIxyz:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}

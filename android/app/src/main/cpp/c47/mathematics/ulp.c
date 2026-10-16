// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ulp.c
 ***********************************************/

#include "c47.h"

void fnUlp(uint16_t unusedButMandatoryParameter) {
  longInteger_t lgInt;

  if(!saveLastX()) {
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      longIntegerInit(lgInt);
      uInt32ToLongInteger(1u, lgInt);
      convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
      longIntegerFree(lgInt);
      break;
    }

    case dtShortInteger: {
      longIntegerInit(lgInt);
      uInt32ToLongInteger(1u, lgInt);
      convertLongIntegerToShortIntegerRegister(lgInt, getRegisterShortIntegerBase(REGISTER_X), REGISTER_X);
      longIntegerFree(lgInt);
      break;
    }

    case dtReal34: {
      real_t ulp;
      int32_t exponent;

      if(real34IsInfinite(REGISTER_REAL34_DATA(REGISTER_X))) {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnUlp:", "cannot use ±∞ input of ULP?", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }

      if(!real34IsNaN(REGISTER_REAL34_DATA(REGISTER_X))) {
        real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &ulp);
        exponent = realIsZero(&ulp) ? DECQUAD_Emin - DECQUAD_Pmax + 1 : ulp.exponent + ulp.digits - DECQUAD_Pmax;
        if(exponent < DECQUAD_Emin - DECQUAD_Pmax + 1) {
          exponent = DECQUAD_Emin - DECQUAD_Pmax + 1;
        }
        realSetOne(&ulp);
        ulp.exponent = exponent;
        convertRealToReal34ResultRegister(&ulp, REGISTER_X);
      }

      setRegisterAngularMode(REGISTER_X, amNone);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot calculate ULP? with %s in X", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnUlp:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}

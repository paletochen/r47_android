// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file shiftDigits.c
 ***********************************************/

#include "c47.h"

/********************************************//**
 * \brief regX ==> regL and SDL(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSdl(uint16_t numberOfShifts) {
  if(getRegisterDataType(REGISTER_X) == dtReal34) {
    real_t real;

    if(!saveLastX()) {
    return;
  }

    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &real);
    real.exponent += numberOfShifts;
    convertRealToReal34ResultRegister(&real, REGISTER_X);
  }

  else
    if(getRegisterDataType(REGISTER_X) == dtLongInteger) {
      longInteger_t y, x;
      uint32_t exponent;

      if(!saveLastX()) {
    return;
  }
      convertLongIntegerRegisterToLongInteger(REGISTER_X, x);
      longIntegerInit(y);
      uInt32ToLongInteger(10u, y);

      for(exponent = numberOfShifts; exponent > 0; exponent--) {
        longIntegerMultiply(y, x, x);
      }

      convertLongIntegerToLongIntegerRegister(x, REGISTER_X);
      longIntegerFree(y);
      longIntegerFree(x);
    }

    else {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot SDL %s", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnSdl:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
}



/********************************************//**
 * \brief regX ==> regL and SDR(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSdr(uint16_t numberOfShifts) {
  if(getRegisterDataType(REGISTER_X) == dtReal34) {
    real_t real;

    if(!saveLastX()) {
    return;
  }

    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &real);
    real.exponent -= numberOfShifts;
    convertRealToReal34ResultRegister(&real, REGISTER_X);
  }

  else
    if(getRegisterDataType(REGISTER_X) == dtLongInteger) {
      longInteger_t y, x;
      uint32_t exponent;

      if(!saveLastX()) {
    return;
  }
      convertLongIntegerRegisterToLongInteger(REGISTER_X, x);
      longIntegerInit(y);
      uInt32ToLongInteger(10u, y);

      for(exponent = numberOfShifts; exponent > 0; exponent--) {
        longIntegerDivide(x, y, x);
        if(longIntegerCompareUInt(x, 0) == 0) {
          //printf("Exit: x/10 resulted in zero\n");
          break;
        }
      }

      convertLongIntegerToLongIntegerRegister(x, REGISTER_X);
      longIntegerFree(y);
      longIntegerFree(x);
    }

    else {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot SDR %s", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnSdr:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
}

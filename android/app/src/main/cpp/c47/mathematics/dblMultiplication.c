// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file dblMultiplication.c
 ***********************************************/

#include "c47.h"

void fnDblMultiply(uint16_t unusedButMandatoryParameter) {
  longInteger_t y, x, ans, wd;
  int32_t base;
  const uint8_t sim = shortIntegerMode;

  if(getRegisterDataType(REGISTER_X) != dtShortInteger) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_T);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "the input type %s is not allowed for DBL" STD_CROSS "!", getDataTypeName(getRegisterDataType(REGISTER_X), false, false));
      moreInfoOnError("In function fnDblMultiply:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  if(getRegisterDataType(REGISTER_Y) != dtShortInteger) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_T);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "the input type %s is not allowed for DBL" STD_CROSS "!", getDataTypeName(getRegisterDataType(REGISTER_Y), false, false));
      moreInfoOnError("In function fnDblMultiply:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(!saveLastX()) {
    return;
  }

  longIntegerInit(wd);
  longInteger2Pow(shortIntegerWordSize, wd);

  convertShortIntegerRegisterToLongInteger(REGISTER_Y, y);
  convertShortIntegerRegisterToLongInteger(REGISTER_X, x);
  base = getRegisterShortIntegerBase(REGISTER_Y);

  longIntegerInit(ans);
  longIntegerMultiply(y, x, ans);
  if(sim == SIM_1COMPL) {
    if((*(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)) & shortIntegerSignBit) != (*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) & shortIntegerSignBit)) {
      longIntegerSubtractUInt(ans, 1u, ans);
    }
    shortIntegerMode = SIM_2COMPL;
  }
  else if(sim == SIM_SIGNMT) {
    longIntegerSetPositiveSign(ans);
    shortIntegerMode = SIM_UNSIGN;
  }

  longIntegerModulo(ans, wd, y);
  longIntegerSubtract(ans, y, x);
  longIntegerDivide(x, wd, x);

  if(sim == SIM_SIGNMT) {
    shortIntegerMode = SIM_SIGNMT;
    if((*(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)) & shortIntegerSignBit) != (*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) & shortIntegerSignBit)) {
      longIntegerSetNegativeSign(x);
    }
  }

  convertLongIntegerToShortIntegerRegister(y, base, REGISTER_Y);
  convertLongIntegerToShortIntegerRegister(x, base, REGISTER_X);

  shortIntegerMode = sim;

  longIntegerFree(x);
  longIntegerFree(y);
  longIntegerFree(ans);
  longIntegerFree(wd);

  clearSystemFlag(FLAG_OVERFLOW);
}

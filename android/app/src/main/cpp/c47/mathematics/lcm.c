// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file lcm.c
 ***********************************************/

#include "c47.h"

static void lcmInt(void) {
  longInteger_t liX, liY;
  bool_t fracX, fracY;

  if(!getRegisterAsLongInt(REGISTER_Y, liY, &fracY)) {
    goto end1;
  }

  if(!getRegisterAsLongInt(REGISTER_X, liX, &fracX)) {
    goto end2;
  }

  if(fracX) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    goto end2;
  }

  if(fracY) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_Y);
    goto end2;
  }

  longIntegerSetPositiveSign(liY);
  longIntegerSetPositiveSign(liX);
  longIntegerLcm(liY, liX, liX);
  convertLongIntegerToLongIntegerRegister(liX, REGISTER_X);

end2:
  longIntegerFree(liX);
end1:
  longIntegerFree(liY);
}

static void lcmShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intLCM(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)), *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}

/********************************************//**
 * \brief regX ==> regL and LCM(regY, regX) ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLcm(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(&lcmInt, NULL, &lcmShoI, &lcmInt);
}


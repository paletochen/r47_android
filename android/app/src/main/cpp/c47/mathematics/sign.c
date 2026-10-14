// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sign.c
 ***********************************************/

#include "c47.h"

static void buildSignResult(int32_t r) {
  longInteger_t lgInt;

  longIntegerInit(lgInt);
  int32ToLongInteger(r, lgInt);
  convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
  longIntegerFree(lgInt);
}

static void signLonI(void) {
  static const int32_t convIntSign[] = {
    [LI_ZERO] = 0,
    [LI_NEGATIVE] = -1,
    [LI_POSITIVE] = 1
  };

  buildSignResult(convIntSign[getRegisterLongIntegerSign(REGISTER_X)]);
}

static void signShoI(void) {
  int32_t signValue;
  uint64_t value = WP34S_extract_value(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)), &signValue);

  buildSignResult(value == 0 ? 0 : 2 * -signValue + 1);
}

static void signReal(void) {
  real34_t *x = getRegisterDataPointer(REGISTER_X);

  if(real34IsNaN(x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function signReal:", "cannot use NaN as X input of SIGN", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  buildSignResult(real34IsZero(x) ? 0 : real34IsNegative(x) ? -1 : 1);
}


/********************************************//**
 * \brief regX ==> regL and sign(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSign(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&signReal, &unitVectorCplx, &signShoI, &signLonI);
}

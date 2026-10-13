// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file changeSign.c
 ***********************************************/

#include "c47.h"

static void chsLonI(void) {
  longInteger_t x;

  if(!getRegisterAsLongInt(REGISTER_X, x, NULL)) {
    goto end;
  }

  longIntegerChangeSign(x);
  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);
end:
  longIntegerFree(x);
}

void chsShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intChs(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}

static void chsZeroCheck(real_t *a) {
  realChangeSign(a);
  if(realIsZero(a) && !getSystemFlag(FLAG_SPCRES)) {
    realSetPositiveSign(a);
  }
}

void chsReal(void) {
  real_t x;
  angularMode_t mode = amNone;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(getRegisterDataType(REGISTER_X) == dtReal34) {
    mode = getRegisterAngularMode(REGISTER_X);
  }

  chsZeroCheck(&x);
  convertRealToResultRegister(&x, REGISTER_X, mode);
}

void chsCplx(void) {
  real_t a, b;

  if(getRegisterAsComplex(REGISTER_X, &a, &b)) {
    chsZeroCheck(&a);
    chsZeroCheck(&b);
    convertComplexToResultRegister(&a, &b, REGISTER_X);
  }
}

/********************************************//**
 * \brief rexX ==> regL and -regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnChangeSign(uint16_t unusedButMandatoryParameter) {
  if(getRegisterDataType(REGISTER_X) == dtTime) {
    real34_t *x = REGISTER_REAL34_DATA(REGISTER_X);
    real34ChangeSign(x);
    if(real34IsZero(x) && !getSystemFlag(FLAG_SPCRES)) {
      real34SetPositiveSign(x);
    }
    return;
  }

  processIntRealComplexMonadicFunction(&chsReal, &chsCplx, &chsShoI, &chsLonI);
}

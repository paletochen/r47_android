// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file 2pow.c
 ***********************************************/

#include "c47.h"
#include "10pow.h"

static void twoPowReal(void);


void realPower2(const real_t *x, real_t *res, realContext_t *realContext) {
  realMultiply(x, const_ln2, res, realContext);
  realExp(res, res, realContext);
}


static void twoPowLonI(void) {
  int32_t exponentSign;
  longInteger_t base, exponent;

  longIntegerInit(base);
  uInt32ToLongInteger(2u, base);
  convertLongIntegerRegisterToLongInteger(REGISTER_X, exponent);

  longIntegerSetPositiveSign(base);

  exponentSign = longIntegerSign(exponent);
  longIntegerSetPositiveSign(exponent);

  if(longIntegerIsZero(exponent)) {
    uInt32ToLongInteger(1u, base);
    convertLongIntegerToLongIntegerRegister(base, REGISTER_X);
    longIntegerFree(base);
    longIntegerFree(exponent);
    return;
  }
  else if(exponentSign == -1) {
    longIntegerFree(base);
    longIntegerFree(exponent);
    twoPowReal();
    return;
  }

  longInteger_t power;

  longIntegerInit(power);
  uInt32ToLongInteger(1u, power);

  while(!longIntegerIsZero(exponent) && lastErrorCode == 0) {
    if(longIntegerIsOdd(exponent)) {
     longIntegerMultiply(power, base, power);
    }

    longIntegerDivideUInt(exponent, 2, exponent);

    if(!longIntegerIsZero(exponent)) {
      longIntegerSquare(base, base);
    }
  }

  convertLongIntegerToLongIntegerRegister(power, REGISTER_X);

  longIntegerFree(power);
  longIntegerFree(base);
  longIntegerFree(exponent);
}


static void twoPowShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_int2pow(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}



static void twoPowReal(void) {
  intPowReal(&realPower2);
}



static void twoPowCplx(void) {
  intPowCplx(const_ln2);
}


/********************************************//**
 * \brief regX ==> regL and 2^regX ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fn2Pow(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&twoPowReal, &twoPowCplx, &twoPowShoI, &twoPowLonI);
}

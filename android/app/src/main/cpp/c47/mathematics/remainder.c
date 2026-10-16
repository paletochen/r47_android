// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file remainder.c
 ***********************************************/

#include "c47.h"

/********************************************//**
 * \brief Computes a mixed integer and Real34 remainder from exact coefficients
 *
 * \param[in] modulo whether a sign correction is required
 * \return true when the mixed pair was completed, false when the decimal path is required
 ***********************************************/
bool_t mixedIntegerRealRemainder(bool_t modulo) {
  const uint32_t typeX = getRegisterDataType(REGISTER_X);
  const uint32_t typeY = getRegisterDataType(REGISTER_Y);
  const bool_t xReal = typeX == dtReal34;
  const bool_t yReal = typeY == dtReal34;
  int32_t exponentX, exponentY, remainderExponent;
  longInteger_t x, y, quotient, remainder;

  if(xReal == yReal) {
    return false;
  }
  if(!convertRegisterToScaledLongInteger(REGISTER_Y, y, &exponentY)) {
    return false;
  }

  if(xReal && real34IsInfinite(REGISTER_REAL34_DATA(REGISTER_X))) {
    if(modulo && !longIntegerIsZero(y) && longIntegerIsNegative(y) != real34IsNegative(REGISTER_REAL34_DATA(REGISTER_X))) {
      setRegisterAngularMode(REGISTER_X, amNone);
    }
    else {
      reallocateRegister(REGISTER_X, dtReal34, REAL34_SIZE_IN_BLOCKS, amNone);
      convertScaledLongIntegerToReal34(y, exponentY, REGISTER_REAL34_DATA(REGISTER_X));
    }
    longIntegerFree(y);
    return true;
  }

  if(!convertRegisterToScaledLongInteger(REGISTER_X, x, &exponentX)) {
    longIntegerFree(y);
    return false;
  }
  if(longIntegerIsZero(x)) {
    longIntegerFree(x);
    longIntegerFree(y);
    return false;
  }

  longIntegerInit(quotient);
  longIntegerInit(remainder);
  longIntegerScaledDivide(y, exponentY, x, exponentX, quotient, remainder, &remainderExponent);

  if(modulo && !longIntegerIsZero(remainder) && longIntegerIsNegative(y) != longIntegerIsNegative(x)) {
    longInteger_t divisor, scale;

    longIntegerInit(divisor);
    longIntegerCopy(x, divisor);
    if(exponentX > remainderExponent) {
      longIntegerInit(scale);
      longIntegerPowerUIntUInt(10u, (uint32_t)(exponentX - remainderExponent), scale);
      longIntegerMultiplyUnbounded(divisor, scale, divisor);
      longIntegerFree(scale);
    }
    longIntegerAdd(remainder, divisor, remainder);
    longIntegerFree(divisor);
  }

  reallocateRegister(REGISTER_X, dtReal34, REAL34_SIZE_IN_BLOCKS, amNone);
  convertScaledLongIntegerToReal34(remainder, remainderExponent, REGISTER_REAL34_DATA(REGISTER_X));
  longIntegerFree(remainder);
  longIntegerFree(quotient);
  longIntegerFree(x);
  longIntegerFree(y);
  return true;
}

/******************************************************************************************************************************************************************************************/
/* long integer rmd ...                                                                                                                                                                     */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(long integer) rmd X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
static void rmdLonI(void) {
  integerRemainder(false, false);
}

/******************************************************************************************************************************************************************************************/
/* short integer rmd ...                                                                                                                                                                    */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(short integer) rmd X(short integer) ==> X(short integer)
 *
 * \param void
 * \return void
 ***********************************************/
static void rmdShoI(void) {
  integerRemainder(false, true);
}

/******************************************************************************************************************************************************************************************/
/* real34 rmd ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34) rmd X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
static void rmdReal(void) {
  real_t x, y, r;
  bool_t real34Operands = getRegisterDataType(REGISTER_X) == dtReal34 && getRegisterDataType(REGISTER_Y) == dtReal34;

  if(mixedIntegerRealRemainder(false)) {
    return;
  }
  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y)) {
    return;
  }

  if(realIsZero(&x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function rmdReal:", "cannot IDIVR a real34 by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(real34Operands) {
    WP34S_Real34Remainder(&y, &x, &r);
  }
  else {
    WP34S_BigMod(&y, &x, &r, &ctxtReal39);
  }
  if(realIsZero(&r)) {
    realSetPositiveSign(&r);
  }
  convertRealToResultRegister(&r, REGISTER_X, amNone);
}

/********************************************//**
 * \brief regX ==> regL and regY rmd regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnRmd(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(&rmdReal, NULL, &rmdShoI, &rmdLonI);
}

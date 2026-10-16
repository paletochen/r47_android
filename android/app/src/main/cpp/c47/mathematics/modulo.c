// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file modulo.c
 ***********************************************/

#include "c47.h"


/******************************************************************************************************************************************************************************************/
/* long integer mod ...                                                                                                                                                                     */
/******************************************************************************************************************************************************************************************/

// MOD and RMD over two integers are one routine. floorMod brings the remainder to the sign of the divisor, which is what makes MOD a floored remainder and RMD a
// truncated one. shortResult sends the answer back as a short integer in Y's base instead of a long integer.
void integerRemainder(bool_t floorMod, bool_t shortResult) {
  longInteger_t x, y, remainder;
  uint32_t baseY = 0;

  if(!getRegisterAsLongInt(REGISTER_X, x, NULL)) {
    goto err1;
  }

  if(longIntegerIsZero(x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function integerRemainder:", "cannot IDIVR an integer by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto err1;
  }
  else {
    if(!getRegisterAsLongInt(REGISTER_Y, y, NULL)) {
      goto err2;
    }
    if(shortResult) {
      baseY = getRegisterShortIntegerBase(REGISTER_Y);
    }

    longIntegerInit(remainder);
    longIntegerDivideRemainder(y, x, remainder);
    if(floorMod && !longIntegerIsZero(remainder) && longIntegerSign(y) != longIntegerSign(x)) {
      longIntegerAdd(remainder, x, remainder);
    }

    if(shortResult) {
      convertLongIntegerToShortIntegerRegister(remainder, baseY, REGISTER_X);
    }
    else {
      convertLongIntegerToLongIntegerRegister(remainder, REGISTER_X);
    }

    longIntegerFree(remainder);
  }
err2:
  longIntegerFree(y);
err1:
  longIntegerFree(x);
}


/********************************************//**
 * \brief Y(long integer) mod X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
static void modLonI(void) {
  integerRemainder(true, false);
}


/******************************************************************************************************************************************************************************************/
/* short integer mod ...                                                                                                                                                                    */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(short integer) mod X(short integer) ==> X(short integer)
 *
 * \param void
 * \return void
 ***********************************************/
static void modShoI(void) {
  integerRemainder(true, true);
}


/******************************************************************************************************************************************************************************************/
/* real34 mod ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34) mod X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void modReal(void) {
  real_t x, y, r;
  bool_t real34Operands = getRegisterDataType(REGISTER_X) == dtReal34 && getRegisterDataType(REGISTER_Y) == dtReal34;

  if(mixedIntegerRealRemainder(true)) {
    return;
  }
  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y)) {
    return;
  }

  if(realIsZero(&x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function modReal:", "cannot IDIVR a real34 by 0", NULL, NULL);
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
  else if(realGetSign(&y) != realGetSign(&x)) {
    realAdd(&r, &x, &r, real34Operands ? &ctxtReal34 : &ctxtReal39);
  }

  convertRealToResultRegister(&r, REGISTER_X, amNone);
}

/********************************************//**
 * \brief regX ==> regL and regY mod regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnMod(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(&modReal, NULL, &modShoI, &modLonI);
}

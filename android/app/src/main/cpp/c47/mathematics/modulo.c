// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file modulo.c
 ***********************************************/

#include "c47.h"


/******************************************************************************************************************************************************************************************/
/* long integer mod ...                                                                                                                                                                     */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(long integer) mod X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
static void modLonI(void) {
  longInteger_t x, y, remainder;

  if(!getRegisterAsLongInt(REGISTER_X, x, NULL)) {
    goto err1;
  }

  if(longIntegerIsZero(x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function modLonI:", "cannot IDIVR a long integer by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto err1;
  }
  else {
    if(!getRegisterAsLongInt(REGISTER_Y, y, NULL)) {
      goto err2;
    }

    longIntegerInit(remainder);
    longIntegerDivideRemainder(y, x, remainder);
    longIntegerAdd(remainder, x, remainder);
    longIntegerDivideRemainder(remainder, x, remainder);

    convertLongIntegerToLongIntegerRegister(remainder, REGISTER_X);

    longIntegerFree(remainder);
  }
err2:
  longIntegerFree(y);
err1:
  longIntegerFree(x);
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
  longInteger_t x, y, remainder;
  uint32_t baseY;

  if(!getRegisterAsLongInt(REGISTER_X, x, NULL)) {
    goto err1;
  }

  if(longIntegerIsZero(x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function modShoI:", "cannot IDIVR a short integer by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto err1;
  }
  else {
    if(!getRegisterAsLongInt(REGISTER_Y, y, NULL)) {
      goto err2;
    }
    baseY = getRegisterShortIntegerBase(REGISTER_Y);

    longIntegerInit(remainder);
    longIntegerDivideRemainder(y, x, remainder);
    longIntegerAdd(remainder, x, remainder);
    longIntegerDivideRemainder(remainder, x, remainder);

    convertLongIntegerToShortIntegerRegister(remainder, baseY, REGISTER_X);

    longIntegerFree(remainder);
  }
err2:
  longIntegerFree(y);
err1:
  longIntegerFree(x);
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

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y)) {
    return;
  }

  if(realIsZero(&x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function modReal:", "cannot IDIVR a real34 by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  WP34S_BigMod(&y, &x, &r, &ctxtReal39);
  if(!realIsZero(&r) && realGetSign(&y) != realGetSign(&x)) {
    realAdd(&r, &x, &r, &ctxtReal39);
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


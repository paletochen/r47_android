// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file idivr.c
 ***********************************************/

#include "c47.h"

TO_QSPI void (* const idivr[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void) = {
// regX |    regY ==>   1              2              3           4           5           6           7           8            9              10
//      V               Long integer   Real34         Complex34   Time        Date        String      Real34 mat  Complex34 m  Short integer  Config data
/*  1 Long integer  */ {idivrLonILonI, idivrRealLonI, idivrError, idivrError, idivrError, idivrError, idivrError, idivrError,  idivrShoILonI, idivrError},
/*  2 Real34        */ {idivrLonIReal, idivrRealReal, idivrError, idivrError, idivrError, idivrError, idivrError, idivrError,  idivrShoIReal, idivrError},
/*  3 Complex34     */ {idivrError,    idivrError,    idivrError, idivrError, idivrError, idivrError, idivrError, idivrError,  idivrError,    idivrError},
/*  4 Time          */ {idivrError,    idivrError,    idivrError, idivrError, idivrError, idivrError, idivrError, idivrError,  idivrError,    idivrError},
/*  5 Date          */ {idivrError,    idivrError,    idivrError, idivrError, idivrError, idivrError, idivrError, idivrError,  idivrError,    idivrError},
/*  6 String        */ {idivrError,    idivrError,    idivrError, idivrError, idivrError, idivrError, idivrError, idivrError,  idivrError,    idivrError},
/*  7 Real34 mat    */ {idivrError,    idivrError,    idivrError, idivrError, idivrError, idivrError, idivrError, idivrError,  idivrError,    idivrError},
/*  8 Complex34 mat */ {idivrError,    idivrError,    idivrError, idivrError, idivrError, idivrError, idivrError, idivrError,  idivrError,    idivrError},
/*  9 Short integer */ {idivrLonIShoI, idivrRealShoI, idivrError, idivrError, idivrError, idivrError, idivrError, idivrError,  idivrShoIShoI, idivrError},
/* 10 Config data   */ {idivrError,    idivrError,    idivrError, idivrError, idivrError, idivrError, idivrError, idivrError,  idivrError,    idivrError}
};



/********************************************//**
 * \brief Data type error in IDiv
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  void idivrError(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    sprintf(errorMessage, "cannot IDIVR %s", getRegisterDataTypeName(REGISTER_Y, true, false));
    sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "by %s", getRegisterDataTypeName(REGISTER_X, true, false));
    moreInfoOnError("In function fnIDivR:", errorMessage, errorMessage + ERROR_MESSAGE_LENGTH/2, NULL);
  }
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)



/********************************************//**
 * \brief regX ==> regL and regY idivr regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnIDivR(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  idivr[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

  adjustResult(REGISTER_X, false, false, REGISTER_X, REGISTER_Y, -1);
  adjustResult(REGISTER_Y, false, false, REGISTER_X, REGISTER_Y, -1);
}



/******************************************************************************************************************************************************************************************/
/* long integer idivr ...                                                                                                                                                                     */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(long integer) idivr X(long integer) ==> X(long integer), Y(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void idivrLonILonI(void) {
  longInteger_t x;

  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);

  if(longIntegerIsZero(x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function idivrLonILonI:", "cannot IDIVR a long integer by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    longInteger_t y, quotient, remainder;

    convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);

    longIntegerInit(quotient);
    longIntegerInit(remainder);
    longIntegerDivideQuotientRemainder(y, x, quotient, remainder);

    convertLongIntegerToLongIntegerRegister(quotient, REGISTER_X);
    convertLongIntegerToLongIntegerRegister(remainder, REGISTER_Y);

    longIntegerFree(y);
    longIntegerFree(quotient);
    longIntegerFree(remainder);
  }

  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(long integer) idivr X(short integer) ==> X(long integer), Y(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void idivrLonIShoI(void) {
  longInteger_t x;

  convertShortIntegerRegisterToLongInteger(REGISTER_X, x);

  if(longIntegerIsZero(x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function idivrLonIShoI:", "cannot IDIVR a long integer by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    longInteger_t y, quotient, remainder;

    convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);

    longIntegerInit(quotient);
    longIntegerInit(remainder);
    longIntegerDivideQuotientRemainder(y, x, quotient, remainder);

    convertLongIntegerToLongIntegerRegister(quotient, REGISTER_X);
    convertLongIntegerToLongIntegerRegister(remainder, REGISTER_Y);

    longIntegerFree(y);
    longIntegerFree(quotient);
    longIntegerFree(remainder);
  }

  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(short integer) idivr X(long integer) ==> X(long integer), Y(short integer)
 *
 * \param void
 * \return void
 ***********************************************/
void idivrShoILonI(void) {
  longInteger_t x;

  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);

  if(longIntegerIsZero(x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function idivrShoILonI:", "cannot IDIVR a short integer by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    longInteger_t y, quotient, remainder;
    uint32_t baseY;

    baseY = getRegisterShortIntegerBase(REGISTER_Y);
    convertShortIntegerRegisterToLongInteger(REGISTER_Y, y);

    longIntegerInit(quotient);
    longIntegerInit(remainder);
    longIntegerDivideQuotientRemainder(y, x, quotient, remainder);

    convertLongIntegerToLongIntegerRegister(quotient, REGISTER_X);
    convertLongIntegerToShortIntegerRegister(remainder, baseY, REGISTER_Y);

    longIntegerFree(y);
    longIntegerFree(quotient);
    longIntegerFree(remainder);
  }

  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(long integer) idivr X(real34) ==> X(long integer), Y(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void idivrLonIReal(void) {
  if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function idivrLonIReal:", "cannot IDIVR a long integer by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  real_t y, x, q;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
  realDivide(&y, &x, &q, &ctxtReal39);
  realToIntegralValue(&q, &q, DEC_ROUND_DOWN, &ctxtReal39);
  convertRealToLongIntegerRegister(&q, REGISTER_X, DEC_ROUND_DOWN);

  WP34S_Mod(&y, &x, &y, &ctxtReal39);
  reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
  convertRealToReal34ResultRegister(&y, REGISTER_Y);
}



/********************************************//**
 * \brief Y(real34) idivr X(long integer) ==> X(long integer), Y(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void idivrRealLonI(void) {
  real_t x;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  if(realIsZero(&x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function idivrRealLonI:", "cannot IDIVR a real34 by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  real_t y, q;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
  realDivide(&y, &x, &q, &ctxtReal39);
  realToIntegralValue(&q, &q, DEC_ROUND_DOWN, &ctxtReal39);
  convertRealToLongIntegerRegister(&q, REGISTER_X, DEC_ROUND_DOWN);

  WP34S_Mod(&y, &x, &y, &ctxtReal39);
  convertRealToReal34ResultRegister(&y, REGISTER_Y);
  setRegisterAngularMode(REGISTER_Y, amNone);
}



/******************************************************************************************************************************************************************************************/
/* short integer idivr ...                                                                                                                                                                    */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(short integer) idivr X(short integer) ==> X(short integer), Y(short integer)
 *
 * \param void
 * \return void
 ***********************************************/
void idivrShoIShoI(void) {
  longInteger_t x;

  convertShortIntegerRegisterToLongInteger(REGISTER_X, x);

  if(longIntegerIsZero(x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function idivrShoIShoI:", "cannot IDIVR a short integer by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    longInteger_t y, quotient, remainder;
    uint32_t baseY;

    baseY = getRegisterShortIntegerBase(REGISTER_Y);
    convertShortIntegerRegisterToLongInteger(REGISTER_Y, y);

    longIntegerInit(quotient);
    longIntegerInit(remainder);
    longIntegerDivideQuotientRemainder(y, x, quotient, remainder);

    convertLongIntegerToShortIntegerRegister(quotient, baseY, REGISTER_X);
    convertLongIntegerToShortIntegerRegister(remainder, baseY, REGISTER_Y);

    longIntegerFree(y);
    longIntegerFree(quotient);
    longIntegerFree(remainder);
  }

  longIntegerFree(x);

}



/********************************************//**
 * \brief Y(short integer) idivr X(real34) ==> X(long integer), Y(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void idivrShoIReal(void) {
  if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function idivrShoIReal:", "cannot IDIVR a short integer by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  real_t y, x, q;

  convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
  realDivide(&y, &x, &q, &ctxtReal39);
  realToIntegralValue(&q, &q, DEC_ROUND_DOWN, &ctxtReal39);
  convertRealToLongIntegerRegister(&q, REGISTER_X, DEC_ROUND_DOWN);

  WP34S_Mod(&y, &x, &y, &ctxtReal39);
  reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
  convertRealToReal34ResultRegister(&y, REGISTER_Y);
}



/********************************************//**
 * \brief Y(real34) idivr X(short integer) ==> X(long integer), Y(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void idivrRealShoI(void) {
  real_t x;

  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  if(realIsZero(&x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function idivrRealShoI:", "cannot IDIVR a real34 by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  real_t y, q;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
  realDivide(&y, &x, &q, &ctxtReal39);
  realToIntegralValue(&q, &q, DEC_ROUND_DOWN, &ctxtReal39);
  convertRealToLongIntegerRegister(&q, REGISTER_X, DEC_ROUND_DOWN);

  WP34S_Mod(&y, &x, &y, &ctxtReal39);
  convertRealToReal34ResultRegister(&y, REGISTER_Y);
  setRegisterAngularMode(REGISTER_Y, amNone);
}



/******************************************************************************************************************************************************************************************/
/* real34 idivr ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34) idivr X(real34) ==> X(long integer), Y(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void idivrRealReal(void) {
  if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function idivrRealReal:", "cannot IDIVR a real34 by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  real_t x, y, q;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
  realDivide(&y, &x, &q, &ctxtReal39);
  realToIntegralValue(&q, &q, DEC_ROUND_DOWN, &ctxtReal39);
  convertRealToLongIntegerRegister(&q, REGISTER_X, DEC_ROUND_DOWN);

  WP34S_Mod(&y, &x, &y, &ctxtReal39);
  convertRealToReal34ResultRegister(&y, REGISTER_Y);
  setRegisterAngularMode(REGISTER_Y, amNone);
}

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file factorial.c
 ***********************************************/

#include "c47.h"

static void factReal(void);

static void factLonI(void) {
  longInteger_t x, f;

  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);

  if(longIntegerIsNegative(x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerRegisterToDisplayString(REGISTER_X, errorMessage, ERROR_MESSAGE_LENGTH, SCREEN_WIDTH, 50, false);   //JM added last parameter: Allow LARGELI
      sprintf(tmpString, "cannot calculate factorial(%s)", errorMessage);
      moreInfoOnError("In function factLonI:", tmpString, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    longIntegerFree(x);
    return;
  }

  if(longIntegerCompareUInt(x, MAX_FACTORIAL) > 0) {                            //JM
    // convert long integers above 450 to reals and return a real factorial result
    convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
    longIntegerFree(x);
    factReal();
    return;
  }


  uint32_t n;
  longIntegerToUInt32(x, n);
  #if defined(LINUX)
    //The more precise formula below is: (n*ln(n) - n + (ln(8n� + 4n� + n + 1/30))/6 + ln(pi)/2) / ln(2)
    longIntegerInitSizeInBits(f, 1 + (uint32_t)((n * log(n) - n) / log(2)));
    uInt32ToLongInteger(1u, f);
    for(uint32_t i=2; i<=n; i++) {
      longIntegerMultiplyUInt(f, i, f);
    }
  #else // !LINUX
    longIntegerInit(f);
    longIntegerFactorial(n, f); //TODO why this line fails?
  #endif // LINUX


  convertLongIntegerToLongIntegerRegister(f, REGISTER_X);

  longIntegerFree(f);
  longIntegerFree(x);
}

static uint64_t fact_uint64(uint64_t value)
{
  uint64_t result;

  if(value <= 1) {
    result = 1;
  }
  else {
    uint64_t m = value;

    if((value & 1) != 0) {
      m += value;
      value -= 1;
    }
    result = m;
    value -= 2;
    while(value > 0) {
      m += value;
      result *= m;
      value -= 2;
    }
  }

  return result;
}


static void factShoI(void) {
  int16_t sign;
  uint64_t value;

  convertShortIntegerRegisterToUInt64(REGISTER_X, &sign, &value);

  if(sign == 1) { // Negative value
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerRegisterToDisplayString(REGISTER_X, errorMessage, ERROR_MESSAGE_LENGTH, SCREEN_WIDTH, 50, false);   //JM added last parameter: Allow LARGELI
      sprintf(tmpString, "cannot calculate factorial(%s)", errorMessage);
      moreInfoOnError("In function factShoI:", tmpString, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(value > 20) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerRegisterToDisplayString(REGISTER_X, errorMessage, ERROR_MESSAGE_LENGTH, SCREEN_WIDTH, 50, false);   //JM added last parameter: Allow LARGELI
      sprintf(tmpString, "cannot calculate factorial(%s)", errorMessage);
      moreInfoOnError("In function factShoI:", tmpString, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  uint64_t f = fact_uint64(value);
  if(f > shortIntegerMask) {
    setSystemFlag(FLAG_OVERFLOW);
  }

  convertUInt64ToShortIntegerRegister(0, f, getRegisterTag(REGISTER_X), REGISTER_X);
}



static void factReal(void) {
  real_t x;

  if (getRegisterAsReal(REGISTER_X, &x)) {
    WP34S_Factorial(&x, &x, &ctxtReal39);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
  }
}



static void factCplx(void) {
  real_t zReal, zImag;

  if (getRegisterAsComplex(REGISTER_X, &zReal, &zImag)) {
    realAdd(&zReal, const_1, &zReal, &ctxtReal39);
    WP34S_ComplexGamma(&zReal, &zImag, &zReal, &zImag, &ctxtReal39);
    convertComplexToResultRegister(&zReal, &zImag, REGISTER_X);
  }
}

/********************************************//**
 * \brief regX ==> regL and fact(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnFactorial(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&factReal, &factCplx, &factShoI, &factLonI);
}

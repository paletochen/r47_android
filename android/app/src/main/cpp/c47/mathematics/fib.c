// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file fib.c
 ***********************************************/

#include "c47.h"

static void fibLonI(void) {
  longInteger_t x, result;

  if(!getRegisterAsLongInt(REGISTER_X, x, NULL)) {
    goto end;
  }

  bool_t neg = longIntegerIsNegative(x);
  if (neg)
    longIntegerChangeSign(x);

  /*if(shortIntegerMode == SIM_UNSIGN && longIntegerCompareUInt(x, 93) > 0) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerRegisterToDisplayString(REGISTER_X, errorMessage, ERROR_MESSAGE_LENGTH, SCREEN_WIDTH, 50, false);   //JM added last parameter: Allow LARGELI
      sprintf(tmpString, "cannot calculate fib(%s), the limit for UNSIGN is 93", errorMessage);
      moreInfoOnError("In function fibLonI:", tmpString, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto end;
  }
  else*/ if(longIntegerCompareUInt(x, 4791) > 0) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerRegisterToDisplayString(REGISTER_X, errorMessage, ERROR_MESSAGE_LENGTH, SCREEN_WIDTH, 50, false);   //JM added last parameter: Allow LARGELI
      sprintf(tmpString, "cannot calculate fib(%s), the limit is 4791, it's to ensure that the 3328 bits limit is not exceeded", errorMessage);
      moreInfoOnError("In function fibLonI:", tmpString, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto end;
  }

  uint32_t n;
  longIntegerToUInt32(x, n);           // Convert x into unsigned int.

  longIntegerInit(result);           // Initialize fib variable
  longIntegerFibonacci(n, result);   // result = FIB(n)

  if (neg && longIntegerIsEven(x))
    longIntegerChangeSign(result);

  convertLongIntegerToLongIntegerRegister(result, REGISTER_X);

  longIntegerFree(result);
end:
  longIntegerFree(x);
}


static uint8_t FibonacciReal(const real_t *n, real_t *res, realContext_t *realContext) {
  // FIB(x) = [ PHI^(x) - PHI^(-x)*COS(PI * x) ] / SQRT(5)

  real_t a, b;

  realPower(const_PHI, n, &a, realContext);                             // a   = PHI^(n)
  realDivide(const_1, &a, &b, realContext);                             // b   = PHI^(-n) = = 1/PHI^(n)
  realMultiply(const_pi, n, res, realContext);                          // res = PI * n
  WP34S_Cvt2RadSinCosTan(res, amRadian, NULL, res, NULL, realContext);  // res = COS(PI * n)
  realMultiply(&b, res, &b, realContext);                               // b   = PHI^(-n) * COS(PI * n)
  realSquareRoot(const_5, res, realContext);                            // res = SQRT(5)
  realSubtract(&a, &b, &a, realContext);                                // a   = PHI^(n) - PHI^(-n) * COS(PI * n)
  realDivide(&a, res, res, realContext);                                // res = [ PHI^n - PHI^(-n) * COS(PI * n) ] / SQRT(5)

  return ERROR_NONE;
}


static uint8_t FibonacciComplex(const real_t *nReal, const real_t *nImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  // FIB(x) = [ PHI^(x) - PHI^(-x)*COS(PI * x) ] / SQRT(5)

  real_t aReal, aImag;
  real_t bReal, bImag;

  PowerComplex(const_PHI, const_0, nReal, nImag, &aReal, &aImag, realContext);       // a   = PHI^(n)
  divRealComplex(const_1, &aReal, &aImag, &bReal, &bImag, realContext);              // b   = PHI^(-n) = 1/PHI^(n)
  mulComplexComplex(const_pi, const_0, nReal, nImag, resReal, resImag, realContext); // res = PI * n
  cosComplex(resReal, resImag, resReal, resImag, realContext);                       // res = COS(PI * n)
  mulComplexComplex(&bReal, &bImag, resReal, resImag, &bReal, &bImag, realContext);  // b   = PHI^(-n) * COS(PI * n)
  realSquareRoot(const_5, resReal, realContext);                                     // res = SQRT(5)
  realZero(resImag);
  realSubtract(&aReal, &bReal, &aReal, realContext);                                 // a   = PHI^(n) - PHI^(-n) * COS(PI * n)
  realSubtract(&aImag, &bImag, &aImag, realContext);
  divComplexComplex(&aReal, &aImag, resReal, resImag, resReal, resImag, realContext);// res = [ PHI^(n) - PHI^(-n) * COS(PI * n) ] / SQRT(5)

  return ERROR_NONE;
}

static void fibReal(void) {
  // FIB(x) = [ PHI^(x) - PHI^(-x)*COS(PI * x) ] / SQRT(5)
  real_t x;

  if(getRegisterAsReal(REGISTER_X, &x)) {;
    FibonacciReal(&x, &x, &ctxtReal39);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
  }
}

static void fibCplx(void) {
  real_t xReal, xImag;

  if(!getRegisterAsComplex(REGISTER_X,  &xReal, &xImag))
    return;

  if(realIsZero(&xImag)) {
    FibonacciReal(&xReal, &xReal, &ctxtReal39);
  }
  else {
    FibonacciComplex(&xReal, &xImag, &xReal, &xImag, &ctxtReal39);
  }

  convertComplexToResultRegister(&xReal, &xImag, REGISTER_X);
}

/********************************************//**
 * \brief regX ==> regL and fib(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnFib(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&fibReal, &fibCplx, NULL, &fibLonI);
}

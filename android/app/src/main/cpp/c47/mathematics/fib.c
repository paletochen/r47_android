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
  if(neg) {
    longIntegerChangeSign(x);
  }

  /*if(shortIntegerMode == SIM_UNSIGN && longIntegerCompareUInt(x, 93) > 0) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerRegisterToDisplayString(REGISTER_X, errorMessage, ERROR_MESSAGE_LENGTH, SCREEN_WIDTH, 50, false);   //JM added last parameter: Allow LARGELI
      sprintf(tmpString, "cannot calculate fib(%s), the limit for UNSIGN is 93", errorMessage);
      moreInfoOnError("In function fibLonI:", tmpString, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto end;
  }
  else*/ if(longIntegerCompareUInt(x, 4791) > 0) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
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

  if(neg && longIntegerIsEven(x)) {
    longIntegerChangeSign(result);
  }

  convertLongIntegerToLongIntegerRegister(result, REGISTER_X);

  longIntegerFree(result);
end:
  longIntegerFree(x);
}


static void fibonacciConstants(real_t *lnPhi, real_t *sqrt5, realContext_t *realContext) {
  realSquareRoot(const_5, sqrt5, realContext);
  realAdd(sqrt5, const_1, lnPhi, realContext);
  realMultiply(lnPhi, const_1on2, lnPhi, realContext);
  WP34S_Ln(lnPhi, lnPhi, realContext);
}


static void cosinePiComplex(const cplx_t *z, cplx_t *res, realContext_t *realContext) {
  real_t sinReal, cosReal, sinhImag, coshImag, piImag;
  const real_t *pi = realContext->digits > 51 ? const75_pi : const39_pi;

  C47_WP34S_Cvt2RadSinCosTan(&z->Real, amMultPi, &sinReal, &cosReal, NULL, realContext);
  realMultiply(&z->Imag, pi, &piImag, realContext);
  WP34S_SinhCosh(&piImag, &sinhImag, &coshImag, realContext);
  multiplyTrigByHyperbolic(&cosReal, &coshImag, &res->Real, realContext);
  multiplyTrigByHyperbolic(&sinReal, &sinhImag, &res->Imag, realContext);
  realChangeSign(&res->Imag);
}


static void sinePiHalfComplex(const cplx_t *z, cplx_t *res, realContext_t *realContext) {
  real_t halfReal, sinReal, cosReal, sinhImag, coshImag, piImag;
  const real_t *piOn2 = realContext->digits > 51 ? const75_piOn2 : const39_piOn2;

  realMultiply(&z->Real, const_1on2, &halfReal, realContext);
  C47_WP34S_Cvt2RadSinCosTan(&halfReal, amMultPi, &sinReal, &cosReal, NULL, realContext);
  realMultiply(&z->Imag, piOn2, &piImag, realContext);
  WP34S_SinhCosh(&piImag, &sinhImag, &coshImag, realContext);
  multiplyTrigByHyperbolic(&sinReal, &coshImag, &res->Real, realContext);
  multiplyTrigByHyperbolic(&cosReal, &sinhImag, &res->Imag, realContext);
}


static uint8_t FibonacciReal(const real_t *n, real_t *res, realContext_t *realContext) {
  // FIB(x) = [ PHI^(x) - PHI^(-x)*COS(PI * x) ] / SQRT(5)

  real_t a, b, lnPhi, sqrt5;

  fibonacciConstants(&lnPhi, &sqrt5, realContext);
  realMultiply(&lnPhi, n, &a, realContext);

  if(realCompareAbsLessThan(n, const_1on10)) {
    real_t oneMinusCos, sinHalf;

    realCopy(&a, &b);
    realChangeSign(&b);
    realExp(&b, &b, realContext);
    realAdd(&a, &a, &a, realContext);
    WP34S_ExpM1(&a, &a, realContext);
    realMultiply(n, const_1on2, &sinHalf, realContext);
    C47_WP34S_Cvt2RadSinCosTan(&sinHalf, amMultPi, &sinHalf, NULL, NULL, realContext);
    realMultiply(&sinHalf, &sinHalf, &oneMinusCos, realContext);
    realAdd(&oneMinusCos, &oneMinusCos, &oneMinusCos, realContext);
    realAdd(&a, &oneMinusCos, &a, realContext);
    realMultiply(&a, &b, &a, realContext);
  }
  else {
    realCopy(&a, &b);
    realChangeSign(&b);
    realExp(&a, &a, realContext);
    realExp(&b, &b, realContext);
    C47_WP34S_Cvt2RadSinCosTan(n, amMultPi, NULL, res, NULL, realContext);
    multiplyTrigByHyperbolic(res, &b, &b, realContext);
    realSubtract(&a, &b, &a, realContext);
  }

  realDivide(&a, &sqrt5, res, realContext);

  return ERROR_NONE;
}


static uint8_t FibonacciComplex(const cplx_t *n, cplx_t *res, realContext_t *realContext) {
  // FIB(x) = [ PHI^(x) - PHI^(-x)*COS(PI * x) ] / SQRT(5)

  real_t lnPhi, sqrt5;
  cplx_t a, b;

  fibonacciConstants(&lnPhi, &sqrt5, realContext);
  mulComplexReal(n, &lnPhi, &a, realContext);

  if(realCompareAbsLessThan(&n->Real, const_1on10) && realCompareAbsLessThan(&n->Imag, const_1on10)) {
    cplx_t c;

    realCopy(&a.Real, &b.Real);
    realCopy(&a.Imag, &b.Imag);
    realChangeSign(&b.Real);
    realChangeSign(&b.Imag);
    expComplex(&b, &b, realContext);
    realAdd(&a.Real, &a.Real, &a.Real, realContext);
    realAdd(&a.Imag, &a.Imag, &a.Imag, realContext);
    expM1Complex(&a, &a, realContext);
    sinePiHalfComplex(n, &c, realContext);
    mulComplexComplex(&c, &c, &c, realContext);
    realAdd(&c.Real, &c.Real, &c.Real, realContext);
    realAdd(&c.Imag, &c.Imag, &c.Imag, realContext);
    realAdd(&a.Real, &c.Real, &a.Real, realContext);
    realAdd(&a.Imag, &c.Imag, &a.Imag, realContext);
    mulComplexComplex(&a, &b, &a, realContext);
  }
  else {
    realCopy(&a.Real, &b.Real);
    realCopy(&a.Imag, &b.Imag);
    realChangeSign(&b.Real);
    realChangeSign(&b.Imag);
    expComplex(&a, &a, realContext);
    expComplex(&b, &b, realContext);
    cosinePiComplex(n, res, realContext);
    mulComplexComplex(&b, res, &b, realContext);
    realSubtract(&a.Real, &b.Real, &a.Real, realContext);
    realSubtract(&a.Imag, &b.Imag, &a.Imag, realContext);
  }

  divComplexReal(&a, &sqrt5, res, realContext);

  return ERROR_NONE;
}


static realContext_t *fibonacciRealContext(const real_t *n) {
  if(realCompareAbsLessThan(n, const_1on10)) {
    return &ctxtReal39;
  }
  return realIsNegative(n) ? &ctxtReal75 : &ctxtReal51;
}


static void fibReal(void) {
  // FIB(x) = [ PHI^(x) - PHI^(-x)*COS(PI * x) ] / SQRT(5)
  real_t x;

  if(getRegisterAsReal(REGISTER_X, &x)) {
    FibonacciReal(&x, &x, fibonacciRealContext(&x));
    convertRealToResultRegister(&x, REGISTER_X, amNone);
  }
}

static void fibCplx(void) {
  cplx_t x;

  if(!getRegisterAsComplex(REGISTER_X,  &x)) {
    return;
  }

  if(realIsZero(&x.Imag)) {
    FibonacciReal(&x.Real, &x.Real, fibonacciRealContext(&x.Real));
  }
  else {
    FibonacciComplex(&x, &x, &ctxtReal75);
  }

  convertComplexToResultRegister(&x, REGISTER_X);
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

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cpyx.c
 ***********************************************/

#include "c47.h"

//=============================================================================
// Cyx/Pyx calculation function
//-----------------------------------------------------------------------------

void logCyxReal(real_t *y, real_t *x, real_t *result, realContext_t *realContext) {
  realSubtract(y, x, result, realContext);
  realAdd(result, const_1, result, realContext);
  WP34S_LnGamma(result, result, realContext);       // r = ln((y-x)!)

  realAdd(x, const_1, x, realContext);
  WP34S_LnGamma(x, x, realContext);                 // x = ln(x!)

  realAdd(y, const_1, y, realContext);
  WP34S_LnGamma(y, y, realContext);                 // y = ln(y!)

  realSubtract(y, result, result, realContext);
  realSubtract(result, x, result, realContext);     // r = ln(y!) - ln((y-x)!) - ln(x!)
}

/********************************************//**
 * \brief Evaluates log(C(y,x)) for finite 0 <= x <= y.
 *
 * The beta form avoids subtraction between log-Gamma values of size
 * proportional to y.
 ***********************************************/
static void logCyxPositiveReal(const real_t *y, const real_t *x, real_t *result, realContext_t *realContext) {
  REAL_T_PTR(a, 75);
  REAL_T_PTR(b, 75);
  REAL_T_PTR(logBeta, 75);
  REAL_T_PTR(t, 75);

  realAdd(x, const_1, a, realContext);
  realSubtract(y, x, b, realContext);
  realAdd(b, const_1, b, realContext);
  C47_LnBetaPositiveReal(a, b, logBeta);

  realAdd(y, const_1, t, realContext);
  WP34S_Ln(t, t, &ctxtReal75);
  realAdd(t, logBeta, result, realContext);
  realChangeSign(result);
}

/********************************************//**
 * \brief Evaluates log(P(y,x)) for finite 0 <= x <= y.
 ***********************************************/
static void logPyxPositiveReal(const real_t *y, const real_t *x, real_t *result, realContext_t *realContext) {
  REAL_T_PTR(xPlusOne, 75);
  REAL_T_PTR(logGamma, 75);

  logCyxPositiveReal(y, x, result, realContext);
  realAdd(x, const_1, xPlusOne, realContext);
  WP34S_LnGamma(xPlusOne, logGamma, &ctxtReal75);
  realAdd(result, logGamma, result, realContext);
}

static void cyxReal(real_t *y, real_t *x, real_t *result, realContext_t *realContext) {
  bool_t inputAreIntegers = (realIsAnInteger(x) && realIsAnInteger(y));

  logCyxPositiveReal(y, x, result, realContext);

  realExp(result, result, realContext);             // r = y! / ((y-x)! × x!)

  if(inputAreIntegers && !realIsAnInteger(result)) {
    realToIntegralValue(result, result, DEC_ROUND_HALF_UP, realContext);
  }
}

static void cyxLong(longInteger_t y, longInteger_t x, longInteger_t result) {
  if(longIntegerCompareInt(x, 0) == 0) {
    uInt32ToLongInteger(1u, result);
  }
  else if(longIntegerCompareInt(x, 400) <= 0) {
    uint32_t loops, counter;

    longIntegerToUInt32(x, loops);
    longIntegerSubtractUInt(y, --loops, result);
    longIntegerCopy(result, y);
    counter = 1;
    while(counter <= loops) {
      counter++;
      longIntegerAddUInt(y, 1, y);
      longIntegerMultiply(result, y, result);
      longIntegerDivideUInt(result, counter, result);
    }
  }
  else {
    real_t xReal, yReal, resultReal;

    convertLongIntegerToReal(x, &xReal, &ctxtReal75);
    convertLongIntegerToReal(y, &yReal, &ctxtReal75);

    realSubtract(&yReal, &xReal, &resultReal, &ctxtReal75);
    realAdd(&resultReal, const_1, &resultReal, &ctxtReal75);
    WP34S_LnGamma(&resultReal, &resultReal, &ctxtReal75);       // r = ln((y-x)!)

    realAdd(&xReal, const_1, &xReal, &ctxtReal75);
    WP34S_LnGamma(&xReal, &xReal, &ctxtReal75);                 // x = ln(x!)

    realAdd(&yReal, const_1, &yReal, &ctxtReal75);
    WP34S_LnGamma(&yReal, &yReal, &ctxtReal75);                 // y = ln(y!)

    realSubtract(&yReal, &resultReal, &resultReal, &ctxtReal75);
    realSubtract(&resultReal, &xReal, &resultReal, &ctxtReal75); // r = ln(y!) - ln((y-x)!) - ln(x!)

    realExp(&resultReal, &resultReal, &ctxtReal75);             // r = y! / ((y-x)! × x!)

    convertRealToLongInteger(&resultReal, result, DEC_ROUND_HALF_UP);
  }
}

/********************************************//**
 * \brief Evaluates log(C(y,x)) through log-Beta in its stable half-plane.
 *
 * \return true when the transformed arguments have positive real parts.
 ***********************************************/
static bool_t logCyxPositiveComplex(const cplx_t *y, const cplx_t *x,
                                    cplx_t *r, realContext_t *realContext) {
  cplx_t a;
  cplx_t b;
  cplx_t logBeta;
  cplx_t t;

  realAdd(&x->Real, const_1, &a.Real, realContext);
  realCopy(&x->Imag, &a.Imag);
  realSubtract(&y->Real, &x->Real, &b.Real, realContext);
  realAdd(&b.Real, const_1, &b.Real, realContext);
  realSubtract(&y->Imag, &x->Imag, &b.Imag, realContext);
  if(realIsSpecial(&a.Real) || realIsSpecial(&a.Imag) || realIsSpecial(&b.Real) || realIsSpecial(&b.Imag) ||
     !realCompareGreaterThan(&a.Real, const_0) || !realCompareGreaterThan(&b.Real, const_0)) {
    return false;
  }

  C47_LnBetaPositiveComplex(&a, &b, &logBeta);
  realAdd(&y->Real, const_1, &t.Real, realContext);
  realCopy(&y->Imag, &t.Imag);
  lnComplex(&t, &t, realContext);
  realAdd(&t.Real, &logBeta.Real, &r->Real, realContext);
  realAdd(&t.Imag, &logBeta.Imag, &r->Imag, realContext);
  realChangeSign(&r->Real);
  realChangeSign(&r->Imag);
  return true;
}

static void cyxCplx(cplx_t *y, cplx_t *x, cplx_t *r, realContext_t *realContext) {
  if(logCyxPositiveComplex(y, x, r, realContext)) {
    expComplex(r, r, realContext);
    return;
  }

  realSubtract(&y->Real, &x->Real, &r->Real, realContext);       // r = y - x
  realSubtract(&y->Imag, &x->Imag, &r->Imag, realContext);

  realAdd(&r->Real, const_1, &r->Real, realContext);             // r = t + 1
  WP34S_ComplexLnGamma(r, r, realContext);                       // r = lnGamma(t + 1) = ln((y - x)!)

  realAdd(&x->Real, const_1, &x->Real, realContext);             // x = x + 1
  WP34S_ComplexLnGamma(x, x, realContext);                       // x = lnGamma(x + 1) = ln(x!)

  realAdd(&y->Real, const_1, &y->Real, realContext);             // y = y + 1
  WP34S_ComplexLnGamma(y, y, realContext);                       // y = lnGamma(y + 1) = ln(y!)

  realSubtract(&y->Real, &r->Real, &r->Real, realContext);       // r = ln(y!) - ln((y - x)!)
  realSubtract(&y->Imag, &r->Imag, &r->Imag, realContext);

  realSubtract(&r->Real, &x->Real, &r->Real, realContext);       // r = ln(y!) - ln((y - x)!) - ln(x!)
  realSubtract(&r->Imag, &x->Imag, &r->Imag, realContext);

  expComplex(r, r, realContext);                                 // r = y! / ((y-x)! × x!)
}

static void pyxReal(real_t *y, real_t *x, real_t *result, realContext_t *realContext) {
  bool_t inputAreIntegers = (realIsAnInteger(x) && realIsAnInteger(y));

  logPyxPositiveReal(y, x, result, realContext);
  realExp(result, result, realContext);

  if(inputAreIntegers && !realIsAnInteger(result)) {
    realToIntegralValue(result, result, DEC_ROUND_HALF_UP, realContext);
  }
}

static void pyxLong(longInteger_t y, longInteger_t x, longInteger_t result) {
  if(longIntegerCompareInt(x, 0) == 0) {
    uInt32ToLongInteger(1u, result);
  }
  else if(longIntegerCompareInt(x, 400) <= 0) {
    uint32_t loops;

    longIntegerToUInt32(x, loops);
    longIntegerSubtractUInt(y, --loops, result);
    longIntegerCopy(result, y);
    while(loops-- > 0) {
      longIntegerAddUInt(y, 1, y);
      longIntegerMultiply(result, y, result);
    }
  }
  else {
    real_t xReal, yReal, resultReal;

    convertLongIntegerToReal(x, &xReal, &ctxtReal75);
    convertLongIntegerToReal(y, &yReal, &ctxtReal75);

    realSubtract(&yReal, &xReal, &resultReal, &ctxtReal75);
    realAdd(&resultReal, const_1, &resultReal, &ctxtReal75);
    WP34S_LnGamma(&resultReal, &resultReal, &ctxtReal75);        // r = ln((y-x)!)

    realAdd(&yReal, const_1, &yReal, &ctxtReal75);
    WP34S_LnGamma(&yReal, &yReal, &ctxtReal75);                  // y = ln(y!)

    realSubtract(&yReal, &resultReal, &resultReal, &ctxtReal75); // r = ln(y!) - ln((y-x)!)

    realExp(&resultReal, &resultReal, &ctxtReal75);              // r = y! / (y-x)!

    convertRealToLongInteger(&resultReal, result, DEC_ROUND_HALF_UP);
  }
}

static void pyxCplx(cplx_t *y, cplx_t *x, cplx_t *r, realContext_t *realContext) {
  if(logCyxPositiveComplex(y, x, r, realContext)) {
    cplx_t xPlusOne;
    cplx_t logGamma;

    realAdd(&x->Real, const_1, &xPlusOne.Real, realContext);
    realCopy(&x->Imag, &xPlusOne.Imag);
    WP34S_ComplexLnGamma(&xPlusOne, &logGamma, realContext);
    realAdd(&r->Real, &logGamma.Real, &r->Real, realContext);
    realAdd(&r->Imag, &logGamma.Imag, &r->Imag, realContext);
    expComplex(r, r, realContext);
    return;
  }

  realSubtract(&y->Real, &x->Real, &r->Real, realContext);       // r = y - x
  realSubtract(&y->Imag, &x->Imag, &r->Imag, realContext);

  realAdd(&r->Real, const_1, &r->Real, realContext);             // r = t + 1
  WP34S_ComplexLnGamma(r, r, realContext);                       // r = lnGamma(t + 1) = ln((y - x)!)

  realAdd(&y->Real, const_1, &y->Real, realContext);             // y = y + 1
  WP34S_ComplexLnGamma(y, y, realContext);                       // y = lnGamma(y + 1) = ln(y!)

  realSubtract(&y->Real, &r->Real, &r->Real, realContext);       // r = ln(y!) - ln((y - x)!)
  realSubtract(&y->Imag, &r->Imag, &r->Imag, realContext);

  expComplex(r, r, realContext);                                 // r = y! / (y-x)!
}

//=============================================================================
// Main function
//-----------------------------------------------------------------------------


//=============================================================================
// Cyx/Pyx(Y(long integer), *)
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief Cyx/Pyx(Y(long integer), X(long integer)) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
static void cpyxLonI(uint16_t combOrPerm) {
  longInteger_t x, y;

  if(!getRegisterAsLongInt(REGISTER_X, x, NULL)) {
    goto end1;
  }
  if(!getRegisterAsLongInt(REGISTER_Y, y, NULL)) {
    goto end2;
  }

  if(longIntegerIsNegative(x) || longIntegerIsNegative(y) || longIntegerCompare(y, x) < 0) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    EXTRA_INFO_MESSAGE("cpyxLonILonI:", "cannot calculate Cyx/Pyx, conditions: x>=0, y>=0, and x<=y.");
  }
  else {
    longInteger_t t;
    longIntegerInit(t);

    (combOrPerm == CP_COMBINATION) ? cyxLong(y, x, t)
                                   : pyxLong(y, x, t);

    convertLongIntegerToLongIntegerRegister(t, REGISTER_X);
    longIntegerFree(t);
  }

end2:
  longIntegerFree(y);
end1:
  longIntegerFree(x);
}

//=============================================================================
// Cyx/Pyx(Y(real34), *)
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief Cyx/Pyx(Y(real34), X(real34)) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
static void cpyxReal(uint16_t combOrPerm) {
  real_t x, y;

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y)) {
    return;
  }

  if(realIsSpecial(&x) || realIsSpecial(&y)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
      return;
    }
    REAL_T_PTR(t, 39);
    if(realIsInfinite(&y) && realIsPositive(&y) && !realIsSpecial(&x) && !realIsNegative(&x)) {
      if(realIsZero(&x)) {
        realSetOne(t);
      }
      else {
        realSetPlusInfinity(t);
      }
    }
    else {
      realSetNaN(t);
    }
    convertRealToResultRegister(t, REGISTER_X, amNone);
    return;
  }

  if(realIsNegative(&x) || realIsNegative(&y) || realCompareGreaterThan(&x, &y)) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    EXTRA_INFO_MESSAGE("cpyxRealReal:", "cannot calculate Cyx/Pyx, conditions: x>=0, y>=0, and x<=y.");
  }
  else {
    real_t t;

    (combOrPerm == CP_COMBINATION) ? cyxReal(&y, &x, &t, &ctxtReal39)
                                   : pyxReal(&y, &x, &t, &ctxtReal39);

    convertRealToResultRegister(&t, REGISTER_X, amNone);
  }
}

//=============================================================================
// Cyx/Pyx(Y(complex34), *)
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief Cyx/Pyx(Y(complex34), X(complex34)) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
static void cpyxCplx(uint16_t combOrPerm) {
  cplx_t x, y;
  cplx_t t;

  if(!getRegisterAsComplex(REGISTER_X, &x) || !getRegisterAsComplex(REGISTER_Y, &y)) {
    return;
  }

  (combOrPerm == CP_COMBINATION) ? cyxCplx(&y, &x, &t, &ctxtReal75)
                                 : pyxCplx(&y, &x, &t, &ctxtReal75);

  convertComplexToResultRegister(&t, REGISTER_X);
}

//=============================================================================
// Cyx/Pyx(Y(short integer), *)
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief Cyx/Pyx(Y(short integer), X(short integer)) ==> X(short integer)
 *
 * \param void
 * \return void
 ***********************************************/
static void cpyxShoI(uint16_t combOrPerm) {
  longInteger_t x, y;

  if(!getRegisterAsLongInt(REGISTER_X, x, NULL)) {
    goto end1;
  }
  if(!getRegisterAsLongInt(REGISTER_Y, y, NULL)) {
    goto end2;
  }

  if(longIntegerIsNegative(x) || longIntegerIsNegative(y) || longIntegerCompare(y, x) < 0) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    EXTRA_INFO_MESSAGE("cpyxShoI:", "cannot calculate Cyx/Pyx, y and x must be greater or equal than zero.");
  }
  else {
    longInteger_t t;

    longIntegerInit(t);
    (combOrPerm == CP_COMBINATION) ? cyxLong(y, x, t)
                                   : pyxLong(y, x, t);

    convertLongIntegerToShortIntegerRegister(t, getRegisterShortIntegerBase(REGISTER_Y), REGISTER_X);

    longIntegerFree(x); // Because convertShortIntegerRegisterToLongInteger reinits the long integer
    convertShortIntegerRegisterToLongInteger(REGISTER_X, x);
    if(longIntegerCompare(t, x) != 0) {
      setSystemFlag(FLAG_OVERFLOW);
    }

    longIntegerFree(t);
  }

end2:
  longIntegerFree(y);
end1:
  longIntegerFree(x);
}


static void doCyxReal(void) {
  cpyxReal(CP_COMBINATION);
}

static void doPyxReal(void) {
  cpyxReal(CP_PERMUTATION);
}

static void doCyxCplx(void) {
  cpyxCplx(CP_COMBINATION);
}

static void doPyxCplx(void) {
  cpyxCplx(CP_PERMUTATION);
}

static void doCyxShoI(void) {
  cpyxShoI(CP_COMBINATION);
}

static void doPyxShoI(void) {
  cpyxShoI(CP_PERMUTATION);
}

static void doCyxLonI(void) {
  cpyxLonI(CP_COMBINATION);
}

static void doPyxLonI(void) {
  cpyxLonI(CP_PERMUTATION);
}

/********************************************//**
 * \brief regX ==> regL and Cyx(regX, RegY) ==> regX
 * enables stack lift and refreshes the stack.
 * C(n,k) = n! / [k! * (n-k)!]
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCyx(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(&doCyxReal, &doCyxCplx, &doCyxShoI, &doCyxLonI);
/*
  if(!saveLastX()) {
    return;
  }

  cpyx[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)](CP_COMBINATION);

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
  adjustResult(REGISTER_Y, true, true, REGISTER_Y, -1, -1);*/
}

/********************************************//**
 * \brief regX ==> regL and Pyx(regX, RegY) ==> regX
 * enables stack lift and refreshes the stack.
 * P(n,k) = n! / (n-k)!
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnPyx(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(&doPyxReal, &doPyxCplx, &doPyxShoI, &doPyxLonI);
}

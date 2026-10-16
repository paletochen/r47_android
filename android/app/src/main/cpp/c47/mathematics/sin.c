// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sin.c
 ***********************************************/

#include "c47.h"

/********************************************//**
 * \brief Multiply trigonometric and hyperbolic factors, preserving exact zeros with an infinite factor.
 *
 * \param[in] trigonometric Trigonometric factor
 * \param[in] hyperbolic Hyperbolic factor
 * \param[out] res Product
 * \param[in] realContext Working context
 * \return void
 ***********************************************/
void multiplyTrigByHyperbolic(const real_t *trigonometric, const real_t *hyperbolic, real_t *res, realContext_t *realContext) {
  if(realIsZero(trigonometric)) {
    realCopy(trigonometric, res);
    if(!realIsNaN(hyperbolic) && realIsNegative(hyperbolic)) {
      realChangeSign(res);
    }
  }
  else if(realIsZero(hyperbolic)) {
    realCopy(hyperbolic, res);
    if(!realIsNaN(trigonometric) && realIsNegative(trigonometric)) {
      realChangeSign(res);
    }
  }
  else if(!realIsInfinite(hyperbolic)) {
    realMultiply(trigonometric, hyperbolic, res, realContext);
  }
  else if(realIsNaN(trigonometric)) {
    realSetNaN(res);
  }
  else {
    realSetPlusInfinity(res);
    if(realIsNegative(trigonometric) != realIsNegative(hyperbolic)) {
      realSetNegativeSign(res);
    }
  }
}

void sinComplex(const cplx_t *z, cplx_t *res, realContext_t *realContext) {
  // sin(a + ib) = sin(a)*cosh(b) + i*cos(a)*sinh(b)
  real_t sina, cosa, sinhb, coshb;

  C47_WP34S_Cvt2RadSinCosTan(&z->Real, amRadian, &sina, &cosa, NULL, realContext);
  WP34S_SinhCosh(&z->Imag, &sinhb, &coshb, realContext);

  multiplyTrigByHyperbolic(&sina, &coshb, &res->Real, realContext);
  multiplyTrigByHyperbolic(&cosa, &sinhb, &res->Imag, realContext);
}


void sinCosReal(trigType_t trigType) {
  real_t x;
  const real_t *r = &x;
  angularMode_t xAngularMode;

  if(!getRegisterAsRealAngle(REGISTER_X, &x, &xAngularMode, ifLongIntegerDoAngleReduction)) {
    return;
  }

  if(realIsSpecial(&x)) {
    r = const_NaN;
  }
  else if(trigType == trigSin && xAngularMode == amRadian && !realIsZero(&x) && realGetExponent(&x) < -18) {   // sin x = x - x³/6 to 72 digits, below |x| by less than half a unit at 34 digits
    WP34S_TinyOddSeries(&x, const_6, false, &x, &ctxtReal75);
  }
  else {
    if(trigType == trigSin) {
      C47_WP34S_Cvt2RadSinCosTan(r = &x, xAngularMode, &x,   NULL, NULL, &ctxtReal34);
    }
    else {
      C47_WP34S_Cvt2RadSinCosTan(r = &x, xAngularMode, NULL, &x,   NULL, &ctxtReal34);
    }
  }
  convertRealToResultRegister(r, REGISTER_X, amNone);
}


void sinCosCplx(trigType_t trigType) {
  cplx_t z;

  if(!getRegisterAsComplex(REGISTER_X, &z)) {
    return;
  }

  if(trigType == trigSin) {
    sinComplex(&z, &z, &ctxtReal39);
  }
  else {
    cosComplex(&z, &z, &ctxtReal39);
  }

  convertComplexToResultRegister(&z, REGISTER_X);
}


static void sinReal(void) {
  sinCosReal(trigSin);
}


static void sinCplx(void) {
  sinCosCplx(trigSin);
}


/********************************************//**
 * \brief regX ==> regL and sin(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSin(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&sinReal, &sinCplx);
}

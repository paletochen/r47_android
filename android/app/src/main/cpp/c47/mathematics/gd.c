// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file gd.c
 ***********************************************/

#include "c47.h"

static void gdError(bool_t gd, uint8_t errorCode) {
  displayCalcErrorMessage(errorCode, ERR_REGISTER_LINE, REGISTER_X);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    if(gd) {
      sprintf(errorMessage, "cannot calculate gd(%s)", getRegisterDataTypeName(REGISTER_X, false, false));
      moreInfoOnError("In function fnGd:", errorMessage, NULL, NULL);
    }
    else {
      sprintf(errorMessage, "cannot calculate invGd(%s)", getRegisterDataTypeName(REGISTER_X, false, false));
      moreInfoOnError("In function fnInvGd:", errorMessage, NULL, NULL);
    }
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
}

static void gdReal(bool_t gd) {
  real_t x;
  uint8_t errorCode;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  errorCode = gd ? GudermannianReal(&x, &x, &ctxtReal39)
                 : InverseGudermannianReal(&x, &x, &ctxtReal39);

  if(errorCode != ERROR_NONE) {
    gdError(gd, errorCode);
  }
  else {
    convertRealToResultRegister(&x, REGISTER_X, amNone);
  }
}

static void gdCplx(bool_t gd) {
  real_t xReal, xImag;
  uint8_t errorCode;

  if(!getRegisterAsComplex(REGISTER_X, &xReal, &xImag))
    return;

  errorCode = gd ? GudermannianComplex(&xReal, &xImag, &xReal, &xImag, &ctxtReal39)
                 : InverseGudermannianComplex(&xReal, &xImag, &xReal, &xImag, &ctxtReal39);

  if(errorCode != ERROR_NONE) {
    gdError(gd, errorCode);
  }
  else {
    convertComplexToResultRegister(&xReal, &xImag, REGISTER_X);
  }
}

uint8_t GudermannianReal(const real_t *x, real_t *res, realContext_t *realContext) {
  if(realIsInfinite(x)) {
    if(realIsPositive(x)) {
      realCopy(const_piOn2, res);
    }
    else {
      realCopy(const_piOn2, res);
      realChangeSign(res);
    }
  }
  else {
    /*
     * Gd(x) = 2 * Arctan(Exp(x)) - PI/2
     */
    realExp(x, res, realContext);
    WP34S_Atan(res, res, realContext);
    realMultiply(res, const_2, res, realContext);
    realSubtract(res, const_piOn2, res, realContext);

    /*
     * Gd(x) = ArchSin(Tanh(x))
     */
    //WP34S_Tanh(x, res, realContext);
    //WP34S_Asin(res, res, realContext);
  }

  return ERROR_NONE;
}

uint8_t GudermannianComplex(const real_t *xReal, const real_t *xImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  /*
   * This implementation provides same results as Mathematica.
   * Gd(x) = 2 * Arctan(Exp(x)) - PI/2
   */
  expComplex(xReal, xImag, resReal, resImag, realContext);
  ArctanComplex(resReal, resImag, resReal,resImag, realContext);

  realMultiply(resReal, const_2, resReal, realContext);
  realMultiply(resImag, const_2, resImag, realContext);
  realSubtract(resReal, const_piOn2, resReal, realContext);

  /*
   * Gd(x) = ArchSin(Tanh(x))
   */
  //TanhComplex(xReal, xImag, resReal, resImag, realContext);
  //ArcsinComplex(resReal, resImag, resReal, resImag, realContext);

  return ERROR_NONE;
}

uint8_t InverseGudermannianReal(const real_t *x, real_t *res, realContext_t *realContext) {
  uint8_t result = ERROR_NONE;

  /*
   * InvGd(x) = Ln(Tan(x/2 + PI/4))
   */
  if(!realIsNaN(x) && realCompareAbsLessThan(x, const_piOn2)) {
    if(realIsZero(x)) {
      realCopy(const_0, res);
    }
    else {
      real_t sin, cos;

      /*
       * InvGd(x) = Ln(Tan(x/2 + PI/4))
       * -PI/2 < x < PI/2
       */
      realMultiply(x, const_1on2, res, realContext);       // r = x/2
      realAdd(res, const_piOn4, res, realContext);    // r = x/2 + pi/4
      WP34S_Cvt2RadSinCosTan(res, amRadian, &sin, &cos, res, &ctxtReal39); // r = Tan(x/2 + pi/4)
      WP34S_Ln(res, res, &ctxtReal39);                // r = Ln(Tan(x/2 + pi/4))

      /*
       * InvGd(x) = ArcSinh(Tan(x))
       * -PI/2 < x < PI/2
       */
      //WP34S_Cvt2RadSinCosTan(x, amRadian, &sin, &cos, res, &ctxtReal39);
      //ArcsinhReal(res, res, &ctxtReal39);
    }
  }
  else {
    result = ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN;
  }

  return result;
}

uint8_t InverseGudermannianComplex(const real_t *xReal, const real_t *xImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  /*
   * This implementation provides same results as Mathematica.
   * InvGd(x) = Ln(Tan(x / 2 + PI / 4))
   */
  realMultiply(xReal, const_1on2, resReal, realContext);               // r = x/2
  realMultiply(xImag, const_1on2, resImag, realContext);

  realAdd(xReal, const_piOn4, resReal, realContext);              // r = x/2 + pi/2

  TanComplex(resReal, resImag, resReal, resImag, realContext);    // r = Tan(x/2 + pi/4)
  lnComplex(resReal, resImag, resReal, resImag, realContext);     // r = Ln(Tan(x/2 + pi/4))

  /*
   * InvGd(x) = ArcSinh(Tan(x))
   */
  //real_t tReal, tImag;
  //
  //TanComplex(xReal, xImag, &tReal, &tImag, realContext);
  //ArcsinhComplex(&tReal, &tImag, resReal, resImag, realContext);

  return ERROR_NONE;
}

static void doGdReal(void) {
  gdReal(true);
}

static void doInvGdReal(void) {
  gdReal(false);
}

static void doGdCplx(void) {
  gdCplx(true);
}

static void doInvGdCplx(void) {
  gdCplx(false);
}

/********************************************//**
 * \brief regX ==> regL and gd(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnGd(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&doGdReal, &doGdCplx);
}

/********************************************//**
 * \brief regX ==> regL and invGd(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnInvGd(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&doInvGdReal, &doInvGdCplx);
}

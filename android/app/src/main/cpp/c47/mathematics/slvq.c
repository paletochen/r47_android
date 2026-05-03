// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file slvq.c
 ***********************************************/

#include "c47.h"

#undef DISCRIMINANT //Note the testSuite tests were revised to remove the discriminant

/********************************************//**
 * \brief (c, b, a) ==> (x1, x2, r) c ==> regL
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSlvq(uint16_t unusedButMandatoryParameter) {
#if !defined(SAVE_SPACE_DM42_12)
  bool_t realCoefs=false, realRoots=true, complexCoefs=false;
  real_t aReal, bReal, cReal, rReal, x1Real, x2Real;
  real_t aImag, bImag, cImag, rImag, x1Imag, x2Imag;

  if(!(getRegisterAsComplexOrReal(REGISTER_X, &cReal, &cImag, &complexCoefs) &&
       getRegisterAsComplexOrReal(REGISTER_Y, &bReal, &bImag, &complexCoefs) &&
       getRegisterAsComplexOrReal(REGISTER_Z, &aReal, &aImag, &complexCoefs))) {
    return;
  }
  realCoefs = !complexCoefs;

  if(   realIsZero(&aReal) && realIsZero(&aImag) && realIsZero(&bReal) && realIsZero(&bImag)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnSlvq:", "cannot use 0 for Y and Z as input of SLVQ", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(!saveLastX()) {
    return;
  }

  if(realCoefs == false) {
    realRoots = false;
  }

#if defined(OPTION_SQUARE_159)
    realContext_t c = ctxtReal75;
    c.digits = 159;
    REAL_T_PTR(x1r, 159);
    REAL_T_PTR(x1i, 159);
    REAL_T_PTR(x2r, 159);
    REAL_T_PTR(x2i, 159);
    REAL_T_PTR(r0r, 159);
    REAL_T_PTR(r0i, 159);
    REAL_T_PTR(aRealH, 159);
    REAL_T_PTR(aImagH, 159);
    REAL_T_PTR(bRealH, 159);
    REAL_T_PTR(bImagH, 159);
    REAL_T_PTR(cRealH, 159);
    REAL_T_PTR(cImagH, 159);

    realPlus(&aReal, aRealH, &c);
    realPlus(&aImag, aImagH, &c);
    realPlus(&bReal, bRealH, &c);
    realPlus(&bImag, bImagH, &c);
    realPlus(&cReal, cRealH, &c);
    realPlus(&cImag, cImagH, &c);
    realSetZero(r0r);
    realSetZero(r0i);
    realSetZero(x1r);
    realSetZero(x1i);
    realSetZero(x2r);
    realSetZero(x2i);
    solveQuadraticEquation159(aRealH, aImagH, bRealH, bImagH, cRealH, cImagH, r0r, r0i, x1r, x1i, x2r, x2i, &c);
    realPlus(r0r, &rReal,  &ctxtReal39);
    realPlus(r0i, &rImag,  &ctxtReal39);
    realPlus(x1r, &x1Real, &ctxtReal39);
    realPlus(x1i, &x1Imag, &ctxtReal39);
    realPlus(x2r, &x2Real, &ctxtReal39);
    realPlus(x2i, &x2Imag, &ctxtReal39);
#else // OPTION_SQUARE_159
  solveQuadraticEquation(&aReal, &aImag, &bReal, &bImag, &cReal, &cImag, &rReal, &rImag, &x1Real, &x1Imag, &x2Real, &x2Imag, &ctxtReal75);
#endif // OPTION_SQUARE_159

  realRoots &= realIsZero(&x1Imag) && realIsZero(&x2Imag);

  if(realRoots) {
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
    #if defined(DISCRIMINANT)
      reallocateRegister(REGISTER_Z, dtReal34, 0, amNone);
    #endif //DISCRIMINANT
    convertRealToReal34ResultRegister(&x1Real, REGISTER_X);
    convertRealToReal34ResultRegister(&x2Real, REGISTER_Y);
    #if defined(DISCRIMINANT)
      realToReal34(&rReal,  REGISTER_REAL34_DATA(REGISTER_Z));
    #endif //DISCRIMINANT
  }
  else { // !realRoots
    if(realIsZero(&x1Imag)) { // x1 is real
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      convertRealToReal34ResultRegister(&x1Real, REGISTER_X);
    }
    else {
      reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
      convertComplexToResultRegister(&x1Real, &x1Imag, REGISTER_X);
    }

    if(realIsZero(&x2Imag)) { // x2 is real
      reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
      convertRealToReal34ResultRegister(&x2Real, REGISTER_Y);
    }
    else {
      reallocateRegister(REGISTER_Y, dtComplex34, 0, amNone);
      convertComplexToResultRegister(&x2Real, &x2Imag, REGISTER_Y);
    }

    #if defined(DISCRIMINANT)
      if(realIsZero(&rImag)) { // r is real
        reallocateRegister(REGISTER_Z, dtReal34, 0, amNone);
        convertRealToReal34ResultRegister(&rReal, REGISTER_Z);
      }
      else {
        reallocateRegister(REGISTER_Z, dtComplex34, 0, amNone);
        convertComplexToResultRegister(&rReal, &rImag, REGISTER_Z);
      }
    #endif //DISCRIMINANT
  }

  temporaryInformation = TI_ROOTS2;
  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
  adjustResult(REGISTER_Y, false, true, REGISTER_Y, -1, -1);
  #if defined(DISCRIMINANT)
    adjustResult(REGISTER_Z, false, true, REGISTER_Z, -1, -1);
  #else
    fnDropZ(0);
  #endif //DISCRIMINANT
#endif // !SAVE_SPACE_DM42_12
}


void solveQuadraticEquation(const real_t *aReal, const real_t *aImag, const real_t *bReal, const real_t *bImag, const real_t *cReal, const real_t *cImag, real_t *rReal, real_t *rImag, real_t *x1Real, real_t *x1Imag, real_t *x2Real, real_t *x2Imag, realContext_t *realContext) {
  bool_t realCoefs = realIsZero(aImag) && realIsZero(bImag) && realIsZero(cImag);

  if(realCoefs) {
    if(realIsZero(aReal)) {
      // bx + c = 0   (b is not 0 here)

      // r = b²
      realMultiply(bReal, bReal, rReal, realContext);
      realSetZero(rImag);

      // x1 = -c/b, x2 = NaN
      realDivide(cReal, bReal, x1Real, realContext);
      realChangeSign(x1Real);
      realSetNaN(x2Real);

      realSetZero(x1Imag);
      realSetZero(x2Imag);
    }
    else if(realIsZero(cReal)) {
      // ax² + bx = x(ax + b) = 0   (a is not 0 here)

      // r = b²
      realMultiply(bReal, bReal, rReal, realContext);
      realSetZero(rImag);

      // x1 = 0
      realSetZero(x1Real);

      // x2 = -b/a
      realDivide(bReal, aReal, x2Real, realContext);
      realChangeSign(x2Real);

      realSetZero(x1Imag);
      realSetZero(x2Imag);
    }
    else {
      // ax² + bx + c = 0   (a and c are not 0 here)

      // r = b² - 4ac
      realMultiply(const__4, aReal, rReal, realContext);

      // Fix potential aliasing in realMultiply, not sure if it 100% helps but I keep it
      real_t temp_multiply;
      realMultiply(cReal, rReal, &temp_multiply, realContext);
      realCopy(&temp_multiply, rReal);

      // Fix potential aliasing in realFMA, not sure if it 100% helps but I keep it
      real_t temp_result;
      realFMA(bReal, bReal, rReal, &temp_result, realContext);
      realCopy(&temp_result, rReal);
      realSetZero(rImag);

      if(realIsPositive(rReal)) { // real roots
        // x1 = (-b - sign(b)*sqrt(r)) / 2a
        realSquareRoot(rReal, x1Real, realContext);
        if(realIsPositive(bReal)) {
          realChangeSign(x1Real);
        }
        realSubtract(x1Real, bReal, x1Real, realContext);
        realMultiply(x1Real, const_1on2, x1Real, realContext);
        realDivide(x1Real, aReal, x1Real, realContext);

        // x2 = c / ax1  (x1 connot be 0 here)
        realDivide(cReal, aReal, x2Real, realContext);
        realDivide(x2Real, x1Real, x2Real, realContext);

        realSetZero(x1Imag);
        realSetZero(x2Imag);
      }
      else { // complex roots
        // x1 = (-b - sign(b)*sqrt(r)) / 2a
        realMinus(rReal, x1Real, realContext);
        realSquareRoot(x1Real, x1Imag, realContext);
        realSetZero(x1Real);
        if(realIsPositive(bReal)) {
          realChangeSign(x1Imag);
        }
        realSubtract(x1Real, bReal, x1Real, realContext);
        realSubtract(x1Imag, bImag, x1Imag, realContext);
        realMultiply(x1Real, const_1on2, x1Real, realContext);
        realMultiply(x1Imag, const_1on2, x1Imag, realContext);
        realDivide(x1Real, aReal, x1Real, realContext);
        realDivide(x1Imag, aReal, x1Imag, realContext);

        // x2 = conj(x1)
        realCopy(x1Real, x2Real);
        realCopy(x1Imag, x2Imag);
        realChangeSign(x2Imag);
      }
    }
  }
  else { // Complex coefficients
    if(realIsZero(aReal) && realIsZero(aImag)) {
      // bx + c = 0   (b is not 0 here)

      // r = b²
      mulComplexComplex(bReal, bImag, bReal, bImag, rReal, rImag, realContext);

      // x1 = -c/b, x2 = NaN
      divComplexComplex(cReal, cImag, bReal, bImag, x1Real, x1Imag, realContext);
      realChangeSign(x1Real);
      realChangeSign(x1Imag);
      realSetNaN(x2Real);
      realSetNaN(x2Imag);
    }
    else if(realIsZero(cReal) && realIsZero(cImag)) {
      // ax² + bx = x(ax + b) = 0   (a is not 0 here)

      // r = b²
      mulComplexComplex(bReal, bImag, bReal, bImag, rReal, rImag, realContext);

      // x1 = 0
      realSetZero(x1Real);
      realSetZero(x1Imag);

      // x2 = -b/a
      divComplexComplex(bReal, bImag, aReal, aImag, x2Real, x2Imag, realContext);
      realChangeSign(x2Real);
      realChangeSign(x2Imag);
    }
    else {
      // ax² + bx + c = 0   (a and c are not 0 here)

      // r = b² - 4ac
      realMultiply(const__4, aReal, rReal, realContext);
      realMultiply(const__4, aImag, rImag, realContext);
      mulComplexComplex(cReal, cImag, rReal, rImag, rReal, rImag, realContext);
      mulComplexComplex(bReal, bImag, bReal, bImag, x1Real, x1Imag, realContext);
      realAdd(x1Real, rReal, rReal, realContext);
      realAdd(x1Imag, rImag, rImag, realContext);

      // x1 = (-b + sqrt(r)) / 2a
      realCopy(rReal, x1Real);
      realCopy(rImag, x1Imag);
      realRectangularToPolar(x1Real, x1Imag, x1Real, x1Imag, realContext);
      realSquareRoot(x1Real, x1Real, realContext);
      realMultiply(x1Imag, const_1on2, x1Imag, realContext);
      realPolarToRectangular(x1Real, x1Imag, x1Real, x1Imag, realContext);

      realSubtract(x1Real, bReal, x1Real, realContext);
      realSubtract(x1Imag, bImag, x1Imag, realContext);
      realMultiply(x1Real, const_1on2, x1Real, realContext);
      realMultiply(x1Imag, const_1on2, x1Imag, realContext);
      divComplexComplex(x1Real, x1Imag, aReal, aImag, x1Real, x1Imag, realContext);

      // x2 = c / ax1  (x1 connot be 0 here)
      divComplexComplex(cReal, cImag, aReal, aImag, x2Real, x2Imag, realContext);
      divComplexComplex(x2Real, x2Imag, x1Real, x1Imag, x2Real, x2Imag, realContext);
    }
  }
}


#if defined(OPTION_SQUARE_159) || defined(OPTION_EIGEN_159)
void solveQuadraticEquation159(const real_t *aReal, const real_t *aImag, const real_t *bReal, const real_t *bImag, const real_t *cReal, const real_t *cImag, real_t *rReal, real_t *rImag, real_t *x1Real, real_t *x1Imag, real_t *x2Real, real_t *x2Imag, realContext_t *realContext) {

  bool_t realCoefs = realIsZero(aImag) && realIsZero(bImag) && realIsZero(cImag);

  if(realCoefs) {
    // All coefficients are real - use 159-digit precision
    REAL_T_PTR(rR, 159);
    REAL_T_PTR(a_h, 159);
    REAL_T_PTR(b_h, 159);
    REAL_T_PTR(c_h, 159);
    REAL_T_PTR(temp, 159);
    REAL_T_PTR(sqrt_r, 159);
    realSetZero(rR);
    realSetZero(a_h);
    realSetZero(b_h);
    realSetZero(c_h);
    realSetZero(temp);
    realSetZero(sqrt_r);

    realCopy(aReal, a_h);
    realCopy(bReal, b_h);
    realCopy(cReal, c_h);

    if(realIsZero(aReal)) {
      // bx + c = 0   (b is not 0 here)

      // r = b²
      realMultiply(b_h, b_h, rReal, realContext);
      realSetZero(rImag);

      // x1 = -c/b, x2 = NaN
      realDivide(c_h, b_h, x1Real, realContext);
      realChangeSign(x1Real);
      realSetNaN(x2Real);
      realSetZero(x1Imag);
      realSetZero(x2Imag);
    }
    else if(realIsZero(cReal)) {
      // ax² + bx = x(ax + b) = 0   (a is not 0 here)

      // r = b²
      realMultiply(b_h, b_h, rReal, realContext);
      realSetZero(rImag);

      // x1 = 0
      realSetZero(x1Real);

      // x2 = -b/a
      realDivide(b_h, a_h, x2Real, realContext);
      realChangeSign(x2Real);

      realSetZero(x1Imag);
      realSetZero(x2Imag);
    }
    else {
      // ax² + bx + c = 0   (a and c are not 0 here)

      // r = b² - 4ac (using FMA for better precision)
      realMultiply(const_4, a_h, temp, realContext);
      realMultiply(c_h, temp, temp, realContext);
      realMinus(temp, temp, realContext);
      realFMA(b_h, b_h, temp, rR, realContext);

      realCopy(rR, rReal);
      realSetZero(rImag);

      if(!realIsNegative(rR)) {
        // Real roots (r ≥ 0, including double root when r = 0)
        realSquareRoot(rR, sqrt_r, realContext);

        // x1 = (-b - sign(b)*sqrt(r)) / 2a
        // Numerically stable formula to avoid cancellation
        realCopy(sqrt_r, temp);
        if(!realIsNegative(b_h)) {
          realChangeSign(temp);
        }

        REAL_T_PTR(neg_b, 159);
        realMinus(b_h, neg_b, realContext);
        realAdd(neg_b, temp, temp, realContext);
        realMultiply(temp, const_1on2, temp, realContext);
        realDivide(temp, a_h, x1Real, realContext);

        // x2 = c / (a*x1)  (using Vieta's formula to avoid cancellation)
        realDivide(c_h, a_h, temp, realContext);
        realDivide(temp, x1Real, x2Real, realContext);

        realSetZero(x1Imag);
        realSetZero(x2Imag);
      }
      else {
        // Complex roots (r < 0)
        REAL_T_PTR(x1R_h, 159);
        REAL_T_PTR(x1I_h, 159);
        REAL_T_PTR(temp_sqrt, 159);
        REAL_T_PTR(temp_calc, 159);
        realSetZero(x1R_h);
        realSetZero(x1I_h);
        realSetZero(temp_sqrt);
        realSetZero(temp_calc);

        // Compute sqrt(|r|)
        realCopy(rR, temp_sqrt);
        realChangeSign(temp_sqrt);  // temp_sqrt = -r = |r|
        realSquareRoot(temp_sqrt, sqrt_r, realContext);

        // Real part: x_real = -b / 2a
        realCopy(b_h, temp_calc);
        realChangeSign(temp_calc);  // temp_calc = -b
        realMultiply(temp_calc, const_1on2, temp_calc, realContext);
        realDivide(temp_calc, a_h, x1R_h, realContext);

        // Imaginary part: x_imag = ±sqrt(|r|) / 2a
        realCopy(sqrt_r, temp_calc);
        if(realIsPositive(b_h)) {
          realChangeSign(temp_calc);
        }
        realMultiply(temp_calc, const_1on2, temp_calc, realContext);
        realDivide(temp_calc, a_h, x1I_h, realContext);

        // Copy to output variables
        realCopy(x1R_h, x1Real);
        realCopy(x1I_h, x1Imag);

        // x2 = conj(x1)
        realCopy(x1Real, x2Real);
        realCopy(x1Imag, x2Imag);
        realChangeSign(x2Imag);
      }
    }
  }
  else {
      // Complex coefficients - use 159-digit precision
      REAL_T_PTR(rR, 159);
      REAL_T_PTR(rI, 159);
      REAL_T_PTR(temp1R, 159);
      REAL_T_PTR(temp1I, 159);
      REAL_T_PTR(temp2R, 159);
      REAL_T_PTR(temp2I, 159);
      REAL_T_PTR(sqrtR, 159);
      REAL_T_PTR(sqrtI, 159);
      realSetZero(rR);
      realSetZero(rI);
      realSetZero(temp1R);
      realSetZero(temp1I);
      realSetZero(temp2R);
      realSetZero(temp2I);
      realSetZero(sqrtR);
      realSetZero(sqrtI);

      REAL_T_PTR(a_h, 159);
      REAL_T_PTR(b_h, 159);
      REAL_T_PTR(c_h, 159);
      REAL_T_PTR(aI_h, 159);
      REAL_T_PTR(bI_h, 159);
      REAL_T_PTR(cI_h, 159);
      realCopy(aReal, a_h);
      realCopy(aImag, aI_h);
      realCopy(bReal, b_h);
      realCopy(bImag, bI_h);
      realCopy(cReal, c_h);
      realCopy(cImag, cI_h);

      if(realIsZero(aReal) && realIsZero(aImag)) {
        // bx + c = 0 => x = -c/b

        // r = b²
        mulComplexComplex(b_h, bI_h, b_h, bI_h, rReal, rImag, realContext);

        // x1 = -c/b, x2 = NaN
        divComplexComplex(c_h, cI_h, b_h, bI_h, x1Real, x1Imag, realContext);
        realChangeSign(x1Real);
        realChangeSign(x1Imag);
        realSetNaN(x2Real);
        realSetNaN(x2Imag);
      }
      else {
        // Discriminant: r = b² - 4ac
        mulComplexComplex(b_h, bI_h, b_h, bI_h, rR, rI, realContext);
        mulComplexComplex(a_h, aI_h, c_h, cI_h, temp1R, temp1I, realContext);
        mulComplexReal(temp1R, temp1I, const_4, temp1R, temp1I, realContext);
        realSubtract(rR, temp1R, rR, realContext);
        realSubtract(rI, temp1I, rI, realContext);

        realCopy(rR, rReal);
        realCopy(rI, rImag);

        // sqrt(r) using 159-digit precision
        sqrtComplex(rR, rI, sqrtR, sqrtI, realContext);

        // x1 = (-b + sqrt(r)) / (2a)
        realMinus(b_h, temp1R, realContext);
        realMinus(bI_h, temp1I, realContext);
        addComplex(temp1R, temp1I, sqrtR, sqrtI, temp1R, temp1I, realContext);
        mulComplexReal(a_h, aI_h, const_2, temp2R, temp2I, realContext);
        divComplexComplex(temp1R, temp1I, temp2R, temp2I, x1Real, x1Imag, realContext);

        // x2 = (-b - sqrt(r)) / (2a)
        realMinus(b_h, temp1R, realContext);
        realMinus(bI_h, temp1I, realContext);
        subComplex(temp1R, temp1I, sqrtR, sqrtI, temp1R, temp1I, realContext);
        divComplexComplex(temp1R, temp1I, temp2R, temp2I, x2Real, x2Imag, realContext);
      }
  }

  // Zero tiny imaginary parts that are below precision threshold
  int eff_exp;
  eff_exp = realGetExponent(x1Imag);
  if(eff_exp < -realContext->digits) {
    realSetZero(x1Imag);
  }
  eff_exp = realGetExponent(x2Imag);
  if(eff_exp < -realContext->digits) {
    realSetZero(x2Imag);
  }
}
#endif //OPTION_SQUARE_159 || OPTION_EIGEN_159

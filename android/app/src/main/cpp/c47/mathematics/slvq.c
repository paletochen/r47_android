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

  if(   realIsZero(&aReal) && realIsZero(&aImag)
     && realIsZero(&bReal) && realIsZero(&bImag)) {
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
    real159_t x1r, x1i, x2r, x2i, r0r, r0i;
    real159_t aRealH, aImagH, bRealH, bImagH, cRealH, cImagH;

    realPlus(&aReal, (real_t *)&aRealH, &c);
    realPlus(&aImag, (real_t *)&aImagH, &c);
    realPlus(&bReal, (real_t *)&bRealH, &c);
    realPlus(&bImag, (real_t *)&bImagH, &c);
    realPlus(&cReal, (real_t *)&cRealH, &c);
    realPlus(&cImag, (real_t *)&cImagH, &c);
    realZero((real_t*)&r0r);
    realZero((real_t*)&r0i);
    realZero((real_t*)&x1r);
    realZero((real_t*)&x1i);
    realZero((real_t*)&x2r);
    realZero((real_t*)&x2i);
    solveQuadraticEquation159((real_t*)&aRealH, (real_t*)&aImagH, (real_t*)&bRealH, (real_t*)&bImagH, (real_t*)&cRealH, (real_t*)&cImagH, (real_t*)&r0r, (real_t*)&r0i, (real_t*)&x1r, (real_t*)&x1i, (real_t*)&x2r, (real_t*)&x2i, &c);
    realPlus((real_t *)&r0r, &rReal,  &ctxtReal39);
    realPlus((real_t *)&r0i, &rImag,  &ctxtReal39);
    realPlus((real_t *)&x1r, &x1Real, &ctxtReal39);
    realPlus((real_t *)&x1i, &x1Imag, &ctxtReal39);
    realPlus((real_t *)&x2r, &x2Real, &ctxtReal39);
    realPlus((real_t *)&x2i, &x2Imag, &ctxtReal39);
#else // OPTION_SQUARE_159
  solveQuadraticEquation(&aReal, &aImag, &bReal, &bImag, &cReal, &cImag, &rReal, &rImag, &x1Real, &x1Imag, &x2Real, &x2Imag, &ctxtReal75);
#endif // OPTION_SQUARE_159

  realRoots &= realIsZero(&x1Imag) && realIsZero(&x2Imag);

  if(realRoots) {
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
    #ifdef DISCRIMINANT
      reallocateRegister(REGISTER_Z, dtReal34, 0, amNone);
    #endif //DISCRIMINANT
    convertRealToReal34ResultRegister(&x1Real, REGISTER_X);
    convertRealToReal34ResultRegister(&x2Real, REGISTER_Y);
    #ifdef DISCRIMINANT
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

    #ifdef DISCRIMINANT
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
  #ifdef DISCRIMINANT
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
      realZero(rImag);

      // x1 = -c/b, x2 = NaN
      realDivide(cReal, bReal, x1Real, realContext);
      realChangeSign(x1Real);
      realCopy(const_NaN, x2Real);

      realZero(x1Imag);
      realZero(x2Imag);
    }
    else if(realIsZero(cReal)) {
      // ax² + bx = x(ax + b) = 0   (a is not 0 here)

      // r = b²
      realMultiply(bReal, bReal, rReal, realContext);
      realZero(rImag);

      // x1 = 0
      realZero(x1Real);

      // x2 = -b/a
      realDivide(bReal, aReal, x2Real, realContext);
      realChangeSign(x2Real);

      realZero(x1Imag);
      realZero(x2Imag);
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
      realZero(rImag);

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

        realZero(x1Imag);
        realZero(x2Imag);
      }
      else { // complex roots
        // x1 = (-b - sign(b)*sqrt(r)) / 2a
        realMinus(rReal, x1Real, realContext);
        realSquareRoot(x1Real, x1Imag, realContext);
        realZero(x1Real);
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
      realCopy(const_NaN, x2Real);
      realCopy(const_NaN, x2Imag);
    }
    else if(realIsZero(cReal) && realIsZero(cImag)) {
      // ax² + bx = x(ax + b) = 0   (a is not 0 here)

      // r = b²
      mulComplexComplex(bReal, bImag, bReal, bImag, rReal, rImag, realContext);

      // x1 = 0
      realZero(x1Real);
      realZero(x1Imag);

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
    real159_t rR, a_h, b_h, c_h, temp, sqrt_r;
    realZero((real_t *)&rR); realZero((real_t *)&a_h);
    realZero((real_t *)&b_h); realZero((real_t *)&c_h);
    realZero((real_t *)&temp); realZero((real_t *)&sqrt_r);

    realCopy(aReal, (real_t *)&a_h);
    realCopy(bReal, (real_t *)&b_h);
    realCopy(cReal, (real_t *)&c_h);

    if(realIsZero(aReal)) {
      // bx + c = 0   (b is not 0 here)

      // r = b²
      realMultiply((real_t *)&b_h, (real_t *)&b_h, rReal, realContext);
      realZero(rImag);

      // x1 = -c/b, x2 = NaN
      realDivide((real_t *)&c_h, (real_t *)&b_h, x1Real, realContext);
      realChangeSign(x1Real);
      realCopy(const_NaN, x2Real);
      realZero(x1Imag);
      realZero(x2Imag);
    }
    else if(realIsZero(cReal)) {
      // ax² + bx = x(ax + b) = 0   (a is not 0 here)

      // r = b²
      realMultiply((real_t *)&b_h, (real_t *)&b_h, rReal, realContext);
      realZero(rImag);

      // x1 = 0
      realZero(x1Real);

      // x2 = -b/a
      realDivide((real_t *)&b_h, (real_t *)&a_h, x2Real, realContext);
      realChangeSign(x2Real);

      realZero(x1Imag);
      realZero(x2Imag);
    }
    else {
      // ax² + bx + c = 0   (a and c are not 0 here)

      // r = b² - 4ac (using FMA for better precision)
      realMultiply(const_4, (real_t *)&a_h, (real_t *)&temp, realContext);
      realMultiply((real_t *)&c_h, (real_t *)&temp, (real_t *)&temp, realContext);
      realMinus((real_t *)&temp, (real_t *)&temp, realContext);
      realFMA((real_t *)&b_h, (real_t *)&b_h, (real_t *)&temp, (real_t *)&rR, realContext);

      realCopy((real_t *)&rR, rReal);
      realZero(rImag);

      if(!realIsNegative((real_t *)&rR)) {
        // Real roots (r ≥ 0, including double root when r = 0)
        realSquareRoot((real_t *)&rR, (real_t *)&sqrt_r, realContext);

        // x1 = (-b - sign(b)*sqrt(r)) / 2a
        // Numerically stable formula to avoid cancellation
        realCopy((real_t *)&sqrt_r, (real_t *)&temp);
        if(!realIsNegative((real_t *)&b_h)) {
          realChangeSign((real_t *)&temp);
        }

        real159_t neg_b;
        realMinus((real_t *)&b_h, (real_t *)&neg_b, realContext);
        realAdd((real_t *)&neg_b, (real_t *)&temp, (real_t *)&temp, realContext);
        realMultiply((real_t *)&temp, const_1on2, (real_t *)&temp, realContext);
        realDivide((real_t *)&temp, (real_t *)&a_h, x1Real, realContext);

        // x2 = c / (a*x1)  (using Vieta's formula to avoid cancellation)
        realDivide((real_t *)&c_h, (real_t *)&a_h, (real_t *)&temp, realContext);
        realDivide((real_t *)&temp, x1Real, x2Real, realContext);

        realZero(x1Imag);
        realZero(x2Imag);
      }
      else {
        // Complex roots (r < 0)
        real159_t x1R_h, x1I_h, temp_sqrt, temp_calc;
        realZero((real_t *)&x1R_h);
        realZero((real_t *)&x1I_h);
        realZero((real_t *)&temp_sqrt);
        realZero((real_t *)&temp_calc);

        // Compute sqrt(|r|)
        realCopy((real_t *)&rR, (real_t *)&temp_sqrt);
        realChangeSign((real_t *)&temp_sqrt);  // temp_sqrt = -r = |r|
        realSquareRoot((real_t *)&temp_sqrt, (real_t *)&sqrt_r, realContext);

        // Real part: x_real = -b / 2a
        realCopy((real_t *)&b_h, (real_t *)&temp_calc);
        realChangeSign((real_t *)&temp_calc);  // temp_calc = -b
        realMultiply((real_t *)&temp_calc, const_1on2, (real_t *)&temp_calc, realContext);
        realDivide((real_t *)&temp_calc, (real_t *)&a_h, (real_t *)&x1R_h, realContext);

        // Imaginary part: x_imag = ±sqrt(|r|) / 2a
        realCopy((real_t *)&sqrt_r, (real_t *)&temp_calc);
        if(realIsPositive((real_t *)&b_h)) {
          realChangeSign((real_t *)&temp_calc);
        }
        realMultiply((real_t *)&temp_calc, const_1on2, (real_t *)&temp_calc, realContext);
        realDivide((real_t *)&temp_calc, (real_t *)&a_h, (real_t *)&x1I_h, realContext);

        // Copy to output variables
        realCopy((real_t *)&x1R_h, x1Real);
        realCopy((real_t *)&x1I_h, x1Imag);

        // x2 = conj(x1)
        realCopy(x1Real, x2Real);
        realCopy(x1Imag, x2Imag);
        realChangeSign(x2Imag);
      }
    }
  }
  else {
      // Complex coefficients - use 159-digit precision
      real159_t rR, rI, temp1R, temp1I, temp2R, temp2I, sqrtR, sqrtI;
      realZero((real_t *)&rR); realZero((real_t *)&rI);
      realZero((real_t *)&temp1R); realZero((real_t *)&temp1I);
      realZero((real_t *)&temp2R); realZero((real_t *)&temp2I);
      realZero((real_t *)&sqrtR); realZero((real_t *)&sqrtI);

      real159_t a_h, b_h, c_h, aI_h, bI_h, cI_h;
      realCopy(aReal, (real_t *)&a_h);
      realCopy(aImag, (real_t *)&aI_h);
      realCopy(bReal, (real_t *)&b_h);
      realCopy(bImag, (real_t *)&bI_h);
      realCopy(cReal, (real_t *)&c_h);
      realCopy(cImag, (real_t *)&cI_h);

      if(realIsZero(aReal) && realIsZero(aImag)) {
        // bx + c = 0 => x = -c/b

        // r = b²
        mulComplexComplex((real_t *)&b_h, (real_t *)&bI_h, (real_t *)&b_h, (real_t *)&bI_h, rReal, rImag, realContext);

        // x1 = -c/b, x2 = NaN
        divComplexComplex((real_t *)&c_h, (real_t *)&cI_h, (real_t *)&b_h, (real_t *)&bI_h, x1Real, x1Imag, realContext);
        realChangeSign(x1Real);
        realChangeSign(x1Imag);
        realCopy(const_NaN, x2Real);
        realCopy(const_NaN, x2Imag);
      }
      else {
        // Discriminant: r = b² - 4ac
        mulComplexComplex((real_t *)&b_h, (real_t *)&bI_h, (real_t *)&b_h, (real_t *)&bI_h, (real_t *)&rR, (real_t *)&rI, realContext);
        mulComplexComplex((real_t *)&a_h, (real_t *)&aI_h, (real_t *)&c_h, (real_t *)&cI_h, (real_t *)&temp1R, (real_t *)&temp1I, realContext);
        mulComplexReal((real_t *)&temp1R, (real_t *)&temp1I, const_4, (real_t *)&temp1R, (real_t *)&temp1I, realContext);
        realSubtract((real_t *)&rR, (real_t *)&temp1R, (real_t *)&rR, realContext);
        realSubtract((real_t *)&rI, (real_t *)&temp1I, (real_t *)&rI, realContext);

        realCopy((real_t *)&rR, rReal);
        realCopy((real_t *)&rI, rImag);

        // sqrt(r) using 159-digit precision
        sqrtComplex((real_t *)&rR, (real_t *)&rI, (real_t *)&sqrtR, (real_t *)&sqrtI, realContext);

        // x1 = (-b + sqrt(r)) / (2a)
        realMinus((real_t *)&b_h, (real_t *)&temp1R, realContext);
        realMinus((real_t *)&bI_h, (real_t *)&temp1I, realContext);
        addComplex((real_t *)&temp1R, (real_t *)&temp1I, (real_t *)&sqrtR, (real_t *)&sqrtI, (real_t *)&temp1R, (real_t *)&temp1I, realContext);
        mulComplexReal((real_t *)&a_h, (real_t *)&aI_h, const_2, (real_t *)&temp2R, (real_t *)&temp2I, realContext);
        divComplexComplex((real_t *)&temp1R, (real_t *)&temp1I, (real_t *)&temp2R, (real_t *)&temp2I, x1Real, x1Imag, realContext);

        // x2 = (-b - sqrt(r)) / (2a)
        realMinus((real_t *)&b_h, (real_t *)&temp1R, realContext);
        realMinus((real_t *)&bI_h, (real_t *)&temp1I, realContext);
        subComplex((real_t *)&temp1R, (real_t *)&temp1I, (real_t *)&sqrtR, (real_t *)&sqrtI, (real_t *)&temp1R, (real_t *)&temp1I, realContext);
        divComplexComplex((real_t *)&temp1R, (real_t *)&temp1I, (real_t *)&temp2R, (real_t *)&temp2I, x2Real, x2Imag, realContext);
      }
  }

  // Zero tiny imaginary parts that are below precision threshold
  int eff_exp;
  eff_exp = realGetExponent(x1Imag);
  if(eff_exp < -realContext->digits) {
    realZero(x1Imag);
  }
  eff_exp = realGetExponent(x2Imag);
  if(eff_exp < -realContext->digits) {
    realZero(x2Imag);
  }
}
#endif //OPTION_SQUARE_159 || OPTION_EIGEN_159

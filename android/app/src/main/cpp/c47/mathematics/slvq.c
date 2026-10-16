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
 * A coefficient vector in X (highest degree first, 2 to 4 elements) returns all roots as a row vector in X instead.
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSlvq(uint16_t unusedButMandatoryParameter) {
#if defined(OPTION_SLVQ_SLVC)
  bool_t realCoefs=false, realRoots=true, complexCoefs=false;
  cplx_t a, b, c, r, x1, x2;

  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix || getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {   // a coefficient vector takes the matrix path: the element count picks the solver
    solveCoefficientVector();
    return;
  }

  if(!(getRegisterAsComplexOrReal(REGISTER_X, &c, &complexCoefs) &&
       getRegisterAsComplexOrReal(REGISTER_Y, &b, &complexCoefs) &&
       getRegisterAsComplexOrReal(REGISTER_Z, &a, &complexCoefs))) {
    return;
  }
  realCoefs = !complexCoefs;

  if(   realIsZero(&a.Real) && realIsZero(&a.Imag) && realIsZero(&b.Real) && realIsZero(&b.Imag)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
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

  solveQuadratic(&a, &b, &c, &r, &x1, &x2);

  realRoots &= realIsZero(&x1.Imag) && realIsZero(&x2.Imag);

  if(realRoots) {
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
    #if defined(DISCRIMINANT)
      reallocateRegister(REGISTER_Z, dtReal34, 0, amNone);
    #endif //DISCRIMINANT
    convertRealToReal34ResultRegister(&x1.Real, REGISTER_X);
    convertRealToReal34ResultRegister(&x2.Real, REGISTER_Y);
    #if defined(DISCRIMINANT)
      realToReal34(&r.Real,  REGISTER_REAL34_DATA(REGISTER_Z));
    #endif //DISCRIMINANT
  }
  else { // !realRoots
    if(realIsZero(&x1.Imag)) { // x1 is real
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      convertRealToReal34ResultRegister(&x1.Real, REGISTER_X);
    }
    else {
      reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
      convertComplexToResultRegister(&x1, REGISTER_X);
    }

    if(realIsZero(&x2.Imag)) { // x2 is real
      reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
      convertRealToReal34ResultRegister(&x2.Real, REGISTER_Y);
    }
    else {
      reallocateRegister(REGISTER_Y, dtComplex34, 0, amNone);
      convertComplexToResultRegister(&x2, REGISTER_Y);
    }

    #if defined(DISCRIMINANT)
      if(realIsZero(&r.Imag)) { // r is real
        reallocateRegister(REGISTER_Z, dtReal34, 0, amNone);
        convertRealToReal34ResultRegister(&r.Real, REGISTER_Z);
      }
      else {
        reallocateRegister(REGISTER_Z, dtComplex34, 0, amNone);
        convertComplexToResultRegister(&r, REGISTER_Z);
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
#endif // OPTION_SLVQ_SLVC
}


// a x^2 + b x + c = 0 at the precision the build selects: 159 digits under OPTION_SQUARE_159, 75 otherwise, results rounded to 39.
#if defined(OPTION_SLVQ_SLVC)
void solveQuadratic(const cplx_t *a, const cplx_t *b, const cplx_t *c, cplx_t *r, cplx_t *x1, cplx_t *x2) {
#if defined(OPTION_SQUARE_159)
  realContext_t ctx159 = ctxtReal75;
  ctx159.digits = 159;
  cplx159_t x1H, x2H, r0, aH, bH, cH;

  realCopy(&a->Real, &aH.Real);
  realCopy(&a->Imag, &aH.Imag);
  realCopy(&b->Real, &bH.Real);
  realCopy(&b->Imag, &bH.Imag);
  realCopy(&c->Real, &cH.Real);
  realCopy(&c->Imag, &cH.Imag);
  realSetZero(&r0.Real);
  realSetZero(&r0.Imag);
  realSetZero(&x1H.Real);
  realSetZero(&x1H.Imag);
  realSetZero(&x2H.Real);
  realSetZero(&x2H.Imag);
  solveQuadraticEquation159(&aH, &bH, &cH, &r0, &x1H, &x2H, &ctx159);
  realPlus(&r0.Real, &r->Real,  &ctxtReal39);
  realPlus(&r0.Imag, &r->Imag,  &ctxtReal39);
  realPlus(&x1H.Real, &x1->Real, &ctxtReal39);
  realPlus(&x1H.Imag, &x1->Imag, &ctxtReal39);
  realPlus(&x2H.Real, &x2->Real, &ctxtReal39);
  realPlus(&x2H.Imag, &x2->Imag, &ctxtReal39);
#else // OPTION_SQUARE_159
  solveQuadraticEquation(a, b, c, r, x1, x2, &ctxtReal75);
#endif // OPTION_SQUARE_159
}
#endif // OPTION_SLVQ_SLVC


void solveQuadraticEquation(const cplx_t *a, const cplx_t *b, const cplx_t *c, cplx_t *r, cplx_t *x1, cplx_t *x2, realContext_t *realContext) {
  bool_t realCoefs = realIsZero(&a->Imag) && realIsZero(&b->Imag) && realIsZero(&c->Imag);

  if(realCoefs) {
    if(realIsZero(&a->Real)) {
      // bx + c = 0   (b is not 0 here)

      // r = b²
      realMultiply(&b->Real, &b->Real, &r->Real, realContext);
      realSetZero(&r->Imag);

      // x1 = -c/b, x2 = NaN
      realDivide(&c->Real, &b->Real, &x1->Real, realContext);
      realChangeSign(&x1->Real);
      realSetNaN(&x2->Real);

      realSetZero(&x1->Imag);
      realSetZero(&x2->Imag);
    }
    else if(realIsZero(&c->Real)) {
      // ax² + bx = x(ax + b) = 0   (a is not 0 here)

      // r = b²
      realMultiply(&b->Real, &b->Real, &r->Real, realContext);
      realSetZero(&r->Imag);

      // x1 = 0
      realSetZero(&x1->Real);

      // x2 = -b/a
      realDivide(&b->Real, &a->Real, &x2->Real, realContext);
      realChangeSign(&x2->Real);

      realSetZero(&x1->Imag);
      realSetZero(&x2->Imag);
    }
    else {
      // ax² + bx + c = 0   (a and c are not 0 here)

      // r = b² - 4ac
      realMultiply(const__4, &a->Real, &r->Real, realContext);

      // Fix potential aliasing in realMultiply, not sure if it 100% helps but I keep it
      real_t temp_multiply;
      realMultiply(&c->Real, &r->Real, &temp_multiply, realContext);
      realCopy(&temp_multiply, &r->Real);

      // Fix potential aliasing in realFMA, not sure if it 100% helps but I keep it
      real_t temp_result;
      realFMA(&b->Real, &b->Real, &r->Real, &temp_result, realContext);
      realCopy(&temp_result, &r->Real);
      realSetZero(&r->Imag);

      if(realIsPositive(&r->Real)) { // real roots
        // x1 = (-b - sign(b)*sqrt(r)) / 2a
        realSquareRoot(&r->Real, &x1->Real, realContext);
        if(realIsPositive(&b->Real)) {
          realChangeSign(&x1->Real);
        }
        realSubtract(&x1->Real, &b->Real, &x1->Real, realContext);
        realMultiply(&x1->Real, const_1on2, &x1->Real, realContext);
        realDivide(&x1->Real, &a->Real, &x1->Real, realContext);

        // x2 = c / ax1  (x1 connot be 0 here)
        realDivide(&c->Real, &a->Real, &x2->Real, realContext);
        realDivide(&x2->Real, &x1->Real, &x2->Real, realContext);

        realSetZero(&x1->Imag);
        realSetZero(&x2->Imag);
      }
      else { // complex roots
        // x1 = (-b - sign(b)*sqrt(r)) / 2a
        realMinus(&r->Real, &x1->Real, realContext);
        realSquareRoot(&x1->Real, &x1->Imag, realContext);
        realSetZero(&x1->Real);
        if(realIsPositive(&b->Real)) {
          realChangeSign(&x1->Imag);
        }
        realSubtract(&x1->Real, &b->Real, &x1->Real, realContext);
        realSubtract(&x1->Imag, &b->Imag, &x1->Imag, realContext);
        realMultiply(&x1->Real, const_1on2, &x1->Real, realContext);
        realMultiply(&x1->Imag, const_1on2, &x1->Imag, realContext);
        realDivide(&x1->Real, &a->Real, &x1->Real, realContext);
        realDivide(&x1->Imag, &a->Real, &x1->Imag, realContext);

        // x2 = conj(x1)
        realCopy(&x1->Real, &x2->Real);
        realCopy(&x1->Imag, &x2->Imag);
        realChangeSign(&x2->Imag);
      }
    }
  }
  else { // Complex coefficients
    if(realIsZero(&a->Real) && realIsZero(&a->Imag)) {
      // bx + c = 0   (b is not 0 here)

      // r = b²
      mulComplexComplex(b, b, r, realContext);

      // x1 = -c/b, x2 = NaN
      divComplexComplex(c, b, x1, realContext);
      realChangeSign(&x1->Real);
      realChangeSign(&x1->Imag);
      realSetNaN(&x2->Real);
      realSetNaN(&x2->Imag);
    }
    else if(realIsZero(&c->Real) && realIsZero(&c->Imag)) {
      // ax² + bx = x(ax + b) = 0   (a is not 0 here)

      // r = b²
      mulComplexComplex(b, b, r, realContext);

      // x1 = 0
      realSetZero(&x1->Real);
      realSetZero(&x1->Imag);

      // x2 = -b/a
      divComplexComplex(b, a, x2, realContext);
      realChangeSign(&x2->Real);
      realChangeSign(&x2->Imag);
    }
    else {
      // ax² + bx + c = 0   (a and c are not 0 here)

      cplx_t ac, b2, sqrtR, q;
      REAL_T_PTR(dot, 75);
      REAL_T_PTR(product, 75);
      bool_t qUsesMinusRoot;

      // r = b² - 4ac
      mulComplexComplex(a, c, &ac, realContext);
      mulComplexReal(&ac, const_4, r, realContext);
      mulComplexComplex(b, b, &b2, realContext);
      realSubtract(&b2.Real, &r->Real, &r->Real, realContext);
      realSubtract(&b2.Imag, &r->Imag, &r->Imag, realContext);

      sqrtComplex(r, &sqrtR, realContext);

      // Choose the sign that makes q = -(b +/- sqrt(r))/2 the larger numerator.
      realMultiply(&b->Imag, &sqrtR.Imag, product, realContext);
      realFMA(&b->Real, &sqrtR.Real, product, dot, realContext);
      qUsesMinusRoot = realIsPositive(dot) && !realIsZero(dot);
      if(qUsesMinusRoot) {
        realAdd(&b->Real, &sqrtR.Real, &q.Real, realContext);
        realAdd(&b->Imag, &sqrtR.Imag, &q.Imag, realContext);
        realChangeSign(&q.Real);
        realChangeSign(&q.Imag);
      }
      else {
        realSubtract(&sqrtR.Real, &b->Real, &q.Real, realContext);
        realSubtract(&sqrtR.Imag, &b->Imag, &q.Imag, realContext);
      }
      realMultiply(&q.Real, const_1on2, &q.Real, realContext);
      realMultiply(&q.Imag, const_1on2, &q.Imag, realContext);

      // Preserve the principal-square-root ordering while using c/q for the smaller numerator.
      if(qUsesMinusRoot) {
        divComplexComplex(c, &q, x1, realContext);
        divComplexComplex(&q, a, x2, realContext);
      }
      else {
        divComplexComplex(&q, a, x1, realContext);
        divComplexComplex(c, &q, x2, realContext);
      }
    }
  }
}


#if defined(OPTION_SQUARE_159) || defined(OPTION_EIGEN_159)
void solveQuadraticEquation159(const cplx159_t *a, const cplx159_t *b, const cplx159_t *c, cplx159_t *r, cplx159_t *x1, cplx159_t *x2, realContext_t *realContext) {

  bool_t realCoefs = realIsZero(&a->Imag) && realIsZero(&b->Imag) && realIsZero(&c->Imag);

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

    realCopy(&a->Real, a_h);
    realCopy(&b->Real, b_h);
    realCopy(&c->Real, c_h);

    if(realIsZero(&a->Real)) {
      // bx + c = 0   (b is not 0 here)

      // r = b²
      realMultiply(b_h, b_h, &r->Real, realContext);
      realSetZero(&r->Imag);

      // x1 = -c/b, x2 = NaN
      realDivide(c_h, b_h, &x1->Real, realContext);
      realChangeSign(&x1->Real);
      realSetNaN(&x2->Real);
      realSetZero(&x1->Imag);
      realSetZero(&x2->Imag);
    }
    else if(realIsZero(&c->Real)) {
      // ax² + bx = x(ax + b) = 0   (a is not 0 here)

      // r = b²
      realMultiply(b_h, b_h, &r->Real, realContext);
      realSetZero(&r->Imag);

      // x1 = 0
      realSetZero(&x1->Real);

      // x2 = -b/a
      realDivide(b_h, a_h, &x2->Real, realContext);
      realChangeSign(&x2->Real);

      realSetZero(&x1->Imag);
      realSetZero(&x2->Imag);
    }
    else {
      // ax² + bx + c = 0   (a and c are not 0 here)

      // r = b² - 4ac (using FMA for better precision)
      realMultiply(const_4, a_h, temp, realContext);
      realMultiply(c_h, temp, temp, realContext);
      realMinus(temp, temp, realContext);
      realFMA(b_h, b_h, temp, rR, realContext);

      realCopy(rR, &r->Real);
      realSetZero(&r->Imag);

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
        realDivide(temp, a_h, &x1->Real, realContext);

        // x2 = c / (a*x1)  (using Vieta's formula to avoid cancellation)
        realDivide(c_h, a_h, temp, realContext);
        realDivide(temp, &x1->Real, &x2->Real, realContext);

        realSetZero(&x1->Imag);
        realSetZero(&x2->Imag);
      }
      else {
        // Complex roots (r < 0)
        REAL_T_PTR(temp_sqrt, 159);
        REAL_T_PTR(temp_calc, 159);
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
        realDivide(temp_calc, a_h, &x1->Real, realContext);

        // Imaginary part: x_imag = ±sqrt(|r|) / 2a
        realCopy(sqrt_r, temp_calc);
        if(realIsPositive(b_h)) {
          realChangeSign(temp_calc);
        }
        realMultiply(temp_calc, const_1on2, temp_calc, realContext);
        realDivide(temp_calc, a_h, &x1->Imag, realContext);

        // x2 = conj(x1)
        realCopy(&x1->Real, &x2->Real);
        realCopy(&x1->Imag, &x2->Imag);
        realChangeSign(&x2->Imag);
      }
    }
  }
  else {
      // Complex coefficients - use 159-digit precision
      cplx159_t rC, temp1, temp2, sqrtZ;
      realSetZero(&rC.Real);
      realSetZero(&rC.Imag);
      realSetZero(&temp1.Real);
      realSetZero(&temp1.Imag);
      realSetZero(&temp2.Real);
      realSetZero(&temp2.Imag);
      realSetZero(&sqrtZ.Real);
      realSetZero(&sqrtZ.Imag);

      cplx159_t aH, bH, cH;
      realCopy(&a->Real, &aH.Real);
      realCopy(&a->Imag, &aH.Imag);
      realCopy(&b->Real, &bH.Real);
      realCopy(&b->Imag, &bH.Imag);
      realCopy(&c->Real, &cH.Real);
      realCopy(&c->Imag, &cH.Imag);

      if(realIsZero(&a->Real) && realIsZero(&a->Imag)) {
        // bx + c = 0 => x = -c/b

        // r = b²
        mulComplexComplex159(&bH, &bH, r, realContext);

        // x1 = -c/b, x2 = NaN
        divComplexComplex159(&cH, &bH, x1, realContext);
        realChangeSign(&x1->Real);
        realChangeSign(&x1->Imag);
        realSetNaN(&x2->Real);
        realSetNaN(&x2->Imag);
      }
      else {
        // Discriminant: r = b² - 4ac
        mulComplexComplex159(&bH, &bH, &rC, realContext);
        mulComplexComplex159(&aH, &cH, &temp1, realContext);
        mulComplexReal159(&temp1, const_4, &temp1, realContext);
        realSubtract(&rC.Real, &temp1.Real, &rC.Real, realContext);
        realSubtract(&rC.Imag, &temp1.Imag, &rC.Imag, realContext);

        realCopy(&rC.Real, &r->Real);
        realCopy(&rC.Imag, &r->Imag);

        // sqrt(r) using 159-digit precision
        sqrtComplex159(&rC, &sqrtZ, realContext);

        // x1 = (-b + sqrt(r)) / (2a)
        realMinus(&bH.Real, &temp1.Real, realContext);
        realMinus(&bH.Imag, &temp1.Imag, realContext);
        addComplex159(&temp1, &sqrtZ, &temp1, realContext);
        mulComplexReal159(&aH, const_2, &temp2, realContext);
        divComplexComplex159(&temp1, &temp2, x1, realContext);

        // x2 = (-b - sqrt(r)) / (2a)
        realMinus(&bH.Real, &temp1.Real, realContext);
        realMinus(&bH.Imag, &temp1.Imag, realContext);
        subComplex159(&temp1, &sqrtZ, &temp1, realContext);
        divComplexComplex159(&temp1, &temp2, x2, realContext);
      }
  }

  // Zero tiny imaginary parts that are below precision threshold
  int eff_exp;
  eff_exp = realGetExponent(&x1->Imag);
  if(eff_exp < -realContext->digits) {
    realSetZero(&x1->Imag);
  }
  eff_exp = realGetExponent(&x2->Imag);
  if(eff_exp < -realContext->digits) {
    realSetZero(&x2->Imag);
  }
}
#endif //OPTION_SQUARE_159 || OPTION_EIGEN_159

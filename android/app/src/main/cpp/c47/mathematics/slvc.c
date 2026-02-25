// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file slvc.c
 ***********************************************/

#include "c47.h"

#undef DISCRIMINANT //Note the testSuite tests were revised to remove the discriminant

struct cmplxPair {
  real_t r, i;
};

#if !defined(SAVE_SPACE_DM42_12)
static int cmplxSortCompare(const void *v1, const void *v2) {
  const struct cmplxPair *p1 = (const struct cmplxPair *)v1;
  const struct cmplxPair *p2 = (const struct cmplxPair *)v2;
  real_t v1a, v2a, c;

  complexMagnitude2(&p1->r, &p1->i, &v1a, &ctxtReal39);
  complexMagnitude2(&p2->r, &p2->i, &v2a, &ctxtReal39);

  // NaN's aren't interesting so sort largest
  if(realIsNaN(&v1a))
    return realIsNaN(&v2a) ? 0 : 1;
  if(realIsNaN(&v2a))
    return -1;

  // Zeros are uninteresting so sort larger
  if(realIsZero(&v1a))
    return realIsZero(&v2a)? 0 : 1;
  if(realIsZero(&v2a))
    return -1;

  // Complex values are less interesting than real ones
  if(realIsZero(&p1->i)) {
    if(!realIsZero(&p2->i))
      return -1;
  }
  else if(realIsZero(&p2->i))
      return 1;

  // Sort on magnitude
  realCompare(&v1a, &v2a, &c, &ctxtReal75);
  if(!realIsZero(&c))
    return realIsNegative(&c) ? -1 : 1;

  // Equal magnitude, favour positive roots over negative
  if(realIsNegative(&p1->r) && !realIsNegative(&p2->r))
    return 1;
  if(!realIsNegative(&p1->r) && realIsNegative(&p2->r))
    return -1;
  if(realIsNegative(&p1->i) && !realIsNegative(&p2->i))
    return 1;
  if(!realIsNegative(&p1->i) && realIsNegative(&p2->i))
    return -1;

  // Favour smaller real parts
  realCompare(&p1->r, &p2->r, &c, &ctxtReal75);
  if(!realIsZero(&c)) {
    if(realIsNegative(&p1->r))
      return realIsNegative(&c) ? 1 : -1;
    return realIsNegative(&c) ? -1 : 1;
  }

  // Favour smaller imaginary parts
  realCompare(&p1->i, &p2->i, &c, &ctxtReal75);
  if(!realIsZero(&c)) {
    if(realIsNegative(&p1->i))
      return realIsNegative(&c) ? 1 : -1;
    else
      return realIsNegative(&c) ? -1 : 1;
  }
  return 0;
}
#endif //SAVE_SPACE_DM42_12


/********************************************//**
 * \brief (d, c, b, a) ==> (x1, x2, r) c ==> regL
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSlvc(uint16_t unusedButMandatoryParameter) {
#if !defined(SAVE_SPACE_DM42_12)
  bool_t complexCoefs=false;
  real_t aReal, bReal, cReal, dReal, rReal;
  real_t aImag, bImag, cImag, dImag, rImag;
  struct cmplxPair x[3];

  if(!(getRegisterAsComplexOrReal(REGISTER_X, &dReal, &dImag, &complexCoefs) &&
       getRegisterAsComplexOrReal(REGISTER_Y, &cReal, &cImag, &complexCoefs) &&
       getRegisterAsComplexOrReal(REGISTER_Z, &bReal, &bImag, &complexCoefs) &&
       getRegisterAsComplexOrReal(REGISTER_T, &aReal, &aImag, &complexCoefs))) {
    return;
  }

  if(   realIsZero(&aReal) && realIsZero(&aImag)
     && realIsZero(&bReal) && realIsZero(&bImag)
     && realIsZero(&cReal) && realIsZero(&cImag)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnSlvc:", "cannot use 0 for Y, Z and T as input of SLVC", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }


  if(!saveLastX()) {
    return;
  }


  if(realIsZero(&aReal) && realIsZero(&aImag)) {
    solveQuadraticEquation(&bReal, &bImag, &cReal, &cImag, &dReal, &dImag, &rReal, &rImag, &x[0].r, &x[0].i, &x[1].r, &x[1].i, &ctxtReal75);
    realCopy(const_NaN, &x[2].r);
    realCopy(const_NaN, &x[2].i);
  }
  else {
    divComplexComplex(&bReal, &bImag, &aReal, &aImag, &bReal, &bImag, &ctxtReal75);
    divComplexComplex(&cReal, &cImag, &aReal, &aImag, &cReal, &cImag, &ctxtReal75);
    divComplexComplex(&dReal, &dImag, &aReal, &aImag, &dReal, &dImag, &ctxtReal75);

#if defined(OPTION_CUBIC_159)
    realContext_t c = ctxtReal75;
    c.digits = 159;
    real159_t x1r, x1i, x2r, x2i, x3r, x3i, r0r, r0i;
    real159_t bRealH, bImagH, cRealH, cImagH, dRealH, dImagH;

    realPlus(&bReal, (real_t *)&bRealH, &c);
    realPlus(&bImag, (real_t *)&bImagH, &c);
    realPlus(&cReal, (real_t *)&cRealH, &c);
    realPlus(&cImag, (real_t *)&cImagH, &c);
    realPlus(&dReal, (real_t *)&dRealH, &c);
    realPlus(&dImag, (real_t *)&dImagH, &c);
    realZero((real_t*)&r0r);
    realZero((real_t*)&r0i);
    realZero((real_t*)&x1r);
    realZero((real_t*)&x1i);
    realZero((real_t*)&x2r);
    realZero((real_t*)&x2i);
    realZero((real_t*)&x3r);
    realZero((real_t*)&x3i);
    solveCubicEquation159((real_t*)&bRealH, (real_t*)&bImagH, (real_t*)&cRealH, (real_t*)&cImagH, (real_t*)&dRealH, (real_t*)&dImagH, (real_t*)&r0r, (real_t*)&r0i, (real_t*)&x1r, (real_t*)&x1i, (real_t*)&x2r, (real_t*)&x2i, (real_t*)&x3r, (real_t*)&x3i, &c);
    realPlus((real_t *)&r0r, &rReal,  &ctxtReal39);
    realPlus((real_t *)&r0i, &rImag,  &ctxtReal39);
    realPlus((real_t *)&x1r, &x[0].r, &ctxtReal39);
    realPlus((real_t *)&x1i, &x[0].i, &ctxtReal39);
    realPlus((real_t *)&x2r, &x[1].r, &ctxtReal39);
    realPlus((real_t *)&x2i, &x[1].i, &ctxtReal39);
    realPlus((real_t *)&x3r, &x[2].r, &ctxtReal39);
    realPlus((real_t *)&x3i, &x[2].i, &ctxtReal39);
#else // OPTION_CUBIC_159
    solveCubicEquation(&bReal, &bImag, &cReal, &cImag, &dReal, &dImag, &rReal, &rImag, &x[0].r, &x[0].i, &x[1].r, &x[1].i, &x[2].r, &x[2].i, &ctxtReal75);
#endif //OPTION_CUBIC_159

  }

  qsort(x, 3, sizeof(x[0]), &cmplxSortCompare);
  for (int i = 0; i < 3; i++) {
    if(realIsZero(&x[i].i) || (realIsNaN(&x[i].r) && realIsNaN(&x[i].i))) {
      convertRealToResultRegister(&x[i].r, REGISTER_X + i, amNone);
    }
    else {
      convertComplexToResultRegister(&x[i].r, &x[i].i, REGISTER_X + i);
    }
    adjustResult(REGISTER_X + i, false, true, REGISTER_X + i, -1, -1);
  }
  temporaryInformation = TI_ROOTS3;

  #ifdef DISCRIMINANT
    if(realIsZero(&rImag)) { // q3r2 is real
      convertRealToResultRegister(&rReal, REGISTER_T, amNone);
    }
    else {
      convertComplexToResultRegister(&rReal, &rImag, REGISTER_T);
    }
    adjustResult(REGISTER_T, false, true, REGISTER_T, -1, -1);
  #else
    fnDropT(0);
  #endif //DISCRIMINANT
#endif // !SAVE_SPACE_DM42_12
}




static bool_t _checkConditionNumberOfAddSub(const real_t *operand1, const real_t *operand2, const real_t *res, realContext_t *realContext) {
  real_t conditionNumber1, conditionNumber2;
  real_t *conditionNumber = &conditionNumber1;

  if(realIsZero(res)) {
    return false;
  }
  else {
    realDivide(res, operand1, &conditionNumber1, realContext); realSetPositiveSign(&conditionNumber1);
    realDivide(res, operand2, &conditionNumber2, realContext); realSetPositiveSign(&conditionNumber2);
    if(realIsZero(operand1)) {
      conditionNumber = &conditionNumber2;
    }
    else if(realIsZero(operand2)) {
      conditionNumber = &conditionNumber1;
    }
    else if(realCompareGreaterThan(&conditionNumber1, &conditionNumber2)) {
      conditionNumber = &conditionNumber2;
    }
    else {
      conditionNumber = &conditionNumber1;
    }
    return realCompareLessThan(conditionNumber, const_1e_37);
  }
}
static void _realCheckedAdd(const real_t *operand1, const real_t *operand2, real_t *res, realContext_t *realContext) {
  real_t r;
  realAdd(operand1, operand2, &r, realContext);
  if(_checkConditionNumberOfAddSub(operand1, operand2, &r, realContext)) {
    realZero(res);
  }
  else {
    realCopy(&r, res);
  }
}
static void _realCheckedSubtract(const real_t *operand1, const real_t *operand2, real_t *res, realContext_t *realContext) {
  real_t r;
  realSubtract(operand1, operand2, &r, realContext);
  if(_checkConditionNumberOfAddSub(operand1, operand2, &r, realContext)) {
    realZero(res);
  }
  else {
    realCopy(&r, res);
  }
}

void solveCubicEquation(const real_t *c2Real, const real_t *c2Imag, const real_t *c1Real, const real_t *c1Imag, const real_t *c0Real, const real_t *c0Imag, real_t *rReal, real_t *rImag, real_t *x1Real, real_t *x1Imag, real_t *x2Real, real_t *x2Imag, real_t *x3Real, real_t *x3Imag, realContext_t *realContext) {
  // x^3 + b x^2 + c x + d = 0
  // Abramowitz & Stegun §3.8.2
  real_t qr, qi, rr, ri, s1r, s1i, s2r, s2i, ar, ai;
  const bool_t realIn = realIsZero(c2Imag) && realIsZero(c1Imag) && realIsZero(c0Imag);

  // Compute q, r and the discriminant
  // This is done by scaling things up so that divisions are avoided until the final step.
  // This reduces rounding problems and gives an exact discriminant for integer (and other)
  // coefficients.
  // q has a denominator of 9, r has a denomination of 54.  q^3 therefore has a denominator
  // of 729 and r^2 of 2916.  729 times 4 is 2916, so we upscale by 2916.

  // q = (c - b^2 / 3) / 3
  // 9q = (3c - b^2)
  mulComplexReal(c1Real, c1Imag, const_3, &rr, &ri, realContext);
  mulComplexComplex(c2Real, c2Imag, c2Real, c2Imag, &qr, &qi, realContext);
  subComplex(&rr, &ri, &qr, &qi, &qr, &qi, realContext);

  // r = (b c - 3 d) / 6 - b^3 / 27
  // 54r = 9(b c - 3 d) - 2 b^3
  mulComplexComplex(c2Real, c2Imag, c1Real, c1Imag, &rr, &ri, realContext);
  mulComplexReal(c0Real, c0Imag, const_3, &ar, &ai, realContext);
  subComplex(&rr, &ri, &ar, &ai, &rr, &ri, realContext);
  mulComplexReal(&rr, &ri, const_9, &rr, &ri, realContext);

  mulComplexComplex(c2Real, c2Imag, c2Real, c2Imag, &ar, &ai, realContext);
  mulComplexComplex(&ar, &ai, c2Real, c2Imag, &ar, &ai, realContext);
  addComplex(&ar, &ai, &ar, &ai, &ar, &ai, realContext);
  subComplex(&rr, &ri, &ar, &ai, &rr, &ri, realContext);

  // q^3 + r^2 = (4 (9q)^3 + (54r)^2) / 2916
  mulComplexComplex(&qr, &qi, &qr, &qi, rReal, rImag, realContext);
  mulComplexComplex(rReal, rImag, &qr, &qi, rReal, rImag, realContext);
  mulComplexReal(rReal, rImag, const_4, rReal, rImag, realContext);
  mulComplexComplex(&rr, &ri, &rr, &ri, &ar, &ai, realContext);
  addComplex(rReal, rImag, &ar, &ai, rReal, rImag, realContext);
  divComplexReal(rReal, rImag, const_2916, rReal, rImag, realContext);

  // Scale r back to it's proper range, q isn't needed anymore so it's good.
  divComplexReal(&rr, &ri, const_54, &rr, &ri, realContext);

  // s1, s2 = cbrt(r ± sqrt(q^3 + r^2))
  sqrtComplex(rReal, rImag, &s1r, &s1i, realContext);
  subComplex(&rr, &ri, &s1r, &s1i, &s2r, &s2i, realContext);
  addComplex(&rr, &ri, &s1r, &s1i, &s1r, &s1i, realContext);
  curtComplex(&s1r, &s1i, &s1r, &s1i, realContext);
  curtComplex(&s2r, &s2i, &s2r, &s2i, realContext);

  // reusing q, r for (s1 ± s2)
  addComplex(&s1r, &s1i, &s2r, &s2i, &qr, &qi, realContext);
  subComplex(&s1r, &s1i, &s2r, &s2i, &rr, &ri, realContext);
  mulComplexComplex(&rr, &ri, const_0, const_root3on2, &rr, &ri, realContext);

  // roots
  divComplexReal(c2Real, c2Imag, const_3, x2Real, x2Imag, realContext);
  _realCheckedSubtract(&qr, x2Real, x1Real, realContext);
  _realCheckedSubtract(&qi, x2Imag, x1Imag, realContext);
  mulComplexReal(&qr, &qi, const_1on2, x3Real, x3Imag, realContext);
  _realCheckedAdd(x3Real, x2Real, x3Real, realContext);
  _realCheckedAdd(x3Imag, x2Imag, x3Imag, realContext);
  chsComplex(x3Real, x3Imag);
  _realCheckedAdd(x3Real, &rr, x2Real, realContext);
  _realCheckedAdd(x3Imag, &ri, x2Imag, realContext);
  _realCheckedSubtract(x3Real, &rr, x3Real, realContext);
  _realCheckedSubtract(x3Imag, &ri, x3Imag, realContext);

  // Force real outputs when the roots are known to be real
  if(realIn) {
    if(realIsZero(rReal) || realIsNegative(rImag)) {
      /* Three real roots */
      realZero(x1Imag);
      realZero(x2Imag);
      realZero(x3Imag);
    }
    else {
      /* One real, two complex roots */
      if(realCompareAbsLessThan(x1Imag, x2Imag)) {
        if(realCompareAbsLessThan(x1Imag, x3Imag))
          realZero(x1Imag);
        else
          realZero(x3Imag);
      }
      else {
        if(realCompareAbsLessThan(x2Imag, x3Imag))
          realZero(x2Imag);
        else
          realZero(x3Imag);
      }
    }
  }

#ifdef DISCRIMINANT
  // discriminant
  mulComplexReal(rReal, rImag, const__108, rReal, rImag, realContext);
#endif
}



#if defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159)
// Calculate of cubic equation using 159-digit precision
// Standard form: x³ + c2·x² + c1·x + c0 = 0
// Abramowitz & Stegun §3.8.2
void solveCubicEquation159(const real_t *c2Real, const real_t *c2Imag,
                           const real_t *c1Real, const real_t *c1Imag,
                           const real_t *c0Real, const real_t *c0Imag,
                           real_t *rReal, real_t *rImag,
                           real_t *x1Real, real_t *x1Imag,
                           real_t *x2Real, real_t *x2Imag,
                           real_t *x3Real, real_t *x3Imag,
                           realContext_t *realContext) {

  const bool_t realIn = realIsZero(c2Imag) && realIsZero(c1Imag) && realIsZero(c0Imag);

  // Compute high-precision constant sqrt(3)/2
  real159_t const159_root3on2;
  realZero((real_t *)&const159_root3on2);
  realSquareRoot(const_3, (real_t *)&const159_root3on2, realContext);
  realDivide((real_t *)&const159_root3on2, const_2, (real_t *)&const159_root3on2, realContext);

  // Intermediate variables in 159-digit precision
  real159_t qr, qi, rr, ri, s1r, s1i, s2r, s2i, ar, ai;

  realZero((real_t *)&qr); realZero((real_t *)&qi);
  realZero((real_t *)&rr); realZero((real_t *)&ri);
  realZero((real_t *)&s1r); realZero((real_t *)&s1i);
  realZero((real_t *)&s2r); realZero((real_t *)&s2i);
  realZero((real_t *)&ar); realZero((real_t *)&ai);

  // Compute q, r and the discriminant
  // This is done by scaling things up so that divisions are avoided until the final step.
  // This reduces rounding problems and gives an exact discriminant for integer (and other)
  // coefficients.
  // q has a denominator of 9, r has a denomination of 54.  q^3 therefore has a denominator
  // of 729 and r^2 of 2916.  729 times 4 is 2916, so we upscale by 2916.

  // q = (c - b^2 / 3) / 3
  // 9q = (3c - b^2)
  mulComplexReal(c1Real, c1Imag, const_3, (real_t *)&rr, (real_t *)&ri, realContext);
  mulComplexComplex(c2Real, c2Imag, c2Real, c2Imag, (real_t *)&qr, (real_t *)&qi, realContext);
  subComplex((real_t *)&rr, (real_t *)&ri, (real_t *)&qr, (real_t *)&qi, (real_t *)&qr, (real_t *)&qi, realContext);

  // r = (b c - 3 d) / 6 - b^3 / 27
  // 54r = 9(b c - 3 d) - 2 b^3
  mulComplexComplex(c2Real, c2Imag, c1Real, c1Imag, (real_t *)&rr, (real_t *)&ri, realContext);
  mulComplexReal(c0Real, c0Imag, const_3, (real_t *)&ar, (real_t *)&ai, realContext);
  subComplex((real_t *)&rr, (real_t *)&ri, (real_t *)&ar, (real_t *)&ai, (real_t *)&rr, (real_t *)&ri, realContext);
  mulComplexReal((real_t *)&rr, (real_t *)&ri, const_9, (real_t *)&rr, (real_t *)&ri, realContext);

  mulComplexComplex(c2Real, c2Imag, c2Real, c2Imag, (real_t *)&ar, (real_t *)&ai, realContext);
  mulComplexComplex((real_t *)&ar, (real_t *)&ai, c2Real, c2Imag, (real_t *)&ar, (real_t *)&ai, realContext);
  addComplex((real_t *)&ar, (real_t *)&ai, (real_t *)&ar, (real_t *)&ai, (real_t *)&ar, (real_t *)&ai, realContext);
  subComplex((real_t *)&rr, (real_t *)&ri, (real_t *)&ar, (real_t *)&ai, (real_t *)&rr, (real_t *)&ri, realContext);

  // q^3 + r^2 = (4 (9q)^3 + (54r)^2) / 2916
//  mulComplexComplex((real_t *)&qr, (real_t *)&qi, (real_t *)&qr, (real_t *)&qi, rReal, rImag, realContext);
//  mulComplexComplex(rReal, rImag, (real_t *)&qr, (real_t *)&qi, rReal, rImag, realContext);
//  mulComplexReal(rReal, rImag, const_4, rReal, rImag, realContext);
//  mulComplexComplex((real_t *)&rr, (real_t *)&ri, (real_t *)&rr, (real_t *)&ri, (real_t *)&ar, (real_t *)&ai, realContext);
//  addComplex(rReal, rImag, (real_t *)&ar, (real_t *)&ai, rReal, rImag, realContext);
//  divComplexReal(rReal, rImag, const_2916, rReal, rImag, realContext);

// Compute discriminant using intermediate 159-digit variables
  real159_t discrimR, discrimI;
  realZero((real_t *)&discrimR);
  realZero((real_t *)&discrimI);
  
  // q^3 + r^2 = (4 (9q)^3 + (54r)^2) / 2916
  mulComplexComplex((real_t *)&qr, (real_t *)&qi, (real_t *)&qr, (real_t *)&qi, (real_t *)&discrimR, (real_t *)&discrimI, realContext);
  mulComplexComplex((real_t *)&discrimR, (real_t *)&discrimI, (real_t *)&qr, (real_t *)&qi, (real_t *)&discrimR, (real_t *)&discrimI, realContext);
  mulComplexReal((real_t *)&discrimR, (real_t *)&discrimI, const_4, (real_t *)&discrimR, (real_t *)&discrimI, realContext);
  mulComplexComplex((real_t *)&rr, (real_t *)&ri, (real_t *)&rr, (real_t *)&ri, (real_t *)&ar, (real_t *)&ai, realContext);
  addComplex((real_t *)&discrimR, (real_t *)&discrimI, (real_t *)&ar, (real_t *)&ai, (real_t *)&discrimR, (real_t *)&discrimI, realContext);
  divComplexReal((real_t *)&discrimR, (real_t *)&discrimI, const_2916, (real_t *)&discrimR, (real_t *)&discrimI, realContext);
  // Copy discriminant to output
  realCopy((real_t *)&discrimR, rReal);
  realCopy((real_t *)&discrimI, rImag);

  // Scale r back to it's proper range, q isn't needed anymore so it's good.
  divComplexReal((real_t *)&rr, (real_t *)&ri, const_54, (real_t *)&rr, (real_t *)&ri, realContext);

  // s1, s2 = cbrt(r ± sqrt(q^3 + r^2))
  sqrtComplex((real_t *)&discrimR, (real_t *)&discrimI, (real_t *)&s1r, (real_t *)&s1i, realContext);
  subComplex((real_t *)&rr, (real_t *)&ri, (real_t *)&s1r, (real_t *)&s1i, (real_t *)&s2r, (real_t *)&s2i, realContext);
  addComplex((real_t *)&rr, (real_t *)&ri, (real_t *)&s1r, (real_t *)&s1i, (real_t *)&s1r, (real_t *)&s1i, realContext);
  curtComplex((real_t *)&s1r, (real_t *)&s1i, (real_t *)&s1r, (real_t *)&s1i, realContext);
  curtComplex((real_t *)&s2r, (real_t *)&s2i, (real_t *)&s2r, (real_t *)&s2i, realContext);

  // reusing q, r for (s1 ± s2)
  addComplex((real_t *)&s1r, (real_t *)&s1i, (real_t *)&s2r, (real_t *)&s2i, (real_t *)&qr, (real_t *)&qi, realContext);
  subComplex((real_t *)&s1r, (real_t *)&s1i, (real_t *)&s2r, (real_t *)&s2i, (real_t *)&rr, (real_t *)&ri, realContext);
  mulComplexComplex((real_t *)&rr, (real_t *)&ri, const_0, (real_t *)&const159_root3on2, (real_t *)&rr, (real_t *)&ri, realContext);

// roots
  // x2 = c2 / 3
  // x1 = x2 - (s1 + s2)
  // x3 = x2 + (s1 - s2)/2 - (s1 + s2)/2
  // s1 = cbrt(r + sqrt(q^3 + r^2))
  // s2 = cbrt(r - sqrt(q^3 + r^2))
  // x2 = c2 / 3; x1 = x2 - (s1 + s2); x3 = x2 + (s1 - s2)/2 - (s1 + s2)/2

  divComplexReal(c2Real, c2Imag, const_3, x2Real, x2Imag, realContext); // x2 = c2/3
  realSubtract((real_t *)&qr, x2Real, x1Real, realContext); // x1Real = qr - x2Real
  realSubtract((real_t *)&qi, x2Imag, x1Imag, realContext); // x1Imag = qi - x2Imag
  mulComplexReal((real_t *)&qr, (real_t *)&qi, const_1on2, x3Real, x3Imag, realContext); // x3 = (qr + i*qi)*1/2
  realAdd(x3Real, x2Real, x3Real, realContext);            // x3Real += x2Real
  realAdd(x3Imag, x2Imag, x3Imag, realContext);            // x3Imag += x2Imag
  chsComplex(x3Real, x3Imag);                              // x3 = -x3 (flip sign)
  realAdd(x3Real, (real_t *)&rr, x2Real, realContext);     // x2Real = x3Real + rr
  realAdd(x3Imag, (real_t *)&ri, x2Imag, realContext);     // x2Imag = x3Imag + ri
  realSubtract(x3Real, (real_t *)&rr, x3Real, realContext);// x3Real -= rr
  realSubtract(x3Imag, (real_t *)&ri, x3Imag, realContext);// x3Imag -= ri

  // Force real outputs when the roots are known to be real
  if(realIn) {
    if(realIsZero(rReal) || realIsNegative(rImag)) {
      /* Three real roots */
      realZero(x1Imag);
      realZero(x2Imag);
      realZero(x3Imag);
    }
    else {
      /* One real, two complex roots */
      if(realCompareAbsLessThan(x1Imag, x2Imag)) {
        if(realCompareAbsLessThan(x1Imag, x3Imag))
          realZero(x1Imag);
        else
          realZero(x3Imag);
      }
      else {
        if(realCompareAbsLessThan(x2Imag, x3Imag))
          realZero(x2Imag);
        else
          realZero(x3Imag);
      }
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

  eff_exp = realGetExponent(x3Imag);
  if(eff_exp < -realContext->digits) {
    realZero(x3Imag);
  }

#ifdef DISCRIMINANT
  // discriminant
  mulComplexReal(rReal, rImag, const__108, rReal, rImag, realContext);
#endif
}
#endif //OPTION_CUBIC_159 || OPTION_EIGEN_159


//--------------------------------------------------------------------------------------------------------------------------------------------------
// Trigonometric method -- not suitable until the new high digit count Trig WP34 SinCosTan
//
//   void solveCubicEquation159(const real_t *c2Real, const real_t *c2Imag, 
//                                    const real_t *c1Real, const real_t *c1Imag, 
//                                    const real_t *c0Real, const real_t *c0Imag, 
//                                    real_t *rReal, real_t *rImag, 
//                                    real_t *x1Real, real_t *x1Imag, 
//                                    real_t *x2Real, real_t *x2Imag, 
//                                    real_t *x3Real, real_t *x3Imag, 
//                                    realContext_t *realContext) {
//   
//   // ALL intermediate variables as real159_t
//   real159_t qr, qi, rr, ri, s1r, s1i, s2r, s2i, ar, ai;
//   real159_t temp1, temp2, temp3, temp4;
//   real159_t mag, angle;
//   
//   const bool_t realIn = realIsZero(c2Imag) && realIsZero(c1Imag) && realIsZero(c0Imag);
//   
//   // Initialize ALL intermediate variables
//   realZero((real_t *)&qr)   ;    realZero((real_t *)&qi)   ;
//   realZero((real_t *)&rr)   ;    realZero((real_t *)&ri)   ;
//   realZero((real_t *)&s1r)  ;    realZero((real_t *)&s1i)  ;
//   realZero((real_t *)&s2r)  ;    realZero((real_t *)&s2i)  ;
//   realZero((real_t *)&ar)   ;    realZero((real_t *)&ai)   ;
//   realZero((real_t *)&temp1);    realZero((real_t *)&temp2);
//   realZero((real_t *)&temp3);    realZero((real_t *)&temp4);
//   realZero((real_t *)&mag)  ;    realZero((real_t *)&angle);
// 
//   // ===== q = (c - b^2 / 3) / 3 =====
//   // 9q = (3c - b^2)
//   
//   // rr+ri = 3*c1
//   realMultiply(c1Real, const_3, (real_t *)&rr, realContext);
//   realMultiply(c1Imag, const_3, (real_t *)&ri, realContext);
//   
//   // qr+qi = c2^2 (complex square: (a+bi)^2 = a²-b² + 2abi)
//   realMultiply(c2Real, c2Real, (real_t *)&temp1, realContext);  // a²
//   realMultiply(c2Imag, c2Imag, (real_t *)&temp2, realContext);  // b²
//   printf("realSubtract 000\n"); realSubtract((real_t *)&temp1, (real_t *)&temp2, (real_t *)&qr, realContext); // a²-b²
//   realMultiply(c2Real, c2Imag, (real_t *)&temp1, realContext);  // ab
//   realAdd((real_t *)&temp1, (real_t *)&temp1, (real_t *)&qi, realContext); // 2ab
//   
//   // qr+qi = rr+ri - (qr+qi) = 3c - b²
//   printf("realSubtract 001\n"); realSubtract((real_t *)&rr, (real_t *)&qr, (real_t *)&qr, realContext);
//   printf("realSubtract 002\n"); realSubtract((real_t *)&ri, (real_t *)&qi, (real_t *)&qi, realContext);
// 
//   // ===== r = (b c - 3 d) / 6 - b^3 / 27 =====
//   // 54r = 9(b c - 3 d) - 2 b^3
//   
//   // rr+ri = c2 * c1 (complex multiply manually)
//   realMultiply(c2Real, c1Real, (real_t *)&temp1, realContext);
//   realMultiply(c2Imag, c1Imag, (real_t *)&temp2, realContext);
//   printf("realSubtract 003\n"); realSubtract((real_t *)&temp1, (real_t *)&temp2, (real_t *)&rr, realContext);
//   realMultiply(c2Real, c1Imag, (real_t *)&temp1, realContext);
//   realMultiply(c2Imag, c1Real, (real_t *)&temp2, realContext);
//   realAdd((real_t *)&temp1, (real_t *)&temp2, (real_t *)&ri, realContext);
//   
//   // ar+ai = 3*c0
//   realMultiply(c0Real, const_3, (real_t *)&ar, realContext);
//   realMultiply(c0Imag, const_3, (real_t *)&ai, realContext);
//   
//   // rr+ri = rr+ri - (ar+ai)
//   printf("realSubtract 004\n"); realSubtract((real_t *)&rr, (real_t *)&ar, (real_t *)&rr, realContext);
//   printf("realSubtract 005\n"); realSubtract((real_t *)&ri, (real_t *)&ai, (real_t *)&ri, realContext);
//   
//   // rr+ri = 9 * (rr+ri)
//   realMultiply((real_t *)&rr, const_9, (real_t *)&rr, realContext);
//   realMultiply((real_t *)&ri, const_9, (real_t *)&ri, realContext);
//   
//   // ar+ai = c2^2 (already computed above as temp for qr+qi, recalculate)
//   realMultiply(c2Real, c2Real, (real_t *)&temp1, realContext);
//   realMultiply(c2Imag, c2Imag, (real_t *)&temp2, realContext);
//   printf("realSubtract 006\n"); realSubtract((real_t *)&temp1, (real_t *)&temp2, (real_t *)&ar, realContext);
//   realMultiply(c2Real, c2Imag, (real_t *)&temp1, realContext);
//   realAdd((real_t *)&temp1, (real_t *)&temp1, (real_t *)&ai, realContext);
//   
//   // ar+ai = (ar+ai) * c2 = c2^3 (complex multiply)
//   realMultiply((real_t *)&ar, c2Real, (real_t *)&temp1, realContext);
//   realMultiply((real_t *)&ai, c2Imag, (real_t *)&temp2, realContext);
//   printf("realSubtract 007\n"); realSubtract((real_t *)&temp1, (real_t *)&temp2, (real_t *)&temp3, realContext);
//   realMultiply((real_t *)&ar, c2Imag, (real_t *)&temp1, realContext);
//   realMultiply((real_t *)&ai, c2Real, (real_t *)&temp2, realContext);
//   realAdd((real_t *)&temp1, (real_t *)&temp2, (real_t *)&temp4, realContext);
//   realCopy((real_t *)&temp3, (real_t *)&ar);
//   realCopy((real_t *)&temp4, (real_t *)&ai);
//   
//   // ar+ai = 2 * (ar+ai)
//   realAdd((real_t *)&ar, (real_t *)&ar, (real_t *)&ar, realContext);
//   realAdd((real_t *)&ai, (real_t *)&ai, (real_t *)&ai, realContext);
//   
//   // rr+ri = rr+ri - (ar+ai)
//   printf("realSubtract 008\n"); realSubtract((real_t *)&rr, (real_t *)&ar, (real_t *)&rr, realContext);
// printRealToConsole((real_t *)&ri,"(real_t *)&ri:","\n");
//   printf("realSubtract 009\n"); realSubtract((real_t *)&ri, (real_t *)&ai, (real_t *)&ri, realContext);
// 
//   // ===== discriminant = q^3 + r^2 = (4 (9q)^3 + (54r)^2) / 2916 =====
//   
//   // rReal+rImag = qr+qi squared (complex square)
//   realMultiply((real_t *)&qr, (real_t *)&qr, (real_t *)&temp1, realContext);
//   realMultiply((real_t *)&qi, (real_t *)&qi, (real_t *)&temp2, realContext);
//   printf("realSubtract 010\n"); realSubtract((real_t *)&temp1, (real_t *)&temp2, rReal, realContext);
//   realMultiply((real_t *)&qr, (real_t *)&qi, (real_t *)&temp1, realContext);
//   realAdd((real_t *)&temp1, (real_t *)&temp1, rImag, realContext);
//   
//   // rReal+rImag = (rReal+rImag) * (qr+qi) = q^3 (complex multiply)
//   realMultiply(rReal, (real_t *)&qr, (real_t *)&temp1, realContext);
//   realMultiply(rImag, (real_t *)&qi, (real_t *)&temp2, realContext);
//   printf("realSubtract 011\n"); realSubtract((real_t *)&temp1, (real_t *)&temp2, (real_t *)&temp3, realContext);
//   realMultiply(rReal, (real_t *)&qi, (real_t *)&temp1, realContext);
//   realMultiply(rImag, (real_t *)&qr, (real_t *)&temp2, realContext);
//   realAdd((real_t *)&temp1, (real_t *)&temp2, (real_t *)&temp4, realContext);
//   realCopy((real_t *)&temp3, rReal);
//   realCopy((real_t *)&temp4, rImag);
//   
//   // rReal+rImag = 4 * (rReal+rImag)
//   realMultiply(rReal, const_4, rReal, realContext);
//   realMultiply(rImag, const_4, rImag, realContext);
//   
//   // ar+ai = rr+ri squared (complex square)
//   realMultiply((real_t *)&rr, (real_t *)&rr, (real_t *)&temp1, realContext);
//   realMultiply((real_t *)&ri, (real_t *)&ri, (real_t *)&temp2, realContext);
//   printf("realSubtract 012\n"); realSubtract((real_t *)&temp1, (real_t *)&temp2, (real_t *)&ar, realContext);
//   realMultiply((real_t *)&rr, (real_t *)&ri, (real_t *)&temp1, realContext);
//   realAdd((real_t *)&temp1, (real_t *)&temp1, (real_t *)&ai, realContext);
//   
//   // rReal+rImag = rReal+rImag + ar+ai
//   realAdd(rReal, (real_t *)&ar, rReal, realContext);
//   realAdd(rImag, (real_t *)&ai, rImag, realContext);
//   
//   // rReal+rImag = (rReal+rImag) / 2916 (complex divide by real)
//   realDivide(rReal, const_2916, rReal, realContext);
//   realDivide(rImag, const_2916, rImag, realContext);
// 
//   // ===== Scale r back: rr+ri = rr+ri / 54 =====
//   realDivide((real_t *)&rr, const_54, (real_t *)&rr, realContext);
//   realDivide((real_t *)&ri, const_54, (real_t *)&ri, realContext);
// 
//   // ===== s1+s2 = cbrt(r ± sqrt(discriminant)) =====
//   
//   // s1r+s1i = sqrt(rReal+rImag) using polar form
//   // mag = sqrt(rReal² + rImag²)
//   realMultiply(rReal, rReal, (real_t *)&temp1, realContext);
//   realMultiply(rImag, rImag, (real_t *)&temp2, realContext);
//   realAdd((real_t *)&temp1, (real_t *)&temp2, (real_t *)&mag, realContext);
//   realSquareRoot((real_t *)&mag, (real_t *)&mag, realContext);
//   
//   // angle = atan2(rImag, rReal) / 2
//   WP34S_Atan2(rReal, rImag, (real_t *)&angle, realContext);
//   realDivide((real_t *)&angle, const_2, (real_t *)&angle, realContext);
//   
//   // mag = sqrt(mag)
//   realSquareRoot((real_t *)&mag, (real_t *)&mag, realContext);
//   
//   // s1r+s1i = mag * (cos(angle) + i*sin(angle))
//   WP34S_Cvt2RadSinCosTan((real_t *)&angle, amRadian, (real_t *)&s1r, (real_t *)&s1i, NULL, realContext);
//   realMultiply((real_t *)&s1r, (real_t *)&mag, (real_t *)&s1r, realContext);
//   realMultiply((real_t *)&s1i, (real_t *)&mag, (real_t *)&s1i, realContext);
//   
//   // s2r+s2i = rr+ri - (s1r+s1i)
//   printf("realSubtract 012\n"); realSubtract((real_t *)&rr, (real_t *)&s1r, (real_t *)&s2r, realContext);
//   printf("realSubtract 013\n"); realSubtract((real_t *)&ri, (real_t *)&s1i, (real_t *)&s2i, realContext);
//   
//   // s1r+s1i = rr+ri + (s1r+s1i)
//   realAdd((real_t *)&rr, (real_t *)&s1r, (real_t *)&s1r, realContext);
//   realAdd((real_t *)&ri, (real_t *)&s1i, (real_t *)&s1i, realContext);
//   
//   // s1r+s1i = cbrt(s1r+s1i) using polar form
//   // mag = (s1r² + s1i²)^(1/6)
//   realMultiply((real_t *)&s1r, (real_t *)&s1r, (real_t *)&temp1, realContext);
//   realMultiply((real_t *)&s1i, (real_t *)&s1i, (real_t *)&temp2, realContext);
//   realAdd((real_t *)&temp1, (real_t *)&temp2, (real_t *)&mag, realContext);
//   realSquareRoot((real_t *)&mag, (real_t *)&mag, realContext); // sqrt = ^(1/2)
//   realPower((real_t *)&mag, (real_t *)&const159_1on3, (real_t *)&mag, realContext); // ^(1/3) => total ^(1/6)
//   
//   // angle = atan2(s1i, s1r) / 3
//   WP34S_Atan2((real_t *)&s1r, (real_t *)&s1i, (real_t *)&angle, realContext);
//   realDivide((real_t *)&angle, const_3, (real_t *)&angle, realContext);
//   
//   // s1r+s1i = mag * (cos(angle) + i*sin(angle))
//   WP34S_Cvt2RadSinCosTan((real_t *)&angle, amRadian, (real_t *)&temp1, (real_t *)&temp2, NULL, realContext);
//   realMultiply((real_t *)&temp1, (real_t *)&mag, (real_t *)&s1r, realContext);
//   realMultiply((real_t *)&temp2, (real_t *)&mag, (real_t *)&s1i, realContext);
//   
//   // s2r+s2i = cbrt(s2r+s2i) using polar form (same process)
//   realMultiply((real_t *)&s2r, (real_t *)&s2r, (real_t *)&temp1, realContext);
//   realMultiply((real_t *)&s2i, (real_t *)&s2i, (real_t *)&temp2, realContext);
//   realAdd((real_t *)&temp1, (real_t *)&temp2, (real_t *)&mag, realContext);
//   realSquareRoot((real_t *)&mag, (real_t *)&mag, realContext);
//   realPower((real_t *)&mag, (real_t *)&const159_1on3, (real_t *)&mag, realContext);
//   
//   WP34S_Atan2((real_t *)&s2r, (real_t *)&s2i, (real_t *)&angle, realContext);
//   realDivide((real_t *)&angle, const_3, (real_t *)&angle, realContext);
//   
//   WP34S_Cvt2RadSinCosTan((real_t *)&angle, amRadian, (real_t *)&temp1, (real_t *)&temp2, NULL, realContext);
//   realMultiply((real_t *)&temp1, (real_t *)&mag, (real_t *)&temp3, realContext);
//   realMultiply((real_t *)&temp2, (real_t *)&mag, (real_t *)&temp4, realContext);
//   realCopy((real_t *)&temp3, (real_t *)&s2r);
//   realCopy((real_t *)&temp4, (real_t *)&s2i);
// 
//   // ===== Compute roots =====
//   
//   // qr+qi = s1+s2
//   realAdd((real_t *)&s1r, (real_t *)&s2r, (real_t *)&qr, realContext);
//   realAdd((real_t *)&s1i, (real_t *)&s2i, (real_t *)&qi, realContext);
//   
//   // rr+ri = (s1-s2) * i*sqrt(3)/2
//   printf("realSubtract 014\n"); realSubtract((real_t *)&s1r, (real_t *)&s2r, (real_t *)&temp1, realContext);
//   printf("realSubtract 015\n"); realSubtract((real_t *)&s1i, (real_t *)&s2i, (real_t *)&temp2, realContext);
//   // Multiply by i*sqrt(3)/2 = multiply by (0 + i*sqrt(3)/2)
//   // result_real = real*0 - imag*sqrt(3)/2
//   // result_imag = real*sqrt(3)/2 + imag*0
//   realMultiply((real_t *)&temp2, (real_t *)&const159_root3on2, (real_t *)&rr, realContext);
//   realChangeSign((real_t *)&rr);
//   realMultiply((real_t *)&temp1, (real_t *)&const159_root3on2, (real_t *)&ri, realContext);
//   
//   // x2Real+x2Imag = c2 / 3
//   realDivide(c2Real, const_3, x2Real, realContext);
//   realDivide(c2Imag, const_3, x2Imag, realContext);
//   
//   // x1 = qr+qi - (x2)
//   printf("realSubtract 016\n"); realSubtract((real_t *)&qr, x2Real, x1Real, realContext);
//   printf("realSubtract 017\n"); realSubtract((real_t *)&qi, x2Imag, x1Imag, realContext);
//   
//   // temp = (qr+qi) / 2
//   realDivide((real_t *)&qr, const_2, (real_t *)&qr, realContext);
//   realDivide((real_t *)&qi, const_2, (real_t *)&qi, realContext);
//   
//   // x3 = temp + x2
//   realAdd((real_t *)&qr, x2Real, x3Real, realContext);
//   realAdd((real_t *)&qi, x2Imag, x3Imag, realContext);
//   
//   // x3 = -x3
//   realChangeSign(x3Real);
//   realChangeSign(x3Imag);
//   
//   // x2 = x3 + (rr+ri)
//   realAdd(x3Real, (real_t *)&rr, x2Real, realContext);
//   realAdd(x3Imag, (real_t *)&ri, x2Imag, realContext);
//   
//   // x3 = x3 - (rr+ri)
//   printf("realSubtract 018\n"); realSubtract(x3Real, (real_t *)&rr, x3Real, realContext);
//   printf("realSubtract 019\n"); realSubtract(x3Imag, (real_t *)&ri, x3Imag, realContext);
// 
//   // ===== Force real outputs when appropriate =====
//   if(realIn) {
//     if(realIsZero(rReal) || realIsNegative(rImag)) {
//       realZero(x1Imag);
//       realZero(x2Imag);
//       realZero(x3Imag);
//     }
//     else {
//       if(realCompareAbsLessThan(x1Imag, x2Imag)) {
//         if(realCompareAbsLessThan(x1Imag, x3Imag))
//           realZero(x1Imag);
//         else
//           realZero(x3Imag);
//       }
//       else {
//         if(realCompareAbsLessThan(x2Imag, x3Imag))
//           realZero(x2Imag);
//         else
//           realZero(x3Imag);
//       }
//     }
//   }
// }




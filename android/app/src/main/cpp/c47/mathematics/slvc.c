// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file slvc.c
 ***********************************************/

#include "c47.h"

#undef DISCRIMINANT //Note the testSuite tests were revised to remove the discriminant

#if defined(OPTION_SLVQ_SLVC)
static int cmplxSortCompare(const void *v1, const void *v2) {
  const cplx_t *p1 = (const cplx_t *)v1;
  const cplx_t *p2 = (const cplx_t *)v2;
  real_t v1a, v2a, c;

  complexMagnitude2(p1, &v1a, &ctxtReal39);
  complexMagnitude2(p2, &v2a, &ctxtReal39);

  // NaN's aren't interesting so sort largest
  if(realIsNaN(&v1a)) {
    return realIsNaN(&v2a) ? 0 : 1;
  }
  if(realIsNaN(&v2a)) {
    return -1;
  }

  // Zeros are uninteresting so sort larger
  if(realIsZero(&v1a)) {
    return realIsZero(&v2a) ? 0 : 1;
  }
  if(realIsZero(&v2a)) {
    return -1;
  }

  // Complex values are less interesting than real ones
  if(realIsZero(&p1->Imag)) {
    if(!realIsZero(&p2->Imag)) {
      return -1;
    }
  }
  else if(realIsZero(&p2->Imag)) {
    return 1;
  }

  // Sort on magnitude
  realCompare(&v1a, &v2a, &c, &ctxtReal75);
  if(!realIsZero(&c)) {
    return 1 - 2*realIsNegative(&c);
  }

  // Equal magnitude, favour positive roots over negative
  if(realIsNegative(&p1->Real) && !realIsNegative(&p2->Real)) {
    return 1;
  }
  if(!realIsNegative(&p1->Real) && realIsNegative(&p2->Real)) {
    return -1;
  }
  if(realIsNegative(&p1->Imag) && !realIsNegative(&p2->Imag)) {
    return 1;
  }
  if(!realIsNegative(&p1->Imag) && realIsNegative(&p2->Imag)) {
    return -1;
  }

  // Favour smaller real parts
  realCompare(&p1->Real, &p2->Real, &c, &ctxtReal75);
  if(!realIsZero(&c)) {
    if(realIsNegative(&p1->Real)) {
      return 1 - 2*realIsPositive(&c);
    }
    return 1 - 2*realIsNegative(&c);
  }

  // Favour smaller imaginary parts
  realCompare(&p1->Imag, &p2->Imag, &c, &ctxtReal75);
  if(!realIsZero(&c)) {
    if(realIsNegative(&p1->Imag)) {
      return 1 - 2*realIsPositive(&c);
    }
    else {
      return 1 - 2*realIsNegative(&c);
    }
  }
  return 0;
}
#endif // OPTION_SLVQ_SLVC


#if defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159) || defined(OPTION_SLVP_POLY)
// The number of coefficient digits of a finite value
static int32_t _realDigits(const real_t *x) {
  return realIsZero(x) ? 1 : realGetExponent(x) - realGetUnadjustedExponent(x) + 1;
}


// The homogeneous form avoids another normalisation. A sign obscured by p159 round-off gives roots indistinguishable at Real34.
bool_t realCubicHasOnlyRealRoots(const real_t *a, const real_t *b, const real_t *c, const real_t *d, int32_t unresolvedDigits, real_t *discriminantOut, realContext_t *realContext) {
  REAL_T_PTR(t0, 159);
  REAL_T_PTR(t1, 159);
  REAL_T_PTR(t2, 159);
  REAL_T_PTR(t3, 159);
  REAL_T_PTR(t4, 159);
  REAL_T_PTR(p, 159);
  REAL_T_PTR(q, 159);
  real_t *term[5] = {t0, t1, t2, t3, t4};
  int32_t factorDigits[5], roundingExponent = INT32_MIN / 2, i, j;

  if(realIsSpecial(a) || realIsSpecial(b) || realIsSpecial(c) || realIsSpecial(d)) {
    return false;
  }

  realMultiply(b, b, p, realContext);                         // b^2 c^2
  realMultiply(c, c, q, realContext);
  realMultiply(p, q, t0, realContext);
  realMultiply(q, c, q, realContext);                         // -4 a c^3
  realMultiply(a, q, t1, realContext);
  realMultiply(t1, const__4, t1, realContext);
  realMultiply(p, b, p, realContext);                         // -4 b^3 d
  realMultiply(p, d, t2, realContext);
  realMultiply(t2, const__4, t2, realContext);
  realMultiply(a, d, p, realContext);                         // -27 a^2 d^2
  realMultiply(p, p, t3, realContext);
  realMultiply(t3, const_3, t3, realContext);
  realMultiply(t3, const_9, t3, realContext);
  realChangeSign(t3);
  realMultiply(a, b, p, realContext);                         // +18 a b c d
  realMultiply(c, d, q, realContext);
  realMultiply(p, q, t4, realContext);
  realMultiply(t4, const_2, t4, realContext);
  realMultiply(t4, const_9, t4, realContext);

  // A term whose factors have fewer digits than realContext together is exact and narrower than realContext; otherwise its roundings stay below
  // 10^(exponent - digits + 2). Summed from the largest term down, a sum of narrower terms rounds only where it is over ten times each term left, which then
  // cannot cancel it, so only the terms bound D.
  i = _realDigits(a);
  j = _realDigits(d);
  factorDigits[3] = 2 + 2 * i + 2 * j;
  factorDigits[1] = 1 + i + 3 * _realDigits(c);
  factorDigits[4] = 2 + i + j + _realDigits(b) + _realDigits(c);
  factorDigits[2] = 1 + 3 * _realDigits(b) + j;
  factorDigits[0] = 2 * _realDigits(b) + 2 * _realDigits(c);
  for(i = 0; i < 5; i++) {
    if(factorDigits[i] >= realContext->digits && !realIsZero(term[i])) {
      roundingExponent = max(roundingExponent, realGetExponent(term[i]) - realContext->digits + 2);
    }
  }

  for(i = 1; i < 5; i++) {
    for(j = i; j > 0 && realCompareAbsGreaterThan(term[j], term[j - 1]); j--) {
      real_t *swap = term[j];

      term[j] = term[j - 1];
      term[j - 1] = swap;
    }
  }
  realCopy(term[0], p);
  realCopyAbs(term[0], q);
  for(i = 1; i < 5; i++) {
    realAdd(p, term[i], p, realContext);
    realSetPositiveSign(term[i]);
    realAdd(q, term[i], q, realContext);
  }

  if(discriminantOut != NULL) {
    realCopy(p, discriminantOut);
  }
  // Five terms, each rounded by less than 10^(roundingExponent + 1), leave D within 10^(roundingExponent + 2)
  return realCompareGreaterEqual(p, const_0)
         || realGetExponent(p) <= roundingExponent + 1
         || (unresolvedDigits > 0 && realGetExponent(p) <= realGetExponent(q) - unresolvedDigits);
}
#endif // OPTION_CUBIC_159 || OPTION_EIGEN_159 || OPTION_SLVP_POLY


#if defined(OPTION_SLVQ_SLVC)
// a x^3 + b x^2 + c x + d = 0: a leading zero degrades to the quadratic at 75 digits with a NaN third root; b, c, d are consumed and the roots come back sorted
static void solveGeneralCubic(const cplx_t *a, cplx_t *b, cplx_t *c, cplx_t *d, cplx_t *r, cplx_t x[3]) {
  if(realIsZero(&a->Real) && realIsZero(&a->Imag)) {
    solveQuadraticEquation(b, c, d, r, &x[0], &x[1], &ctxtReal75);
    realSetNaN(&x[2].Real);
    realSetNaN(&x[2].Imag);
  }
  else if(realIsZero(&d->Real) && realIsZero(&d->Imag)) {
    solveQuadraticEquation(a, b, c, r, &x[0], &x[1], &ctxtReal75);
    realSetZero(&x[2].Real);
    realSetZero(&x[2].Imag);
  }
  else {
    solveCubic(a, b, c, d, r, &x[0], &x[1], &x[2]);
  }
  qsort(x, 3, sizeof(x[0]), &cmplxSortCompare);
}
#endif //OPTION_SLVQ_SLVC


/********************************************//**
 * \brief (d, c, b, a) ==> (x1, x2, r) c ==> regL
 * enables stack lift and refreshes the stack
 * A coefficient vector in X (highest degree first, 2 to 4 elements) returns all roots as a row vector in X instead.
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSlvc(uint16_t unusedButMandatoryParameter) {
#if defined(OPTION_SLVQ_SLVC)
  bool_t complexCoefs=false;
  cplx_t a, b, c, d, r;
  cplx_t x[3];

  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix || getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {   // a coefficient vector takes the matrix path: the element count picks the solver
    solveCoefficientVector();
    return;
  }

  if(!(getRegisterAsComplexOrReal(REGISTER_X, &d, &complexCoefs) &&
       getRegisterAsComplexOrReal(REGISTER_Y, &c, &complexCoefs) &&
       getRegisterAsComplexOrReal(REGISTER_Z, &b, &complexCoefs) &&
       getRegisterAsComplexOrReal(REGISTER_T, &a, &complexCoefs))) {
    return;
  }

  if(   realIsZero(&a.Real) && realIsZero(&a.Imag)
     && realIsZero(&b.Real) && realIsZero(&b.Imag)
     && realIsZero(&c.Real) && realIsZero(&c.Imag)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnSlvc:", "cannot use 0 for Y, Z and T as input of SLVC", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }


  if(!saveLastX()) {
    return;
  }


  solveGeneralCubic(&a, &b, &c, &d, &r, x);

  for(int i = 0; i < 3; i++) {
    if(realIsZero(&x[i].Imag) || (realIsNaN(&x[i].Real) && realIsNaN(&x[i].Imag))) {
      convertRealToResultRegister(&x[i].Real, REGISTER_X + i, amNone);
    }
    else {
      convertComplexToResultRegister(&x[i], REGISTER_X + i);
    }
    adjustResult(REGISTER_X + i, false, true, REGISTER_X + i, -1, -1);
  }
  temporaryInformation = TI_ROOTS3;

  #if defined(DISCRIMINANT)
    if(realIsZero(&r.Imag)) { // q3r2 is real
      convertRealToResultRegister(&r.Real, REGISTER_T, amNone);
    }
    else {
      convertComplexToResultRegister(&r, REGISTER_T);
    }
    adjustResult(REGISTER_T, false, true, REGISTER_T, -1, -1);
  #else // !DISCRIMINANT
    fnDropT(0);
  #endif // DISCRIMINANT
#endif // OPTION_SLVQ_SLVC
}


#if defined(OPTION_SLVQ_SLVC)
// X = a 1 x m or m x 1 coefficient vector, highest degree first: the element count picks the solver, 2 the linear, 3 the quadratic and 4 the cubic,
// from either command. All roots come back as a row vector replacing X, real when every root is real; the rest of the stack is not consumed.
void solveCoefficientVector(void) {
  real34Matrix_t xr;
  complex34Matrix_t xc;
  bool_t complexInput, resultIsComplex, allZero;
  uint16_t rows, cols, nRoots;
  uint32_t m, j;
  cplx_t co[4];                                          // co[0] the leading coefficient
  cplx_t r;
  cplx_t x[3];

  if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complexInput = true;
    linkToComplexMatrixRegister(REGISTER_X, &xc);
    rows = xc.header.matrixRows;
    cols = xc.header.matrixColumns;
  }
  else {
    complexInput = false;
    linkToRealMatrixRegister(REGISTER_X, &xr);
    rows = xr.header.matrixRows;
    cols = xr.header.matrixColumns;
  }

  m = (uint32_t)rows * cols;
  if((rows != 1 && cols != 1) || m < 2 || m > 4) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "a coefficient vector holds 2 to 4 elements, not (%d" STD_CROSS "%d)", rows, cols);
      moreInfoOnError("In function solveCoefficientVector:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  for(j = 0; j < m; j++) {
    if(complexInput) {
      real34ToReal(VARIABLE_REAL34_DATA(xc.matrixElements + j), &co[j].Real);
      real34ToReal(VARIABLE_IMAG34_DATA(xc.matrixElements + j), &co[j].Imag);
    }
    else {
      real34ToReal(xr.matrixElements + j, &co[j].Real);
      realSetZero(&co[j].Imag);
    }
  }

  allZero = true;                                        // the stack forms' refusal: the constant term alone is no equation
  for(j = 0; j + 1 < m; j++) {
    allZero = allZero && realIsZero(&co[j].Real) && realIsZero(&co[j].Imag);
  }
  if(allZero) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function solveCoefficientVector:", "every coefficient above the constant term is 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(m == 2) {                                           // the trivial linear case keeps the family symmetrical: the single root is -b/a, as SLVP delivers it
    if(realIsZero(&co[0].Imag) && realIsZero(&co[1].Imag)) {
      realDivide(&co[1].Real, &co[0].Real, &x[0].Real, &ctxtReal39);
      realChangeSign(&x[0].Real);
      realSetZero(&x[0].Imag);
    }
    else {
      divComplexComplex(&co[1], &co[0], &x[0], &ctxtReal75);
      chsComplex(&x[0]);
    }
    nRoots = 1;
  }
  else if(m == 3) {
    solveQuadratic(&co[0], &co[1], &co[2], &r, &x[0], &x[1]);
    nRoots = 2;
  }
  else {
    solveGeneralCubic(&co[0], &co[1], &co[2], &co[3], &r, x);
    nRoots = 3;
  }

  resultIsComplex = false;                               // a NaN pair is the stack forms' real NaN root, not a complex value
  for(j = 0; j < nRoots; j++) {
    if(!realIsZero(&x[j].Imag) && !(realIsNaN(&x[j].Real) && realIsNaN(&x[j].Imag))) {
      resultIsComplex = true;
      break;
    }
  }

  if(resultIsComplex) {                                  // the result matrix is allocated, then L, then X is overwritten: every failure leaves X and L untouched
    complex34Matrix_t res;
    if(!complexMatrixInit(&res, 1, nRoots)) {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function solveCoefficientVector:", "Ram full", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    if(!saveLastX()) {
      complexMatrixFree(&res);
      return;
    }
    for(j = 0; j < nRoots; j++) {
      realToReal34(&x[j].Real, VARIABLE_REAL34_DATA(res.matrixElements + j));
      realToReal34(&x[j].Imag, VARIABLE_IMAG34_DATA(res.matrixElements + j));
    }
    convertComplex34MatrixToComplex34MatrixRegister(&res, REGISTER_X);
    complexMatrixFree(&res);
  }
  else {
    real34Matrix_t res;
    if(!realMatrixInit(&res, 1, nRoots)) {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function solveCoefficientVector:", "Ram full", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    if(!saveLastX()) {
      realMatrixFree(&res);
      return;
    }
    for(j = 0; j < nRoots; j++) {
      realToReal34(&x[j].Real, res.matrixElements + j);
    }
    convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
    realMatrixFree(&res);
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}
#endif // OPTION_SLVQ_SLVC




static bool_t _checkConditionNumberOfAddSub(const real_t *operand1, const real_t *operand2, const real_t *res, realContext_t *realContext) {
  real_t conditionNumber1, conditionNumber2;
  real_t *conditionNumber = &conditionNumber1;

  if(realIsZero(res)) {
    return false;
  }
  else {
    realDivide(res, operand1, &conditionNumber1, realContext);
    realSetPositiveSign(&conditionNumber1);
    realDivide(res, operand2, &conditionNumber2, realContext);
    realSetPositiveSign(&conditionNumber2);
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
    realSetZero(res);
  }
  else {
    realCopy(&r, res);
  }
}
static void _realCheckedSubtract(const real_t *operand1, const real_t *operand2, real_t *res, realContext_t *realContext) {
  real_t minus;
  realCopy(operand2, &minus);
  realChangeSign(&minus);
  _realCheckedAdd(operand1, &minus, res, realContext);
}

#if defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159)
static void _solveMonicQuadratic159(const cplx159_t *b, const cplx159_t *c, cplx159_t *x1, cplx159_t *x2, realContext_t *realContext) {
  cplx159_t discriminant, sqrtDiscriminant, plus, minus;
  REAL_T_PTR(plusMagnitude2, 159);
  REAL_T_PTR(minusMagnitude2, 159);
  REAL_T_PTR(magnitudeTerm, 159);

  if(realIsZero(&c->Real) && realIsZero(&c->Imag)) {
    realMinus(&b->Real, &x1->Real, realContext);
    realMinus(&b->Imag, &x1->Imag, realContext);
    realSetZero(&x2->Real);
    realSetZero(&x2->Imag);
    return;
  }

  mulComplexComplex159(b, b, &discriminant, realContext);
  mulComplexReal159(c, const_4, &plus, realContext);
  subComplex159(&discriminant, &plus, &discriminant, realContext);
  sqrtComplex159(&discriminant, &sqrtDiscriminant, realContext);

  realMinus(&b->Real, &plus.Real, realContext);
  realMinus(&b->Imag, &plus.Imag, realContext);
  realCopy(&plus.Real, &minus.Real);
  realCopy(&plus.Imag, &minus.Imag);
  addComplex159(&plus, &sqrtDiscriminant, &plus, realContext);
  subComplex159(&minus, &sqrtDiscriminant, &minus, realContext);
  realMultiply(&plus.Real, &plus.Real, plusMagnitude2, realContext);
  realMultiply(&plus.Imag, &plus.Imag, magnitudeTerm, realContext);
  realAdd(plusMagnitude2, magnitudeTerm, plusMagnitude2, realContext);
  realMultiply(&minus.Real, &minus.Real, minusMagnitude2, realContext);
  realMultiply(&minus.Imag, &minus.Imag, magnitudeTerm, realContext);
  realAdd(minusMagnitude2, magnitudeTerm, minusMagnitude2, realContext);

  if(realCompareGreaterEqual(plusMagnitude2, minusMagnitude2)) {
    mulComplexReal159(&plus, const_1on2, x1, realContext);
  }
  else {
    mulComplexReal159(&minus, const_1on2, x1, realContext);
  }
  divComplexComplex159(c, x1, x2, realContext);
}


static void _deflateCubic159(const cplx159_t *c2, const cplx159_t *c1, const cplx159_t *c0,
                             cplx159_t *x1, cplx159_t *x2, cplx159_t *x3, realContext_t *realContext) {
  REAL_T_PTR(magnitude1, 159);
  REAL_T_PTR(magnitude2, 159);
  REAL_T_PTR(magnitude3, 159);
  REAL_T_PTR(magnitudeTerm, 159);
  cplx159_t root, b, c;

  realMultiply(&x1->Real, &x1->Real, magnitude1, realContext);
  realMultiply(&x1->Imag, &x1->Imag, magnitudeTerm, realContext);
  realAdd(magnitude1, magnitudeTerm, magnitude1, realContext);
  realMultiply(&x2->Real, &x2->Real, magnitude2, realContext);
  realMultiply(&x2->Imag, &x2->Imag, magnitudeTerm, realContext);
  realAdd(magnitude2, magnitudeTerm, magnitude2, realContext);
  realMultiply(&x3->Real, &x3->Real, magnitude3, realContext);
  realMultiply(&x3->Imag, &x3->Imag, magnitudeTerm, realContext);
  realAdd(magnitude3, magnitudeTerm, magnitude3, realContext);
  if(realCompareGreaterEqual(magnitude1, magnitude2) && realCompareGreaterEqual(magnitude1, magnitude3)) {
    realCopy(&x1->Real, &root.Real);
    realCopy(&x1->Imag, &root.Imag);
  }
  else if(realCompareGreaterEqual(magnitude2, magnitude3)) {
    realCopy(&x2->Real, &root.Real);
    realCopy(&x2->Imag, &root.Imag);
  }
  else {
    realCopy(&x3->Real, &root.Real);
    realCopy(&x3->Imag, &root.Imag);
  }

  if(realIsZero(&root.Real) && realIsZero(&root.Imag)) {
    return;
  }

  divComplexComplex159(c0, &root, &c, realContext);
  realChangeSign(&c.Real);
  realChangeSign(&c.Imag);
  subComplex159(&c, c1, &b, realContext);
  divComplexComplex159(&b, &root, &b, realContext);
  realCopy(&root.Real, &x1->Real);
  realCopy(&root.Imag, &x1->Imag);
  _solveMonicQuadratic159(&b, &c, x2, x3, realContext);
  addComplex159(x2, x3, &root, realContext);
  addComplex159(c2, &root, x1, realContext);
  realChangeSign(&x1->Real);
  realChangeSign(&x1->Imag);
}
#endif // OPTION_CUBIC_159 || OPTION_EIGEN_159


#if defined(OPTION_SLVQ_SLVC) && defined(OPTION_CUBIC_159)
// Real coefficients with D < 0 give a real root x1 and a pair u +- iv with D = -4 v^2 a^4 ((x1 - u)^2 + v^2)^2. Next to a double root the deflated quadratic
// cancels v^2 against u^2 at p159, while D has exact terms: where v^2 is 40 orders or more below (x1 - u)^2, v = sqrt(-D) / (2 a^2 (x1 - u)^2) to that order.
// u = (ad - bc) / (2 (a^2 x1^2 + ac)) at any size, as d - bc = (x1 + x2)(x1 + x3)(x2 + x3) for a = 1: ad - bc is exact and 0 for a pair on the imaginary axis,
// and the denominator is 2 a^2 ((x1 + u)^2 + v^2), with no cancellation.
// The real root is the one whose imaginary part is the smallest, as the solver sets it. When the solver has taken all three as real, its own test of the
// discriminant being coarser, the pair is the two closest roots.
static void _cubicPairFromDiscriminant(const real_t *a, const real_t *b, const real_t *c, const real_t *d, const real_t *discriminant, cplx159_t *x[3], realContext_t *realContext) {
  REAL_T_PTR(v, 159);
  REAL_T_PTR(denominator, 159);
  uint32_t r = 0, p, q;
  const bool_t allReal = realIsZero(&x[0]->Imag) && realIsZero(&x[1]->Imag) && realIsZero(&x[2]->Imag);

  if(allReal) {
    realSubtract(&x[0]->Real, &x[1]->Real, v, realContext);                  // r is the root outside the closest pair
    realSetPositiveSign(v);
    realSubtract(&x[1]->Real, &x[2]->Real, denominator, realContext);
    realSetPositiveSign(denominator);
    r = 2;
    if(realCompareLessThan(denominator, v)) {
      realCopy(denominator, v);
      r = 0;
    }
    realSubtract(&x[0]->Real, &x[2]->Real, denominator, realContext);
    realSetPositiveSign(denominator);
    if(realCompareLessThan(denominator, v)) {
      r = 1;
    }
  }
  else {
    if(realCompareAbsLessThan(&x[1]->Imag, &x[r]->Imag)) {
      r = 1;
    }
    if(realCompareAbsLessThan(&x[2]->Imag, &x[r]->Imag)) {
      r = 2;
    }
  }
  p = (r + 1) % 3;
  q = (r + 2) % 3;

  realMultiply(a, &x[r]->Real, v, realContext);
  realMultiply(v, v, v, realContext);
  realFMA(a, c, v, denominator, realContext);
  if(!realIsZero(denominator)) {
    realMultiply(b, c, v, realContext);
    realChangeSign(v);
    realFMA(a, d, v, v, realContext);
    realDivide(v, denominator, v, realContext);
    realMultiply(v, const_1on2, &x[p]->Real, realContext);
    realCopy(&x[p]->Real, &x[q]->Real);
  }
  realSubtract(&x[r]->Real, &x[p]->Real, denominator, realContext);
  realMultiply(denominator, denominator, denominator, realContext);
  realMultiply(&x[p]->Imag, &x[p]->Imag, v, realContext);                    // v^2 from the solver
  if(realIsZero(denominator) || (!allReal && realGetExponent(v) > realGetExponent(denominator) - 40)) {
    return;
  }
  realMultiply(a, a, v, realContext);
  realMultiply(denominator, v, denominator, realContext);
  realMultiply(denominator, const_2, denominator, realContext);
  realMinus(discriminant, v, realContext);
  realSquareRoot(v, v, realContext);
  realDivide(v, denominator, v, realContext);

  realCopy(v, &x[p]->Imag);                                                  // the roots are sorted afterwards
  realMinus(v, &x[q]->Imag, realContext);
}
#endif // OPTION_SLVQ_SLVC && OPTION_CUBIC_159


// a x^3 + b x^2 + c x + d = 0 at the precision the build selects: 159 digits under OPTION_CUBIC_159, 75 otherwise, results rounded to 39.
#if defined(OPTION_SLVQ_SLVC)
void solveCubic(const cplx_t *a, const cplx_t *b, const cplx_t *c, const cplx_t *d, cplx_t *r, cplx_t *x1, cplx_t *x2, cplx_t *x3) {
#if defined(OPTION_CUBIC_159)
  realContext_t ctx159 = ctxtReal75;
  ctx159.digits = 159;
  const bool_t realCoefficients = realIsZero(&a->Imag) && realIsZero(&b->Imag) && realIsZero(&c->Imag) && realIsZero(&d->Imag);
  REAL_T_PTR(discriminant, 159);
  realSetZero(discriminant);
  const bool_t onlyRealRoots = realCoefficients && realCubicHasOnlyRealRoots(&a->Real, &b->Real, &c->Real, &d->Real, 0, discriminant, &ctx159);
  cplx159_t x1H, x2H, x3H, r0, aH, bH, cH, dH;
  cplx159_t *x[3] = {&x1H, &x2H, &x3H};
  cplx159_t *t = &bH, *p = &cH;                                                    // free once the solver is done
  int32_t i;

  realCopy(&a->Real, &aH.Real);
  realCopy(&a->Imag, &aH.Imag);
  realCopy(&b->Real, &bH.Real);
  realCopy(&b->Imag, &bH.Imag);
  realCopy(&c->Real, &cH.Real);
  realCopy(&c->Imag, &cH.Imag);
  realCopy(&d->Real, &dH.Real);
  realCopy(&d->Imag, &dH.Imag);
  divComplexComplex159(&bH, &aH, &bH, &ctx159);
  divComplexComplex159(&cH, &aH, &cH, &ctx159);
  divComplexComplex159(&dH, &aH, &dH, &ctx159);
  solveCubicEquation159(&bH, &cH, &dH, &r0, &x1H, &x2H, &x3H, &ctx159);
  if(onlyRealRoots) {
    realSetZero(&x1H.Imag);
    realSetZero(&x2H.Imag);
    realSetZero(&x3H.Imag);
  }
  else if(realCoefficients && realIsNegative(discriminant)) {
    _cubicPairFromDiscriminant(&a->Real, &b->Real, &c->Real, &d->Real, discriminant, x, &ctx159);
  }

  // A root rounded to 34 digits, with a part more than 34 digits below the other dropped, is exact where the cubic takes it to 0 with no rounding: with
  // 34-digit coefficients the Horner terms stay within 159 digits. A root on an axis then has an exact 0 in place of the noise of the solver.
  for(i = 0; i < 3; i++) {
    ctx159.digits = 34;
    realPlus(&x[i]->Real, &t->Real, &ctx159);
    realPlus(&x[i]->Imag, &t->Imag, &ctx159);
    ctx159.digits = 159;
    if(realGetExponent(&t->Imag) < realGetExponent(&t->Real) - 34) {
      realSetZero(&t->Imag);
    }
    else if(realGetExponent(&t->Real) < realGetExponent(&t->Imag) - 34) {
      realSetZero(&t->Real);
    }
    ctx159.status = 0;
    mulComplexComplex159(&aH, t, p, &ctx159);
    realAdd(&p->Real, &b->Real, &p->Real, &ctx159);
    realAdd(&p->Imag, &b->Imag, &p->Imag, &ctx159);
    mulComplexComplex159(p, t, p, &ctx159);
    realAdd(&p->Real, &c->Real, &p->Real, &ctx159);
    realAdd(&p->Imag, &c->Imag, &p->Imag, &ctx159);
    mulComplexComplex159(p, t, p, &ctx159);
    realAdd(&p->Real, &d->Real, &p->Real, &ctx159);
    realAdd(&p->Imag, &d->Imag, &p->Imag, &ctx159);
    if(realIsZero(&p->Real) && realIsZero(&p->Imag) && !(ctx159.status & DEC_Inexact)) {
      realCopy(&t->Real, &x[i]->Real);
      realCopy(&t->Imag, &x[i]->Imag);
    }
  }
  realPlus(&r0.Real, &r->Real,  &ctxtReal39);
  realPlus(&r0.Imag, &r->Imag,  &ctxtReal39);
  realPlus(&x1H.Real, &x1->Real, &ctxtReal39);
  realPlus(&x1H.Imag, &x1->Imag, &ctxtReal39);
  realPlus(&x2H.Real, &x2->Real, &ctxtReal39);
  realPlus(&x2H.Imag, &x2->Imag, &ctxtReal39);
  realPlus(&x3H.Real, &x3->Real, &ctxtReal39);
  realPlus(&x3H.Imag, &x3->Imag, &ctxtReal39);
#else // OPTION_CUBIC_159
  // y = a x gives y^3 + b y^2 + a c y + a^2 d, whose coefficients stay exact where those of x^3 + b/a x^2 + c/a x + d/a are rounded
  cplx_t ac, a2d;
  mulComplexComplex(c, a, &ac, &ctxtReal75);
  mulComplexComplex(d, a, &a2d, &ctxtReal75);
  mulComplexComplex(&a2d, a, &a2d, &ctxtReal75);
  solveCubicEquation(b, &ac, &a2d, r, x1, x2, x3, &ctxtReal75);
  divComplexComplex(x1, a, x1, &ctxtReal75);
  divComplexComplex(x2, a, x2, &ctxtReal75);
  divComplexComplex(x3, a, x3, &ctxtReal75);
#endif // OPTION_CUBIC_159
}
#endif // OPTION_SLVQ_SLVC


void solveCubicEquation(const cplx_t *c2, const cplx_t *c1, const cplx_t *c0, cplx_t *r, cplx_t *x1, cplx_t *x2, cplx_t *x3, realContext_t *realContext) {
  // x^3 + b x^2 + c x + d = 0
  // Abramowitz & Stegun §3.8.2
  cplx_t q, rr, s1, s2, a;
  const bool_t realIn = realIsZero(&c2->Imag) && realIsZero(&c1->Imag) && realIsZero(&c0->Imag);
  bool_t swapped;

  // Compute q, r and the discriminant
  // This is done by scaling things up so that divisions are avoided until the final step.
  // This reduces rounding problems and gives an exact discriminant for integer (and other)
  // coefficients.
  // q has a denominator of 9, r has a denomination of 54.  q^3 therefore has a denominator
  // of 729 and r^2 of 2916.  729 times 4 is 2916, so we upscale by 2916.

  // q = (c - b^2 / 3) / 3
  // -9q = b^2 - 3c
  mulComplexReal(c1, const_3, &rr, realContext);
  mulComplexComplex(c2, c2, &a, realContext);
  subComplex(&a, &rr, &q, realContext);

  // r = (b c - 3 d) / 6 - b^3 / 27
  // 54r = 9(b c - 3 d) - 2 b^3
  mulComplexComplex(c2, c1, &rr, realContext);
  mulComplexReal(c0, const_3, &s1, realContext);
  subComplex(&rr, &s1, &rr, realContext);
  mulComplexReal(&rr, const_9, &rr, realContext);

  mulComplexComplex(&a, c2, &a, realContext);
  addComplex(&a, &a, &a, realContext);
  subComplex(&rr, &a, &rr, realContext);

  // q^3 + r^2 = (-4 (-9q)^3 + (54r)^2) / 2916
  mulComplexComplex(&q, &q, r, realContext);
  mulComplexComplex(r, &q, r, realContext);
  mulComplexReal(r, const__4, r, realContext);
  mulComplexComplex(&rr, &rr, &a, realContext);
  addComplex(r, &a, r, realContext);
  divComplexReal(r, const_2916, r, realContext);

  // Scale r back to it's proper range, q isn't needed anymore so it's good.
  divComplexReal(&rr, const_54, &rr, realContext);

  // s1 = cbrt(r + sqrt(q^3 + r^2)) and s2 = -q / s1, as two cube roots taken apart can differ by a cube root of unity. The square root takes the sign that
  // makes r + sqrt the larger sum, Re(r conj(sqrt)) >= 0; with the sign changed, s1 and s2 trade places, and s1 - s2 is negated to keep the order of the roots.
  sqrtComplex(r, &s2, realContext);
  realMultiply(&rr.Real, &s2.Real, &a.Real, realContext);
  realFMA(&rr.Imag, &s2.Imag, &a.Real, &a.Real, realContext);
  swapped = realIsNegative(&a.Real);
  if(swapped) {
    chsComplex(&s2);
  }
  addComplex(&rr, &s2, &s1, realContext);
  curtComplex(&s1, &s1, realContext);
  if(!realIsZero(&s1.Real)) {                               // a cube root with no real part is 0, and then r, the square root and s2 are 0 too
    divComplexComplex(&q, &s1, &s2, realContext);
    divComplexReal(&s2, const_9, &s2, realContext);
  }

  // reusing q, r for (s1 ± s2)
  addComplex(&s1, &s2, &q, realContext);
  subComplex(&s1, &s2, &rr, realContext);
  if(swapped) {
    chsComplex(&rr);
  }
  realSetZero(&a.Real);
  realCopy(const39_root3on2, &a.Imag);
  mulComplexComplex(&rr, &a, &rr, realContext);

  // roots
  divComplexReal(c2, const_3, x2, realContext);
  _realCheckedSubtract(&q.Real, &x2->Real, &x1->Real, realContext);
  _realCheckedSubtract(&q.Imag, &x2->Imag, &x1->Imag, realContext);
  mulComplexReal(&q, const_1on2, x3, realContext);
  _realCheckedAdd(&x3->Real, &x2->Real, &x3->Real, realContext);
  _realCheckedAdd(&x3->Imag, &x2->Imag, &x3->Imag, realContext);
  chsComplex(x3);
  _realCheckedAdd(&x3->Real, &rr.Real, &x2->Real, realContext);
  _realCheckedAdd(&x3->Imag, &rr.Imag, &x2->Imag, realContext);
  _realCheckedSubtract(&x3->Real, &rr.Real, &x3->Real, realContext);
  _realCheckedSubtract(&x3->Imag, &rr.Imag, &x3->Imag, realContext);

  // Force real outputs when the roots are known to be real. With one real root and a pair, s1 and s2 are real and x1 is the real root already.
  if(realIn && realCompareLessEqual(&r->Real, const_0)) {
    realSetZero(&x1->Imag);
    realSetZero(&x2->Imag);
    realSetZero(&x3->Imag);
  }

#if defined(DISCRIMINANT)
  // discriminant
  mulComplexReal(r, const__108, r, realContext);
#endif // DISCRIMINANT
}



#if defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159)
// Calculate of cubic equation using 159-digit precision
// Standard form: x³ + c2·x² + c1·x + c0 = 0
// Abramowitz & Stegun §3.8.2
void solveCubicEquation159(const cplx159_t *c2, const cplx159_t *c1, const cplx159_t *c0, cplx159_t *r, cplx159_t *x1, cplx159_t *x2, cplx159_t *x3, realContext_t *realContext) {

  const bool_t realIn = realIsZero(&c2->Imag) && realIsZero(&c1->Imag) && realIsZero(&c0->Imag);

  // Compute high-precision constant sqrt(3)/2
  REAL_T_PTR(const159_root3on2, 159);
  realSquareRoot(const_3, const159_root3on2, realContext);
  realMultiply(const159_root3on2, const_1on2, const159_root3on2, realContext);

  // Intermediate variables in 159-digit precision
  cplx159_t q, rr, s1, s2, a;

  realSetZero(&q.Real);
  realSetZero(&q.Imag);
  realSetZero(&rr.Real);
  realSetZero(&rr.Imag);
  realSetZero(&s1.Real);
  realSetZero(&s1.Imag);
  realSetZero(&s2.Real);
  realSetZero(&s2.Imag);
  realSetZero(&a.Real);
  realSetZero(&a.Imag);

  // Compute q, r and the discriminant
  // This is done by scaling things up so that divisions are avoided until the final step.
  // This reduces rounding problems and gives an exact discriminant for integer (and other)
  // coefficients.
  // q has a denominator of 9, r has a denomination of 54.  q^3 therefore has a denominator
  // of 729 and r^2 of 2916.  729 times 4 is 2916, so we upscale by 2916.

  // q = (c - b^2 / 3) / 3
  // 9q = (3c - b^2)
  mulComplexReal159(c1, const_3, &rr, realContext);
  mulComplexComplex159(c2, c2, &q, realContext);
  subComplex159(&rr, &q, &q, realContext);

  // r = (b c - 3 d) / 6 - b^3 / 27
  // 54r = 9(b c - 3 d) - 2 b^3
  mulComplexComplex159(c2, c1, &rr, realContext);
  mulComplexReal159(c0, const_3, &a, realContext);
  subComplex159(&rr, &a, &rr, realContext);
  mulComplexReal159(&rr, const_9, &rr, realContext);

  mulComplexComplex159(c2, c2, &a, realContext);
  mulComplexComplex159(&a, c2, &a, realContext);
  addComplex159(&a, &a, &a, realContext);
  subComplex159(&rr, &a, &rr, realContext);

  // q^3 + r^2 = (4 (9q)^3 + (54r)^2) / 2916
//  mulComplexComplex(qr, qi, qr, qi, rReal, rImag, realContext);
//  mulComplexComplex(rReal, rImag, qr, qi, rReal, rImag, realContext);
//  mulComplexReal(rReal, rImag, const_4, rReal, rImag, realContext);
//  mulComplexComplex(rr, ri, rr, ri, ar, ai, realContext);
//  addComplex(rReal, rImag, ar, ai, rReal, rImag, realContext);
//  divComplexReal(rReal, rImag, const_2916, rReal, rImag, realContext);

// Compute discriminant using intermediate 159-digit variables
  cplx159_t discrim;
  realSetZero(&discrim.Real);
  realSetZero(&discrim.Imag);

  // q^3 + r^2 = (4 (9q)^3 + (54r)^2) / 2916
  mulComplexComplex159(&q, &q, &discrim, realContext);
  mulComplexComplex159(&discrim, &q, &discrim, realContext);
  mulComplexReal159(&discrim, const_4, &discrim, realContext);
  mulComplexComplex159(&rr, &rr, &a, realContext);
  addComplex159(&discrim, &a, &discrim, realContext);
  divComplexReal159(&discrim, const_2916, &discrim, realContext);
  // Copy discriminant to output
  realCopy(&discrim.Real, &r->Real);
  realCopy(&discrim.Imag, &r->Imag);

  // Scale r back to it's proper range, q isn't needed anymore so it's good.
  divComplexReal159(&rr, const_54, &rr, realContext);

  // s1, s2 = cbrt(r ± sqrt(q^3 + r^2))
  sqrtComplex159(&discrim, &s1, realContext);
  subComplex159(&rr, &s1, &s2, realContext);
  addComplex159(&rr, &s1, &s1, realContext);
  // Two cube roots taken apart can differ by a cube root of unity, which turns two of the roots by 120 degrees: the larger of r +- sqrt takes the cube root, and
  // the other is -q over it, as s1 s2 = -q.
  realMultiply(&s1.Real, &s1.Real, &a.Real, realContext);
  realFMA(&s1.Imag, &s1.Imag, &a.Real, &a.Real, realContext);
  realMultiply(&s2.Real, &s2.Real, &a.Imag, realContext);
  realFMA(&s2.Imag, &s2.Imag, &a.Imag, &a.Imag, realContext);
  {
    const bool_t s2Larger = realCompareLessThan(&a.Real, &a.Imag);
    cplx159_t *root = s2Larger ? &s2 : &s1;
    cplx159_t *other = s2Larger ? &s1 : &s2;

    curtComplex159(root, root, realContext);
    if(realIsZero(&root->Real) && realIsZero(&root->Imag)) {
      realSetZero(&other->Real);
      realSetZero(&other->Imag);
    }
    else {
      divComplexComplex159(&q, root, other, realContext);
      divComplexReal159(other, const_9, other, realContext);                             // qr holds 9q
      realChangeSign(&other->Real);
      realChangeSign(&other->Imag);
    }
  }

  // reusing q, r for (s1 ± s2)
  addComplex159(&s1, &s2, &q, realContext);
  subComplex159(&s1, &s2, &rr, realContext);
  realSetZero(&a.Real);
  realCopy(const159_root3on2, &a.Imag);
  mulComplexComplex159(&rr, &a, &rr, realContext);

// roots
  // x2 = c2 / 3
  // x1 = x2 - (s1 + s2)
  // x3 = x2 + (s1 - s2)/2 - (s1 + s2)/2
  // s1 = cbrt(r + sqrt(q^3 + r^2))
  // s2 = cbrt(r - sqrt(q^3 + r^2))
  // x2 = c2 / 3; x1 = x2 - (s1 + s2); x3 = x2 + (s1 - s2)/2 - (s1 + s2)/2

  divComplexReal159(c2, const_3, x2, realContext);                         // x2 = c2/3
  realSubtract(&q.Real, &x2->Real, &x1->Real, realContext);        // x1Real = qr - x2Real
  realSubtract(&q.Imag, &x2->Imag, &x1->Imag, realContext);        // x1Imag = qi - x2Imag
  mulComplexReal159(&q, const_1on2, x3, realContext);                 // x3 = (qr + i*qi)*1/2
  realAdd(&x3->Real, &x2->Real, &x3->Real, realContext);           // x3Real += x2Real
  realAdd(&x3->Imag, &x2->Imag, &x3->Imag, realContext);           // x3Imag += x2Imag
  realChangeSign(&x3->Real);                                   // x3 = -x3 (flip sign)
  realChangeSign(&x3->Imag);
  realAdd(&x3->Real, &rr.Real, &x2->Real, realContext);            // x2Real = x3Real + rr
  realAdd(&x3->Imag, &rr.Imag, &x2->Imag, realContext);            // x2Imag = x3Imag + ri
  realSubtract(&x3->Real, &rr.Real, &x3->Real, realContext);       // x3Real -= rr
  realSubtract(&x3->Imag, &rr.Imag, &x3->Imag, realContext);       // x3Imag -= ri

  // One Cardano root has no subtraction between unlike root scales. Deflation reconstructs the other two without that loss.
  _deflateCubic159(c2, c1, c0, x1, x2, x3, realContext);

  // Force real outputs when the roots are known to be real
  if(realIn) {
    if(realCubicHasOnlyRealRoots(const_1, &c2->Real, &c1->Real, &c0->Real, 156, NULL, realContext)) {
      /* Three real roots */
      realSetZero(&x1->Imag);
      realSetZero(&x2->Imag);
      realSetZero(&x3->Imag);
    }
    else {
      /* One real, two complex roots */
      if(realCompareAbsLessThan(&x1->Imag, &x2->Imag)) {
        if(realCompareAbsLessThan(&x1->Imag, &x3->Imag)) {
          realSetZero(&x1->Imag);
        }
        else {
          realSetZero(&x3->Imag);
        }
      }
      else {
        if(realCompareAbsLessThan(&x2->Imag, &x3->Imag)) {
          realSetZero(&x2->Imag);
        }
        else {
          realSetZero(&x3->Imag);
        }
      }
    }
  }

#if defined(DISCRIMINANT)
  // discriminant
  mulComplexReal159(r, const__108, r, realContext);
#endif // DISCRIMINANT
}
#endif //OPTION_CUBIC_159 || OPTION_EIGEN_159


//--------------------------------------------------------------------------------------------------------------------------------------------------
// Trigonometric method -- not suitable until the new high digit count Trig WP34 SinCosTan supports 159
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
//   REAL_T_PTR(qr, 159);
//   REAL_T_PTR(qi, 159);
//   REAL_T_PTR(rr, 159);
//   REAL_T_PTR(ri, 159);
//   REAL_T_PTR(s1r, 159);
//   REAL_T_PTR(s1i, 159);
//   REAL_T_PTR(s2r, 159);
//   REAL_T_PTR(s2i, 159);
//   REAL_T_PTR(ar, 159);
//   REAL_T_PTR(ai, 159);
//   REAL_T_PTR(temp1, 159);
//   REAL_T_PTR(temp2, 159);
//   REAL_T_PTR(temp3, 159);
//   REAL_T_PTR(temp4, 159);
//   REAL_T_PTR(mag, 159);
//   REAL_T_PTR(angle, 159);
//
//   const bool_t realIn = realIsZero(c2Imag) && realIsZero(c1Imag) && realIsZero(c0Imag);
//
//   // Initialize ALL intermediate variables
//   realSetZero(qr);
//   realSetZero(qi);
//   realSetZero(rr);
//   realSetZero(ri);
//   realSetZero(s1r);
//   realSetZero(s1i);
//   realSetZero(s2r);
//   realSetZero(s2i);
//   realSetZero(ar);
//   realSetZero(ai);
//   realSetZero(temp1);
//   realSetZero(temp2);
//   realSetZero(temp2);
//   realSetZero(temp2);
//   realSetZero(mag);
//   realSetZero(angle);
//
//   // ===== q = (c - b^2 / 3) / 3 =====
//   // 9q = (3c - b^2)
//
//   // rr+ri = 3*c1
//   realMultiply(c1Real, const_3, rr, realContext);
//   realMultiply(c1Imag, const_3, ri, realContext);
//
//   // qr+qi = c2^2 (complex square: (a+bi)^2 = a²-b² + 2abi)
//   realMultiply(c2Real, c2Real, temp1, realContext); // a²
//   realMultiply(c2Imag, c2Imag, temp2, realContext); // b²
//   printf("realSubtract 000\n");
//   realSubtract(temp1, temp2, qr, realContext);      // a²-b²
//   realMultiply(c2Real, c2Imag, temp1, realContext); // ab
//   realAdd(temp1, temp1, qi, realContext); // 2ab
//
//   // qr+qi = rr+ri - (qr+qi) = 3c - b²
//   printf("realSubtract 001\n");
//   realSubtract(rr, qr, qr, realContext);
//   printf("realSubtract 002\n");
//   realSubtract(ri, qi, qi, realContext);
//
//   // ===== r = (b c - 3 d) / 6 - b^3 / 27 =====
//   // 54r = 9(b c - 3 d) - 2 b^3
//
//   // rr+ri = c2 * c1 (complex multiply manually)
//   realMultiply(c2Real, c1Real, temp1, realContext);
//   realMultiply(c2Imag, c1Imag, temp2, realContext);
//   printf("realSubtract 003\n");
//   realSubtract(temp1, temp2, rr, realContext);
//   realMultiply(c2Real, c1Imag, temp1, realContext);
//   realMultiply(c2Imag, c1Real, temp2, realContext);
//   realAdd(temp1, temp2, ri, realContext);
//
//   // ar+ai = 3*c0
//   realMultiply(c0Real, const_3, ar, realContext);
//   realMultiply(c0Imag, const_3, ai, realContext);
//
//   // rr+ri = rr+ri - (ar+ai)
//   printf("realSubtract 004\n");
//   realSubtract(rr, ar, rr, realContext);
//   printf("realSubtract 005\n");
//   realSubtract(ri, ai, ri, realContext);
//
//   // rr+ri = 9 * (rr+ri)
//   realMultiply(rr, const_9, rr, realContext);
//   realMultiply(ri, const_9, ri, realContext);
//
//   // ar+ai = c2^2 (already computed above as temp for qr+qi, recalculate)
//   realMultiply(c2Real, c2Real, temp1, realContext);
//   realMultiply(c2Imag, c2Imag, temp2, realContext);
//   printf("realSubtract 006\n");
//   realSubtract(temp1, temp2, ar, realContext);
//   realMultiply(c2Real, c2Imag, temp1, realContext);
//   realAdd(temp1, temp1, ai, realContext);
//
//   // ar+ai = (ar+ai) * c2 = c2^3 (complex multiply)
//   realMultiply(ar, c2Real, temp1, realContext);
//   realMultiply(ai, c2Imag, temp2, realContext);
//   printf("realSubtract 007\n");
//   realSubtract(temp1, temp2, temp2, realContext);
//   realMultiply(ar, c2Imag, temp1, realContext);
//   realMultiply(ai, c2Real, temp2, realContext);
//   realAdd(temp1, temp2, temp2, realContext);
//   realCopy(temp2, ar);
//   realCopy(temp2, ai);
//
//   // ar+ai = 2 * (ar+ai)
//   realAdd(ar, ar, ar, realContext);
//   realAdd(ai, ai, ai, realContext);
//
//   // rr+ri = rr+ri - (ar+ai)
//   printf("realSubtract 008\n");
//   realSubtract(rr, ar, rr, realContext);
//   printRealToConsole(ri, "ri:", "\n");
//   printf("realSubtract 009\n");
//   realSubtract(ri, ai, ri, realContext);
//
//   // ===== discriminant = q^3 + r^2 = (4 (9q)^3 + (54r)^2) / 2916 =====
//
//   // rReal+rImag = qr+qi squared (complex square)
//   realMultiply(qr, qr, temp1, realContext);
//   realMultiply(qi, qi, temp2, realContext);
//   printf("realSubtract 010\n");
//   realSubtract(temp1, temp2, rReal, realContext);
//   realMultiply(qr, qi, temp1, realContext);
//   realAdd(temp1, temp1, rImag, realContext);
//
//   // rReal+rImag = (rReal+rImag) * (qr+qi) = q^3 (complex multiply)
//   realMultiply(rReal, qr, temp1, realContext);
//   realMultiply(rImag, qi, temp2, realContext);
//   printf("realSubtract 011\n");
//   realSubtract(temp1, temp2, temp2, realContext);
//   realMultiply(rReal, qi, temp1, realContext);
//   realMultiply(rImag, qr, temp2, realContext);
//   realAdd(temp1, temp2, temp2, realContext);
//   realCopy(temp2, rReal);
//   realCopy(temp2, rImag);
//
//   // rReal+rImag = 4 * (rReal+rImag)
//   realMultiply(rReal, const_4, rReal, realContext);
//   realMultiply(rImag, const_4, rImag, realContext);
//
//   // ar+ai = rr+ri squared (complex square)
//   realMultiply(rr, rr, temp1, realContext);
//   realMultiply(ri, ri, temp2, realContext);
//   printf("realSubtract 012\n");
//   realSubtract(temp1, temp2, ar, realContext);
//   realMultiply(rr, ri, temp1, realContext);
//   realAdd(temp1, temp1, ai, realContext);
//
//   // rReal+rImag = rReal+rImag + ar+ai
//   realAdd(rReal, ar, rReal, realContext);
//   realAdd(rImag, ai, rImag, realContext);
//
//   // rReal+rImag = (rReal+rImag) / 2916 (complex divide by real)
//   realDivide(rReal, const_2916, rReal, realContext);
//   realDivide(rImag, const_2916, rImag, realContext);
//
//   // ===== Scale r back: rr+ri = rr+ri / 54 =====
//   realDivide(rr, const_54, rr, realContext);
//   realDivide(ri, const_54, ri, realContext);
//
//   // ===== s1+s2 = cbrt(r ± sqrt(discriminant)) =====
//
//   // s1r+s1i = sqrt(rReal+rImag) using polar form
//   // mag = sqrt(rReal² + rImag²)
//   realMultiply(rReal, rReal, temp1, realContext);
//   realMultiply(rImag, rImag, temp2, realContext);
//   realAdd(temp1, temp2, mag, realContext);
//   realSquareRoot(mag, mag, realContext);
//
//   // angle = atan2(rImag, rReal) / 2
//   C47_WP34S_Atan2(rReal, rImag, angle, realContext);
//   realMultiply(angle, const_1on2, angle, realContext);
//
//   // mag = sqrt(mag)
//   realSquareRoot(mag, mag, realContext);
//
//   // s1r+s1i = mag * (cos(angle) + i*sin(angle))
//   C47_WP34S_Cvt2RadSinCosTan(angle, amRadian, s1r, s1i, NULL, realContext);
//   realMultiply(s1r, mag, s1r, realContext);
//   realMultiply(s1i, mag, s1i, realContext);
//
//   // s2r+s2i = rr+ri - (s1r+s1i)
//   printf("realSubtract 012\n");
//   realSubtract(rr, s1r, s2r, realContext);
//   printf("realSubtract 013\n");
//   realSubtract(ri, s1i, s2i, realContext);
//
//   // s1r+s1i = rr+ri + (s1r+s1i)
//   realAdd(rr, s1r, s1r, realContext);
//   realAdd(ri, s1i, s1i, realContext);
//
//   // s1r+s1i = cbrt(s1r+s1i) using polar form
//   // mag = (s1r² + s1i²)^(1/6)
//   realMultiply(s1r, s1r, temp1, realContext);
//   realMultiply(s1i, s1i, temp2, realContext);
//   realAdd(temp1, temp2, mag, realContext);
//   realSquareRoot(mag, mag, realContext); // sqrt = ^(1/2)
//   realPower(mag, const159_1on3, mag, realContext); // ^(1/3) => total ^(1/6)
//
//   // angle = atan2(s1i, s1r) / 3
//   C47_WP34S_Atan2(s1r, s1i, angle, realContext);
//   realDivide(angle, const_3, angle, realContext);
//
//   // s1r+s1i = mag * (cos(angle) + i*sin(angle))
//   C47_WP34S_Cvt2RadSinCosTan(angle, amRadian, temp1, temp2, NULL, realContext);
//   realMultiply(temp1, mag, s1r, realContext);
//   realMultiply(temp2, mag, s1i, realContext);
//
//   // s2r+s2i = cbrt(s2r+s2i) using polar form (same process)
//   realMultiply(s2r, s2r, temp1, realContext);
//   realMultiply(s2i, s2i, temp2, realContext);
//   realAdd(temp1, temp2, mag, realContext);
//   realSquareRoot(mag, mag, realContext);
//   realPower(mag, const159_1on3, mag, realContext);
//
//   C47_WP34S_Atan2(s2r, s2i, angle, realContext);
//   realDivide(angle, const_3, angle, realContext);
//
//   C47_WP34S_Cvt2RadSinCosTan(angle, amRadian, temp1, temp2, NULL, realContext);
//   realMultiply(temp1, mag, temp2, realContext);
//   realMultiply(temp2, mag, temp2, realContext);
//   realCopy(temp2, s2r);
//   realCopy(temp2, s2i);
//
//   // ===== Compute roots =====
//
//   // qr+qi = s1+s2
//   realAdd(s1r, s2r, qr, realContext);
//   realAdd(s1i, s2i, qi, realContext);
//
//   // rr+ri = (s1-s2) * i*sqrt(3)/2
//   printf("realSubtract 014\n");
//   realSubtract(s1r, s2r, temp1, realContext);
//   printf("realSubtract 015\n");
//   realSubtract(s1i, s2i, temp2, realContext);
//   // Multiply by i*sqrt(3)/2 = multiply by (0 + i*sqrt(3)/2)
//   // result_real = real*0 - imag*sqrt(3)/2
//   // result_imag = real*sqrt(3)/2 + imag*0
//   realMultiply(temp2, const159_root3on2, rr, realContext);
//   realChangeSign(rr);
//   realMultiply(temp1, const159_root3on2, ri, realContext);
//
//   // x2Real+x2Imag = c2 / 3
//   realDivide(c2Real, const_3, x2Real, realContext);
//   realDivide(c2Imag, const_3, x2Imag, realContext);
//
//   // x1 = qr+qi - (x2)
//   printf("realSubtract 016\n");
//   realSubtract(qr, x2Real, x1Real, realContext);
//   printf("realSubtract 017\n");
//   realSubtract(qi, x2Imag, x1Imag, realContext);
//
//   // temp = (qr+qi) / 2
//   realDivide(qr, const_2, qr, realContext);
//   realDivide(qi, const_2, qi, realContext);
//
//   // x3 = temp + x2
//   realAdd(qr, x2Real, x3Real, realContext);
//   realAdd(qi, x2Imag, x3Imag, realContext);
//
//   // x3 = -x3
//   realChangeSign(x3Real);
//   realChangeSign(x3Imag);
//
//   // x2 = x3 + (rr+ri)
//   realAdd(x3Real, rr, x2Real, realContext);
//   realAdd(x3Imag, ri, x2Imag, realContext);
//
//   // x3 = x3 - (rr+ri)
//   printf("realSubtract 018\n");
//   realSubtract(x3Real, rr, x3Real, realContext);
//   printf("realSubtract 019\n");
//   realSubtract(x3Imag, ri, x3Imag, realContext);
//
//   // ===== Force real outputs when appropriate =====
//   if(realIn) {
//     if(realIsZero(rReal) || realIsNegative(rImag)) {
//       realSetZero(x1Imag);
//       realSetZero(x2Imag);
//       realSetZero(x3Imag);
//     }
//     else {
//       if(realCompareAbsLessThan(x1Imag, x2Imag)) {
//         if(realCompareAbsLessThan(x1Imag, x3Imag)) {
//           realSetZero(x1Imag);
//         }
//         else {
//           realSetZero(x3Imag);
//         }
//       }
//       else {
//         if(realCompareAbsLessThan(x2Imag, x3Imag)) {
//           realSetZero(x2Imag);
//         }
//         else {
//           realSetZero(x3Imag);
//         }
//       }
//     }
//   }
// }

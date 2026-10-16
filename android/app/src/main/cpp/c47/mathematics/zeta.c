// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file zeta.c
 ***********************************************/

#include "c47.h"

/**************************************************************************/
/* Complex zeta function implementation based on Jean-Marc Baillard's from:
 * http://hp41programs.yolasite.com/zeta.php
 */

#if defined(OPTION_ZETA_BETA)
static bool_t zeta_calc_complex(cplx_t *reg45, const cplx_t *reg67, realContext_t *realContext) {
  real_t reg1, reg2, reg3;
  cplx_t sr, qp, reg0, reg89;
  const real_t *ln2 = realContext->digits > 39 ? const75_ln2 : const39_ln2;
  const real_t *piOn2 = realContext->digits > 39 ? const75_piOn2 : const39_piOn2;

  realCopyAbs(&reg67->Imag, &qp.Imag);
  realMultiply(piOn2, &qp.Imag, &qp.Real, realContext);
  realMultiply(&qp.Imag, const_2, &qp.Imag, realContext);
  WP34S_Ln1P(&qp.Imag, &qp.Imag, realContext);
  realAdd(&qp.Real, &qp.Imag, &qp.Imag, realContext);
  realCopy(const_3, &qp.Real);
  qp.Real.exponent += 10;
  WP34S_Ln(&qp.Real, &qp.Real, realContext);
  realAdd(&qp.Imag, &qp.Real, &qp.Imag, realContext);
  realSquareRoot(const_8, &qp.Real, realContext);
  realAdd(&qp.Real, const_3, &qp.Real, realContext);
  WP34S_Ln(&qp.Real, &qp.Real, realContext);
  realDivide(&qp.Imag, &qp.Real, &qp.Imag, realContext);
  realToIntegralValue(&qp.Imag, &qp.Imag, DEC_ROUND_DOWN, realContext);
  realAdd(&qp.Imag, const_1, &qp.Imag, realContext);
  realMultiply(&qp.Imag, const_4, &qp.Imag, realContext); // for extra digits we have
  if(realCompareGreaterThan(&qp.Imag, const_10000)) {
    realSetNaN(&reg45->Real);
    realSetNaN(&reg45->Imag);
    return false;
  }
  realCopy(&qp.Imag, &reg0.Real);
  realSetZero(&reg0.Imag);
  realSetOne(&reg1);
  realCopy(&qp.Imag, &reg2);
  realSetOne(&reg3);
  realSetOne(&reg45->Real);
  realPower(const__1, &qp.Imag, &qp.Imag, realContext);
  realChangeSign(&qp.Imag);
  realCopy(&qp.Imag, &reg45->Imag);
  realSetZero(&reg89.Real);
  realSetZero(&reg89.Imag);

  do { // zeta_loop
    int32_t loop = realToInt32C47(&reg0.Real, NULL);
    if(monitorExit(&loop, "Iter: ")) {
      break;
    }

    realMinus(&reg67->Real, &qp.Real, realContext);
    realMinus(&reg67->Imag, &qp.Imag, realContext);
    PowerComplex(&reg0, &qp, &sr, realContext);
    realChangeSign(&reg45->Imag);
    realMultiply(&reg45->Real, &reg45->Imag, &qp.Imag, realContext);
    realMultiply(&qp.Imag, &sr.Imag, &sr.Imag, realContext);
    realMultiply(&qp.Imag, &sr.Real, &sr.Real, realContext);
    realAdd(&reg89.Real, &sr.Real, &reg89.Real, realContext);
    realAdd(&reg89.Imag, &sr.Imag, &reg89.Imag, realContext);
    realMultiply(&reg0.Real, const_2, &qp.Imag, realContext);
    realMultiply(&qp.Imag, &reg0.Real, &qp.Imag, realContext);
    realSubtract(&qp.Imag, &reg0.Real, &qp.Imag, realContext);
    realMultiply(&qp.Imag, &reg3, &qp.Imag, realContext);
    realMultiply(&reg2, &reg2, &qp.Real, realContext);
    realSubtract(&reg0.Real, const_1, &sr.Real, realContext);
    realMultiply(&sr.Real, &sr.Real, &sr.Real, realContext);
    realSubtract(&qp.Real, &sr.Real, &qp.Real, realContext);
    realMultiply(&qp.Real, const_2, &qp.Real, realContext);
    realDivide(&qp.Imag, &qp.Real, &qp.Imag, realContext);
    realCopy(&qp.Imag, &reg3);
    realAdd(&reg45->Real, &qp.Imag, &reg45->Real, realContext);
    realSubtract(&reg0.Real, const_1, &reg0.Real, realContext);
  } while(realCompareGreaterThan(&reg0.Real, const_0));
  realDivide(&reg89.Real, &reg45->Real, &reg89.Real, realContext);
  realDivide(&reg89.Imag, &reg45->Real, &reg89.Imag, realContext);
  realSubtract(const_1, &reg67->Real, &qp.Imag, realContext);
  realMultiply(ln2, &qp.Imag, &qp.Imag, realContext);
  WP34S_ExpM1(&qp.Imag, &reg1, realContext);
  realMinus(&reg67->Imag, &qp.Imag, realContext);
  realMultiply(&qp.Imag, ln2, &qp.Imag, realContext);
  realPolarToRectangular(const_1, &qp.Imag, &qp, realContext);
  realSubtract(&qp.Real, const_1, &sr.Imag, realContext);
  realMultiply(&qp.Real, &reg1, &qp.Real, realContext);
  realMultiply(&reg1, &qp.Imag, &sr.Real, realContext);
  realAdd(&sr.Real, &qp.Imag, &qp.Imag, realContext);
  realAdd(&qp.Real, &sr.Imag, &qp.Real, realContext);

  divComplexComplex(&reg89, &qp, reg45, realContext);
  return true;
}
#endif // OPTION_ZETA_BETA

void ComplexZeta(const cplx_t *x, cplx_t *res, realContext_t *realContext) {
#if defined(OPTION_ZETA_BETA)
  cplx_t qp, sr, reg45, reg67, reg1011;
  const real_t *pi = realContext->digits > 39 ? const75_pi : const39_pi;

  if(realIsZero(&x->Imag)) {
    WP34S_Zeta(&x->Real, &res->Real, realContext);
    realCopy(&x->Imag, &res->Imag);
    return;
  }
  if(realIsNaN(&x->Real) || realIsSpecial(&x->Imag) || (realIsInfinite(&x->Real) && realIsNegative(&x->Real))) {
    realSetNaN(&res->Real);
    realSetNaN(&res->Imag);
    return;
  }
  if(realIsInfinite(&x->Real)) {
    realSetOne(&res->Real);
    realSetZero(&res->Imag);
    return;
  }

  realCopy(&x->Real, &reg67.Real);
  realCopy(&x->Imag, &reg67.Imag);
  realCopy(&x->Real, &reg1011.Real);
  realCopy(&x->Imag, &reg1011.Imag);
  if(realCompareGreaterEqual(&x->Real, const_1on2)) {
    if(!zeta_calc_complex(&reg45, &reg67, realContext)) {
      realSetNaN(&res->Real);
      realSetNaN(&res->Imag);
      return;
    }
    realCopy(&reg45.Real, &res->Real);
    realCopy(&reg45.Imag, &res->Imag);
  }
  else { // zeta_neg
    realSubtract(const_1, &x->Real, &reg67.Real, realContext);
    realChangeSign(&reg67.Imag);
    if(!zeta_calc_complex(&reg45, &reg67, realContext)) {
      realSetNaN(&res->Real);
      realSetNaN(&res->Imag);
      return;
    }
    realSubtract(const_1, &reg1011.Real, &qp.Real, realContext);
    realSubtract(const_0, &reg1011.Imag, &qp.Imag, realContext);
    realMultiply(&qp.Real, const_1on2, &qp.Real, realContext);
    realMultiply(&qp.Imag, const_1on2, &qp.Imag, realContext);
    WP34S_ComplexGamma(&qp, &sr, realContext);
    mulComplexComplex(&sr, &reg45, &reg45, realContext);
    realCopy(&reg1011.Real, &qp.Real);
    realCopy(&reg1011.Imag, &qp.Imag);
    realMultiply(&qp.Real, const_1on2, &reg1011.Real, realContext);
    realMultiply(&qp.Imag, const_1on2, &reg1011.Imag, realContext);
    realSubtract(&qp.Real, const_1on2, &qp.Real, realContext);
    realCopy(pi, &reg67.Real);
    realSetZero(&reg67.Imag);
    PowerComplex(&reg67, &qp, &sr, realContext);
    mulComplexComplex(&sr, &reg45, &reg45, realContext);
    WP34S_ComplexGamma(&reg1011, &qp, realContext);

    divComplexComplex(&reg45, &qp, res, realContext);
  }
#endif // OPTION_ZETA_BETA
}

#if defined(OPTION_ZETA_BETA)
static void doRealZeta(void) {
  real_t x, r;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  WP34S_Zeta(&x, &r, &ctxtReal75);
  convertRealToResultRegister(&r, REGISTER_X, amNone);
}

static void doComplexZeta(void) {
  cplx_t x, r;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  ComplexZeta(&x, &r, &ctxtReal75);
  convertComplexToResultRegister(&r, REGISTER_X);
}
#endif // OPTION_ZETA_BETA

/********************************************//**
 * \brief regX ==> regL and zeta(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnZeta(uint16_t unusedButMandatoryParameter) {
#if defined(OPTION_ZETA_BETA)
  processRealComplexMonadicFunction(&doRealZeta, &doComplexZeta);
#endif // OPTION_ZETA_BETA
}

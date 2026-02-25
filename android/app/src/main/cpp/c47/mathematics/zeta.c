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

#if !defined(SAVE_SPACE_DM42_12)
static void zeta_calc_complex(real_t *reg4, real_t *reg5, real_t *reg6, real_t *reg7, realContext_t *realContext) {
  real_t s, p, q, r, reg0, reg1, reg2, reg3, reg8, reg9;

  realCopyAbs(reg7, &p);
  realMultiply(const_piOn2, &p, &q, realContext);
  realMultiply(&p, const_2, &p, realContext);
  WP34S_Ln1P(&p, &p, realContext);
  realAdd(&q, &p, &p, realContext);
  realCopy(const_3, &q); q.exponent += 10;
  WP34S_Ln(&q, &q, realContext);
  realAdd(&p, &q, &p, realContext);
  realSquareRoot(const_8, &q, realContext);
  realAdd(&q, const_3, &q, realContext);
  WP34S_Ln(&q, &q, realContext);
  realDivide(&p, &q, &p, realContext);
  realToIntegralValue(&p, &p, DEC_ROUND_DOWN, realContext);
  realAdd(&p, const_1, &p, realContext);
  realMultiply(&p, const_2, &p, realContext); // for extra digits
  realMultiply(&p, const_2, &p, realContext); // we have
  realCopy(&p, &reg0); realCopy(const_1, &reg1);
  realCopy(&p, &reg2); realCopy(const_1, &reg3);
  realCopy(const_1, reg4);
  realPower(const__1, &p, &p, realContext);
  realChangeSign(&p);
  realCopy(&p, reg5);
  realCopy(const_0, &reg8); realCopy(const_0, &reg9);

  do { // zeta_loop

    #if !defined(TESTSUITE_BUILD)
    int32_t loop = realToInt32C47(&reg0);
      if(monitorExit(&loop, "Iter: ")) {
        break;
      }
    #endif //!TESTSUITE_BUILD

    realMultiply(reg6, const__1, &q, realContext); realMultiply(reg7, const__1, &p, realContext);
    PowerComplex(&reg0, const_0, &q, &p, &s, &r, realContext);
    realChangeSign(reg5);
    realMultiply(reg4, reg5, &p, realContext);
    realMultiply(&p, &r, &r, realContext); realMultiply(&p, &s, &s, realContext);
    realAdd(&reg8, &s, &reg8, realContext); realAdd(&reg9, &r, &reg9, realContext);
    realMultiply(&reg0, const_2, &p, realContext);
    realMultiply(&p, &reg0, &p, realContext);
    realSubtract(&p, &reg0, &p, realContext);
    realMultiply(&p, &reg3, &p, realContext);
    realMultiply(&reg2, &reg2, &q, realContext);
    realSubtract(&reg0, const_1, &s, realContext);
    realMultiply(&s, &s, &s, realContext);
    realSubtract(&q, &s, &q, realContext);
    realMultiply(&q, const_2, &q, realContext);
    realDivide(&p, &q, &p, realContext);
    realCopy(&p, &reg3);
    realAdd(reg4, &p, reg4, realContext);
    realSubtract(&reg0, const_1, &reg0, realContext);
  } while(realCompareGreaterThan(&reg0, const_0));
  realDivide(&reg8, reg4, &reg8, realContext); realDivide(&reg9, reg4, &reg9, realContext);
  realSubtract(const_1, reg6, &p, realContext);
  realMultiply(const_ln2, &p, &p, realContext);
  WP34S_ExpM1(&p, &reg1, realContext);
  realMultiply(reg7, const__1, &p, realContext);
  realMultiply(&p, const_ln2, &p, realContext);
  realPolarToRectangular(const_1, &p, &q, &p, realContext);
  realSubtract(&q, const_1, &r, realContext);
  realMultiply(&q, &reg1, &q, realContext);
  realMultiply(&reg1, &p, &s, realContext);
  realAdd(&s, &p, &s, realContext);
  realAdd(&q, &r, &q, realContext);

  divComplexComplex(&reg8, &reg9, &q, &s, reg4, reg5, realContext);
}
#endif // !SAVE_SPACE_DM42_12

void ComplexZeta(const real_t *xReal, const real_t *xImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
#if !defined(SAVE_SPACE_DM42_12)
  real_t p, q, r, s, reg4, reg5, reg6, reg7, reg10, reg11;

  if(realIsZero(xReal) && realIsZero(xImag)) {
    realCopy(const_1on2, resReal);
    realChangeSign(resReal);
    realZero(resImag);
    return;
  }

  realCopy(xReal, &reg6);  realCopy(xImag, &reg7);
  realCopy(xReal, &reg10); realCopy(xImag, &reg11);
  if(realCompareGreaterEqual(xReal, const_1on2)) {
    zeta_calc_complex(&reg4, &reg5, &reg6, &reg7, realContext);
    realCopy(&reg4, resReal); realCopy(&reg5, resImag);
  }
  else { // zeta_neg
    realSubtract(const_1, xReal, &reg6, realContext);
    realChangeSign(&reg7);
    zeta_calc_complex(&reg4, &reg5, &reg6, &reg7, realContext);
    realSubtract(const_1, &reg10, &q, realContext); realSubtract(const_0, &reg11, &p, realContext);
    realMultiply(&q, const_1on2, &q, realContext); realMultiply(&p, const_1on2, &p, realContext);
    WP34S_ComplexGamma(&q, &p, &s, &r, realContext);
    mulComplexComplex(&s, &r, &reg4, &reg5, &reg4, &reg5, realContext);
    realCopy(&reg10, &q); realCopy(&reg11, &p);
    realMultiply(&q, const_1on2, &reg10, realContext); realMultiply(&p, const_1on2, &reg11, realContext);
    realSubtract(&q, const_1on2, &q, realContext);
    PowerComplex(const_pi, const_0, &q, &p, &s, &r, realContext);
    mulComplexComplex(&s, &r, &reg4, &reg5, &reg4, &reg5, realContext);
    WP34S_ComplexGamma(&reg10, &reg11, &q, &p, realContext);

    divComplexComplex(&reg4, &reg5, &q, &p, resReal, resImag, realContext);
  }
#endif // !SAVE_SPACE_DM42_12
}

static void doRealZeta(void) {
  real_t x, r;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;
  WP34S_Zeta(&x, &r, &ctxtReal39);
  convertRealToResultRegister(&r, REGISTER_X, amNone);
}

static void doComplexZeta(void) {
  real_t xr, xi, rr, ri;

  if(!getRegisterAsComplex(REGISTER_X, &xr, &xi))
    return;

  if(!saveLastX())
    return;

  ComplexZeta(&xr, &xi, &rr, &ri, &ctxtReal39);
  convertComplexToResultRegister(&rr, &ri, REGISTER_X);
}

/********************************************//**
 * \brief regX ==> regL and zeta(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnZeta(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&doRealZeta, &doComplexZeta);
}


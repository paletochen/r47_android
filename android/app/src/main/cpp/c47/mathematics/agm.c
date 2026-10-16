// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file agm.c
 ***********************************************/

#include "c47.h"

typedef enum {
    AGM_MODE_NORMAL,
    AGM_MODE_E,
    AGM_MODE_STEP
} AGM_MODE;

static int _realAgm(AGM_MODE mode, const real_t *a, const real_t *b, real_t *c, real_t *res, real_t *_a, real_t *_b, size_t _sz, realContext_t *realContext) {
  real_t aReal, bReal, cReal;
  real_t cCoeff;
  const real_t *tol;
  int n = 0;

  if(realContext->digits <= 39) {
    tol = const_1e_37;
  }
  else if(realContext->digits <= 51) {
    tol = const_1e_49;
  }
  else {
    tol = const_1e_71;
  }

  realCopy(a, &aReal);
  realCopy(b, &bReal);
  if(mode==AGM_MODE_E) {
    realSetOne(&cCoeff);
  }
  if(mode==AGM_MODE_STEP) {
    realCopy(&aReal, _a);
    realCopy(&bReal, _b);
  }


  // a NaN or infinite mean never meets the tolerance, so the iteration also ends on one
  while(!realIsSpecial(&aReal) && !WP34S_RelativeError(&aReal, &bReal, tol, realContext)) {
    if(mode==AGM_MODE_E) {
      realMultiply(&cCoeff, const_2, &cCoeff, realContext);
      realSubtract(&aReal, &bReal, &cReal, realContext);     // c = a - b
      realMultiply(&cReal, const_1on2, &cReal, realContext); // c = (a - b) / 2
      realMultiply(&cReal, &cReal, &cReal, realContext);     // c^2
      realFMA(&cReal, &cCoeff, c, c, realContext);
    }
    realAdd(&aReal, &bReal, &cReal, realContext);          // c = a + b
    realMultiply(&aReal, &bReal, &bReal, realContext);     // b = a * b
    realSquareRoot(&bReal, &bReal, realContext);           // b = sqrt(a * b)
    realMultiply(&cReal, const_1on2, &aReal, realContext); // a = (a + b) / 2
    ++n;
    if(mode==AGM_MODE_STEP) {
      realCopy(&aReal, _a + n);
      realCopy(&bReal, _b + n);
      if(n >= (int)_sz - 1) {
        break;
      }
    }
  }

  if(mode==AGM_MODE_E) {
    realMultiply(c, const_1on2, c, realContext);
  }

  realCopy(&aReal, res);
  return n;
}

static int _complexAgm(AGM_MODE mode, const cplx_t *a0, const cplx_t *b0, cplx_t *cSum, cplx_t *res, cplx_t *_a, cplx_t *_b, size_t _sz, realContext_t *realContext) {
  cplx_t a, b, c;
  real_t aArg, bArg, cArg;
  real_t cCoeff;
  const real_t *tol;
  int n = 0;

  if(realContext->digits <= 39) {
    tol = const_1e_37;
  }
  else if(realContext->digits <= 51) {
    tol = const_1e_49;
  }
  else {
    tol = const_1e_71;
  }

  realCopy(&a0->Real, &a.Real);
  realCopy(&a0->Imag, &a.Imag);
  realCopy(&b0->Real, &b.Real);
  realCopy(&b0->Imag, &b.Imag);
  if(mode==AGM_MODE_E) {
    realSetOne(&cCoeff);
  }
  if(mode==AGM_MODE_STEP) {
    realCopy(&a.Real, &_a->Real);
    realCopy(&a.Imag, &_a->Imag);
    realCopy(&b.Real, &_b->Real);
    realCopy(&b.Imag, &_b->Imag);
  }

  // a NaN or infinite mean never meets the tolerance, so the iteration also ends on one
  while(!realIsSpecial(&a.Real) && !realIsSpecial(&a.Imag)
        && (!WP34S_RelativeError(&a.Real, &b.Real, tol, realContext) || !WP34S_RelativeError(&a.Imag, &b.Imag, tol, realContext))) {
    if(mode==AGM_MODE_E) {
      realMultiply(&cCoeff, const_2, &cCoeff, realContext);
      realSubtract(&a.Real, &b.Real, &c.Real, realContext);
      realSubtract(&a.Imag, &b.Imag, &c.Imag, realContext);                           // c = a - b
      realMultiply(&c.Real, const_1on2, &c.Real, realContext);
      realMultiply(&c.Imag, const_1on2, &c.Imag, realContext);                        // c = (a - b) / 2
      mulComplexComplex(&c, &c, &c, realContext);                                     // c^2
      realFMA(&c.Real, &cCoeff, &cSum->Real, &cSum->Real, realContext);
      realFMA(&c.Imag, &cCoeff, &cSum->Imag, &cSum->Imag, realContext);
    }

    realRectangularToPolar(&a, &c.Mag, &aArg, realContext);
    realRectangularToPolar(&b, &c.Mag, &bArg, realContext);
    realAdd(&a.Real, &b.Real, &c.Real, realContext);                                  // c = a + b real part
    realAdd(&a.Imag, &b.Imag, &c.Imag, realContext);                                  // c = a + b imag part

    mulComplexComplex(&a, &b, &b, realContext);                                       // b = a * b

    // b = sqrt(a * b)
    sqrtComplex(&b, &b, realContext);

    realMultiply(&c.Real, const_1on2, &a.Real, realContext); // a = (a + b) / 2 real part
    realMultiply(&c.Imag, const_1on2, &a.Imag, realContext); // a = (a + b) / 2 imag part

    ++n;

    // choose appropriate one of 2 square roots
    realRectangularToPolar(&b, &c.Mag, &cArg, realContext);
    realSubtract(&aArg, &cArg, &aArg, realContext);
    realSetPositiveSign(&aArg);
    realSubtract(&cArg, &bArg, &cArg, realContext);
    realSetPositiveSign(&cArg);
    realAdd(&aArg, &cArg, &bArg, realContext);
    if(realCompareGreaterThan(&bArg, const39_pi)) {
      realChangeSign(&b.Real);
      realChangeSign(&b.Imag);
    }

    if(mode==AGM_MODE_STEP) {
      realCopy(&a.Real, &_a[n].Real);
      realCopy(&a.Imag, &_a[n].Imag);
      realCopy(&b.Real, &_b[n].Real);
      realCopy(&b.Imag, &_b[n].Imag);
      if(n >= (int)_sz - 1) {
        break;
      }
    }
  }

  if(mode==AGM_MODE_E) {
    realMultiply(&cSum->Real, const_1on2, &cSum->Real, realContext);
    realMultiply(&cSum->Imag, const_1on2, &cSum->Imag, realContext);
  }

  realCopy(&a.Real, &res->Real);
  realCopy(&a.Imag, &res->Imag);
  return n;
}

size_t realAgm(const real_t *a, const real_t *b, real_t *res, realContext_t *realContext) {
  return _realAgm(AGM_MODE_NORMAL, a, b, NULL, res, NULL, NULL, 0, realContext);
}

size_t complexAgm(const cplx_t *a, const cplx_t *b, cplx_t *res, realContext_t *realContext) {
  return _complexAgm(AGM_MODE_NORMAL, a, b, NULL, res, NULL, NULL, 0, realContext);
}

size_t realAgmForE(const real_t *a, const real_t *b, real_t *c, real_t *res, realContext_t *realContext) {
  return _realAgm(AGM_MODE_E, a, b, c, res, NULL, NULL, 0, realContext);
}

size_t complexAgmForE(const cplx_t *a, const cplx_t *b, cplx_t *c, cplx_t *res, realContext_t *realContext) {
  return _complexAgm(AGM_MODE_E, a, b, c, res, NULL, NULL, 0, realContext);
}

size_t realAgmStep(const real_t *a, const real_t *b, real_t *res, real_t *aStep, real_t *bStep, size_t bufSize, realContext_t *realContext) {
  return _realAgm(AGM_MODE_STEP, a, b, NULL, res, aStep, bStep, bufSize, realContext);
}

size_t complexAgmStep(const cplx_t *a, const cplx_t *b, cplx_t *res, cplx_t *aStep, cplx_t *bStep, size_t bufSize, realContext_t *realContext){
  return _complexAgm(AGM_MODE_STEP, a, b, NULL, res, aStep, bStep, bufSize, realContext);
}
// Complex AGM from the stack
static void doComplexAGM(void) {
  cplx_t a, b, r;

  if(!getRegisterAsComplex(REGISTER_X, &a) || !getRegisterAsComplex(REGISTER_Y, &b)) {
    return;
  }

  if(realIsNaN(&a.Real) || realIsNaN(&a.Imag) || realIsNaN(&b.Real) || realIsNaN(&b.Imag)) {
    realSetNaN(&r.Real);
    realSetNaN(&r.Imag);
    convertComplexToResultRegister(&r, REGISTER_X);
    return;
  }

  if(realIsInfinite(&a.Real) || realIsInfinite(&a.Imag) || realIsInfinite(&b.Real) || realIsInfinite(&b.Imag)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      return;
    }

    if(realIsZero(&a.Imag) && realIsZero(&b.Imag) &&
       realCompareGreaterEqual(&a.Real, const_0) && realCompareGreaterEqual(&b.Real, const_0)) {
      if(realIsZero(&a.Real) || realIsZero(&b.Real)) {
        realSetZero(&r.Real);
      }
      else {
        realSetPlusInfinity(&r.Real);
      }
      realSetZero(&r.Imag);
    }
    else {
      realSetNaN(&r.Real);
      realSetNaN(&r.Imag);
    }
    convertComplexToResultRegister(&r, REGISTER_X);
    return;
  }

  complexAgm(&a, &b, &r, &ctxtReal75);
  convertComplexToResultRegister(&r, REGISTER_X);
}

// Real AGM from the stack
static void doRealAGM(void) {
  real_t a, b, r;

  if(!getRegisterAsReal(REGISTER_X, &a) || !getRegisterAsReal(REGISTER_Y, &b)) {
    return;
  }

  if(realIsNaN(&a) || realIsNaN(&b)) {
    convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
    return;
  }

  // Quick check for zero results
  realAdd(&a, &b, &r, &ctxtReal39);
  if(realIsZero(&a) || realIsZero(&b) || realIsZero(&r)) {
    convertRealToResultRegister(const_0, REGISTER_X, amNone);
    return;
  }

  if(realIsNegative(&a) || realIsNegative(&b)) {
    if(getFlag(FLAG_CPXRES)) {
      doComplexAGM();
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function doRealAGM:", "cannot use negative X and Y as input of AGM", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else if(realIsInfinite(&a) || realIsInfinite(&b)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      convertRealToResultRegister(const_plusInfinity, REGISTER_X, amNone);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    }
  }
  else {
    realAgm(&a, &b, &r, &ctxtReal39);
    convertRealToResultRegister(&r, REGISTER_X, amNone);
  }
}

// AGM command routine
void fnAgm(uint16_t unusedButMandatoryParameter) {
  processRealComplexDyadicFunction(&doRealAGM, &doComplexAGM);
}

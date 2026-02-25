// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file agm.c
 ***********************************************/

#include "c47.h"

typedef enum {
    AGM_MODE_NORMAL,
    AGM_MODE_E,
    AGM_MODE_STEP,
    AGM_MODE_F
} AGM_MODE;

static int _realAgm(AGM_MODE mode, const real_t *a, const real_t *b, real_t *c, real_t *res, real_t *_a, real_t *_b, size_t _sz, realContext_t *realContext) {
  real_t aReal, bReal, cReal;
  real_t cCoeff, prevDelta, z;
  int n = 0;

  realCopy(a, &aReal);
  realCopy(b, &bReal);
  if(mode==AGM_MODE_E) {
    realCopy(const_1, &cCoeff);
  }
  if(mode==AGM_MODE_STEP) {
    realCopy(&aReal, _a);
    realCopy(&bReal, _b);
  }
  if(mode==AGM_MODE_F) {
    realCopy(const_plusInfinity, &prevDelta);
    realCopy(const_0, &z);
    realDivide(c, const_pi, &cCoeff, realContext);
    realToIntegralValue(&cCoeff, &cCoeff, DEC_ROUND_DOWN, realContext);
    realDivideRemainder(c, const_pi, c, realContext);
  }

  while(!WP34S_RelativeError(&aReal, &bReal, const_1e_37, realContext)) {
    if(mode==AGM_MODE_E) {
      realMultiply(&cCoeff, const_2, &cCoeff, realContext);
      realSubtract(&aReal, &bReal, &cReal, realContext);     // c = a - b
      realMultiply(&cReal, const_1on2, &cReal, realContext); // c = (a - b) / 2
      realMultiply(&cReal, &cReal, &cReal, realContext);     // c^2
      realFMA(&cReal, &cCoeff, c, c, realContext);
    }
    if(mode==AGM_MODE_F) {
      real_t d, e, tanphi, ba;
      WP34S_Cvt2RadSinCosTan(c, amRadian, &d, &e, &tanphi, realContext);
      realDivide(&bReal, &aReal, &ba, realContext);
      realDivide(const_1, &tanphi, &d, realContext);
      realFMA(&ba, &tanphi, &d, &d, realContext);
      realSubtract(&ba, const_1, &e, realContext);
      WP34S_Atan2(&e, &d, &d, realContext);
      realAdd(&cCoeff, &cCoeff, &cCoeff, realContext);
      if(realCompareAbsLessThan(&prevDelta, &d)) {
        realAdd(&cCoeff, const_1, &cCoeff, realContext);
      }
      realCopy(&d, &prevDelta);
      realAdd(&d, c, &d, realContext);
      realAdd(&d, c, c, realContext);
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
  if(mode==AGM_MODE_F) {
    realFMA(&cCoeff, const_pi, c, c, realContext);
  }

  realCopy(&aReal, res);
  return n;
}

static int _complexAgm(AGM_MODE mode, const real_t *ar, const real_t *ai, const real_t *br, const real_t *bi, real_t *cr, real_t *ci, real_t *resr, real_t *resi, real_t *_ar, real_t *_ai, real_t *_br, real_t *_bi, size_t _sz, realContext_t *realContext) {
  real_t aReal, bReal, cReal;
  real_t aImag, bImag, cImag;
  real_t aArg, bArg, cArg;
  real_t cCoeff;
  int n = 0;

  realCopy(ar, &aReal); realCopy(ai, &aImag);
  realCopy(br, &bReal); realCopy(bi, &bImag);
  if(mode==AGM_MODE_E) {
    realCopy(const_1, &cCoeff);
  }
  if(mode==AGM_MODE_STEP) {
    realCopy(&aReal, _ar);
    realCopy(&aImag, _ai);
    realCopy(&bReal, _br);
    realCopy(&bImag, _bi);
  }

  while(!WP34S_RelativeError(&aReal, &bReal, const_1e_37, realContext) || !WP34S_RelativeError(&aImag, &bImag, const_1e_37, realContext)) {
    if(mode==AGM_MODE_E) {
      realMultiply(&cCoeff, const_2, &cCoeff, realContext);
      realSubtract(&aReal, &bReal, &cReal, realContext); realSubtract(&aImag, &bImag, &cImag, realContext);     // c = a - b
      realMultiply(&cReal, const_1on2, &cReal, realContext); realMultiply(&cImag, const_1on2, &cImag, realContext); // c = (a - b) / 2
      mulComplexComplex(&cReal, &cImag, &cReal, &cImag, &cReal, &cImag, realContext);     // c^2
      realFMA(&cReal, &cCoeff, cr, cr, realContext);
      realFMA(&cImag, &cCoeff, ci, ci, realContext);
    }

    realRectangularToPolar(&aReal, &aImag, &cReal, &aArg, realContext);
    realRectangularToPolar(&bReal, &bImag, &cReal, &bArg, realContext);
    realAdd(&aReal, &bReal, &cReal, realContext);                                   // c = a + b real part
    realAdd(&aImag, &bImag, &cImag, realContext);                                   // c = a + b imag part

    mulComplexComplex(&aReal, &aImag, &bReal, &bImag, &bReal, &bImag, realContext); // b = a * b

    // b = sqrt(a * b)
    sqrtComplex(&bReal, &bImag, &bReal, &bImag, realContext);

    realMultiply(&cReal, const_1on2, &aReal, realContext); // a = (a + b) / 2 real part
    realMultiply(&cImag, const_1on2, &aImag, realContext); // a = (a + b) / 2 imag part

    ++n;

    // choose appropriate one of 2 square roots
    realRectangularToPolar(&bReal, &bImag, &cReal, &cArg, realContext);
    realSubtract(&aArg, &cArg, &aArg, realContext);
    realSetPositiveSign(&aArg);
    realSubtract(&cArg, &bArg, &cArg, realContext);
    realSetPositiveSign(&cArg);
    realAdd(&aArg, &cArg, &bArg, realContext);
    if(realCompareGreaterThan(&bArg, const_pi)) {
      realChangeSign(&bReal);
      realChangeSign(&bImag);
    }

    if(mode==AGM_MODE_STEP) {
      realCopy(&aReal, _ar + n);
      realCopy(&aImag, _ai + n);
      realCopy(&bReal, _br + n);
      realCopy(&bImag, _bi + n);
      if(n >= (int)_sz - 1) {
        break;
      }
    }
  }

  if(mode==AGM_MODE_E) {
    realMultiply(cr, const_1on2, cr, realContext); realMultiply(ci, const_1on2, ci, realContext);
  }

  realCopy(&aReal, resr); realCopy(&aImag, resi);
  return n;
}

size_t realAgm(const real_t *a, const real_t *b, real_t *res, realContext_t *realContext) {
  return _realAgm(AGM_MODE_NORMAL, a, b, NULL, res, NULL, NULL, 0, realContext);
}

size_t complexAgm(const real_t *ar, const real_t *ai, const real_t *br, const real_t *bi, real_t *resr, real_t *resi, realContext_t *realContext) {
  return _complexAgm(AGM_MODE_NORMAL, ar, ai, br, bi, NULL, NULL, resr, resi, NULL, NULL, NULL, NULL, 0, realContext);
}

size_t realAgmForE(const real_t *a, const real_t *b, real_t *c, real_t *res, realContext_t *realContext) {
  return _realAgm(AGM_MODE_E, a, b, c, res, NULL, NULL, 0, realContext);
}

size_t complexAgmForE(const real_t *ar, const real_t *ai, const real_t *br, const real_t *bi, real_t *cr, real_t *ci, real_t *resr, real_t *resi, realContext_t *realContext) {
  return _complexAgm(AGM_MODE_E, ar, ai, br, bi, cr, ci, resr, resi, NULL, NULL, NULL, NULL, 0, realContext);
}

size_t realAgmForF(const real_t *a, const real_t *b, real_t *c, real_t *res, realContext_t *realContext) {
  return _realAgm(AGM_MODE_F, a, b, c, res, NULL, NULL, 0, realContext);
}

size_t realAgmStep(const real_t *a, const real_t *b, real_t *res, real_t *aStep, real_t *bStep, size_t bufSize, realContext_t *realContext) {
  return _realAgm(AGM_MODE_STEP, a, b, NULL, res, aStep, bStep, bufSize, realContext);
}

size_t complexAgmStep(const real_t *ar, const real_t *ai, const real_t *br, const real_t *bi, real_t *resr, real_t *resi, real_t *aStep, real_t *aiStep, real_t *bStep, real_t *biStep, size_t bufSize, realContext_t *realContext){
  return _complexAgm(AGM_MODE_STEP, ar, ai, br, bi, NULL, NULL, resr, resi, aStep, aiStep, bStep, biStep, bufSize, realContext);
}
// Complex AGM from the stack
static void doComplexAGM(void) {
  real_t aReal, bReal, rReal;
  real_t aImag, bImag, rImag;

  if(!getRegisterAsComplex(REGISTER_X, &aReal, &aImag)
      || !getRegisterAsComplex(REGISTER_Y, &bReal, &bImag))
    return;

  complexAgm(&aReal, &aImag, &bReal, &bImag, &rReal, &rImag, &ctxtReal75);
  convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
}

// Real AGM from the stack
static void doRealAGM(void) {
  real_t a, b, r;

  if(!getRegisterAsReal(REGISTER_X, &a) || !getRegisterAsReal(REGISTER_Y, &b))
    return;

  // Quick check for zero results
  realAdd(&a, &b, &r, &ctxtReal39);
  if(realIsZero(&a) || realIsZero(&b) || realIsZero(&r)) {
    convertRealToResultRegister(const_0, REGISTER_X, amNone);
    return;
  }

  if(realIsNegative(&a) || realIsNegative(&b)) {
    if(getFlag(FLAG_CPXRES))
      doComplexAGM();
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function doRealAGM:", "cannot use negative X and Y as input of AGM", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    realAgm(&a, &b, &r, &ctxtReal75);
    convertRealToResultRegister(&r, REGISTER_X, amNone);
  }
}

// AGM command routine
void fnAgm(uint16_t unusedButMandatoryParameter) {
  processRealComplexDyadicFunction(&doRealAGM, &doComplexAGM);
}

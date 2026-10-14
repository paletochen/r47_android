// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file slvp.c
 ***********************************************/

#include "c47.h"

#if defined(OPTION_SLVP_POLY)

// Coefficients and roots live in interleaved complex slots of real_t at 75 digits: slot i is (c[2i] re, c[2i+1] im), the layout calculateEigenvalues works on.

static bool_t _slotIsZero(const real_t *c) {
  return realIsZero(c) && realIsZero(c + 1);
}

// All d roots of x^d = w, w != 0, in angle order; every root has magnitude |w|^(1/d).
// Exact and O(d) where the companion matrix through the eigenvalue solver is O(d^3).
// cleanNoise: only real coefficients cap how small a genuine root component can be; complex ones carry independent exponents, so their output stays raw.
static void _rootsOfXdEqualsW(const real_t *wRe, const real_t *wIm, uint16_t d, real_t *roots, bool_t cleanNoise, realContext_t *realContext) {
  real_t mag, theta, rho, oneOnD, dReal, kReal, angle;
  int32_t rhoExponent;
  uint16_t k;

  realRectangularToPolar(wRe, wIm, &mag, &theta, realContext);
  int32ToReal(d, &dReal);
  realDivide(const_1, &dReal, &oneOnD, realContext);
  realPower(&mag, &oneOnD, &rho, realContext);
  rhoExponent = realGetExponent(&rho);

  for(k = 0; k < d; k++) {
    real_t *re = roots + k * 2;
    real_t *im = re + 1;
    int32ToReal(k, &kReal);
    realMultiply(const75_2pi, &kReal, &angle, realContext);
    realAdd(&angle, &theta, &angle, realContext);
    realDivide(&angle, &dReal, &angle, realContext);
    realPolarToRectangular(&rho, &angle, re, im, realContext);
    // for real w a component sitting a full working precision below the root magnitude is angle-placement noise, not data
    if(cleanNoise && !realIsZero(re) && realGetExponent(re) < rhoExponent - (int32_t)realContext->digits + 5) {
      realSetZero(re);
    }
    if(cleanNoise && !realIsZero(im) && realGetExponent(im) < rhoExponent - (int32_t)realContext->digits + 5) {
      realSetZero(im);
    }
  }
}
#endif // OPTION_SLVP_POLY



/********************************************//**
 * \brief X = coefficient vector (highest degree first) ==> X = row vector of all roots, input to regL
 * SLVP executes no user code: no engine nesting and no engineNestingRefused interaction.
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSlvp(uint16_t unusedButMandatoryParameter) {
#if defined(OPTION_SLVP_POLY)
  real34Matrix_t xr;
  complex34Matrix_t xc;
  bool_t complexCoefs, realContent;
  uint16_t rows, cols, n, d, k, i;
  uint32_t m, lead, j;
  real_t *coef, *roots;
  size_t wsSize;

  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    complexCoefs = false;
    linkToRealMatrixRegister(REGISTER_X, &xr);
    rows = xr.header.matrixRows;
    cols = xr.header.matrixColumns;
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complexCoefs = true;
    linkToComplexMatrixRegister(REGISTER_X, &xc);
    rows = xc.header.matrixRows;
    cols = xc.header.matrixColumns;
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnSlvp:", "SLVP expects a coefficient vector in X", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(rows != 1 && cols != 1) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "SLVP needs a coefficient vector, not (%d" STD_CROSS "%d)", rows, cols);
      moreInfoOnError("In function fnSlvp:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  m = (uint32_t)rows * cols;

  wsSize = (size_t)REAL_SIZE_IN_BLOCKS(75) * m * 4;                      // m coefficient slots plus up to m-1 root slots
  if(!(coef = allocC47Blocks(wsSize))) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnSlvp:", "Ram full", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  roots = coef + m * 2;

  realContent = true;                                                    // real coefficients cap how small a genuine root component can be (a near-real pair a+-bi
  for(j = 0; j < m; j++) {                                               // is encoded through b squared); complex ones carry independent exponents and get no cleanup
    if(complexCoefs) {
      real34ToReal(VARIABLE_REAL34_DATA(xc.matrixElements + j), coef + j * 2);
      real34ToReal(VARIABLE_IMAG34_DATA(xc.matrixElements + j), coef + j * 2 + 1);
      realContent = realContent && realIsZero(coef + j * 2 + 1);
    }
    else {
      real34ToReal(xr.matrixElements + j, coef + j * 2);
      realSetZero(coef + j * 2 + 1);
    }
  }

  lead = 0;                                                              // a leading zero coefficient carries no degree: drop it silently, the SLVC precedent
  while(lead < m && _slotIsZero(coef + lead * 2)) {
    lead++;
  }
  if(m - lead < 2) {                                                     // a constant, or nothing at all, has no roots
    freeC47Blocks(coef, wsSize);
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnSlvp:", "the polynomial needs degree 1 or higher after leading zeros are dropped", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  n = (uint16_t)(m - lead - 1);                                          // the degree, and the number of roots delivered

  k = 0;                                                                 // each trailing zero coefficient is one exact root at 0: deflate, solve degree d = n - k
  while(k < n && _slotIsZero(coef + (m - 1 - k) * 2)) {
    k++;
  }
  d = n - k;

  {
    const real_t *aLead = coef + lead * 2;

    if(d == 0) {                                                         // x^n: every root is 0, the appended zeros below are the whole answer
    }
    else if(d == 1) {
      divComplexComplex(coef + (lead + 1) * 2, coef + (lead + 1) * 2 + 1, aLead, aLead + 1, roots, roots + 1, &ctxtReal75);
      realChangeSign(roots);
      realChangeSign(roots + 1);
    }
    else {
      bool_t middleZero = true;
      for(j = lead + 1; j < lead + d; j++) {
        if(!_slotIsZero(coef + j * 2)) {
          middleZero = false;
          break;
        }
      }

      if(middleZero) {                                                   // x^d = w in closed form
        real_t wRe, wIm;
        divComplexComplex(coef + (lead + d) * 2, coef + (lead + d) * 2 + 1, aLead, aLead + 1, &wRe, &wIm, &ctxtReal75);
        realChangeSign(&wRe);
        realChangeSign(&wIm);
        _rootsOfXdEqualsW(&wRe, &wIm, d, roots, realContent, &ctxtReal75);
      }
      else {                                                             // monic companion matrix through the EIGEN QR solver
        const size_t bulkSize = (size_t)REAL_SIZE_IN_BLOCKS(75) * ((size_t)d * d * 2 * 2 + d * 7);
        real_t *bulk, *a, *scratch, *eig;

        if(!(bulk = allocC47Blocks(bulkSize))) {
          freeC47Blocks(coef, wsSize);
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function fnSlvp:", "Ram full", NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return;
        }
        a   = bulk;
        eig = bulk + (size_t)d * d * 2;
        scratch = bulk + (size_t)d * d * 2 * 2;

        for(j = 0; j < (uint32_t)d * d * 2; j++) {
          realSetZero(a + j);
        }
        for(i = 0; i + 1 < d; i++) {                                     // superdiagonal of ones
          realCopy(const_1, a + ((size_t)i * d + i + 1) * 2);
        }
        for(i = 0; i < d; i++) {                                         // bottom row element i = -b_i, b_i the monic coefficient of x^i
          real_t *dst = a + ((size_t)(d - 1) * d + i) * 2;
          divComplexComplex(coef + (lead + d - i) * 2, coef + (lead + d - i) * 2 + 1, aLead, aLead + 1, dst, dst + 1, &ctxtReal75);
          realChangeSign(dst);
          realChangeSign(dst + 1);
        }

        calculateEigenvalues(a, scratch, eig, d, true, true, &ctxtReal75);
        if(lastErrorCode != ERROR_NONE) {                                // abort or refusal: write nothing, the central undo puts X and L back
          freeC47Blocks(bulk, bulkSize);
          freeC47Blocks(coef, wsSize);
          return;
        }
        for(i = 0; i < d; i++) {
          real_t *re = roots + i * 2;
          real_t *im = re + 1;
          int32_t topExponent;
          realCopy(eig + ((size_t)i * d + i) * 2,     re);
          realCopy(eig + ((size_t)i * d + i) * 2 + 1, im);
          if(realContent) {                                               // a component 36 orders below the root's dominant component is QR convergence residue:
            topExponent = realGetExponent(realIsZero(re) || (!realIsZero(im) && realGetExponent(im) > realGetExponent(re)) ? im : re);
            if(!realIsZero(re) && realGetExponent(re) < topExponent - 36) { // 34-digit real coefficients cannot encode a root feature that small; complex ones
              realSetZero(re);                                              // can, so their output stays raw
            }
            if(!realIsZero(im) && realGetExponent(im) < topExponent - 36) {
              realSetZero(im);
            }
          }
        }
        freeC47Blocks(bulk, bulkSize);
      }
    }
  }

  for(i = d; i < n; i++) {                                               // the deflated roots at 0, after the solved roots
    realSetZero(roots + i * 2);
    realSetZero(roots + i * 2 + 1);
  }

  {
    bool_t isComplex = false;
    for(i = 0; i < n; i++) {
      if(!realIsZero(roots + i * 2 + 1)) {
        isComplex = true;
        break;
      }
    }

    if(isComplex) {                                                      // the result matrix is allocated, then L, then X is overwritten: every failure leaves X and L untouched
      complex34Matrix_t res;
      if(!complexMatrixInit(&res, 1, n)) {
        freeC47Blocks(coef, wsSize);
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnSlvp:", "Ram full", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
      if(!saveLastX()) {
        complexMatrixFree(&res);
        freeC47Blocks(coef, wsSize);
        return;
      }
      for(i = 0; i < n; i++) {
        realToReal34(roots + i * 2,     VARIABLE_REAL34_DATA(res.matrixElements + i));
        realToReal34(roots + i * 2 + 1, VARIABLE_IMAG34_DATA(res.matrixElements + i));
      }
      convertComplex34MatrixToComplex34MatrixRegister(&res, REGISTER_X);
      complexMatrixFree(&res);
    }
    else {
      real34Matrix_t res;
      if(!realMatrixInit(&res, 1, n)) {
        freeC47Blocks(coef, wsSize);
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnSlvp:", "Ram full", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
      if(!saveLastX()) {
        realMatrixFree(&res);
        freeC47Blocks(coef, wsSize);
        return;
      }
      for(i = 0; i < n; i++) {
        realToReal34(roots + i * 2, res.matrixElements + i);
      }
      convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
      realMatrixFree(&res);
    }
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
  freeC47Blocks(coef, wsSize);
#endif // OPTION_SLVP_POLY
}

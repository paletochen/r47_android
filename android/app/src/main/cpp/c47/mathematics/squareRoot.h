// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file squareRoot.h
 */
#if !defined(SQUAREROOT_H)
  #define SQUAREROOT_H

  void fnSquareRoot(uint16_t unusedButMandatoryParameter);
  void sqrtComplex (const cplx_t *z, cplx_t *res, realContext_t *realContext);
  #if defined(OPTION_SQUARE_159) || defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159)
    void sqrtComplex159(const cplx159_t *z, cplx159_t *res, realContext_t *realContext);
  #endif // OPTION_SQUARE_159 || OPTION_CUBIC_159 || OPTION_EIGEN_159
  void rootLonI    (int32_t n);

#endif // !SQUAREROOT_H

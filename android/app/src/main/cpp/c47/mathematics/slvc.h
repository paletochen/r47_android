// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file slvc.h
 ***********************************************/
#if !defined(SLVC_H)
  #define SLVC_H

  void fnSlvc(uint16_t unusedButMandatoryParameter);

  /**
   * Solve the polynomial whose coefficient vector is in X, highest degree first, 1 x m or m x 1:
   * 3 elements solve the quadratic, 4 the cubic, from either SLVQ or SLVC. All roots replace X
   * as a row vector, real when every root is real; the rest of the stack is not consumed.
   */
  void solveCoefficientVector(void);

  /**
   * Solve cubic equation a x^3 + b x^2 + c x + d = 0 at the precision the build options select:
   * 159 digits under OPTION_CUBIC_159, 75 otherwise, results rounded to 39 digits.
   * Parameters as solveCubicEquation, with the leading coefficient added and without the context.
   */
  void solveCubic(const cplx_t *a, const cplx_t *b, const cplx_t *c, const cplx_t *d, cplx_t *r, cplx_t *x1, cplx_t *x2, cplx_t *x3);

  /**
   * Solve monic cubic equation x^3 + b x^2 + c x + d = 0.
   *
   * \param[in] c2 the quadratic coefficient
   * \param[in] c1 the linear coefficient
   * \param[in] c0 the constant term
   * \param[out] r the discriminant b^2 c^2 - 4 c^3 - 4 b^3 d - 27 d^2 + 18 b c d
   * \param[out] x1 the 1st root
   * \param[out] x2 the 2nd root
   * \param[out] x3 the 3rd root
   * \param[in] realContext
   */
  void solveCubicEquation(const cplx_t *c2, const cplx_t *c1, const cplx_t *c0, cplx_t *r, cplx_t *x1, cplx_t *x2, cplx_t *x3, realContext_t *realContext);

  /**
   * Whether every root of a x^3 + b x^2 + c x + d = 0 with real coefficients is real, from the sign of the discriminant formed without dividing by a, which for a = 0
   * is b^2 times the discriminant of b x^2 + c x + d. A discriminant unresolvedDigits orders below the sum of its terms counts as zero: realContext has 159
   * digits, and the rounding of the terms and of their sum stays within about 1E-157 of that sum, which 156 covers. For finite coefficients D is also written to
   * discriminantOut unless it is NULL.
   */
  bool_t realCubicHasOnlyRealRoots(const real_t *a, const real_t *b, const real_t *c, const real_t *d, int32_t unresolvedDigits, real_t *discriminantOut, realContext_t *realContext);

  void solveCubicEquation159(const cplx159_t *c2, const cplx159_t *c1, const cplx159_t *c0, cplx159_t *r, cplx159_t *x1, cplx159_t *x2, cplx159_t *x3, realContext_t *realContext);

#endif // !SLVC_H

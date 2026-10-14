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
   * Solve monic cubic equation x^3 + b x^2 + c x + d = 0 at the precision the build options select:
   * 159 digits under OPTION_CUBIC_159, 75 otherwise, results rounded to 39 digits.
   * Parameters as solveCubicEquation, without the context.
   */
  void solveCubic(const real_t *bReal, const real_t *bImag,
                  const real_t *cReal, const real_t *cImag,
                  const real_t *dReal, const real_t *dImag,
                        real_t *rReal,       real_t *rImag,
                        real_t *x1Real,      real_t *x1Imag,
                        real_t *x2Real,      real_t *x2Imag,
                        real_t *x3Real,      real_t *x3Imag);

  /**
   * Solve monic cubic equation x^3 + b x^2 + c x + d = 0.
   *
   * \param[in] c2Real the real part of the quadratic coefficient
   * \param[in] c2Imag the imaginary part of the quadratic coefficient
   * \param[in] c1Real the real part of the linear coefficient
   * \param[in] c1Imag the imaginary part of the linear coefficient
   * \param[in] c0Real the real part of the constant term
   * \param[in] c0Imag the imaginary part of the constant term
   * \param[out] rReal the real part of the discriminant b^2 c^2 - 4 c^3 - 4 b^3 d - 27 d^2 + 18 b c d
   * \param[out] rImag the imaginary part of the discriminant
   * \param[out] x1Real the real part of the 1st root
   * \param[out] x1Imag the imaginary part of the 1st root
   * \param[out] x2Real the real part of the 2nd root
   * \param[out] x2Imag the imaginary part of the 2nd root
   * \param[out] x3Real the real part of the 3rd root
   * \param[out] x3Imag the imaginary part of the 3rd root
   * \param[in] realContext
   */
  void solveCubicEquation(const real_t *c2Real, const real_t *c2Imag,
                          const real_t *c1Real, const real_t *c1Imag,
                          const real_t *c0Real, const real_t *c0Imag,
                                real_t *rReal,        real_t *rImag,
                                real_t *x1Real,       real_t *x1Imag,
                                real_t *x2Real,       real_t *x2Imag,
                                real_t *x3Real,       real_t *x3Imag, realContext_t *realContext);

  void solveCubicEquation159(const real_t *c2Real, const real_t *c2Imag,
                                const real_t *c1Real, const real_t *c1Imag,
                                const real_t *c0Real, const real_t *c0Imag,
                                real_t *rReal, real_t *rImag,
                                real_t *x1Real, real_t *x1Imag,
                                real_t *x2Real, real_t *x2Imag,
                                real_t *x3Real, real_t *x3Imag, realContext_t *realContext);

#endif // !SLVC_H

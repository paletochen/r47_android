// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file slvq.h
 ***********************************************/
#if !defined(SLVQ_H)
  #define SLVQ_H

  void fnSlvq                (uint16_t unusedButMandatoryParameter);

  /**
   * Solve quadratic equation a x^2 + b x + c = 0.
   *
   * \param[in] aReal the real part of the quadratic coefficient
   * \param[in] aImag the imaginary part of the quadratic coefficient
   * \param[in] bReal the real part of the linear coefficient
   * \param[in] bImag the imaginary part of the linear coefficient
   * \param[in] cReal the real part of the constant term
   * \param[in] cImag the imaginary part of the constant term
   * \param[out] rReal the real part of the discriminant b^2 - 4 a c
   * \param[out] rImag the imaginary part of the discriminant
   * \param[out] x1Real the real part of the 1st root
   * \param[out] x1Imag the imaginary part of the 1st root
   * \param[out] x2Real the real part of the 2nd root
   * \param[out] x2Imag the imaginary part of the 2nd root
   * \param[in] realContext
   */
  void solveQuadraticEquation(const real_t *aReal, const real_t *aImag,
                              const real_t *bReal, const real_t *bImag,
                              const real_t *cReal, const real_t *cImag,
                                    real_t *rReal,       real_t *rImag,
                                    real_t *x1Real,      real_t *x1Imag,
                                    real_t *x2Real,      real_t *x2Imag, realContext_t *realContext);

  void solveQuadraticEquation159(const real_t *aReal, const real_t *aImag, 
                              const real_t *bReal, const real_t *bImag, 
                              const real_t *cReal, const real_t *cImag, 
                                    real_t *rReal,       real_t *rImag, 
                                    real_t *x1Real,      real_t *x1Imag, 
                                    real_t *x2Real,      real_t *x2Imag, realContext_t *realContext);

#endif // !SLVQ_H

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file slvq.h
 ***********************************************/
#if !defined(SLVQ_H)
  #define SLVQ_H

  void fnSlvq                (uint16_t unusedButMandatoryParameter);

  /**
   * Solve quadratic equation a x^2 + b x + c = 0 at the precision the build options select:
   * 159 digits under OPTION_SQUARE_159, 75 otherwise.  The caller converts the work results.
   * Parameters as solveQuadraticEquation, without the context.
   */
  void solveQuadratic(const cplx_t *a, const cplx_t *b, const cplx_t *c, cplx_t *r, cplx_t *x1, cplx_t *x2);

  /**
   * Solve quadratic equation a x^2 + b x + c = 0.
   *
   * \param[in] a the quadratic coefficient
   * \param[in] b the linear coefficient
   * \param[in] c the constant term
   * \param[out] r the discriminant b^2 - 4 a c
   * \param[out] x1 the 1st root
   * \param[out] x2 the 2nd root
   * \param[in] realContext
   */
  void solveQuadraticEquation(const cplx_t *a, const cplx_t *b, const cplx_t *c, cplx_t *r, cplx_t *x1, cplx_t *x2, realContext_t *realContext);

  void solveQuadraticEquation159(const cplx159_t *a, const cplx159_t *b, const cplx159_t *c, cplx159_t *r, cplx159_t *x1, cplx159_t *x2, realContext_t *realContext);

#endif // !SLVQ_H

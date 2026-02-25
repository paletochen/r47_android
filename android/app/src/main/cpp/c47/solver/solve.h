// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file solve.h
 ***********************************************/
#if !defined(SOLVE_H)
  #define SOLVE_H

  void fnPgmSlv          (uint16_t label);
  void fnSolve           (uint16_t labelOrVariable);
  void fnSolveVar        (uint16_t unusedButMandatoryParameter);

  /**
   * Solves an equation f(x) = 0 stored as a program.
   *
   * \param[in]  variable   Register number where the unknown is stored
   * \param[in]  y          1st guess of the unknown
   * \param[in]  x          2nd guess of the unknown
   * \param[out] resZ       The last value f(x)
   * \param[out] resY       The 2nd last root tested
   * \param[out] resX       The resulting root x
   * \return                0 if a root found,
   *                        1 if large delta and opposite signs (may be a pole),
   *                        2 if the solver reached a local extremum,
   *                        3 if at least one of the initial guesses is out of the domain,
   *                        4 if the function values look constant
   */
  int solver             (calcRegister_t variable, const real34_t *y, const real34_t *x, real34_t *resZ, real34_t *resY, real34_t *resX);
#endif // !SOLVE_H

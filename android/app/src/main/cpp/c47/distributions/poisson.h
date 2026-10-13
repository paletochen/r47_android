// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file poisson.h
 ***********************************************/
#if !defined(POISSON_H)
  #define POISSON_H

  void fnPoissonP                (uint16_t unusedButMandatoryParameter);
  void fnPoissonL                (uint16_t unusedButMandatoryParameter);
  void fnPoissonR                (uint16_t unusedButMandatoryParameter);
  void fnPoissonI                (uint16_t unusedButMandatoryParameter);

  void WP34S_normal_moment_approx(const real_t *prob, const real_t *var, const real_t *mean, real_t *res, realContext_t *realContext);

  void WP34S_Pdf_Poisson         (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_Poisson        (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_Poisson         (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_Poisson2        (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext);
  void WP34S_Qf_Poisson          (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext);
#endif // !POISSON_H

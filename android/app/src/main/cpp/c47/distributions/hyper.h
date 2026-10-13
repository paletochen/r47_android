// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file hyper.h
 ***********************************************/
#if !defined(HYPER_H)
  #define HYPER_H

  void fnHypergeometricP  (uint16_t unusedButMandatoryParameter);
  void fnHypergeometricL  (uint16_t unusedButMandatoryParameter);
  void fnHypergeometricR  (uint16_t unusedButMandatoryParameter);
  void fnHypergeometricI  (uint16_t unusedButMandatoryParameter);

  void pdf_Hypergeometric (const real_t *x, const real_t *p0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext);
  void cdfu_Hypergeometric(const real_t *x, const real_t *p0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext);
  void cdf_Hypergeometric (const real_t *x, const real_t *p0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext);
  void cdf_Hypergeometric2(const real_t *x, const real_t *p0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext);
  void qf_Hypergeometric  (const real_t *x, const real_t *p0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext);
#endif // !HYPER_H

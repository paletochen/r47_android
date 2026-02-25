// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cauchy.h
 ***********************************************/
#if !defined(CAUCHY_H)
  #define CAUCHY_H

  void fnCauchyP              (uint16_t unusedButMandatoryParameter);
  void fnCauchyL              (uint16_t unusedButMandatoryParameter);
  void fnCauchyR              (uint16_t unusedButMandatoryParameter);
  void fnCauchyI              (uint16_t unusedButMandatoryParameter);

  void WP34S_Pdf_Cauchy       (const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_Cauchy      (const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_Cauchy       (const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext);
  void WP34S_Qf_Cauchy        (const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext);

  void WP34S_cdf_cauchy_common(const real_t *x, const real_t *x0, const real_t *gamma, bool_t complementary, real_t *res, realContext_t *realContext);
  void WP34S_cdf_cauchy_xform (const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext);
#endif // !CAUCHY_H

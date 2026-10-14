// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file weibull.h
 ***********************************************/
#if !defined(WEIBULL_H)
  #define WEIBULL_H

  void fnWeibullP     (uint16_t unusedButMandatoryParameter);
  void fnWeibullL     (uint16_t unusedButMandatoryParameter);
  void fnWeibullR     (uint16_t unusedButMandatoryParameter);
  void fnWeibullI     (uint16_t unusedButMandatoryParameter);

  void WP34S_Pdf_Weib (const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_Weib(const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_Weib (const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext);
  void WP34S_Qf_Weib  (const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext);
#endif // !WEIBULL_H

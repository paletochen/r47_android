// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file binomial.h
 ***********************************************/
#if !defined(BINOMIAL_H)
  #define BINOMIAL_H

  void fnBinomialP        (uint16_t unusedButMandatoryParameter);
  void fnBinomialL        (uint16_t unusedButMandatoryParameter);
  void fnBinomialR        (uint16_t unusedButMandatoryParameter);
  void fnBinomialI        (uint16_t unusedButMandatoryParameter);

  void WP34S_Pdf_Binomial (const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_Binomial(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_Binomial (const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_Binomial2(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext);
  void WP34S_Qf_Binomial  (const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext);
#endif // !BINOMIAL_H

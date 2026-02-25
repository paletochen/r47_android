// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file negBinom.h
 ***********************************************/
#if !defined(NEGBINOM_H)
  #define NEGBINOM_H

  void fnNegBinomialP  (uint16_t unusedButMandatoryParameter);
  void fnNegBinomialL  (uint16_t unusedButMandatoryParameter);
  void fnNegBinomialR  (uint16_t unusedButMandatoryParameter);
  void fnNegBinomialI  (uint16_t unusedButMandatoryParameter);

  void pdf_NegBinomial (const real_t *x, const real_t *p0, const real_t *r, real_t *res, realContext_t *realContext);
  void cdfu_NegBinomial(const real_t *x, const real_t *p0, const real_t *r, real_t *res, realContext_t *realContext);
  void cdf_NegBinomial (const real_t *x, const real_t *p0, const real_t *r, real_t *res, realContext_t *realContext);
  void cdf_NegBinomial2(const real_t *x, const real_t *p0, const real_t *r, real_t *res, realContext_t *realContext);
  void qf_NegBinomial  (const real_t *x, const real_t *p0, const real_t *r, real_t *res, realContext_t *realContext);
#endif // !NEGBINOM_H

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file logistic.h
 ***********************************************/
#if !defined(LOGISTIC_H)
  #define LOGISTIC_H

  void fnLogisticP     (uint16_t unusedButMandatoryParameter);
  void fnLogisticL     (uint16_t unusedButMandatoryParameter);
  void fnLogisticR     (uint16_t unusedButMandatoryParameter);
  void fnLogisticI     (uint16_t unusedButMandatoryParameter);

  void WP34S_Pdf_Logit (const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_Logit(const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_Logit (const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext);
  void WP34S_Qf_Logit  (const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext);
#endif // !LOGISTIC_H

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file exponential.h
 ***********************************************/
#if !defined(EXPONENTIAL_H)
  #define EXPONENTIAL_H

  void fnExponentialP  (uint16_t unusedButMandatoryParameter);
  void fnExponentialL  (uint16_t unusedButMandatoryParameter);
  void fnExponentialR  (uint16_t unusedButMandatoryParameter);
  void fnExponentialI  (uint16_t unusedButMandatoryParameter);

  void WP34S_Pdf_Expon (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_Expon(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_Expon (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext);
  void WP34S_Qf_Expon  (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext);
#endif // !EXPONENTIAL_H

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file normal.h
 ***********************************************/
#if !defined(NORMAL_H)
  #define NORMAL_H

  void fnStdNormalP  (uint16_t unusedButMandatoryParameter);
  void fnStdNormalL  (uint16_t unusedButMandatoryParameter);
  void fnStdNormalR  (uint16_t unusedButMandatoryParameter);
  void fnStdNormalI  (uint16_t unusedButMandatoryParameter);

  void fnNormalP     (uint16_t unusedButMandatoryParameter);
  void fnNormalL     (uint16_t unusedButMandatoryParameter);
  void fnNormalR     (uint16_t unusedButMandatoryParameter);
  void fnNormalI     (uint16_t unusedButMandatoryParameter);

  void fnLogNormalP  (uint16_t unusedButMandatoryParameter);
  void fnLogNormalL  (uint16_t unusedButMandatoryParameter);
  void fnLogNormalR  (uint16_t unusedButMandatoryParameter);
  void fnLogNormalI  (uint16_t unusedButMandatoryParameter);

  void WP34S_Pdf_Q   (const real_t *x, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_Q  (const real_t *x, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_Q   (const real_t *x, real_t *res, realContext_t *realContext);
  void WP34S_qf_q_est(const real_t *x, real_t *res, real_t* resY, realContext_t *realContext);
  void WP34S_Qf_Q    (const real_t *x, real_t *res, realContext_t *realContext);
#endif // !NORMAL_H

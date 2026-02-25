// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file chi2.h
 ***********************************************/
#if !defined(CHI2_H)
  #define CHI2_H

  bool_t checkRegisterNoFP(const real_t *reg);

  void fnChi2P        (uint16_t unusedButMandatoryParameter);
  void fnChi2L        (uint16_t unusedButMandatoryParameter);
  void fnChi2R        (uint16_t unusedButMandatoryParameter);
  void fnChi2I        (uint16_t unusedButMandatoryParameter);

  void WP34S_Pdf_Chi2 (const real_t *x, const real_t *k, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_Chi2(const real_t *x, const real_t *k, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_Chi2 (const real_t *x, const real_t *k, real_t *res, realContext_t *realContext);
  void WP34S_Qf_Chi2  (const real_t *x, const real_t *k, real_t *res, realContext_t *realContext);
#endif // !CHI2_H

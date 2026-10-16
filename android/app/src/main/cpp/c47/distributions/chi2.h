// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file chi2.h
 ***********************************************/
#if !defined(CHI2_H)
  #define CHI2_H

  #if defined(OPTION_DIST_CHI2)
  bool_t chi2CheckDomain(bool_t registersValid, real_t *x, real_t *first, real_t *second, real_t *third, uint16_t parameter);
  bool_t checkRegisterNoFP(const real_t *reg);
  #endif // OPTION_DIST_CHI2

  void fnChi2P        (uint16_t unusedButMandatoryParameter);
  void fnChi2L        (uint16_t unusedButMandatoryParameter);
  void fnChi2R        (uint16_t unusedButMandatoryParameter);
  void fnChi2I        (uint16_t unusedButMandatoryParameter);

  #if defined(OPTION_DIST_CHI2)
  void WP34S_Pdf_Chi2 (const real_t *x, const real_t *k, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_Chi2(const real_t *x, const real_t *k, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_Chi2 (const real_t *x, const real_t *k, real_t *res, realContext_t *realContext);
  void WP34S_Qf_Chi2  (const real_t *x, const real_t *k, real_t *res, realContext_t *realContext);
  #endif // OPTION_DIST_CHI2
#endif // !CHI2_H

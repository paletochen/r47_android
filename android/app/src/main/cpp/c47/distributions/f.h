// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file f.h
 ***********************************************/
#if !defined(F_H)
  #define F_H

  void fnF_P          (uint16_t unusedButMandatoryParameter);
  void fnF_L          (uint16_t unusedButMandatoryParameter);
  void fnF_R          (uint16_t unusedButMandatoryParameter);
  void fnF_I          (uint16_t unusedButMandatoryParameter);

  void WP34S_Pdf_F    (const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_F   (const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_F    (const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext);
  void WP34S_Qf_F     (const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext);

  void   distributionLogBetaPrefix(const real_t *a, const real_t *b, const real_t *y, const real_t *z, const real_t *logit,
                                   real_t *res, realContext_t *realContext);
  bool_t distributionBetaPair     (const real_t *a, const real_t *b, const real_t *y, const real_t *z, const real_t *logit,
                                   real_t *lower, real_t *upper, realContext_t *realContext);

  void WP34S_Qf_Newton(uint32_t r_dist, const real_t *target, const real_t *estimate, const real_t *p1, const real_t *p2, const real_t *p3, real_t *res, realContext_t *realContext);
  #if defined(OPTION_DIST_F)
  bool_t fCheckDomain(bool_t registersValid, real_t *x, real_t *first, real_t *second, real_t *third, uint16_t parameter);
  #endif // OPTION_DIST_F
#endif // !F_H

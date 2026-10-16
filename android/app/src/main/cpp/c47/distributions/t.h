// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file t.h
 ***********************************************/
#if !defined(T_H)
  #define T_H

  void fnT_P       (uint16_t unusedButMandatoryParameter);
  void fnT_L       (uint16_t unusedButMandatoryParameter);
  void fnT_R       (uint16_t unusedButMandatoryParameter);
  void fnT_I       (uint16_t unusedButMandatoryParameter);

  #if defined(OPTION_DIST_T)
  bool_t studentTCheckDomain(bool_t registersValid, real_t *x, real_t *first, real_t *second, real_t *third, uint16_t parameter);
  void WP34S_Pdf_T (const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_T(const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_T (const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext);
  void WP34S_Qf_T  (const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext);
  #endif // OPTION_DIST_T
#endif // !T_H

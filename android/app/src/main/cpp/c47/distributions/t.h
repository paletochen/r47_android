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

  void WP34S_Pdf_T (const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_T(const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_T (const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext);
  void WP34S_Qf_T  (const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext);
#endif // !T_H

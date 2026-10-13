// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file geometric.h
 ***********************************************/
#if !defined(GEOMETRIC_H)
  #define GEOMETRIC_H

  void fnGeometricP           (uint16_t unusedButMandatoryParameter);
  void fnGeometricL           (uint16_t unusedButMandatoryParameter);
  void fnGeometricR           (uint16_t unusedButMandatoryParameter);
  void fnGeometricI           (uint16_t unusedButMandatoryParameter);

  void WP34S_Pdf_Geom         (const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext);
  void WP34S_Cdfu_Geom        (const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext);
  void WP34S_Cdf_Geom         (const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext);
  void WP34S_Qf_Geom          (const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext);

  void WP34S_qf_discrete_final(uint16_t dist, const real_t *r, const real_t *p, const real_t *i, const real_t *j, const real_t *k, real_t *res, realContext_t *realContext);
#endif // !GEOMETRIC_H

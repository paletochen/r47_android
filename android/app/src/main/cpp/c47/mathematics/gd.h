// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file gd.h
 ***********************************************/
#if !defined(GD_H)
  #define GD_H

  void fnGd   (uint16_t unusedButMandatoryParameter);
  void fnInvGd(uint16_t unusedButMandatoryParameter);

  uint8_t GudermannianReal(const real_t *x, real_t *res, realContext_t *realContext);
  uint8_t GudermannianComplex(const cplx_t *x, cplx_t *res, realContext_t *realContext);

  uint8_t InverseGudermannianReal(const real_t *x, real_t *res, realContext_t *realContext);
  uint8_t InverseGudermannianComplex(const cplx_t *x, cplx_t *res, realContext_t *realContext);
#endif // !GD_H

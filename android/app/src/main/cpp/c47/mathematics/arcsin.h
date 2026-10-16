// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arcsin.h
 ***********************************************/
#if !defined(ARCSIN_H)
  #define ARCSIN_H

  void fnArcsin   (uint16_t unusedButMandatoryParameter);

  uint8_t ArcsinAcosComplex(const cplx_t *z, cplx_t *r, bool_t calculateAcos, realContext_t *realContext);
  bool_t exactArcSinCosAngle(const real_t *x, bool_t cosine, real_t *res);
  uint8_t ArcsinComplex(const cplx_t *x, cplx_t *r, realContext_t *realContext);
#endif // !ARCSIN_H

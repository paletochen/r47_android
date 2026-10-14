// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file bessel.h
 ***********************************************/
#if !defined(BESSEL_H)
  #define BESSEL_H

  void fnBesselJ           (uint16_t unusedButMandatoryParameter);
  void fnBesselY           (uint16_t unusedButMandatoryParameter);
  #if defined(OPTION_BESSEL)
  void WP34S_BesselJ       (const real_t *alpha, const real_t *x, real_t *res, realContext_t *realContext);
  void WP34S_BesselY       (const real_t *alpha, const real_t *x, real_t *res, realContext_t *realContext);
  #endif // OPTION_BESSEL
#endif // !BESSEL_H

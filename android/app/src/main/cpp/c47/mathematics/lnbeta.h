// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file lnbeta.h
 ***********************************************/
#if !defined(LNBETA_H)
  #define LNBETA_H

  void fnLnBeta      (uint16_t unusedButMandatoryParameter);
  void LnBeta(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext);
#endif // !LNBETA_H

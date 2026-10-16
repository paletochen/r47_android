// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file zeta.h
 ***********************************************/
#if !defined(ZETA_H)
  #define ZETA_H

  void fnZeta     (uint16_t unusedButMandatoryParameter);
  void ComplexZeta(const cplx_t *x, cplx_t *res, realContext_t *realContext);
#endif // !ZETA_H

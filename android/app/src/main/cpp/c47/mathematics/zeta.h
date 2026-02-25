// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file zeta.h
 ***********************************************/
#if !defined(ZETA_H)
  #define ZETA_H

  void fnZeta     (uint16_t unusedButMandatoryParameter);
  void ComplexZeta(const real_t *xReal, const real_t *xImag, real_t *resReal, real_t *resImag, realContext_t *realContext);
#endif // !ZETA_H

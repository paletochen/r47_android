// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file tan.h
 ***********************************************/
#if !defined(TAN_H)
  #define TAN_H

  void fnTan                    (uint16_t unusedButMandatoryParameter);

  uint8_t TanComplex(const cplx_t *x, cplx_t *r, realContext_t *realContext);
#endif // !TAN_H

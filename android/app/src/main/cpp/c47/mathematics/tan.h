// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file tan.h
 ***********************************************/
#if !defined(TAN_H)
  #define TAN_H

  void fnTan                    (uint16_t unusedButMandatoryParameter);

  uint8_t TanComplex(const real_t *xReal, const real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext);
#endif // !TAN_H

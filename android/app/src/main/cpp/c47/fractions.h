// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file fractions.h
 */
#if !defined(FRACTIONS_H)
  #define FRACTIONS_H

  void   fnDenMax            (uint16_t D);
  bool_t fraction            (calcRegister_t regist, int16_t *sign, uint64_t *intPart, uint64_t *numer, uint64_t *denom, int16_t *lessEqualGreater);
#endif // FRACTIONS_H

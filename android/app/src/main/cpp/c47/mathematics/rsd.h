// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file rsd.h
 ***********************************************/
#if !defined(RSD_H)
  #define RSD_H

  void roundToSignificantDigits(const real_t *source, real_t *destination, uint16_t digits, realContext_t *realContext);
  void senaryDigitToDecimal    (bool_t pre_grouped, real_t *val, realContext_t *realContext);
  void decimalDigitToSenary    (bool_t pre_grouped, real_t *val, realContext_t *realContext);
  void fnRsd                   (uint16_t digits);
  void rsdError                (uint16_t unusedButMandatoryParameter);
  void rsdTime                 (uint16_t digits);
  void rsdRema                 (uint16_t digits);
  void rsdCxma                 (uint16_t digits);
  void rsdReal                 (uint16_t digits);
  void rsdCplx                 (uint16_t digits);
#endif // !RSD_H

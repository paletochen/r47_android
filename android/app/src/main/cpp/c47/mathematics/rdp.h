// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file rdp.h
 ***********************************************/
#if !defined(RDP_H)
  #define RDP_H

  void roundToDecimalPlace(const real_t *source, real_t *destination, uint16_t digits, realContext_t *realContext);
  void fnRdp              (uint16_t digits);
  void rdpError           (uint16_t unusedButMandatoryParameter);
  void rdpTime            (uint16_t digits);
  void rdpRema            (uint16_t digits);
  void rdpCxma            (uint16_t digits);
  void rdpReal            (uint16_t digits);
  void rdpCplx            (uint16_t digits);
#endif // !RDP_H

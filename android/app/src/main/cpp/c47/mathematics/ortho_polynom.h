// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ortho_polynom.h
 ***********************************************/
#if !defined(ORTHO_POLYNOM_H)
  #define ORTHO_POLYNOM_H

  void fnOrthoPoly    (uint16_t kind);
  void fnHermite      (uint16_t unusedButMandatoryParameter);
  void fnHermiteP     (uint16_t unusedButMandatoryParameter);
  void fnLaguerre     (uint16_t unusedButMandatoryParameter);
  void fnLaguerreAlpha(uint16_t unusedButMandatoryParameter);
  void fnLegendre     (uint16_t unusedButMandatoryParameter);
  void fnChebyshevT   (uint16_t unusedButMandatoryParameter);
  void fnChebyshevU   (uint16_t unusedButMandatoryParameter);
#endif // !ORTHO_POLYNOM_H

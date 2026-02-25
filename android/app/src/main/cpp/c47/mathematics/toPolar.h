// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file toPolar.h
 ***********************************************/
#if !defined(TOPOLAR_H)
  #define TOPOLAR_H

  void fnToPolar2              (uint16_t unusedButMandatoryParameter);
  void fnToPolar_HP            (uint16_t unusedButMandatoryParameter);
  void fnToPolar_CX            (uint16_t unusedButMandatoryParameter);
  void real34RectangularToPolar(const real34_t *real34, const real34_t *imag34, real34_t *magnitude34, real34_t *theta34);
  void realRectangularToPolar  (const real_t   *real,   const real_t   *imag,   real_t   *magnitude,   real_t   *theta,  realContext_t *realContext);
#endif // !TOPOLAR_H

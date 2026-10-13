// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file magnitude.h
 ***********************************************/
#if !defined(MAGNITUDE_H)
  #define MAGNITUDE_H

  void fnMagnitude   (uint16_t unusedButMandatoryParameter);
  void complexMagnitude(const real_t *a, const real_t *b, real_t *c, realContext_t *realContext);
  void complexMagnitude2(const real_t *a, const real_t *b, real_t *c, realContext_t *realContext);
#endif // !MAGNITUDE_H

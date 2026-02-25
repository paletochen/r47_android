// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arccosh.h
 ***********************************************/
#if !defined(ARCCOSH_H)
  #define ARCCOSH_H

  void fnArccosh   (uint16_t unusedButMandatoryParameter);

  void realArcosh  (const real_t *x, real_t *res, realContext_t *realContext);
#endif // !ARCCOSH_H

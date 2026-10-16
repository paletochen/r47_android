// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file mathematics/sqrt1Px2.h
 */
#if !defined(SQRT1PX2_H)
  #define SQRT1PX2_H

  void fnSqrt1Px2   (uint16_t unusedButMandatoryParameter);

  void sqrt1Px2Complex(const cplx_t *z, cplx_t *res, realContext_t *realContext);
#endif // !SQRT1PX2_H

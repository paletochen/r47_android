// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file mathematics/log10.h
 */
#if !defined(LOG10_H)
  #define LOG10_H

  void   fnLog10   (uint16_t unusedButMandatoryParameter);
  void   realLog10 (const real_t *x, real_t *res, realContext_t *realContext);

  void logxyCplx(const real_t *denom);
  void logBaseCplx(const real_t *denom);   // the same with the near unit branch, which log10 and log2 both take
  void logxyReal(const real_t *denom);
  void logxyLonI(const real_t *denom);
#endif // !LOG10_H

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file algdep.h
 ***********************************************/
#if !defined(ALGDEP_H)
  #define ALGDEP_H

  /**
   * Highest degree the lattice is allowed to reach. Bounds the GMP working set, which grows with the square of the degree: measured 800 bytes at degree 1
   * and 9216 at degree 10 on a refusal sweep, beside the fixed 5240 byte lattice frame in the arena. A 34 digit input cannot support a useful relation
   * past degree 8 in any case.
   */
  #define ALGDEP_MAX_DEGREE   10

  /**
   * Decimal scale factor exponent: lattice column holds nint(10^ALGDEP_SCALE * x^i). The Gram determinants run to about 2*ALGDEP_SCALE digits regardless
   * of degree, which is what keeps the working set flat.
   */
  #define ALGDEP_SCALE        30

  /**
   * Digits of margin a candidate must clear before it is reported: the confidence less the digits the relation itself takes to specify. Zero false positives over 400 pseudo random 34 digit values swept to degree 6.
   */
  #define ALGDEP_MARGIN       8

  /**
   * Ceiling on the confidence, and so on the margin. A 34 digit register value is a RATIONAL, so an exact integer relation among its powers always exists at some height;
   * treating a zero residual as infinite confidence would report that relation instead of refusing. The input carries 34 digits and no verification can be
   * worth more than the data behind it.
   */
  #define ALGDEP_MAX_CONFIDENCE 34

  /**
   * The recovered relation rendered as x³ - x - 1, for the temporary information line. Empty until a search succeeds.
   */
  const char *algdepPolynomialString(void);

  void fnAlgdep   (uint16_t maxDegree);
  void fnLindep   (uint16_t unusedButMandatoryParameter);
#endif // !ALGDEP_H

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file gamma.h
 ***********************************************/
#if !defined(GAMMA_H)
  #define GAMMA_H

  void fnGamma     (uint16_t unusedButMandatoryParameter);
  void fnLnGamma   (uint16_t unusedButMandatoryParameter);

  void complexLnGamma(const real_t *xReal, const real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext);

#endif // !GAMMA_H

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors

/**
 * \file mathematics/cubeRoot.h
 */
#if !defined(CUBEROOT_H)
  #define CUBEROOT_H

  void fnCubeRoot(uint16_t unusedButMandatoryParameter);

  void curtComplex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext);
//  void curtComplex159(const real_t *zReal, const real_t *zImag, real_t *resReal, real_t *resImag, realContext_t *realContext);
  void curtReal   (void);

#endif // !CUBEROOT_H

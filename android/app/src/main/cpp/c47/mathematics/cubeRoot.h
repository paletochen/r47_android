// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors

/**
 * \file mathematics/cubeRoot.h
 */
#if !defined(CUBEROOT_H)
  #define CUBEROOT_H

  void fnCubeRoot(uint16_t unusedButMandatoryParameter);

  void curtComplex(const cplx_t *z, cplx_t *res, realContext_t *realContext);
  #if defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159)
    void curtComplex159(const cplx159_t *z, cplx159_t *res, realContext_t *realContext);
  #endif // OPTION_CUBIC_159 || OPTION_EIGEN_159
  void curtReal   (void);

#endif // !CUBEROOT_H

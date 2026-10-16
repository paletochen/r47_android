// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file elliptic.h
 ***********************************************/
#if !defined(ELLIPTIC_H)
  #define ELLIPTIC_H

  void fnEllipse        (uint16_t unusedButMandatoryParameter);
  void fnKtoM           (uint16_t unusedButMandatoryParameter);
  void fnMtoK           (uint16_t unusedButMandatoryParameter);
  void fnMtoTheta       (uint16_t unusedButMandatoryParameter);
  void fnThetatoM       (uint16_t unusedButMandatoryParameter);

  void fnJacobiSn       (uint16_t unusedButMandatoryParameter);
  void fnJacobiCn       (uint16_t unusedButMandatoryParameter);
  void fnJacobiDn       (uint16_t unusedButMandatoryParameter);
  void fnJacobiAmplitude(uint16_t unusedButMandatoryParameter);

  void fnEllipticK      (uint16_t unusedButMandatoryParameter);
  void fnEllipticE      (uint16_t unusedButMandatoryParameter);
  void fnEllipticPi     (uint16_t unusedButMandatoryParameter);
  void fnEllipticFphi   (uint16_t unusedButMandatoryParameter);
  void fnEllipticEphi   (uint16_t unusedButMandatoryParameter);
  void fnJacobiZeta     (uint16_t unusedButMandatoryParameter);

  #if defined(OPTION_ELLIPTIC)
  /**
   * Computes Jacobi elliptic functions am, sn, cn and dn.
   * \c am, \c sn, \c cn or \c dn can be \c NULL if the corresponding results are not needed.
   *
   * \param[in] u argument 𝑢
   * \param[in] m parameter 𝑚 = 𝑘² = sin²𝛼
   * \param[out] am amplitude am(𝑢|𝑚)
   * \param[out] sn <em>sinus amplitudinis</em> sn(𝑢|𝑚)
   * \param[out] cn <em>cosinus amplitudinis</em> cn(𝑢|𝑚)
   * \param[out] dn <em>delta amplitudinis</em> dn(𝑢|𝑚)
   */
  void jacobiElliptic   (const real_t *u, const real_t *m, real_t *am, real_t *sn, real_t *cn, real_t *dn, realContext_t *realContext);

  /**
   * Computes elliptic amplitude for complex argument.
   *
   * \param[in] u argument 𝑢
   * \param[in] m parameter 𝑚 = 𝑘² = sin²𝛼
   * \param[out] r result
   */
  void jacobiComplexAm  (const cplx_t *u, const real_t *m, cplx_t *r, realContext_t *realContext);

  /**
   * Computes <em>sinus amplitudinis</em> for complex argument.
   *
   * \param[in] u argument 𝑢
   * \param[in] m parameter 𝑚 = 𝑘² = sin²𝛼
   * \param[out] r result
   */
  void jacobiComplexSn  (const cplx_t *u, const real_t *m, cplx_t *r, realContext_t *realContext);

  /**
   * Computes <em>cosinus amplitudinis</em> for complex argument.
   *
   * \param[in] u argument 𝑢
   * \param[in] m parameter 𝑚 = 𝑘² = sin²𝛼
   * \param[out] r result
   */
  void jacobiComplexCn  (const cplx_t *u, const real_t *m, cplx_t *r, realContext_t *realContext);

  /**
   * Computes <em>delta amplitudinis</em> for complex argument.
   *
   * \param[in] u argument 𝑢
   * \param[in] m parameter 𝑚 = 𝑘² = sin²𝛼
   * \param[out] r result
   */
  void jacobiComplexDn  (const cplx_t *u, const real_t *m, cplx_t *r, realContext_t *realContext);

  /**
   * Computes complete elliptic integral of the 1st and 2nd kinds.
   * \c k or \c e can be \c NULL if the corresponding result is not needed.
   * Returns NaN to \c k and \c e for \c m > 1, where the integrals are complex.
   *
   * \param[in] m parameter 𝑚 = 𝑘² = sin²𝛼
   * \param[out] k elliptic integral of the 1st kind K(𝑚)
   * \param[out] e elliptic integral of the 2nd kind E(𝑚)
   */
  void ellipticKE       (const real_t *m, real_t *k, real_t *e, realContext_t *realContext);

  /**
   * Computes complete elliptic integral of the 1st and 2nd kinds, complex for \c m > 1.
   * \c k or \c e can be \c NULL if the corresponding result is not needed.
   *
   * \param[in] m parameter 𝑚 = 𝑘² = sin²𝛼
   * \param[out] k elliptic integral of the 1st kind K(𝑚)
   * \param[out] e elliptic integral of the 2nd kind E(𝑚)
   */
  void ellipticComplexKE(const real_t *m, cplx_t *k, cplx_t *e, realContext_t *realContext);

  /**
   * Computes incomplete elliptic integral of the 3rd kind Π(𝑛 | 𝑚).
   *
   * \param[in] n characteristic 𝑛
   * \param[in] m parameter 𝑚 = 𝑘² = sin²𝛼
   * \param[out] res the result
   */
  void ellipticPi       (const real_t *n, const real_t *m, cplx_t *res, realContext_t *realContext);

  /**
   * Computes incomplete elliptic integral of the 1st kind F(𝜑 + i 𝜓 | 𝑚).
   *
   * \param[in] amplitude amplitude 𝜑 + i𝜓
   * \param[in] m parameter 𝑚 = 𝑘² = sin²𝛼
   * \param[out] res the result
   */
  void ellipticF        (const cplx_t *amplitude, const real_t *m, cplx_t *res, realContext_t *realContext);

  /**
   * Computes incomplete elliptic integral of the 2nd kind E(𝜑 + i 𝜓 | 𝑚).
   *
   * \param[in] amplitude amplitude 𝜑 + i𝜓
   * \param[in] m parameter 𝑚 = 𝑘² = sin²𝛼
   * \param[out] res the result
   */
  void ellipticE        (const cplx_t *amplitude, const real_t *m, cplx_t *res, realContext_t *realContext);

  /**
   * Computes Jacobi zeta function Ζ(𝜑 + i 𝜓 | 𝑚).
   *
   * \param[in] amplitude amplitude 𝜑 + i𝜓
   * \param[in] m parameter 𝑚 = 𝑘² = sin²𝛼
   * \param[out] res the result
   */
  void jacobiZeta       (const cplx_t *amplitude, const real_t *m, cplx_t *res, realContext_t *realContext);
  #endif // OPTION_ELLIPTIC
#endif // !ELLIPTIC_H

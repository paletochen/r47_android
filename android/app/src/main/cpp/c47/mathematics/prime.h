// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file prime.h
 ***********************************************/
#if !defined(PRIME_H)
  #define PRIME_H

  void fnIsPrime      (uint16_t unusedButMandatoryParameter);
  void fnNextPrime    (uint16_t unusedButMandatoryParameter);
  void fnPrimeFactors (uint16_t unusedButMandatoryParameter);


  #define M_SIGMA_0  0  // k = 0
  #define M_SIGMA_1  1  // k = 1
  #define M_SIGMA_k  2  // k > 1
  #define M_SIGMA_p1 3  // k = 1 proper
  #define M_SIGMA_pk 4  // k > 1 proper genereralized
  #define M_FACTORS  5
  #define M_PHI_EUL  6
  void fnEvPFacts     (uint16_t unusedButMandatoryParameter);

#endif // !PRIME_H

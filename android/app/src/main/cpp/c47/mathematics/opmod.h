// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file opmod.h
 ***********************************************/
#if !defined(OPMOD_H)
  #define OPMOD_H

  void fnMulMod          (uint16_t unusedButMandatoryParameter);
  void fnExpMod          (uint16_t unusedButMandatoryParameter);
  void fnOpMod           (uint16_t mode);
  void opModError        (uint16_t mode);

  void longInteger_mulmod(const longInteger_t a, int32_t exp_a, const longInteger_t b, int32_t exp_b, const longInteger_t c, int32_t exp_c, longInteger_t res, int32_t *exp_res);
  void longInteger_expmod(const longInteger_t a, const longInteger_t b, const longInteger_t c, longInteger_t res);

  //      RegZRegYRegX
  void opModLonILonILonI (uint16_t mode);
  void opModLonILonIShoI (uint16_t mode);
  void opModLonILonIReal (uint16_t mode);
  void opModLonIShoILonI (uint16_t mode);
  void opModLonIShoIShoI (uint16_t mode);
  void opModLonIShoIReal (uint16_t mode);
  void opModLonIRealLonI (uint16_t mode);
  void opModLonIRealShoI (uint16_t mode);
  void opModLonIRealReal (uint16_t mode);

  void opModShoILonILonI (uint16_t mode);
  void opModShoILonIShoI (uint16_t mode);
  void opModShoILonIReal (uint16_t mode);
  void opModShoIShoILonI (uint16_t mode);
  void opModShoIShoIShoI (uint16_t mode);
  void opModShoIShoIReal (uint16_t mode);
  void opModShoIRealLonI (uint16_t mode);
  void opModShoIRealShoI (uint16_t mode);
  void opModShoIRealReal (uint16_t mode);

  void opModRealLonILonI (uint16_t mode);
  void opModRealLonIShoI (uint16_t mode);
  void opModRealLonIReal (uint16_t mode);
  void opModRealShoILonI (uint16_t mode);
  void opModRealShoIShoI (uint16_t mode);
  void opModRealShoIReal (uint16_t mode);
  void opModRealRealLonI (uint16_t mode);
  void opModRealRealShoI (uint16_t mode);
  void opModRealRealReal (uint16_t mode);
#endif // !OPMOD_H

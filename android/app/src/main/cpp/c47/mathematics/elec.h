// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file elec.c
 ***********************************************/

#if !defined(ELEC_H)
#define ELEC_H

  void fnDeltaToStar    (uint16_t unusedButMandatoryParameter);
  void fnStarToDelta    (uint16_t unusedButMandatoryParameter);
  void fnSymToAbc       (uint16_t unusedButMandatoryParameter);
  void fnAbcToSym       (uint16_t unusedButMandatoryParameter);
  void fnCopyXtoAbc     (uint16_t unusedButMandatoryParameter);
  void fnTripleZfromVI  (uint16_t unusedButMandatoryParameter);
  void fnTripleVfromIZ  (uint16_t unusedButMandatoryParameter);
  void fnTripleIfromVZ  (uint16_t unusedButMandatoryParameter);
  void fnTripleFlipPolar(uint16_t unusedButMandatoryParameter);

#endif // !ELEC_H

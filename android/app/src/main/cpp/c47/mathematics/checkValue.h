// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file checkValue.h
 ***********************************************/
#if !defined(CHECKVALUE_H)
  #define CHECKVALUE_H

  void fnCheckType          (uint16_t type);
  void fnGetType            (uint16_t unusedButMandatoryParameter);
  void fnCheckNumber        (uint16_t type);
  void fnCheckAngle         (uint16_t unusedButMandatoryParameter);
  void fnCheckSpecial       (uint16_t unusedButMandatoryParameter);
  void fnCheckNaN           (uint16_t unusedButMandatoryParameter);
  void fnCheckInfinite      (uint16_t unusedButMandatoryParameter);
  void fnCheckPlusZero      (uint16_t unusedButMandatoryParameter);
  void fnCheckMinusZero     (uint16_t unusedButMandatoryParameter);
  void fnCheckMatrix        (uint16_t unusedButMandatoryParameter);
  void fnCheckMatrixSquare  (uint16_t unusedButMandatoryParameter);
  void fnCheckReIsZero      (uint16_t unusedButMandatoryParameter);
  void fnCheckImIsZero      (uint16_t unusedButMandatoryParameter);
  void fnCheckReNotZero     (uint16_t unusedButMandatoryParameter);
  void fnCheckImNotZero     (uint16_t unusedButMandatoryParameter);
  void fnCheckIsVect2d      (uint16_t unusedButMandatoryParameter);
  void fnCheckIsVect3d      (uint16_t unusedButMandatoryParameter);

#endif // !CHECKVALUE_H

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file compare.h
 ***********************************************/
#if !defined(COMPARE_H)
  #define COMPARE_H

  bool_t registerCmp       (calcRegister_t regist1, calcRegister_t regist2, int8_t *result);
  void  registerMax        (calcRegister_t regist1, calcRegister_t regist2, calcRegister_t dest);
  void  registerMin        (calcRegister_t regist1, calcRegister_t regist2, calcRegister_t dest);

  void  fnXLessThan        (uint16_t regist);
  void  fnXLessEqual       (uint16_t regist);
  void  fnXGreaterThan     (uint16_t regist);
  void  fnXGreaterEqual    (uint16_t regist);
  void  fnXEqualsTo        (uint16_t regist);
  void  fnXNotEqual        (uint16_t regist);
  void  fnXAlmostEqual     (uint16_t regist);

  void  fnIsConverged      (uint16_t mode);

  void compareTypeErrorX   (void);
#endif // !COMPARE_H

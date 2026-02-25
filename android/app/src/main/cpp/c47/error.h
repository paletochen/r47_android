// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file error.h
 ***********************************************/
#if !defined(ERROR_H)
  #define ERROR_H

  void fnRaiseError             (uint16_t errorCode);
  void fnErrorMessage           (uint16_t errorCode);

  #if !defined(TESTSUITE_BUILD)
    void displayBugScreen     (const char *msg);
  #endif // TESTSUITE_BUILD

  void displayCalcErrorMessage  (uint8_t errorCode, calcRegister_t errMessageRegisterLine, calcRegister_t errRegisterLine);
  void displayDomainErrorMessage(uint8_t errorCode, calcRegister_t errMessageRegisterLine, calcRegister_t errRegisterLine);
  void moreInfoOnError          (const char *m1, const char *m2, const char *m3, const char *m4);
  #if (EXTRA_INFO_ON_CALC_ERROR != 1)
    void typeError              (void);
  #endif // (EXTRA_INFO_ON_CALC_ERROR != 1)
#endif // !ERROR_H

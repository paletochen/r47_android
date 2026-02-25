// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file lblGtoXeq.h
 ***********************************************/
#if !defined(LBLGTOXEQ_H)
  #define LBLGTOXEQ_H

  void    fnGoto            (uint16_t label);
  void    fnGotoDot         (uint16_t globalStepNumber);
  void    fnExecute         (uint16_t label);
  void    fnExecutePlusSkip (uint16_t label);
  void    fnReturn          (uint16_t skip);
  void    fnRunProgram      (uint16_t unusedButMandatoryParameter);
  void    fnStopProgram     (uint16_t unusedButMandatoryParameter);
  void    fnCheckLabel      (uint16_t label);
  void    fnIsTopRoutine    (uint16_t unusedButMandatoryParameter);

  /**
   * Executes one step
   *
   * \param[in]  step   Instruction pointer
   * \return            -1 if already the pointer is set,
   *                    0 if the end of the routine reached,
   *                    1 if the routine continues,
   *                    > 1 if the next step shall be skipped
   */
  int16_t executeOneStep    (uint8_t *step);
  void    runProgram        (bool_t singleStep, uint16_t menuLabel);
  void    execProgram       (uint16_t label);

  void    goToGlobalStep    (int32_t step);
  void    goToPgmStep       (uint16_t program, uint16_t step);
#endif // !LBLGTOXEQ_H

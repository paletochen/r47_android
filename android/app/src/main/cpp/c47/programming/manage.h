// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file manage.h
 ***********************************************/
#if !defined(MANAGE_H)
  #define MANAGE_H

  uint32_t _getProgramSize                     (void);
  void scanLabelsAndPrograms                   (void);
  void defineCurrentProgramFromGlobalStepNumber(int16_t globalStepNumber);
  void defineCurrentProgramFromCurrentStep     (void);
  void deleteStepsFromTo                       (uint8_t *from, uint8_t *to);
  void fnClPAll                                (uint16_t confirmation);
  void fnClP                                   (uint16_t label);
  void fnPem                                   (uint16_t unusedButMandatoryParameter);
  void scrollPemBackwards                      (void);
  void scrollPemForwards                       (void);
  void pemAlpha                                (int16_t item);
  void pemCloseAlphaInput                      (void);
  void pemAlphaEdit                            (uint16_t unusedButMandatoryParameter);
  void pemAddNumber                            (int16_t item, bool doInsertInProgram);
  void pemCloseNumberInput                     (void);
  void insertStepInProgram                     (const int16_t func);
  void insertUserItemInProgram                 (int16_t func, char *funcParam);
  void addStepInProgram                        (int16_t func);

  calcRegister_t findNamedLabel                (const char *labelName);
  calcRegister_t findNamedLabelWithDuplicate   (const char *labelName, int16_t dupNum);
  uint16_t       getNumberOfSteps              (void);

  bool_t         isAtEndOfPrograms             (const uint8_t *step); // check for .END.
  bool_t         checkOpCodeOfStep             (const uint8_t *step, uint16_t op);

  static inline bool_t isAtEndOfProgram        (const uint8_t *step) { // check for END
    return checkOpCodeOfStep(step, ITM_END);
  }
#endif // !MANAGE_H

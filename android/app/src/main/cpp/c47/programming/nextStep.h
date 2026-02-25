// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file nextStep.h
 ***********************************************/
#if !defined(NEXTSTEP_H)
  #define NEXTSTEP_H

  uint8_t *findNextStep            (uint8_t *step);
  uint8_t *findKey2ndParam         (uint8_t *step);
  uint8_t *findPreviousStep        (uint8_t *step);
  void     defineCurrentStep       (void);
  void     defineFirstDisplayedStep(void);
  void     fnBst                   (uint16_t unusedButMandatoryParameter);
  void     fnSst                   (uint16_t unusedButMandatoryParameter);
  void     fnBack                  (uint16_t numberOfSteps);
  void     fnSkip                  (uint16_t numberOfSteps);
  void     fnCase                  (uint16_t regist);
#endif // !NEXTSTEP_H

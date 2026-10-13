// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file nextStep.h
 ***********************************************/
#if !defined(NEXTSTEP_H)
  #define NEXTSTEP_H

  bool_t   programBytesAvailable   (const uint8_t *address, uint16_t numberOfBytes);

  // Shared parameter-tail grammar: a tail is a fixed byte count (>= 0), a length byte followed by that many bytes, or for literals a base byte first, or invalid.
  // Used by the walkers here and by the program-file screening pass in saveRestorePrograms.c.
  #define PARAM_TAIL_INVALID              (-1)
  #define PARAM_TAIL_LENGTH_PREFIXED      (-2)
  #define PARAM_TAIL_BASE_LENGTH_PREFIXED (-3)
  int16_t  paramTailBytes          (uint16_t paramMode, uint16_t op, uint8_t opParam);
  int16_t  literalTailBytes        (uint8_t literalType);
  uint8_t *findNextStep            (uint8_t *step);
  uint8_t *findKey2ndParam         (uint8_t *step);
  uint8_t *findPreviousStep        (uint8_t *step);
  void     defineCurrentStep       (void);
  void     defineFirstDisplayedStep(void);
  void     showStep                (void);
  void     fnBst                   (uint16_t unusedButMandatoryParameter);
  void     fnSst                   (uint16_t unusedButMandatoryParameter);
  void     fnBack                  (uint16_t numberOfSteps);
  void     fnSkip                  (uint16_t numberOfSteps);
  void     fnCase                  (uint16_t regist);
#endif // !NEXTSTEP_H

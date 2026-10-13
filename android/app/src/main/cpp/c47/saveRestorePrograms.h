// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors

/**
 * \file saveRestorePrograms.h
 */
#if !defined(SAVERESTOREPROGRAMS_H)
  #define SAVERESTOREPROGRAMS_H

  void fnSaveProgram    (uint16_t label);
  void fnExportProgram  (uint16_t label);
  void fnLoadProgram    (uint16_t unusedButMandatoryParameter);
  void fnSaveAllPrograms(uint16_t unusedButMandatoryParameter);

#endif // !SAVERESTOREPROGRAMS_H

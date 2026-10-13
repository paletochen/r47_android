// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file unitVector.h
 ***********************************************/
#if !defined(UNITVECTOR_H)
  #define UNITVECTOR_H

  void fnUnitVector   (uint16_t unusedButMandatoryParameter);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    void unitVectorError(void);
  #else // (EXTRA_INFO_ON_CALC_ERROR != 1)
    #define unitVectorError typeError
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  void unitVectorCplx (void);
  void unitVectorRema (void);
  void unitVectorCxma (void);
#endif // !UNITVECTOR_H

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file rotateBits.h
 ***********************************************/
#if !defined(ROTATEBITS_H)
#define ROTATEBITS_H

void fnAsr   (uint16_t numberOfShifts);
void fnSl    (uint16_t numberOfShifts);
void fnSr    (uint16_t numberOfShifts);
void fnRl    (uint16_t numberOfShifts);
void fnRlc   (uint16_t numberOfShifts);
void fnRr    (uint16_t numberOfShifts);
void fnRrc   (uint16_t numberOfShifts);
void fnLj    (uint16_t unusedButMandatoryParameter);
void fnRj    (uint16_t unusedButMandatoryParameter);
void fnMirror(uint16_t unusedButMandatoryParameter);
void fnZip   (uint16_t unusedButMandatoryParameter);
void fnUnzip (uint16_t unusedButMandatoryParameter);
void fnSwapEndian(uint16_t bitWidth);                  //JM
#endif // !ROTATEBITS_H

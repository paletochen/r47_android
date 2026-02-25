// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file inlineTest.h
 ***********************************************/

#if !defined(INLINETEST_H)
#define INLINETEST_H

extern bool_t   testEnabled;
extern uint16_t testBitset;



void     fnSwStart            (uint8_t nr);       // Start StopWatch, 0..3
void     fnSwStop             (uint8_t nr);       // Stop StopWatch, 0..3

void fnSetInlineTest          (uint16_t drConfig);
void fnGetInlineTestBsToX     (uint16_t unusedButMandatoryParameter);
void fnSetInlineTestXToBs     (uint16_t unusedButMandatoryParameter);
void fnSysFreeMem             (uint16_t unusedButMandatoryParameter);
bool_t fnTestBitIsSet         (uint8_t bit);

#endif // !INLINETEST_H

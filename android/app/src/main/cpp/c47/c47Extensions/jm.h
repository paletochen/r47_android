// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#if !defined(JM_H)
#define JM_H

void jm_show_calc_state(char comment[]);
void jm_show_comment   (char comment[]);


// Additional routines needed in jm.c
void fnSigmaAssign(uint16_t sigmaAssign);
void fnGetSigmaAssignToX(uint16_t unusedButMandatoryParameter);

//void fnInfo(bool_t Info);

void fnJM(uint16_t JM_OPCODE);


void fnStrInputReal34 (char inp1[]);
void fnStrInputLongint(char inp1[]);


#endif // !JM_H

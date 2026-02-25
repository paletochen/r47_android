// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/* ADDITIONAL C47 functions and routines */


/********************************************//** //JM
 * \file jmgraph.c TEXTFILES module
 ***********************************************/

#if !defined(TEXTFILES_H)
#define TEXTFILES_H

#define ONELINE true

void         displaywords(char *line1);
int16_t      export_string_to_file(const char line1[TMP_STR_LENGTH]);
void         stackregister_csv_out(int16_t reg_b, int16_t reg_e, bool_t oneLine);
void         tmpString_csv_out(uint8_t nn);
void         copyRegisterToClipboardString2(calcRegister_t regist, char *clipboardString);

#endif // !TEXTFILES_H

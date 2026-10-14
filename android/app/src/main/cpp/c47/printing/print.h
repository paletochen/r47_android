// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file printing/print.h
 * Printing related functions.
 */

#if !defined(PRINT_H)
#define PRINT_H

#define PRINTING_ANNUNCIATOR LIT_EQ
#define PAPER_WIDTH 166

#define TRACE_DATA_ENTRY 0xffffffff

#define PRN_ALL     0
#define PRN_STK     1
#define PRN_REGS    2
#define PRN_GLOBALr 3
#define PRN_LOCALr  4
#define PRN_NAMEDr  5
#define PRN_Xr      6
#define PRN_XYr     7
#define PRN_TMP     8

// Print functions
void fnSetPrinter     (uint16_t model);
void fnP_GetDelay     (uint16_t unusedButMandatoryParameter);
void fnP_SetDelay     (uint16_t delay);
void fnP_All_Regs     (uint16_t option);
void fnP_Sigma        (uint16_t unusedButMandatoryParameter);
void fnP_Regs         (uint16_t registerNo);
void fnP_Alpha        (uint16_t registerNo);
void fnP_Advance      (uint16_t unusedButMandatoryParameter);
void fnP_Byte         (uint16_t byte);
void fnP_Char         (uint16_t registerNo);
void fnP_Tab          (uint16_t column);
void fnP_User         (uint16_t unusedButMandatoryParameter);
void fnP_LCD          (uint16_t unusedButMandatoryParameter);
void fnP_PrinterOnOff (uint16_t op);
void fnP_PrinterMode  (uint16_t mode);
void fnP_PrinterList  (uint16_t lines);
void fnP_PrintAllItems(uint16_t unusedButMandatoryParameter);
void printProgram     (bool_t list, uint16_t lines);

// Print routines
void printTrace               (int16_t func, uint16_t param);
void printTraceX              (uint16_t where);
void printTraceMatElement     (uint16_t where);
void printTraceErrorFunction  (int16_t func,  char *errorString);
void printTraceError          (char *errorString);
void printTraceTI             ();
void printTraceString         (char *string, uint16_t where);
void printInputPrompt         (uint16_t func, uint16_t regist);
void printViewAview           (uint16_t func, uint16_t regist);
void nameAlias                (uint16_t op, char *nameOp);

#endif // PRINT_H

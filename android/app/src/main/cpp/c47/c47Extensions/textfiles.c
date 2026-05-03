// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

typedef struct {
  char     itemName[30];
} nstr;


TO_QSPI static const nstr ClipBoardMsg[] = {
  /*0*/  { "Real matrix " },
  /*1*/  { " too large for transfer" },
  /*2*/  { "Complex matrix " },
  /*3*/  { "res/PROGRAMS" },
  /*4*/  { "C47_LOG.TXT" },
  /*5*/  { "Alpha buffer: " },
};



void copyRegisterToClipboardString2(calcRegister_t regist, char *clipboardString) {
    switch(getRegisterDataType(regist)) {
      case dtLongInteger:
      case dtTime:
      case dtDate:
      case dtString:
      case dtShortInteger:
        copyRegisterToClipboardString(regist, clipboardString, false);
        addChrBothSides(34, clipboardString);   //JMCSV
        break;

      case dtReal34Matrix: {
        matrixHeader_t *matrixHeader = REGISTER_MATRIX_HEADER(regist);
        uint16_t rows, columns;
        rows = matrixHeader->matrixRows;
        columns = matrixHeader->matrixColumns;
        if(rows*columns*46 < TMP_STR_LENGTH) {
          copyRegisterToClipboardString(regist, clipboardString, false);
          //printf(">>>:: %u ?? %u\n", rows*columns*46, stringByteLength(clipboardString));
        }
        else {
          sprintf(clipboardString, "%s%" PRIu16 "x%" PRIu16 "%s", ClipBoardMsg[0].itemName, rows, columns, ClipBoardMsg[1].itemName);  //Real matrix   too large for transfer
        }
        break;
      }

      case dtComplex34Matrix: {
        matrixHeader_t *matrixHeader = REGISTER_MATRIX_HEADER(regist);
        uint16_t rows, columns;
        rows = matrixHeader->matrixRows;
        columns = matrixHeader->matrixColumns;
        if(rows*columns*92 < TMP_STR_LENGTH) {
          copyRegisterToClipboardString(regist, clipboardString, false);
          //printf(">>>:: %u ?? %u\n", rows*columns*92, stringByteLength(clipboardString));
        }
        else {
          sprintf(clipboardString, "%s%" PRIu16 "x%" PRIu16 "%s", ClipBoardMsg[2].itemName, rows, columns, ClipBoardMsg[1].itemName);  //Complex matrix   too large for transfer
        }
        break;
      }

      default:
        copyRegisterToClipboardString(regist, clipboardString, false);
        break;
    }
}


//USING tmpString !!
void stackregister_csv_out(int16_t reg_b, int16_t reg_e, bool_t oneLine) {
    char tmp_b[100], tmp_e[100];

    int16_t ix = reg_b;
    while(ix <= reg_e) {
      tmpString[0] = 0;
      tmp_b[0] = 0;
      tmp_e[0] = 0;
      if(REGISTER_X <= ix && ix <= REGISTER_W) {
        tmp_b[1] = 0;
        tmp_b[0] = letteredRegisterName((calcRegister_t)ix);
        strcat(tmp_b, CSV_TAB);
        }

      else if(FIRST_GLOBAL_REGISTER <= ix && ix < REGISTER_X) {
        sprintf(tmp_b, "%sR%02d%s%s", CSV_STR, ix, CSV_STR, CSV_TAB);
      }
      else if(FIRST_LOCAL_REGISTER <= ix && ix <= LAST_LOCAL_REGISTER) {
        sprintf(tmp_b, "%sR.%03d%s%s", CSV_STR, ix-100, CSV_STR, CSV_TAB);
      }
      else if(FIRST_NAMED_VARIABLE <= ix && ix <= LAST_NAMED_VARIABLE) {
        sprintf(tmp_b, "%sN%03d%s%s%s%s%s%s", CSV_STR, ix-100, CSV_STR, CSV_TAB, CSV_STR, (char *)allNamedVariables[ix - FIRST_NAMED_VARIABLE].variableName + 1, CSV_STR, CSV_TAB);
      }

      #if (VERBOSE_LEVEL >= 1)
        print_linestr("-2b", false);
      #endif // VERBOSE_LEVEL >= 1

      char tmpString2[TMP_STR_LENGTH];
      copyRegisterToClipboardString2((calcRegister_t)ix, tmpString);
      utf8ToString((uint8_t *)tmpString, tmpString2);
      stringToASCII(tmpString2, tmpString);

      #if (VERBOSE_LEVEL >= 1)
        char tmpTmp[TMP_STR_LENGTH+100];
        sprintf(tmpTmp, "-2c: len=%u:%s", (uint16_t)stringByteLength(tmpString), tmpString);
        print_linestr(tmpTmp, false);
      #endif // VERBOSE_LEVEL >= 1

      if(!oneLine || ix == reg_e) {         //use tabs, except at the last register, use newline
        strcat(tmp_e, CSV_NEWLINE);
      }
      else {
        strcat(tmp_e, CSV_TAB);
      }

      //printf(">>>: §%s§%s§%s§\n", tmp_b, tmp, tmp_e);
      addStrBothSides(tmpString, tmp_b, tmp_e);
      //printf(">>>: §%s§\n", tmp);

      #if (VERBOSE_LEVEL >= 1)
        sprintf(tmpTmp, "-2d: len=%u:%s", (uint16_t)stringByteLength(tmpString), tmpString);
        print_linestr(tmpTmp, false);
      #endif // VERBOSE_LEVEL >= 1

      export_append_line(tmpString);                    //Output append to CSV file

      #if (VERBOSE_LEVEL >= 1)
        sprintf(tmpString, ":(ix=%u)------->", ix);
        print_linestr(tmpString, false);
      #endif // VERBOSE_LEVEL >= 1

      ++ix;
    }
}

void tmpString_csv_out(uint8_t nn) {
  export_append_line(CSV_STR);                    //Output append to CSV file
  export_append_line(ClipBoardMsg[nn].itemName);  //"Alpha buffer: " //Output append to CSV file
  export_append_line(CSV_STR);                    //Output append to CSV file
  export_append_line(CSV_TAB);                    //Output append to CSV file
  export_append_line(CSV_STR);                    //Output append to CSV file
  export_append_line(tmpString);                  //Output append to CSV file    //aimBuffer now in tmpString
  export_append_line(CSV_STR);                    //Output append to CSV file
  export_append_line(CSV_NEWLINE);                //Output append to CSV file
}



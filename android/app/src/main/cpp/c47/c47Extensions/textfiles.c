// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

//#define DISPLOADING

#include "c47.h"

typedef struct {
  char     itemName[30];
} nstr;


#if !defined(TESTSUITE_BUILD)
  TO_QSPI static const nstr ClipBoardMsg[] = {
  /*0*/  { "Real matrix " },
  /*1*/  { " too large for transfer" },
  /*2*/  { "Complex matrix " },
  /*3*/  { "res/PROGRAMS" },
  /*4*/  { "C47_LOG.TXT" },
  /*5*/  { "Alpha buffer: " },
  };
#endif //TESTSUITE_BUILD



void copyRegisterToClipboardString2(calcRegister_t regist, char *clipboardString) {
  #if !defined(TESTSUITE_BUILD)
    switch(getRegisterDataType(regist)) {
      case dtLongInteger:
      case dtTime:
      case dtDate:
      case dtString:
      case dtShortInteger:
        copyRegisterToClipboardString(regist, clipboardString);
        addChrBothSides(34,clipboardString);   //JMCSV
        break;

      case dtReal34Matrix: {
        matrixHeader_t *matrixHeader = REGISTER_MATRIX_HEADER(regist);
        uint16_t rows, columns;
        rows = matrixHeader->matrixRows;
        columns = matrixHeader->matrixColumns;
        if(rows*columns*46 < TMP_STR_LENGTH) {
          copyRegisterToClipboardString(regist, clipboardString);
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
          copyRegisterToClipboardString(regist, clipboardString);
          //printf(">>>:: %u ?? %u\n", rows*columns*92, stringByteLength(clipboardString));
        }
        else {
          sprintf(clipboardString, "%s%" PRIu16 "x%" PRIu16 "%s", ClipBoardMsg[2].itemName, rows, columns, ClipBoardMsg[1].itemName);  //Complex matrix   too large for transfer
        }
        break;
      }

      default:
        copyRegisterToClipboardString(regist, clipboardString);
        break;
    }
  #endif // !TESTSUITE_BUILD
}


//USING tmpString !!
void stackregister_csv_out(int16_t reg_b, int16_t reg_e, bool_t oneLine) {
  #if !defined(TESTSUITE_BUILD)
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
        sprintf(tmpTmp,"-2d: len=%u:%s", (uint16_t)stringByteLength(tmpString), tmpString);
        print_linestr(tmpTmp, false);
      #endif // VERBOSE_LEVEL >= 1

      export_append_line(tmpString);                    //Output append to CSV file

      #if (VERBOSE_LEVEL >= 1)
        sprintf(tmpString, ":(ix=%u)------->", ix);
        print_linestr(tmpString, false);
      #endif // VERBOSE_LEVEL >= 1

      ++ix;
    }
  #endif // !TESTSUITE_BUILD
}

void tmpString_csv_out(uint8_t nn) {
  #if !defined(TESTSUITE_BUILD)
    export_append_line(CSV_STR);                    //Output append to CSV file
    export_append_line(ClipBoardMsg[nn].itemName);  //"Alpha buffer: " //Output append to CSV file
    export_append_line(CSV_STR);                    //Output append to CSV file
    export_append_line(CSV_TAB);                    //Output append to CSV file
    export_append_line(CSV_STR);                    //Output append to CSV file
    export_append_line(tmpString);                  //Output append to CSV file    //aimBuffer now in tmpString
    export_append_line(CSV_STR);                    //Output append to CSV file
    export_append_line(CSV_NEWLINE);                //Output append to CSV file
  #endif // !TESTSUITE_BUILD
}


//**********************************************************************************************************
#if !defined(TESTSUITE_BUILD)
  int16_t export_string_to_file(const char line1[TMP_STR_LENGTH]) {
    return export_string_to_filename(line1, APPEND, ClipBoardMsg[3].itemName, ClipBoardMsg[4].itemName);  //"res/PROGRAMS", "C47_LOG.TXT"
  }
#endif // !TESTSUITE_BUILD


//################################################################################################
// OUTPUT is in TMPSTR
void displaywords(char *line1) {  //Preprocessor and display
  char bb[2];
  char aa[2];
  bool_t state_comments = false;
  aa[1] = 0;
  bb[1] = 0;
  bb[0] = 0;

  #if (VERBOSE_LEVEL >= 2)
    print_linestr("Code:", true);
  #endif // VERBOSE_LEVEL >= 2
  //printf("4:%s\n", line1);

  #if (VERBOSE_LEVEL >= 2)
    char tmp[400];          //Messages
    sprintf(tmp, " F: Displaywords: %lu bytes.\n", stringByteLength(line1));
    print_linestr(tmp, false);
    print_linestr(line1, false);
  #endif // VERBOSE_LEVEL >= 2

  if(line1[stringByteLength(line1)-1] != 32) {
    strcat(line1, " ");
  }

  tmpString[0] = 32;
  tmpString[1] =  0;

  int16_t ix = 0;
  while(line1[ix] != 0) {
    aa[0] = line1[ix];
    bb[0] = line1[ix+1];

    if((aa[0]=='/' && bb[0]=='/') && state_comments == false) {         //start comment on double slash
      state_comments = true;
      ix++; //skip the second slash
    }
    else if(state_comments && (aa[0]==13 || aa[0]==10)) {    //stop comment mode at end of line
      state_comments = false;
      strcat(tmpString, " ");
    }
    else if(!state_comments) {                //proceed only if not comment mode
      switch(aa[0]) {
        case ' ':                         //remove all whitespace
        case  8:
        case 13:
        case 10:
        case ',': if(strlen(tmpString) != 0) {
                    if(tmpString[strlen(tmpString)-1] != ' ') {
                      strcat(tmpString, " ");
                    }
                  }
                  break;
        default: strcat(tmpString, aa);
      }
    }

    ix++;
  }
  //printf("5:%s\n", line1);

  #if defined(DISPLOADING)
    char ll[50];
    ll[0] = 0;
  #endif // DISPLOADING
  aa[0] = 0;
  aa[1] = 0;                  //remove consequtive spaces
  ix = 1;
  line1[0] = 0;
  while(tmpString[ix] != 0) {
    aa[0] = tmpString[ix];
    if(tmpString[ix-1] != ' ') {
      strcat(line1, aa);
      #if defined(DISPLOADING)
        strcat(ll, aa);
        if(strlen(ll) > 30 && aa[0] == ' ') {
          #if (VERBOSE_LEVEL >= 2)
            print_linestr(ll, false);
          #endif
          ll[0] = 0;
        }
      #endif // DISPLOADING
    }
    else {
      if(aa[0] != ' ') {
        strcat(line1, aa);
        #if defined(DISPLOADING)
          strcat(ll, aa);
          if(strlen(ll) > 36) {
            #if (VERBOSE_LEVEL >= 2)
              print_linestr(ll, false);
            #endif // VERBOSE_LEVEL >= 2
            ll[0] = 0;
          }
        #endif // DISPLOADING
      }
    }
    ix++;
  }
  #if defined(DISPLOADING)
  if(ll[0] != 0) {
    #if (VERBOSE_LEVEL >= 2)
      print_linestr(ll, false);
    #endif
  }
  #endif // DISPLOADING
//printf("6:%s\n", line1);
}



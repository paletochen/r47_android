// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ui/matrixEditor.c
 ***********************************************/



#include "c47.h"

#define addFlag true
#define STRIP_INTEGER_MATRIX_RADIX true


  any34Matrix_t         openMatrixMIMPointer;
  uint16_t              scrollRow;
  uint16_t              scrollColumn;
  uint16_t              tmpRow;
  uint16_t              matrixIndex = INVALID_VARIABLE;

  // Callers derive maxCols from cols - sCol on unsigned.
  static uint16_t boundScrollColumn(bool_t forEditor, uint16_t sCol, uint16_t cols) {
    if(forEditor && sCol >= cols) {
      scrollColumn = 0;
      return 0;
    }
    return sCol;
  }

  static bool_t incIReal(real34Matrix_t *matrix) {
    setIRegisterAsInt(true, getIRegisterAsInt(true) + 1);
    wrapIJ(matrix->header.matrixRows, matrix->header.matrixColumns);
    return false;
  }

  static bool_t decIReal(real34Matrix_t *matrix) {
    setIRegisterAsInt(true, getIRegisterAsInt(true) - 1);
    wrapIJ(matrix->header.matrixRows, matrix->header.matrixColumns);
    return false;
  }

  static bool_t incJReal(real34Matrix_t *matrix) {
    setJRegisterAsInt(true, getJRegisterAsInt(true) + 1);
    if(wrapIJ(matrix->header.matrixRows, matrix->header.matrixColumns)) {
      insRowRealMatrix(matrix, matrix->header.matrixRows, !addFlag);
      return true;
    }
    else {
      return false;
    }
  }

  static bool_t decJReal(real34Matrix_t *matrix) {
    setJRegisterAsInt(true, getJRegisterAsInt(true) - 1);
    wrapIJ(matrix->header.matrixRows, matrix->header.matrixColumns);
    return false;
  }

  static bool_t incIComplex(complex34Matrix_t *matrix) {
    return incIReal((real34Matrix_t *)matrix);
  }

  static bool_t decIComplex(complex34Matrix_t *matrix) {
    return decIReal((real34Matrix_t *)matrix);
  }

  static bool_t incJComplex(complex34Matrix_t *matrix) {
    setJRegisterAsInt(true, getJRegisterAsInt(true) + 1);
    if(wrapIJ(matrix->header.matrixRows, matrix->header.matrixColumns)) {
      insRowComplexMatrix(matrix, matrix->header.matrixRows, !addFlag);
      return true;
    }
    else {
      return false;
    }
  }

  static bool_t decJComplex(complex34Matrix_t *matrix) {
    return decJReal((real34Matrix_t *)matrix);
  }

void fnEditMatrix(uint16_t regist) {
  if(isRegisterMatrixVector((regist == NOPARAM) ? REGISTER_X : regist) && getVectorRegisterPolarMode((regist == NOPARAM) ? REGISTER_X : regist) != 0) {  //refuse editing a polar vector in the matrix editor
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType((regist == NOPARAM) ? REGISTER_X : regist));
      moreInfoOnError("In function fnEditMatrix:", errorMessage, "Cannot edit polar format as a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  leaveTamModeIfEnabled();

  saveStatsMatrix();
  const uint16_t reg = (regist == NOPARAM) ? REGISTER_X : regist;
  if((getRegisterDataType(reg) == dtReal34Matrix) || (getRegisterDataType(reg) == dtComplex34Matrix)) {
    calcMode = CM_MIM;
    matrixIndex = reg;

    getMatrixFromRegister(reg);

    setIRegisterAsInt(true, 0);
    setJRegisterAsInt(true, 0);
    aimBuffer[0] = 0;
    nimBufferDisplay[0] = 0;
    scrollRow = scrollColumn = 0;
    showMatrixEditor();
    #if defined(OPTION_IR_PRINTING)
      refreshScreen(80);
      printTraceMatElement(LINE_FULL);
    #endif //OPTION_IR_PRINTING
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(reg));
      moreInfoOnError("In function fnEditMatrix:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnOldMatrix(uint16_t unusedParamButMandatory) {
  if(calcMode == CM_MIM) {
    aimBuffer[0] = 0;
    nimBufferDisplay[0] = 0;
    hideCursor();
    cursorEnabled = false;

    if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
      if(openMatrixMIMPointer.realMatrix.matrixElements) {
        realMatrixFree(&openMatrixMIMPointer.realMatrix);
      }
      convertReal34MatrixRegisterToReal34Matrix(matrixIndex, &openMatrixMIMPointer.realMatrix);
    }
    else {
      if(openMatrixMIMPointer.complexMatrix.matrixElements) {
        complexMatrixFree(&openMatrixMIMPointer.complexMatrix);
      }
      convertComplex34MatrixRegisterToComplex34Matrix(matrixIndex, &openMatrixMIMPointer.complexMatrix);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function fnOldMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnGoToElement(uint16_t unusedParamButMandatory) {
  if(calcMode == CM_MIM) {
    mimEnter(false);
    runFunction(ITM_M_GOTO_ROW);
  }
  else {
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function fnGoToElement:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnGoToRow(uint16_t row) {
  if(calcMode == CM_MIM) {
    tmpRow = row;
  }
  else {
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function fnGoToRow:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnGoToColumn(uint16_t col) {
  if(calcMode == CM_MIM) {
    if(tmpRow == 0 || tmpRow > openMatrixMIMPointer.header.matrixRows || col == 0 || col > openMatrixMIMPointer.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "(%" PRIu16 ", %" PRIu16 ") out of range", tmpRow, col);
        moreInfoOnError("In function fnGoToColumn:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
        convertReal34MatrixToReal34MatrixRegister(&openMatrixMIMPointer.realMatrix, matrixIndex);
      }
      else {
        convertComplex34MatrixToComplex34MatrixRegister(&openMatrixMIMPointer.complexMatrix, matrixIndex);
      }
      setIRegisterAsInt(false, tmpRow);
      setJRegisterAsInt(false, col);
    }
    calcModeNormalGui();
  }
  else {
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function fnGoToColumn:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnSetGrowMode(uint16_t growFlag) {
  if(growFlag) {
    setSystemFlag(FLAG_GROW);
  }
  else {
    clearSystemFlag(FLAG_GROW);
  }
}


void fnIncDecI(uint16_t mode) {
  callByIndexedMatrix((mode == DEC_FLAG) ? decIReal : incIReal, (mode == DEC_FLAG) ? decIComplex : incIComplex);
}


void fnIncDecJ(uint16_t mode) {
  callByIndexedMatrix((mode == DEC_FLAG) ? decJReal : incJReal, (mode == DEC_FLAG) ? decJComplex : incJComplex);
}



void _fnInsRow(bool_t add) {
  if(calcMode == CM_MIM) {
    mimEnter(false);
    if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
      insRowRealMatrix(&openMatrixMIMPointer.realMatrix, getIRegisterAsInt(true), add);
    }
    else {
      insRowComplexMatrix(&openMatrixMIMPointer.complexMatrix, getIRegisterAsInt(true), add);
    }
    mimEnter(true);
  }
  else {
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function _fnInsRow:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnInsRow(uint16_t unusedParamButMandatory) {
  _fnInsRow(!addFlag);
}
void fnAddRow(uint16_t unusedParamButMandatory) {
  _fnInsRow(addFlag);
}


void _fnInsCol(bool_t add) {
  if(calcMode == CM_MIM) {
    mimEnter(false);
    if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
      insColRealMatrix(&openMatrixMIMPointer.realMatrix, getJRegisterAsInt(true), add);
    }
    else {
      insColComplexMatrix(&openMatrixMIMPointer.complexMatrix, getJRegisterAsInt(true), add);
    }
    mimEnter(true);
  }
  else {
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function _fnInsCol:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

void fnInsCol(uint16_t unusedParamButMandatory) {
  _fnInsCol(!addFlag);
}
void fnAddCol(uint16_t unusedParamButMandatory) {
  _fnInsCol(addFlag);
}


void fnDelRow(uint16_t unusedParamButMandatory) {
  if(calcMode == CM_MIM) {
    mimEnter(false);
    if(openMatrixMIMPointer.header.matrixRows > 1) {
      if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
        delRowRealMatrix(&openMatrixMIMPointer.realMatrix, getIRegisterAsInt(true));
      }
      else {
        delRowComplexMatrix(&openMatrixMIMPointer.complexMatrix, getIRegisterAsInt(true));
      }
    }
    mimEnter(true);
  }
  else {
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function fnDelRow:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnDelCol(uint16_t unusedParamButMandatory) {
  if(calcMode == CM_MIM) {
    mimEnter(false);
    if(openMatrixMIMPointer.header.matrixColumns > 1) {
      if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
        delColRealMatrix(&openMatrixMIMPointer.realMatrix, getJRegisterAsInt(true));
      }
      else {
        delColComplexMatrix(&openMatrixMIMPointer.complexMatrix, getJRegisterAsInt(true));
      }
    }
    mimEnter(true);
  }
  else {
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function fnDelCol:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


static int16_t getRegisterAsInt(bool_t asArrayPointer, calcRegister_t reg) {
  int16_t ret;
  longInteger_t tmp_lgInt;

  if(getRegisterDataType(reg) == dtLongInteger) {
    convertLongIntegerRegisterToLongInteger(reg, tmp_lgInt);
  }
  else if(getRegisterDataType(reg) == dtReal34) {
    convertReal34ToLongInteger(REGISTER_REAL34_DATA(reg), tmp_lgInt, DEC_ROUND_DOWN);
  }
  else {
    longIntegerInit(tmp_lgInt);
  }
  longIntegerToInt32(tmp_lgInt, ret);

  longIntegerFree(tmp_lgInt);

  if(asArrayPointer) {
    ret--;
  }

  return ret;
}

static void setRegisterAsInt(bool_t asArrayPointer, int16_t toStore, calcRegister_t reg) {
  if(asArrayPointer) {
    toStore++;
  }
  longInteger_t tmp_lgInt;
  longIntegerInit(tmp_lgInt);

  int32ToLongInteger(toStore, tmp_lgInt);
  convertLongIntegerToLongIntegerRegister(tmp_lgInt, reg);

  longIntegerFree(tmp_lgInt);
}

// The shadow row/column pair: the Matrix Editor's cursor while it is open, and the vector functions' walking index while they cross a matrix.
// The user's I and J keep their value and their type. While the shadow is closed the accessors address the real registers, so INDEX, STOIJ,
// RCLIJ and storing to I or J drive the matrix index. Code reading REGISTER_I/J without these accessors reports the user's registers, not the cursor:
// the I+/J+ labels in softmenus.c and items.c, lastI/lastJ in screen.c. None of it runs while the editor is open, as menu_M_EDIT carries no I+/J+.
// shadowI/J go to backup.cfg beside matrixIndex, so the editor reopens on the cell it was left on.
int16_t       shadowI, shadowJ; // 0-based, i.e. what asArrayPointer=true reports
static bool_t ijShadowActive;   // for the vector functions; the editor is spotted by its calcMode

static bool_t ijIsShadowed(void) {
  return calcMode == CM_MIM || ijShadowActive;
}

static void beginShadowedIJ(void) {
  ijShadowActive = true;
}

static void endShadowedIJ(void) {
  ijShadowActive = false;
}

//Row of Matrix
int16_t getIRegisterAsInt(bool_t asArrayPointer) {
  if(ijIsShadowed()) {
    return asArrayPointer ? shadowI : shadowI + 1;
  }
  return getRegisterAsInt(asArrayPointer, REGISTER_I);
}

//Col of Matrix
int16_t getJRegisterAsInt(bool_t asArrayPointer) {
  if(ijIsShadowed()) {
    return asArrayPointer ? shadowJ : shadowJ + 1;
  }
  return getRegisterAsInt(asArrayPointer, REGISTER_J);
}

//Row of Matrix
void setIRegisterAsInt(bool_t asArrayPointer, int16_t toStore) {
  if(ijIsShadowed()) {
    shadowI = asArrayPointer ? toStore : toStore - 1;
    return;
  }
  setRegisterAsInt(asArrayPointer, toStore, REGISTER_I);
}

//ColOfMatrix
void setJRegisterAsInt(bool_t asArrayPointer, int16_t toStore) {
  if(ijIsShadowed()) {
    shadowJ = asArrayPointer ? toStore : toStore - 1;
    return;
  }
  setRegisterAsInt(asArrayPointer, toStore, REGISTER_J);
}

void saveMatrixIndexState(matrixIndexState_t *state) {
  state->matrixIndex = matrixIndex;
  state->savedI      = shadowI;
  state->savedJ      = shadowJ;
  beginShadowedIJ(); // from here the walking index goes to the shadow; the user's I and J are not touched
}

void restoreMatrixIndexState(const matrixIndexState_t *state) {
  endShadowedIJ();
  shadowI     = state->savedI; // an editor open underneath keeps the cursor it had
  shadowJ     = state->savedJ;
  matrixIndex = state->matrixIndex;
}

bool_t wrapIJ(uint16_t rows, uint16_t cols) {
  clearSystemFlag(FLAG_WRAPEDG);
  clearSystemFlag(FLAG_WRAPEND);
  if(getIRegisterAsInt(true) < 0) {
    setIRegisterAsInt(true, rows - 1);
    setSystemFlag(FLAG_WRAPEDG);
    setJRegisterAsInt(true, (getJRegisterAsInt(true) == 0) ? cols - 1 : getJRegisterAsInt(true) - 1);
    if(getJRegisterAsInt(true) == cols - 1 && getIRegisterAsInt(true) == rows - 1) {
      setSystemFlag(FLAG_WRAPEND);
    }
    //printf("WRAP1 I=%u J=%u EDG:%u END:%u\n",getIRegisterAsInt(true), getJRegisterAsInt(true), getSystemFlag(FLAG_WRAPEDG),getSystemFlag(FLAG_WRAPEND));
  }
  else {
    if(getIRegisterAsInt(true) == rows) {
      setIRegisterAsInt(true, 0);
      setSystemFlag(FLAG_WRAPEDG);
      setJRegisterAsInt(true, (getJRegisterAsInt(true) == cols - 1) ? 0 : getJRegisterAsInt(true) + 1);
      if(getJRegisterAsInt(true) == 0 && getIRegisterAsInt(true) == 0) {
        setSystemFlag(FLAG_WRAPEND);
      }
      //printf("WRAP2 I=%u J=%u EDG:%u END:%u\n",getIRegisterAsInt(true), getJRegisterAsInt(true), getSystemFlag(FLAG_WRAPEDG),getSystemFlag(FLAG_WRAPEND));
    }
  }

  if(getJRegisterAsInt(true) < 0) {
    setJRegisterAsInt(true, cols - 1);
    setSystemFlag(FLAG_WRAPEDG);
    setIRegisterAsInt(true, (getIRegisterAsInt(true) == 0) ? rows - 1 : getIRegisterAsInt(true) - 1);
    if(getJRegisterAsInt(true) == cols - 1 && getIRegisterAsInt(true) == rows - 1) {
      setSystemFlag(FLAG_WRAPEND);
    }
    //printf("WRAP3 I=%u J=%u EDG:%u END:%u\n",getIRegisterAsInt(true), getJRegisterAsInt(true), getSystemFlag(FLAG_WRAPEDG),getSystemFlag(FLAG_WRAPEND));
  }
  else {
    if(getJRegisterAsInt(true) == cols) {
      setJRegisterAsInt(true, 0);
      setSystemFlag(FLAG_WRAPEDG);
      setIRegisterAsInt(true, ((!getSystemFlag(FLAG_GROW)) && (getIRegisterAsInt(true) == rows - 1)) ? 0 : getIRegisterAsInt(true) + 1);
      if(getIRegisterAsInt(true) == 0 && getJRegisterAsInt(true) == 0) {
        setSystemFlag(FLAG_WRAPEND);
      }
      //printf("WRAP4 I=%u J=%u EDG:%u END:%u\n",getIRegisterAsInt(true), getJRegisterAsInt(true), getSystemFlag(FLAG_WRAPEDG),getSystemFlag(FLAG_WRAPEND));
    }
  }
  //printf("-----WRAP I=%u J=%u EDG:%u END:%u\n",getIRegisterAsInt(true), getJRegisterAsInt(true), getSystemFlag(FLAG_WRAPEDG),getSystemFlag(FLAG_WRAPEND));
  return getIRegisterAsInt(true) == rows;
}

void showMatrixEditor() {
  int rows = openMatrixMIMPointer.header.matrixRows;
  int cols = openMatrixMIMPointer.header.matrixColumns;
  int16_t width = 0;

  for(int i = 0; i < SOFTMENU_STACK_SIZE; i++) {
    if(softmenu[softmenuStack[i].softmenuId].menuItem == -MNU_M_EDIT) {
      width = 1;
      break;
    }
  }
  if(width == 0) {
    showSoftmenu(-MNU_M_EDIT);
  }
  if(softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_M_EDIT) {
    calcModeNormalGui();
  }

  bool_t colVector = false;
  if(cols == 1 && rows > 1) {
    colVector = true;
    cols = rows;
    rows = 1;
  }

  if(wrapIJ(colVector ? cols : rows, colVector ? 1 : cols)) {
    if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
      insRowRealMatrix(&openMatrixMIMPointer.realMatrix, rows, addFlag); // addFlag: append at true end (rows is swapped to 1 for colVector)
      convertReal34MatrixToReal34MatrixRegister(&openMatrixMIMPointer.realMatrix, matrixIndex);
    }
    else {
      insRowComplexMatrix(&openMatrixMIMPointer.complexMatrix, rows, addFlag); // addFlag: append at true end (rows is swapped to 1 for colVector)
      convertComplex34MatrixToComplex34MatrixRegister(&openMatrixMIMPointer.complexMatrix, matrixIndex);
    }
  }

  int16_t matSelRow = colVector ? getJRegisterAsInt(true) : getIRegisterAsInt(true);
  int16_t matSelCol = colVector ? getIRegisterAsInt(true) : getJRegisterAsInt(true);

  if(matSelRow == 0 || rows <= 5) {
    scrollRow = 0;
  }
  else if(matSelRow == rows - 1) {
    scrollRow = matSelRow - 4;
  }
  else if(matSelRow < scrollRow + 1) {
    scrollRow = matSelRow - 1;
  }
  else if(matSelRow > scrollRow + 3) {
    scrollRow = matSelRow - 3;
  }

  if(aimBuffer[0] == 0) {
    clearRegisterLine(NIM_REGISTER_LINE, true, true);
    if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
      showRealMatrix(&openMatrixMIMPointer.realMatrix, 0, !regXp, NULL);
    }
    else {
      showComplexMatrix(&openMatrixMIMPointer.complexMatrix, 0, currentAngularMode, getSystemFlag(FLAG_POLAR), !regXp);
    }
  }
  else {
    clearRegisterLine(NIM_REGISTER_LINE, false, true);
  }

  sprintf(tmpString, "%" PRIi16 ";%" PRIi16 "=" STD_SPACE_4_PER_EM "%s%s%s", (int16_t)(colVector ? matSelCol+1 : matSelRow+1), (int16_t)(colVector ? 1 : matSelCol+1), aimBuffer[0] == 0 ? STD_SPACE_HAIR : "", (aimBuffer[0] == 0 || aimBuffer[0] == '-') ? "" : " ", nimBufferDisplay);
  width = stringWidth(tmpString, &numericFont, true, true) + 1;
  if(aimBuffer[0] == 0) {
    if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
      real34ToDisplayString(&openMatrixMIMPointer.realMatrix.matrixElements[matSelRow*cols+matSelCol], amNone, &tmpString[strlen(tmpString)], &numericFont, SCREEN_WIDTH - width, NUMBER_OF_DISPLAY_DIGITS, LIMITEXP, FRONTSPACE, LIGHTIRFRAC);
    }
    else {
      complex34ToDisplayString(&openMatrixMIMPointer.complexMatrix.matrixElements[matSelRow*cols+matSelCol], &tmpString[strlen(tmpString)], &numericFont, SCREEN_WIDTH - width, NUMBER_OF_DISPLAY_DIGITS, LIMITEXP, FRONTSPACE, LIMITIRFRAC, currentAngularMode, getSystemFlag(FLAG_POLAR));
    }

    showString(tmpString, &numericFont, 0, Y_POSITION_OF_NIM_LINE, vmNormal, true, false);
  }
  else {
    if(aimBuffer[0] != 0 && aimBuffer[strlen(aimBuffer)-1]=='/') {
      char lastBase[12];
      char *lb = lastBase;
      if(lastDenominator >= 1000) {
        *(lb++) = STD_SUB_0[0];
        *(lb++) = STD_SUB_0[1] + (lastDenominator / 1000);
      }
      if(lastDenominator >= 100) {
        *(lb++) = STD_SUB_0[0];
        *(lb++) = STD_SUB_0[1] + (lastDenominator % 1000 / 100);
      }
      if(lastDenominator >= 10) {
        *(lb++) = STD_SUB_0[0];
        *(lb++) = STD_SUB_0[1] + (lastDenominator % 100 / 10);
      }
      *(lb++) = STD_SUB_0[0];
      *(lb++) = STD_SUB_0[1] + (lastDenominator % 10);
      *(lb++) = 0;
      displayNim(tmpString, lastBase, stringWidth(lastBase, &numericFont, true, true), stringWidth(lastBase, &standardFont, true, true));
    }
    else {
      displayNim(tmpString, "", 0, 0);
    }
  }

  if(temporaryInformation == TI_SHOW_REGISTER && calcMode == CM_MIM) {
    mimShowElement();
    clearRegisterLine(REGISTER_T, true, true);
    refreshRegisterLine(REGISTER_T);
    if(tmpString[SHOWLineSize]) {
      clearRegisterLine(REGISTER_Z, true, true);
      refreshRegisterLine(REGISTER_Z);
    }
  }

  if(lastErrorCode != ERROR_NONE) {
    refreshRegisterLine(errorMessageRegisterLine);
  }
}

void mimEnter(bool_t commit) {
  bool_t realChanged = false;
  int cols = openMatrixMIMPointer.header.matrixColumns;
#if defined(OPTION_VECTOR_EDIT)
  int rows = openMatrixMIMPointer.header.matrixRows;
  int tag  = openMatrixMIMPointer.header.mtag;
#endif //OPTION_VECTOR_EDIT
  int16_t row = getIRegisterAsInt(true);
  int16_t col = getJRegisterAsInt(true);

  if(aimBuffer[0] != 0) {
    #if defined(OPTION_IR_PRINTING)
      if(aimBuffer[0] == '+') {
        printTraceString(aimBuffer + 1, LINE_NOLF);
      }
      else {
        printTraceString(aimBuffer, LINE_NOLF);
      }
    #endif //OPTION_IR_PRINTING
    if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
      real34_t real34tmp;
      real34_t *real34Ptr = &openMatrixMIMPointer.realMatrix.matrixElements[row * cols + col];

      switch(nimNumberPart) {
        case NP_FRACTION_DENOMINATOR:
        case NP_HP32SII_DENOMINATOR:
          closeNimWithFraction(&real34tmp);
          realChanged = true;
          break;
        case NP_COMPLEX_INT_PART:
        case NP_COMPLEX_FLOAT_PART:
        case NP_COMPLEX_EXPONENT:
        case NP_COMPLEX_FRACTION_DENOMINATOR:
        case NP_COMPLEX_HP32SII_DENOMINATOR: {
          complex34_t *complex34Ptr;
          complex34Matrix_t cxma;
          convertReal34MatrixToComplex34Matrix(&openMatrixMIMPointer.realMatrix, &cxma);
          realMatrixFree(&openMatrixMIMPointer.realMatrix);
          convertComplex34MatrixToComplex34MatrixRegister(&cxma, matrixIndex);
          openMatrixMIMPointer.complexMatrix.header.matrixRows = cxma.header.matrixRows;
          openMatrixMIMPointer.complexMatrix.header.matrixColumns = cxma.header.matrixColumns;
          openMatrixMIMPointer.complexMatrix.matrixElements = cxma.matrixElements;
          complex34Ptr = &openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col];
          closeNimWithComplex(VARIABLE_REAL34_DATA(complex34Ptr), VARIABLE_IMAG34_DATA(complex34Ptr));
          break;
        }
        default:
          stringToReal34(aimBuffer, &real34tmp);
          realChanged = true;
      }

      if(realChanged) {
#if defined(OPTION_VECTOR_EDIT)
        if(isMatrixVector(rows, cols)) {
          if(isMatrix2dVectorPOL(rows, cols, tag)) {
            real_t magnitude, theta;
            int ang = (tag & 0x07) == amNone ? currentAngularMode : (tag & 0x07);
            real34ToReal(&openMatrixMIMPointer.realMatrix.matrixElements[0], &magnitude);
            real34ToReal(&openMatrixMIMPointer.realMatrix.matrixElements[1], &theta);
            realRectangularToPolar(&magnitude, &theta, &magnitude, &theta, &ctxtReal39); // theta in radian
            convertAngleFromTo(&theta, amRadian, ang, &ctxtReal39);
            if(col == 0) {
              real34ToReal(&real34tmp, &magnitude);
            }
            if(col == 1) {
              real34ToReal(&real34tmp, &theta);
            }
            convertAngleFromTo(&theta, ang, amRadian, &ctxtReal39);
            if(realCompareLessThan(&magnitude, const_0)) {
              realSetPositiveSign(&magnitude);
              realAdd(&theta, const_pi, &theta, &ctxtReal39);
            }
            realPolarToRectangular(&magnitude, &theta, &magnitude, &theta, &ctxtReal39); // theta in radian
            realToReal34(&magnitude, &openMatrixMIMPointer.realMatrix.matrixElements[0]);
            realToReal34(&theta,     &openMatrixMIMPointer.realMatrix.matrixElements[1]);
          }
          else if(isMatrix3dVectorSPH(rows, cols, tag)) {
//add code for SPH editing           //
          }
          else if(isMatrix3dVectorCYL(rows, cols, tag)) {
//add code for CYL editing           //
          }
          else {
            real34Copy(&real34tmp, real34Ptr);
          }
        }
        else
#endif //OPTION_VECTOR_EDIT
        {
          real34Copy(&real34tmp, real34Ptr);
        }
      }
    }
    else {
      complex34_t *complex34Ptr = &openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col];

      switch(nimNumberPart) {
        case NP_FRACTION_DENOMINATOR:
        case NP_HP32SII_DENOMINATOR:
          closeNimWithFraction(VARIABLE_REAL34_DATA(complex34Ptr));
          real34SetZero(VARIABLE_IMAG34_DATA(complex34Ptr));
          break;
        case NP_COMPLEX_INT_PART:
        case NP_COMPLEX_FLOAT_PART:
        case NP_COMPLEX_EXPONENT:
        case NP_COMPLEX_FRACTION_DENOMINATOR:
        case NP_COMPLEX_HP32SII_DENOMINATOR:
          closeNimWithComplex(VARIABLE_REAL34_DATA(complex34Ptr), VARIABLE_IMAG34_DATA(complex34Ptr));
          break;
        default:
          stringToReal34(aimBuffer, VARIABLE_REAL34_DATA(complex34Ptr));
          real34SetZero(VARIABLE_IMAG34_DATA(complex34Ptr));
      }
    }

    aimBuffer[0] = 0;
    nimBufferDisplay[0] = 0;
    hideCursor();
    cursorEnabled = false;

    setSystemFlag(FLAG_ASLIFT);
  }
  if(commit) {
    if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
      convertReal34MatrixToReal34MatrixRegister(&openMatrixMIMPointer.realMatrix, matrixIndex);
      setRegisterTag(matrixIndex, openMatrixMIMPointer.header.mtag);
    }
    else {
      convertComplex34MatrixToComplex34MatrixRegister(&openMatrixMIMPointer.complexMatrix, matrixIndex);
      setRegisterTag(matrixIndex, openMatrixMIMPointer.header.mtag);
    }
  }
  updateMatrixHeightCache();
}

static void _resetCursorPos() {
  clearRegisterLine(NIM_REGISTER_LINE, false, true);
  sprintf(tmpString, "%" PRIi16";%" PRIi16"= ", (int16_t)getIRegisterAsInt(false), (int16_t)getJRegisterAsInt(false));
  xCursor = showString(tmpString, &numericFont, 0, Y_POSITION_OF_NIM_LINE, vmNormal, true, true) + 1;
  yCursor = Y_POSITION_OF_NIM_LINE;
  cursorEnabled = true;
  cursorFont = &numericFont;
  setLastintegerBasetoZero();
}

void mimAddNumber(int16_t item) {
  const int cols = openMatrixMIMPointer.header.matrixColumns;
  const int16_t row = getIRegisterAsInt(true);
  const int16_t col = getJRegisterAsInt(true);

  switch(item) {
    case ITM_EXPONENT: {
      if(aimBuffer[0] == 0) {
        aimBuffer[0] = '+';
        aimBuffer[1] = '1';
        aimBuffer[2] = '.';
        aimBuffer[3] = 0;
        nimNumberPart = NP_REAL_FLOAT_PART;
        _resetCursorPos();
      }
      break;
    }

    case ITM_PERIOD: {
      if(aimBuffer[0] == 0) {
        aimBuffer[0] = '+';
        aimBuffer[1] = '0';
        aimBuffer[2] = 0;
        nimNumberPart = NP_INT_10;
        _resetCursorPos();
      }
      break;
    }

    case ITM_0 :
    case ITM_1 :
    case ITM_2 :
    case ITM_3 :
    case ITM_4 :
    case ITM_5 :
    case ITM_6 :
    case ITM_7 :
    case ITM_8 :
    case ITM_9: {
      if(aimBuffer[0] == 0) {
        aimBuffer[0] = '+';
        aimBuffer[1] = 0;
        nimNumberPart = NP_INT_10;
        _resetCursorPos();
      }
      break;
    }

    case ITM_BACKSPACE: {
      if(aimBuffer[0] == 0) {
        const int cols = openMatrixMIMPointer.header.matrixColumns;
        const int16_t row = getIRegisterAsInt(true);
        const int16_t col = getJRegisterAsInt(true);

        if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
          real34SetZero(&openMatrixMIMPointer.realMatrix.matrixElements[row * cols + col]);
        }
        else {
          real34SetZero(VARIABLE_REAL34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col]));
          real34SetZero(VARIABLE_IMAG34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col]));
        }
        setSystemFlag(FLAG_ASLIFT);
        return;
      }
      else if((aimBuffer[0] == '+') && (aimBuffer[1] != 0) && (aimBuffer[2] == 0)) {
        aimBuffer[1] = 0;
        hideCursor();
        cursorEnabled = false;
      }
      break;
    }

    case ITM_CHS: {
      if(aimBuffer[0] == 0) {
        if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
          real34ChangeSign(&openMatrixMIMPointer.realMatrix.matrixElements[row * cols + col]);
        }
        else {
          real34ChangeSign(VARIABLE_REAL34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col]));
          real34ChangeSign(VARIABLE_IMAG34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col]));
        }
        setSystemFlag(FLAG_ASLIFT);
        return;
      }
      break;
    }

    case ITM_op_j_pol:
    case ITM_op_j:
    case ITM_CC: {
      if(aimBuffer[0] == 0) {
        if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
          complex34Matrix_t cxma;
          convertReal34MatrixToComplex34Matrix(&openMatrixMIMPointer.realMatrix, &cxma);
          realMatrixFree(&openMatrixMIMPointer.realMatrix);
          convertComplex34MatrixToComplex34MatrixRegister(&cxma, matrixIndex);
          if((getSystemFlag(FLAG_POLAR) && !temporaryFlagRect) || temporaryFlagPolar) { // polar mode
            setRegisterTag(matrixIndex, currentAngularMode | amPolar);
          }
          openMatrixMIMPointer.complexMatrix.header.matrixRows = cxma.header.matrixRows;
          openMatrixMIMPointer.complexMatrix.header.matrixColumns = cxma.header.matrixColumns;
          openMatrixMIMPointer.complexMatrix.matrixElements = cxma.matrixElements;
        }
        else {
          complex34_t *elm = &openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col];
          if((getSystemFlag(FLAG_POLAR) && !temporaryFlagRect) || temporaryFlagPolar) { // polar mode
            real_t theta;
            realCopy(const39_piOn2, &theta);
            convertAngleFromTo(&theta, amRadian, currentAngularMode, &ctxtReal39);
            real34SetOne(VARIABLE_REAL34_DATA(elm));
            real34Copy(&theta, VARIABLE_IMAG34_DATA(elm));
          }
          else {
            real34SetZero(VARIABLE_REAL34_DATA(elm));
            real34SetOne(VARIABLE_IMAG34_DATA(elm));
          }
        }
        #if defined(OPTION_IR_PRINTING)
          printTrace(lastFunc, item);
        #endif //OPTION_IR_PRINTING
        return;
      }
      break;
    }

    case ITM_CONSTpi: {
      if(aimBuffer[0] == 0) {
        if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
          realToReal34(const39_pi, &openMatrixMIMPointer.realMatrix.matrixElements[row * cols + col]);
        }
        else {
          realToReal34(const39_pi, VARIABLE_REAL34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col]));
          real34SetZero(VARIABLE_IMAG34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col]));
        }
      }
      else if(nimNumberPart == NP_COMPLEX_INT_PART && aimBuffer[strlen(aimBuffer) - 1] == 'i') {
        strcat(aimBuffer, "3.141592653589793238462643383279503");
        reallyRunFunction(ITM_ENTER, NOPARAM);
      }
      return;
    }

    default: {
      return;
    }
  }
  addItemToNimBuffer(item);
  calcMode = CM_MIM;
}

void mimRunFunction(int16_t func, uint16_t param) {
  int16_t i = getIRegisterAsInt(true);
  int16_t j = getJRegisterAsInt(true);
  bool_t isComplex = (getRegisterDataType(matrixIndex) == dtComplex34Matrix);
  real34_t re, im, re1, im1;
  bool_t converted = false;
  bool_t liftStackFlag = getSystemFlag(FLAG_ASLIFT);
  bool_t dyadic = false;

  if(isComplex) {
    real34Copy(VARIABLE_REAL34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]), &re1);
    real34Copy(VARIABLE_IMAG34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]), &im1);
  }
  else {
    real34Copy(&openMatrixMIMPointer.realMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j], &re1);
    real34SetZero(&im1);
  }

  mimEnter(true);                                                           //mimEnter converts the matrix to complex value if complex mim entry                           
  isComplex = (getRegisterDataType(matrixIndex) == dtComplex34Matrix);      //Check again if the type cnaged to complex
  clearSystemFlag(FLAG_ASLIFT);
  lastErrorCode = ERROR_NONE;

  if(isComplex) {
    real34Copy(VARIABLE_REAL34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]), &re);
    real34Copy(VARIABLE_IMAG34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]), &im);
  }
  else {
    real34Copy(&openMatrixMIMPointer.realMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j], &re);
    real34SetZero(&im);
  }
  if(isComplex) {
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    real34Copy(&re, REGISTER_REAL34_DATA(REGISTER_X));
    real34Copy(&im, REGISTER_IMAG34_DATA(REGISTER_X));
  }
  else {
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    real34Copy(&re, REGISTER_REAL34_DATA(REGISTER_X));
  }

  saveForUndo();                                                           //an error in the editor destroys the undo buffer, re-store the stack
  switch(func) {                                                           //dyadic fn needs the mx input copied to Y prior to the operation
    case ITM_ADD:
    case ITM_SUB:
    case ITM_MULT:
    case ITM_DIV:
    case ITM_PC:
    case ITM_DELTAPC:
    case ITM_YX:
    case ITM_XTHROOT: {
      dyadic = true;
      if(isComplex) {
        reallocateRegister(REGISTER_Y, dtComplex34, 0, amNone);
        real34Copy(&re1, REGISTER_REAL34_DATA(REGISTER_Y));
        real34Copy(&im1, REGISTER_IMAG34_DATA(REGISTER_Y));
      }
      else {
        reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
        real34Copy(&re1, REGISTER_REAL34_DATA(REGISTER_Y));
      }
      break;
    }
    default: {
      break;
    }
  }

  reallyRunFunction(func, param);

  if(dyadic) {                                                             //the editor left the stack as it was; undo buffer to restore the stack
    for(calcRegister_t regist=getStackTop(); regist>=REGISTER_Y; regist--) {
      copySourceRegisterToDestRegister(SAVED_REGISTER_X - REGISTER_X + regist, regist);
    }
  }

  switch(getRegisterDataType(REGISTER_X)) {
      case dtLongInteger: {
        convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
        break;
      }
      case dtShortInteger: {
        convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
        break;
      }
    case dtReal34:
        case dtComplex34: {
        break;
      }
    default: {
      lastErrorCode = ERROR_INVALID_DATA_TYPE_FOR_OP;
    }
  }

  if(lastErrorCode == ERROR_NONE) {
    if(isComplex && getRegisterDataType(REGISTER_X) == dtComplex34) {
      complex34Copy(REGISTER_COMPLEX34_DATA(REGISTER_X), &openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]);
    }
    else if(isComplex) {
      real34Copy(REGISTER_REAL34_DATA(REGISTER_X), VARIABLE_REAL34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]));
      real34SetZero(                                  VARIABLE_IMAG34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]));
    }
    else if(getRegisterDataType(REGISTER_X) == dtComplex34) { // Convert to a complex matrix
      complex34Matrix_t cxma;
      complex34_t ans;

      complex34Copy(REGISTER_COMPLEX34_DATA(REGISTER_X), &ans);
      converted = true;
      convertReal34MatrixToComplex34Matrix(&openMatrixMIMPointer.realMatrix, &cxma);
      realMatrixFree(&openMatrixMIMPointer.realMatrix);
      convertComplex34MatrixToComplex34MatrixRegister(&cxma, matrixIndex);
      openMatrixMIMPointer.complexMatrix.header.matrixRows = cxma.header.matrixRows;
      openMatrixMIMPointer.complexMatrix.header.matrixColumns = cxma.header.matrixColumns;
      openMatrixMIMPointer.complexMatrix.matrixElements = cxma.matrixElements;

      complex34Copy(&ans, &openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]);
    }
    else {
      real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &openMatrixMIMPointer.realMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]);
    }
  }

  if(matrixIndex == REGISTER_X && !converted) {
    if(isComplex) {
      complex34Matrix_t linkedMatrix;
      convertComplex34MatrixToComplex34MatrixRegister(&openMatrixMIMPointer.complexMatrix, REGISTER_X);
      linkToComplexMatrixRegister(REGISTER_X, &linkedMatrix);
      real34Copy(&re1, VARIABLE_REAL34_DATA(&linkedMatrix.matrixElements[i * linkedMatrix.header.matrixColumns + j]));
      real34Copy(&im1, VARIABLE_IMAG34_DATA(&linkedMatrix.matrixElements[i * linkedMatrix.header.matrixColumns + j]));
    }
    else {
      real34Matrix_t linkedMatrix;
      convertReal34MatrixToReal34MatrixRegister(&openMatrixMIMPointer.realMatrix, REGISTER_X);
      linkToRealMatrixRegister(REGISTER_X, &linkedMatrix);
      real34Copy(&re1, &linkedMatrix.matrixElements[i * linkedMatrix.header.matrixColumns + j]);
    }
  }

    if(liftStackFlag) {
      setSystemFlag(FLAG_ASLIFT);
    }

  updateMatrixHeightCache();
    #if defined(PC_BUILD)
    refreshLcd(NULL);
  #endif // PC_BUILD
}

void mimFinalize(void) {
  if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
    if(openMatrixMIMPointer.realMatrix.matrixElements) {
      realMatrixFree(&openMatrixMIMPointer.realMatrix);
    }
  }
  else if(getRegisterDataType(matrixIndex) == dtComplex34Matrix) {
    if(openMatrixMIMPointer.complexMatrix.matrixElements) {
      complexMatrixFree(&openMatrixMIMPointer.complexMatrix);
    }
  }
  matrixIndex = INVALID_VARIABLE;
}

void mimRestore(void) {
  uint16_t idx = matrixIndex;
  mimFinalize();
  if(idx != INVALID_VARIABLE) {
    getMatrixFromRegister(idx);
    matrixIndex = idx;
  }
}


static void displayVectorAngle(const real34Matrix_t *matrix, int j, int rows, int cols, uint8_t *toBeAngle){
#if defined(OPTION_VECTOR)
  if((getTagAngularMode(matrix->header.mtag)) != amNone) {
    if(isMatrix3dVector(rows, cols)) {
      if((is3dVectorPolarSPH(matrix->header.mtag)) && (j == 1 || j ==2)) {
        *toBeAngle = getTagAngularMode(matrix->header.mtag);
      }
      else if((is3dVectorPolarCYL(matrix->header.mtag)) && (j == 1)) {
        *toBeAngle = getTagAngularMode(matrix->header.mtag);
      }
    }
    else if(isMatrix2dVector(rows, cols)) {
      if((is2dVectorPolar(matrix->header.mtag)) && (j == 1)) {
        *toBeAngle = getTagAngularMode(matrix->header.mtag);
      }
    }
  }
#endif //OPTION_VECTOR
}



static void extractVectorElement34(const real34Matrix_t *matrix, int j, int ii, int rows, int cols, real34_t *element, uint8_t *toBeAngle, uint16_t digits, real_t *aa, real_t *bb, real_t *cc) {
#if defined(OPTION_VECTOR)
  bool_t is2d    = isMatrix2dVector(rows, cols);
  bool_t is3d    = isMatrix3dVector(rows, cols);
  if(!is2d && !is3d) {
    goto noPolarVector;
  }

  decContext c = ctxtReal39;
  if(!getSystemFlag(FLAG_IRFRAC)) {
    c.digits = digits + 3; //NUMBER_OF_DISPLAY_REAL_CONTEXT_DIGITS; //speedup for display purposes (FIX max 19); for vector element, at least two elements in vector, so no mor ethan 10+5 digits possible
  }

  if((isMatrix3dVectorSPH(rows, cols, matrix->header.mtag))) {
    convert3DtoSPH(matrix, aa, bb, cc, *toBeAngle, &c);
    if(getSystemFlag(FLAG_3DPHYS)) {
      switch(j) {
        case 0: realToReal34(aa, element); break;
        case 1: realToReal34(cc, element); break;
        case 2: realToReal34(bb, element); break;
        default:;
      }
    }
    else {
      switch(j) {
        case 0: realToReal34(aa, element); break;
        case 1: realToReal34(bb, element); break;
        case 2: realToReal34(cc, element); break;
        default:;
      }
    }

    //printRealToConsole(aa, "SPH aa=", "\n");
    //printRealToConsole(bb, "SPH bb=", "\n");
    //printRealToConsole(cc, "SPH cc=", "\n");
  }
  else if((isMatrix3dVectorCYL(rows, cols, matrix->header.mtag))) {
    convert3DtoCYL(matrix, aa, bb, cc, *toBeAngle, &c);
    switch(j) {
      case 0: realToReal34(aa, element); break;
      case 1: realToReal34(bb, element); break;
      case 2: realToReal34(cc, element); break;
      default:;
    }
    //printRealToConsole(aa, "CYL aa=", "\n");
    //printRealToConsole(bb, "CYL bb=", "\n");
    //printRealToConsole(cc, "CYL cc=", "\n");
  }
  else if((isMatrix2dVectorPOL(rows, cols, matrix->header.mtag))) {
    convert2DtoPOL(matrix, aa, bb, *toBeAngle, &c);
    switch(j) {
      case 0: realToReal34(aa, element); break;
      case 1: realToReal34(bb, element); break;
      default:;
    }
    //printRealToConsole(aa,"POL aa=","\n");
    //printRealToConsole(bb,"POL bb=","\n");
  }
  else {
noPolarVector:
#endif //OPTION_VECTOR

    real34Copy(&matrix->matrixElements[ii], element);
    //printReal34ToConsole(element," RECT =","\n");
#if defined(OPTION_VECTOR)
  }
#endif //OPTION_VECTOR
}





#define NUMERIC_FONT_HEIGHT_ (NUMERIC_FONT_HEIGHT - 4)        // reduce font spacing to easily bind the matrix lines without any complicated pixel manipulation
#define STANDARD_FONT_HEIGHT_ (STANDARD_FONT_HEIGHT - 2)      // reduce font spacing to easily bind the matrix lines without any complicated pixel manipulation
#define UNSHRUNK_WIDTH        9999                            // above any rendered string, so real34ToDisplayString keeps the requested digit count

void getRealMatrixIntegerColumns(const real34Matrix_t *matrix, uint16_t dispFormat, uint16_t cols, uint16_t sRow, uint16_t sCol, uint16_t maxRows, uint16_t maxCols, bool_t *allElementsInColAreIntegers) {
  // the one copy of the integer-column rule, used by the viewer and by updateMatrixHeightCache so the height cache and the drawn matrix cannot drift apart
  bool_t allowIntegerDisplay = dispFormat != DF_ENG && dispFormat != DF_UN && dispFormat != DF_SF && !(isMatrixVector(maxRows, maxCols) && (is3dVectorPolarSPH(matrix->header.mtag) || is3dVectorPolarCYL(matrix->header.mtag) || is2dVectorPolar(matrix->header.mtag)));
  for(int j = 0; j < maxCols; j++) {
    allElementsInColAreIntegers[j] = allowIntegerDisplay;
    if(allElementsInColAreIntegers[j]) {
      for(int i = 0; i < maxRows; i++) {
        const real34_t *element = &matrix->matrixElements[(i+sRow)*cols+j+sCol];
        if(!real34IsAnInteger(element) || (real34Digits(element) > 1 && real34GetExponent(element) + real34Digits(element) - 1 >= 15) ){
          // integer column is in FIX 0, and >1E15 would clamp the digits out after the decimal in E-form. Take the col out of integer mode when too big for display.
          allElementsInColAreIntegers[j]=false;
          break;
        }
      }
    }
  }
}

// The SHOW page test: Not used on VIEW and stack display
#if defined(OPTION_MX_SHOW)
  #define MX_SHOW_PAGE(prefixWidth, regXposition) (SHOWMODE && (prefixWidth) > 0 && !(regXposition))
#else // !OPTION_MX_SHOW
  #define MX_SHOW_PAGE(prefixWidth, regXposition) false
#endif // OPTION_MX_SHOW

// The upright fit test
// ALL draws every significant digit a value has, so where the layout the stack line uses shows the whole matrix in ALL there is nothing left for another page
// to open up. A column vector is measured in the one-line form it takes there. The measurement is in the standard font and in ALL, with the trailing radix of
// an integer stripped, the form an integer column is drawn in.
// Real only. The complex path passes false for keepOneLine, so a complex matrix takes the rolled out page whenever it fits, whether or not the upright page
// shows it whole. Replicating this needs a second measuring function over complex34Matrix_t: getComplexMatrixColumnWidths measures that same width already but
// calls showsVerticalVector itself, so it cannot be the pre-test without circularity. The gain is a small complex matrix on the one line it already fits on.
#if defined(OPTION_MX_SHOW)
static bool_t matrixFitsUpright(const real34Matrix_t *matrix, int16_t prefixWidth) {
  const int rows = matrix->header.matrixRows;
  const int cols = matrix->header.matrixColumns;
  const bool_t colVector = (cols == 1 && rows > 1);
  const int upRows = colVector ? 1 : rows;
  const int upCols = colVector ? rows : cols;
  char tmpStr[200];
  const uint16_t tmpFormat = displayFormat;
  const uint8_t tmpFormatDigits = displayFormatDigits;
  int16_t width = stringWidth("[" STD_MAT_BR, &standardFont, true, true);
  if(upCols > MATRIX_MAX_COLUMNS || upRows > MATRIX_MAX_ROWS_ON_SHOW) {
    return false;                                      // more than the upright layout draws
  }
  displayFormat = DF_ALL;
  displayFormatDigits = 0;
  for(int j = 0; j < upCols; j++) {
    int16_t widest = 0;
    for(int i = 0; i < upRows; i++) {
      real34ToDisplayString(&matrix->matrixElements[colVector ? j : i * cols + j], amNone, tmpStr, &standardFont, UNSHRUNK_WIDTH, 34, LIMITEXP, FRONTSPACE, LIMITIRFRAC);
      #if STRIP_INTEGER_MATRIX_RADIX
        stripTrailingRadix(tmpStr);
      #endif //STRIP_INTEGER_MATRIX_RADIX
      const int16_t element = stringWidth(tmpStr, &standardFont, true, true);
      widest = element > widest ? element : widest;
    }
    width += widest + stringWidth(STD_SPACE_FIGURE, &standardFont, true, true);
  }
  displayFormat = tmpFormat;
  displayFormatDigits = tmpFormatDigits;
  return width <= MATRIX_LINE_WIDTH - prefixWidth;
}
#else // !OPTION_MX_SHOW
  #define matrixFitsUpright(matrix, prefixWidth) false
#endif // OPTION_MX_SHOW

// The SHOW vertical vector test
// A plain vector shows one element per line, and so does a matrix whose elements plus one blank line between its rows fit the page. Tagged and polar
// vectors keep the one-line form, and so does the stack line during an active SHOW state. keepOneLine is the matrix the upright layout already shows whole.
#if defined(OPTION_MX_SHOW)
static bool_t showsVerticalVector(const matrixHeader_t *header, int16_t prefixWidth, bool_t regXposition, bool_t keepOneLine) {
  const uint32_t rows = header->matrixRows;
  const uint32_t cols = header->matrixColumns;
  return SHOWMODE && prefixWidth > 0 && !regXposition && !keepOneLine
      && ((rows == 1 || cols == 1) ? rows * cols >= 2 : rows * cols + rows - 1 <= MATRIX_MAX_ROWS_ON_SHOW)
      && getTagAngularMode(header->mtag) == amNone && !is2dVectorPolar(header->mtag);
}
#else // !OPTION_MX_SHOW
  #define showsVerticalVector(header, prefixWidth, regXposition, keepOneLine) false
#endif // OPTION_MX_SHOW

// The vertical vector budget
// Screen width less prefix, brackets and the ^T of a transposed row vector.
static int16_t showsVerticalVectorMaxWidth(const matrixHeader_t *header, const font_t *font, int16_t prefixWidth) {
  return SCREEN_WIDTH - 1 - prefixWidth - stringWidth("[", font, true, true) - stringWidth(STD_MAT_BR, font, true, true)
       - ((header->matrixRows > 1 && header->matrixColumns > 1) ? stringWidth("[]", font, true, true) : 0)   // the rolled out page sets the matrix bracket outside the row brackets
       - (header->matrixRows == 1 ? stringWidth(STD_SUP_BOLD_T, font, true, true) : 0);
}

// SHOW format and vector reshape
// M.ALL set: the optimised ALL page. M.ALL clear: the stashed user format. A row vector folds to a column, marked ^T.
static bool_t reshapeVerticalVector(const matrixHeader_t *header, int16_t prefixWidth, bool_t regXposition, bool_t keepOneLine, int *rows, int *cols, bool_t *colVector, bool_t *transposedVector) {
  const bool_t verticalVector = showsVerticalVector(header, prefixWidth, regXposition, keepOneLine);
  *colVector = false;
  *transposedVector = false;
  #if defined(OPTION_MX_SHOW)
    if(SHOWMODE && prefixWidth > 0) {
      displayFormat = getSystemFlag(FLAG_M_ALL) ? DF_ALL : showMatrixUserDisplayFormat;
      displayFormatDigits = getSystemFlag(FLAG_M_ALL) ? 0 : showMatrixUserDisplayFormatDigits;   // one M.ALL page, digits 0
    }
  #endif // OPTION_MX_SHOW
  if(verticalVector) {
    if(*rows == 1) {                                   // a row vector folds, marked ^T
      *transposedVector = true;
      *rows = *cols;
      *cols = 1;
    }
    else if(*cols > 1) {                               // a matrix rolls out row by row into one column
      *rows = *rows * *cols;
      *cols = 1;
    }
  }
  else if(*cols == 1 && *rows > 1) {
    *colVector = true;
    *cols = *rows;
    *rows = 1;
  }
  return verticalVector;
}

// The SIG fit of one value
// Lowers the SIG count with the window at 34; the internal width shrink flips plain values to sci. Returns true when digits shed.
static bool_t sigFitReal34(const real34_t *value, uint8_t tag, char *dest, const font_t *font, int16_t budget, bool_t frontSpace) {
  const uint8_t tmpDigits = displayFormatDigits;
  int ddFree = 33;
  for(int dd = 33; dd >= 1; dd--) {                    // SIG n shows n+1 digits
    displayFormatDigits = dd;
    real34ToDisplayString(value, tag, dest, font, UNSHRUNK_WIDTH, 34, LIMITEXP, frontSpace, LIMITIRFRAC);
    if(strstr(dest, STD_SUB_10) == NULL) {             // the unconstrained best count
      ddFree = dd;
      break;
    }
  }
  int chosen = 0;
  int fallback = 0;
  for(int dd = 33; dd >= 1; dd--) {
    displayFormatDigits = dd;
    real34ToDisplayString(value, tag, dest, font, UNSHRUNK_WIDTH, 34, LIMITEXP, frontSpace, LIMITIRFRAC);
    if(stringWidth(dest, font, true, true) <= budget) {
      if(strstr(dest, STD_SUB_10) == NULL) {           // plain wins over sci
        chosen = dd;
        break;
      }
      if(fallback == 0) {
        fallback = dd;                                 // sci-only fallback count
      }
    }
  }
  if(chosen == 0) {
    chosen = fallback > 0 ? fallback : 1;
    displayFormatDigits = chosen;
    real34ToDisplayString(value, tag, dest, font, UNSHRUNK_WIDTH, 34, LIMITEXP, frontSpace, LIMITIRFRAC);
  }
  displayFormatDigits = tmpDigits;
  return chosen < ddFree;                              // shed when below the free fit
}

// The laid flat page test
// A vector, a two row matrix or a two column matrix that the upright page cannot show whole runs its rows, or its columns, along the line and wraps.
// A column runs flat once the page cannot take every row, a row once it is wider than the columns the line takes. keepOneLine is the matrix the upright
// layout already shows whole, which needs no other page.
#if defined(OPTION_MX_SHOW)
static bool_t laysFlatOnShow(const matrixHeader_t *header, int16_t prefixWidth, bool_t regXposition, bool_t keepOneLine) {
  const uint32_t rows = header->matrixRows;
  const uint32_t cols = header->matrixColumns;
  if(!(SHOWMODE && prefixWidth > 0 && !regXposition) || keepOneLine || getTagAngularMode(header->mtag) != amNone || is2dVectorPolar(header->mtag)) {
    return false;
  }
  if(rows == 1 || cols == 1) {
    return rows * cols > MATRIX_MAX_ROWS_ON_SHOW;      // the upright page shows them all up to its line count
  }
  if(rows == 2) {
    return rows * cols + rows - 1 > MATRIX_MAX_ROWS_ON_SHOW;   // wider than the rolled out page takes
  }
  if(cols == 2) {
    return rows > MATRIX_MAX_ROWS_ON_SHOW;
  }
  return false;
}
#else // !OPTION_MX_SHOW
  #define laysFlatOnShow(header, prefixWidth, regXposition, keepOneLine) false
#endif // OPTION_MX_SHOW

#if defined(OPTION_MX_SHOW)
// The width of a rendered element up to and including its radix mark, which is what the cells are aligned on
static int16_t flatLeftWidth(const char *str, const font_t *font) {
  char head[200];
  int16_t used = 0;
  for(const char *scan = str; *scan != 0; scan++) {
    head[used++] = *scan;
    if(*scan == '.' || *scan == ',') {
      break;
    }
  }
  head[used] = 0;
  return stringWidth(head, font, true, true);
}

// The laid flat page
// One matrix row, or one matrix column, runs along the line and wraps to the next until the page is full. Every line after the first names the row or the
// column it carries. The brackets sit at the two ends only, and a page read by column closes with the transpose mark.
static void showRealMatrixFlat(const real34Matrix_t *matrix, int16_t prefixWidth) {
  char elem[200];
  const font_t *font = &standardFont;
  const int rows = matrix->header.matrixRows;
  const int cols = matrix->header.matrixColumns;
  const bool_t byColumn = (cols == 1) || (rows > 2 && cols == 2);
  const int runs = byColumn ? cols : rows;
  const int runLength = byColumn ? rows : cols;
  const int16_t gap = stringWidth(STD_SPACE_FIGURE, font, true, true);
  const int16_t labelWidth = (runs > 1) ? stringWidth("r2: ", font, true, true) : 0;   // the row and column names sit left of the bracket
  const int16_t leftMargin = prefixWidth > labelWidth ? prefixWidth : labelWidth;
  const int16_t avail = SCREEN_WIDTH - 1 - leftMargin - stringWidth("[]", font, true, true) - (byColumn ? stringWidth(STD_SUP_BOLD_T, font, true, true) : 0);
  const uint16_t tmpDisplayFormat = displayFormat;
  const uint8_t tmpDisplayFormatDigits = displayFormatDigits;
  int16_t maxLeft = 0, maxRight = 0, cellWidth = avail, fontHeight = STANDARD_FONT_HEIGHT_;
  int perLine = 1, linesPerRun = runLength, totalLines = runs * runLength, digits = 34;

  bool_t allIntegers = true;                           // the whole page is one stream, so the integer rule is taken over all of it
  getRealMatrixIntegerColumns(matrix, displayFormat, cols, 0, 0, rows, 1, &allIntegers);
  for(int element = 0; allIntegers && element < rows * cols; element++) {
    const real34_t *value = &matrix->matrixElements[element];
    if(!real34IsAnInteger(value) || (real34Digits(value) > 1 && real34GetExponent(value) + real34Digits(value) - 1 >= 15)) {
      allIntegers = false;
    }
  }
  if(allIntegers) {
    displayFormat = DF_FIX;
    displayFormatDigits = 0;
  }

  for(digits = 34; digits >= 2; digits--) {            // the widest count whose whole page fits, the rule the upright page uses; below two the mantissa goes
    maxLeft = 0;
    maxRight = 0;
    for(int element = 0; element < rows * cols; element++) {
      real34ToDisplayString(&matrix->matrixElements[element], amNone, elem, font, UNSHRUNK_WIDTH, digits, LIMITEXP, FRONTSPACE, LIMITIRFRAC);
      #if STRIP_INTEGER_MATRIX_RADIX
        if(allIntegers) {
          stripTrailingRadix(elem);
        }
      #endif //STRIP_INTEGER_MATRIX_RADIX
      const int16_t left = flatLeftWidth(elem, font);
      const int16_t whole = stringWidth(elem, font, true, true);
      maxLeft = left > maxLeft ? left : maxLeft;
      maxRight = (whole - left) > maxRight ? (whole - left) : maxRight;
    }
    cellWidth = maxLeft + maxRight;
    perLine = (avail + gap) / (cellWidth + gap);
    perLine = perLine < 1 ? 1 : perLine;
    linesPerRun = (runLength + perLine - 1) / perLine;
    totalLines = runs * linesPerRun;
    if(displayFormat != DF_ALL || totalLines <= MATRIX_MAX_ROWS_ON_SHOW) {
      break;                                           // a format the user set keeps its own count, so only the ALL page searches for one
    }
  }
  const bool_t overflows = totalLines > MATRIX_MAX_ROWS_ON_SHOW;
  if(overflows) {
    totalLines = (MATRIX_MAX_ROWS_ON_SHOW / runs) * runs;   // the page ends on a whole set of rows, or of columns
  }
  if(totalLines > MATRIX_MAX_ROWS) {
    fontHeight = STANDARD_FONT_HEIGHT_ - 1;
  }

  const int16_t X_POS = leftMargin;
  int16_t Y_POS = Y_POSITION_OF_REGISTER_T_LINE - REGISTER_LINE_HEIGHT + 1 + totalLines * fontHeight;
  Y_POS += (totalLines == 1 ? STANDARD_FONT_HEIGHT_ : REGISTER_LINE_HEIGHT - STANDARD_FONT_HEIGHT_);
  lcd_fill_rect(X_POS, Y_POS - (totalLines - 1) * fontHeight, SCREEN_WIDTH - X_POS, (totalLines - 1) * fontHeight + STANDARD_FONT_HEIGHT, LCD_SET_VALUE);

  char runLabel[4] = {byColumn ? 'c' : 'r', '0', ':', 0};
  const int16_t cellsX = X_POS + stringWidth("[", font, true, true);
  int lastCell = 0;
  for(int line = 0; line < totalLines; line++) {
    const int16_t lineY = Y_POS - (totalLines - 1 - line) * fontHeight;
    const int run = line % runs;                       // the rows, or the columns, alternate line by line so their elements stay side by side
    if(line < runs) {                                  // the matrix bracket keeps the height it has on any other page, one line per row or column
      showString((runs == 1) ? "[" : (line == 0) ? STD_MAT_TL : STD_MAT_BL, font, X_POS, lineY, vmNormal, true, false);
    }
    if(line > 0 && runs > 1) {                         // a vector is one run, so there is nothing to name
      runLabel[1] = '1' + run;
      showString(runLabel, font, 1, lineY, vmNormal, true, false);
    }
    for(int cell = 0; cell < perLine; cell++) {
      const int within = (line / runs) * perLine + cell;
      if(within >= runLength) {
        break;
      }
      if(overflows && line >= totalLines - runs && cell + 1 == perLine) {   // every row, or every column, ends on its own ellipsis
        showString(STD_ELLIPSIS, font, cellsX + cell * (cellWidth + gap), lineY, vmNormal, true, false);
        break;
      }
      real34ToDisplayString(&matrix->matrixElements[byColumn ? within * cols + run : run * runLength + within], amNone, elem, font, UNSHRUNK_WIDTH, digits, LIMITEXP, FRONTSPACE, LIMITIRFRAC);
      #if STRIP_INTEGER_MATRIX_RADIX
        if(allIntegers) {
          stripTrailingRadix(elem);
        }
      #endif //STRIP_INTEGER_MATRIX_RADIX
      const int16_t elemX = cellsX + cell * (cellWidth + gap) + maxLeft - flatLeftWidth(elem, font);
      showString(elem, font, elemX, lineY, vmNormal, true, false);
    }
  }
  // The closing bracket keeps the same height and sits at the end of the last row, or of the last column
  if(overflows) {                                      // the last cell of every row holds the ellipsis, so the bracket follows that
    lastCell = cellsX + (perLine - 1) * (cellWidth + gap) + stringWidth(STD_ELLIPSIS, font, true, true);
  }
  else {
    const int lastChunk = runLength - ((totalLines / runs) - 1) * perLine;
    lastCell = cellsX + (lastChunk > perLine ? perLine : lastChunk) * (cellWidth + gap) - gap;
  }
  for(int line = totalLines - runs; line < totalLines; line++) {
    const int16_t lineY = Y_POS - (totalLines - 1 - line) * fontHeight;
    showString((runs == 1) ? "]" : (line + 1 == totalLines) ? STD_MAT_BR : STD_MAT_TR, font, lastCell, lineY, vmNormal, true, false);
  }
  if(byColumn) {
    showString(STD_SUP_BOLD_T, font, lastCell + stringWidth("]", font, true, true), Y_POS, vmNormal, true, false);
  }
  displayFormat = tmpDisplayFormat;
  displayFormatDigits = tmpDisplayFormatDigits;
}
#endif // OPTION_MX_SHOW

// dest NULL draws the matrix. dest non-NULL collects a one-row vector into dest as a string and draws nothing. Each element is formatted at the end of dest,
// so dest is also the element scratch. tmpString is touched only when dest is NULL, so tmpString itself is a valid dest.
void showRealMatrix(const real34Matrix_t *matrix, int16_t prefixWidth, bool_t regXposition, char *dest) {
  int rows = matrix->header.matrixRows;
  int cols = matrix->header.matrixColumns;
  //printf("matrix->header.mtag = %d\n",matrix->header.mtag);
  //printf("openMatrixMIMPointer.header.mtag) %d\n", openMatrixMIMPointer.header.mtag);
  int16_t Y_POS = Y_POSITION_OF_REGISTER_X_LINE;
  int16_t X_POS = 0;
  int16_t totalWidth = 0, width = 0;
  const font_t *font;
  int16_t fontHeight = NUMERIC_FONT_HEIGHT_;
  int16_t maxWidth = MATRIX_LINE_WIDTH - prefixWidth;
  int16_t colWidth[MATRIX_MAX_COLUMNS] = {}, rPadWidth[MATRIX_MAX_ROWS_ON_SHOW * MATRIX_MAX_COLUMNS] = {};
  bool_t allElementsInColAreIntegers[MATRIX_MAX_COLUMNS] = {};
  const bool_t forEditor = matrix == &openMatrixMIMPointer.realMatrix;
  const uint16_t sRow = forEditor ? scrollRow : 0;
  uint16_t sCol = forEditor ? scrollColumn : 0;
  const uint16_t tmpDisplayFormat = displayFormat;
  const uint8_t tmpDisplayFormatDigits = displayFormatDigits;

  Y_POS = Y_POSITION_OF_REGISTER_X_LINE - NUMERIC_FONT_HEIGHT_;

  // Reshape and page format
  bool_t colVector, transposedVector;
  const bool_t fitsUpright = matrixFitsUpright(matrix, prefixWidth);   // the upright layout shows it whole, so no other page is tried
  const bool_t verticalVector = reshapeVerticalVector(&matrix->header, prefixWidth, regXposition, fitsUpright, &rows, &cols, &colVector, &transposedVector);
  const uint16_t showFormat = displayFormat;           // restored on the font retry
  const bool_t showPage = MX_SHOW_PAGE(prefixWidth, regXposition);   // the SHOW page, never the stack line
  const bool_t fixPage = showPage && displayFormat == DF_FIX;

  // one row only: a multi-row matrix or the editor's matrix gives an empty dest and shows [n×n Matrix]
  if(dest != NULL && (forEditor || rows > 1)) {
    dest[0] = 0;
    return;
  }

  #if defined(OPTION_MX_SHOW)
    if(dest == NULL && !forEditor && laysFlatOnShow(&matrix->header, prefixWidth, regXposition, fitsUpright)) {
      showRealMatrixFlat(matrix, prefixWidth);
      displayFormat = tmpDisplayFormat;
      displayFormatDigits = tmpDisplayFormatDigits;
      return;
    }
  #endif // OPTION_MX_SHOW

  sCol = boundScrollColumn(forEditor, sCol, cols);

  const bool_t toDisplay = (dest == NULL);

  if(dest != NULL) {
    strcpy(dest, "[");
  }

  // The row limit per context
  uint16_t maxCols = cols > MATRIX_MAX_COLUMNS ? MATRIX_MAX_COLUMNS : cols;
  const uint16_t rowLimit = (!regXposition && prefixWidth > 0) ? (SHOWMODE ? MATRIX_MAX_ROWS_ON_SHOW : MATRIX_MAX_ROWS_ON_VIEW) : MATRIX_MAX_ROWS_ON_STACK;
  const uint16_t maxRows = rows > rowLimit ? rowLimit : rows;
    if(maxCols + sCol >= cols) {
      maxCols = cols - sCol;
    }

  // The rolled out page: every element on its own line, one blank line between the rows of the matrix it came from
  const int groupCols = (verticalVector && matrix->header.matrixRows >= 2 && matrix->header.matrixColumns >= 2) ? matrix->header.matrixColumns : 0;
  const int totalLines = groupCols ? maxRows + matrix->header.matrixRows - 1 : maxRows;

  int16_t matSelRow = colVector ? getJRegisterAsInt(true) : getIRegisterAsInt(true);
  int16_t matSelCol = colVector ? getIRegisterAsInt(true) : getJRegisterAsInt(true);

  videoMode_t vm = vmNormal;

  // The font choice
  font = &numericFont;
  if(rows >= (showPage ? 6 : (forEditor ? 4 : 5)) || (verticalVector && displayFormat == DF_SF && getSystemFlag(FLAG_SIGZEROS))){   // SHOW fits 5 numeric rows; padded SIG takes the small font
smallFont:
    font = &standardFont;
    fontHeight = STANDARD_FONT_HEIGHT_;
    Y_POS = Y_POSITION_OF_REGISTER_X_LINE - STANDARD_FONT_HEIGHT_;
    //maxWidth = MATRIX_LINE_WIDTH_SMALL * 4 - 20;
  }
  // The vertical page layout
  if(verticalVector) {
    maxWidth = showsVerticalVectorMaxWidth(&matrix->header, font, prefixWidth);   // same budget the column-width function fits against
  }
  #if defined(OPTION_MX_SHOW)
    if(totalLines > MATRIX_MAX_ROWS) {
      fontHeight = STANDARD_FONT_HEIGHT_ - 1;                    // 11 rows at 19 px fit the screen
    }
  #endif // OPTION_MX_SHOW

  if(!forEditor) {
    Y_POS += REGISTER_LINE_HEIGHT;
  }
  const bool_t rightEllipsis = (cols > maxCols) && (cols > maxCols + sCol);
  const bool_t leftEllipsis = (sCol > 0);
  int16_t digits;

  if(!regXposition && prefixWidth > 0) {
    Y_POS = Y_POSITION_OF_REGISTER_T_LINE - REGISTER_LINE_HEIGHT + 1 + totalLines * fontHeight;
  }
  if(!regXposition && prefixWidth > 0 && font == &standardFont) {
    Y_POS += (totalLines == 1 ? STANDARD_FONT_HEIGHT_ : REGISTER_LINE_HEIGHT - STANDARD_FONT_HEIGHT_);
  }

  // Measure the column widths
  #if defined(OPTION_MX_SHOW)
    getRealMatrixIntegerColumns(matrix, displayFormat, cols, sRow, sCol, maxRows, maxCols, allElementsInColAreIntegers);      // the selected page format
  #else // !OPTION_MX_SHOW
    getRealMatrixIntegerColumns(matrix, tmpDisplayFormat, cols, sRow, sCol, maxRows, maxCols, allElementsInColAreIntegers);   // the user format
  #endif // OPTION_MX_SHOW

  int16_t baseWidth = (leftEllipsis ? stringWidth(STD_ELLIPSIS " ", font, true, true) : 0) + (rightEllipsis ? stringWidth(" " STD_ELLIPSIS, font, true, true) : 0);
  int16_t mtxWidth = getRealMatrixColumnWidths(matrix, prefixWidth, regXposition, font, colWidth, rPadWidth, &digits, maxCols, allElementsInColAreIntegers);
  bool_t noFix = (mtxWidth < 0);
  mtxWidth = abs(mtxWidth);
  totalWidth = baseWidth + mtxWidth;

  if(displayFormat == DF_ALL && noFix && getSystemFlag(FLAG_M_ALL)) { //user format kept unless M.ALL allows the bias to make it fit
    displayFormat = getSystemFlag(FLAG_ENGOVR) ? DF_ENG : DF_SCI;
    displayFormatDigits = digits;
  }
  // Shed digits: retry small font
  if(totalWidth > maxWidth || leftEllipsis || (showPage && font == &numericFont && digits < 34)) {
    if(font == &numericFont) {
      displayFormat = showFormat;
      displayFormatDigits = tmpDisplayFormatDigits;
      goto smallFont;
    }
    else {
      if(tmpDisplayFormat == DF_ALL || getSystemFlag(FLAG_M_ALL)) {     // user format used unless ALL will be biased to make it fit
        displayFormat = getSystemFlag(FLAG_ENGOVR) ? DF_ENG : DF_SCI;   // ENGOVR decides the exponent form everywhere
        displayFormatDigits = 3;
      }
      mtxWidth = getRealMatrixColumnWidths(matrix, prefixWidth + baseWidth, regXposition, font, colWidth, rPadWidth, &digits, maxCols, allElementsInColAreIntegers);   // the ellipsis takes its width off the budget, else the fit is measured without it
      noFix = (mtxWidth < 0);
      mtxWidth = abs(mtxWidth);
      totalWidth = baseWidth + mtxWidth;
      if(totalWidth > maxWidth && maxCols > 1) {     // the width call fits without the ellipsis the caller adds, so an unfloored shrink strips every column
        maxCols--;
        goto smallFont;
      }
    }
  }

  if(forEditor) {
    if((matSelCol < sCol) && leftEllipsis) {
      scrollColumn--;
      sCol--;
      goto smallFont;
    }
    else if((matSelCol >= sCol + maxCols) && rightEllipsis) {
      scrollColumn++;
      sCol++;
      goto smallFont;
    }
  }

  for(int j = 0; j < maxCols; j++) {
    baseWidth += colWidth[j] + stringWidth(STD_SPACE_FIGURE, font, true, true);
  }
  baseWidth -= stringWidth(STD_SPACE_FIGURE, font, true, true);
  baseWidth += 3;

  // Brackets and x position
  char endChar[6];
  endChar[0] = 0;
  #if defined(OPTION_VECTOR)
    strcpy(endChar, (isMatrix3dVectorCYL(rows, cols, matrix->header.mtag)) ? \
                      "]" STD_SPACE_HAIR STD_SUP_c : \
                    (isMatrix3dVectorSPH(rows, cols, matrix->header.mtag)) ? \
                      "]" STD_SPACE_HAIR STD_SUP_s : \
                    (isMatrix2dVectorPOL(rows, cols, matrix->header.mtag)) ? \
                      "]" STD_SPACE_HAIR STD_SUP_p : \
                      "]");
  //printf("BBBB: CYL:%i SPH:%i string:%s\n",is3dVectorPolarCYL(matrix->header.tag), is3dVectorPolarSPH(matrix->header.tag), endChar);
  #else
    strcpy(endChar, "]");
  #endif //OPTION_VECTOR


  if(!regXposition && prefixWidth > 0) {
    X_POS = prefixWidth;
  }
  else if(!forEditor) {
    X_POS = SCREEN_WIDTH - 1 - ((colVector ? stringWidth("[", font, true, true) + stringWidth(endChar, font, true, true) + stringWidth(STD_SUP_BOLD_T, font, true, true) : stringWidth("[", font, true, true) + stringWidth(endChar, font, true, true)) + baseWidth) - (font == &standardFont ? 0 : 1);
  }

// Blank the page area
if(toDisplay) {
  if(forEditor) {
    clearRegisterLine(REGISTER_X, true, true);
    clearRegisterLine(REGISTER_Y, true, true);
      if(rows >= (font == &standardFont ? 3 : 2)) {
        clearRegisterLine(REGISTER_Z, true, true);
      }
      if(rows >= (font == &standardFont ? 4 : 3)) {
        clearRegisterLine(REGISTER_T, true, true);
      }
  }
  else if(!regXposition && prefixWidth > 0) {
    lcd_fill_rect(X_POS, Y_POS - (totalLines - 1) * fontHeight, stringWidth("[", font, true, true) + baseWidth + stringWidth(endChar, font, true, true) + (transposedVector ? stringWidth(STD_SUP_BOLD_T, font, true, true) : 0), (totalLines - 1) * fontHeight + (font == &numericFont ? NUMERIC_FONT_HEIGHT : STANDARD_FONT_HEIGHT), LCD_SET_VALUE); //blank the area behind the matrix
  }
  else {   // the stack position: a partial refresh leaves stale register text in the band the matrix covers
    lcd_fill_rect(X_POS, Y_POS - (maxRows - 1) * fontHeight, stringWidth("[", font, true, true) + baseWidth + stringWidth(endChar, font, true, true), (maxRows - 1) * fontHeight + (font == &numericFont ? NUMERIC_FONT_HEIGHT_ : STANDARD_FONT_HEIGHT_), LCD_SET_VALUE);
  }
}
  // Draw the rows
  const uint16_t displayFormat1 = displayFormat;
  const uint8_t displayFormatDigits1 = displayFormatDigits;

int16_t colX = 0;
real_t aa, bb, cc;

  // The rolled out page labels each row after the first in the prefix column
  const int16_t outerBracket = groupCols ? stringWidth("[", font, true, true) : 0;
  if(toDisplay && groupCols) {
    char rowLabel[4] = {'r', '0', ':', 0};
    for(int group = 1; group < matrix->header.matrixRows; group++) {
      rowLabel[1] = '1' + group;
      showString(rowLabel, &standardFont, 1, Y_POS - (totalLines - 1 - group * (groupCols + 1)) * fontHeight, vmNormal, true, false);
    }
  }

  for(int i = 0; i < maxRows; i++) {
    const int lineFromBottom = groupCols ? (totalLines - 1 - (i + i / groupCols)) : (maxRows - 1 - i);
    if(toDisplay) {
      colX = stringWidth("[", font, true, true);
      showString(groupCols ? "[" : (maxRows == 1) ? "[" : (i == 0) ? STD_MAT_TL : (i + 1 == maxRows) ? STD_MAT_BL : STD_MAT_ML, font, X_POS + 5 + outerBracket, Y_POS - lineFromBottom * fontHeight, vmNormal, false, false);
      if(groupCols && i == 0) {                        // the matrix bracket opens outside the row bracket of the first element
        showString("[", font, X_POS + 5, Y_POS - lineFromBottom * fontHeight, vmNormal, false, false);
      }
      if(leftEllipsis) {
        showString(STD_ELLIPSIS " ", font, X_POS + 5 + outerBracket + stringWidth("[", font, true, true), Y_POS - lineFromBottom * fontHeight, vmNormal, true, false);
        colX += stringWidth(STD_ELLIPSIS " ", font, true, true);
      }
    }

//from here, convert to use a single string
    for(int j = 0; j < maxCols + (rightEllipsis ? 1 : 0); j++) {

      if(allElementsInColAreIntegers[j]) {
        displayFormat = DF_FIX;
        displayFormatDigits = 0;
      }
      else {
        displayFormat = displayFormat1;
        displayFormatDigits = displayFormatDigits1;
      }

      char *elem;
      if(dest == NULL) {
        elem = tmpString;
      }
      else {
        if(j > 0) {
          strcat(dest, " ");
        }
        elem = dest + stringByteLength(dest);
      }

      if(((i == maxRows - 1) && (rows > maxRows + sRow)) || ((j == maxCols) && rightEllipsis) || ((i == 0) && (sRow > 0))) {
        strcpy(elem, " " STD_ELLIPSIS);
        vm = vmNormal;
      }
      else {

        uint8_t toBeAngle = amNone;
        displayVectorAngle(matrix, j, rows, cols, &toBeAngle);
        real34_t element;

        if(displayFormat != DF_ALL) {
          digits = fixPage ? 33 : (showPage ? 34 : 15);   // SHOW opens to 34 digits; the FIX page window is 33 so e33 up takes the sci form
        }
        extractVectorElement34(matrix, j, (i+sRow)*cols+j+sCol, rows, cols, &element, &toBeAngle, digits, &aa, &bb, &cc);
        if(displayFormat == DF_SF && verticalVector) {
          sigFitReal34(&element, toBeAngle, elem, font, maxWidth - 4, FRONTSPACE);   // same budget as measured
        }
        else {
          real34ToDisplayString(&element, toBeAngle, elem, font, colWidth[j], digits, LIMITEXP, FRONTSPACE, cols*rows > 3 ? LIMITIRFRAC : LIGHTIRFRAC);
        }

        #if STRIP_INTEGER_MATRIX_RADIX
          if(allElementsInColAreIntegers[j]) {
            stripTrailingRadix(elem);
          }
        #endif //STRIP_INTEGER_MATRIX_RADIX

        if(toDisplay) {
          if(forEditor && matSelRow == (i + sRow) && matSelCol == (j + sCol)) {
            lcd_fill_rect(X_POS + 5 + outerBracket + colX, Y_POS - lineFromBottom * fontHeight, colWidth[j], font == &numericFont ? 32 : 20, LCD_EMPTY_VALUE);
            vm = vmReverse;
          }
          else {
            vm = vmNormal;
          }
        }
      }
      if(toDisplay) {
        width = stringWidth(elem, font, true, true) + 1;
        showString(elem, font, X_POS + 5 + outerBracket + colX + (((j == maxCols) && rightEllipsis) ? -stringWidth(" ", font, true, true) : (colWidth[j] - width) - rPadWidth[i * MATRIX_MAX_COLUMNS + j]), Y_POS - lineFromBottom * fontHeight, vm, true, false);
        colX += colWidth[j] + stringWidth(STD_SPACE_FIGURE, font, true, true) - 1;
      }
    }
//end string creation

//printf("AAAA: CYL:%i SPH:%i string:%s\n",is3dVectorPolarCYL(matrix->header.tag), is3dVectorPolarSPH(matrix->header.tag), endChar);
    if(toDisplay) {
      showString(groupCols ? "]" : (maxRows == 1) ? endChar : (i == 0) ? STD_MAT_TR : (i + 1 == maxRows) ? STD_MAT_BR : STD_MAT_MR, font, X_POS + outerBracket + stringWidth("[", font, true, true) + baseWidth, Y_POS - lineFromBottom * fontHeight, vmNormal, true, false);
      if(groupCols && i + 1 == maxRows) {              // the matrix bracket closes outside the row bracket of the last element
        showString("]", font, X_POS + outerBracket + stringWidth("[]", font, true, true) + baseWidth, Y_POS - lineFromBottom * fontHeight, vmNormal, true, false);
      }
      if(colVector == true) {
        showString(STD_SUP_BOLD_T, font, X_POS + stringWidth("[", font, true, true) + stringWidth(endChar, font, true, true) + baseWidth, Y_POS - lineFromBottom * fontHeight, vmNormal, true, false);
      }
    }
    if(dest != NULL) {
      strcat(dest, endChar);
      if(colVector == true) {
        strcat(dest, STD_SUP_BOLD_T);
      }
    }

  }

  if(toDisplay && transposedVector) {                  // the ^T on the closing bracket
    showString(STD_SUP_BOLD_T, font, X_POS + stringWidth("[", font, true, true) + stringWidth(STD_MAT_BR, font, true, true) + baseWidth, Y_POS, vmNormal, true, false);
  }

  // Restore the format globals
  displayFormat = tmpDisplayFormat;
  displayFormatDigits = tmpDisplayFormatDigits;

}

int16_t getRealMatrixColumnWidths(const real34Matrix_t *matrix, int16_t prefixWidth, bool_t regXposition, const font_t *font, int16_t *colWidth, int16_t *rPadWidth, int16_t *digits, uint16_t maxCols, bool_t *allElementsInColAreIntegers) {
  // The measured page shape
  char tmpString[200];
  const bool_t showPage = MX_SHOW_PAGE(prefixWidth, regXposition);   // the SHOW page, never the stack line
  const bool_t fixPage = showPage && displayFormat == DF_FIX;
  const bool_t verticalVector = showsVerticalVector(&matrix->header, prefixWidth, regXposition, matrixFitsUpright(matrix, prefixWidth));   // one shared column under SHOW
  const bool_t colVector = !verticalVector && matrix->header.matrixColumns == 1 && matrix->header.matrixRows > 1;
  const int rows = verticalVector ? matrix->header.matrixRows * matrix->header.matrixColumns : colVector ? 1 : matrix->header.matrixRows;
  const int actualCols = verticalVector ? 1 : colVector ? matrix->header.matrixRows : matrix->header.matrixColumns;
  const int cols = (actualCols > maxCols) ? maxCols : actualCols;   // clamp for safety
  const int rowLimit = showPage ? MATRIX_MAX_ROWS_ON_SHOW : MATRIX_MAX_ROWS;   // SHOW takes more rows
  const int maxRows = rows > rowLimit ? rowLimit : rows;
  const bool_t forEditor = matrix == &openMatrixMIMPointer.realMatrix;
  const uint16_t sRow = forEditor ? scrollRow : 0;
  const uint16_t sCol = forEditor ? scrollColumn : 0;
  // The width budget
  const int16_t maxWidth = verticalVector ? showsVerticalVectorMaxWidth(&matrix->header, font, prefixWidth) : MATRIX_LINE_WIDTH - prefixWidth;
  int16_t totalWidth = 0;
  int16_t maxRightWidth[MATRIX_MAX_COLUMNS] = {};
  int16_t maxLeftWidth[MATRIX_MAX_COLUMNS] = {};
  const int16_t exponentOutOfRange = 0x4000;
  bool_t noFix = false; const int16_t dspDigits = displayFormatDigits;
  bool_t sfShed = false;                               // a row shed digits

  // The SHOW starting count
  uint16_t startDigitCountDown = max(min(displayFormatDigits*(displayFormat == DF_ALL ? 2 : 1), max((50/cols-2), 0) ), 10);
  if(showPage) {
    startDigitCountDown = 34;   // start at full precision
    if(displayFormat == DF_SF) {
      startDigitCountDown = 33;                        // SIG n shows n+1 digits
    }
    else if(displayFormat == DF_FIX) {                 // cap FIX at 34 total digits
      int16_t maxE = 0;
      for(int i = 0; i < maxRows; i++) {
        for(int j = 0; j < maxCols; j++) {
          const real34_t *e34 = &matrix->matrixElements[(i+sRow)*actualCols+j+sCol];
          const int16_t e = real34IsZero(e34) ? 0 : real34GetExponent(e34) + real34Digits(e34) - 1;
          if(!allElementsInColAreIntegers[j] && e > maxE && e <= 32) {   // plain-renderable rows only; e33 up takes the sci form through the 33 window
            maxE = e;
          }
        }
      }
      startDigitCountDown = max(min(33 - maxE, 99), 1);
    }
  }
  else if(isMatrix3dVector(rows, cols)) {
    startDigitCountDown = 5;
  }
  else if(isMatrix2dVector(rows, cols)) {
    startDigitCountDown = 7;
  }

  begin:
  for(int k = startDigitCountDown; k >= 1; k--) {                                    //HERE IS THE TIME WASTER - CYCLING THROUGH 15 PRECISIONS !! REDUCE SIGNIFICANTLY from 15 to settingx2 or setting
      if(displayFormat == DF_ALL) {
        *digits = k;
      }
      else if(showPage) {   // the shared maximised count
        displayFormatDigits = (displayFormat == DF_SF && verticalVector) ? 34 : k;
        *digits = k;
      }
    if(displayFormat == DF_ALL && noFix && getSystemFlag(FLAG_M_ALL)) { // something like SCI
      displayFormat = getSystemFlag(FLAG_ENGOVR) ? DF_ENG : DF_SCI;
      displayFormatDigits = k;
    }

    const uint16_t displayFormat1 = displayFormat;
    const uint8_t displayFormatDigits1 = displayFormatDigits;
    real_t aa, bb, cc;

    for(int i = 0; i < maxRows; i++) {
      for(int j = 0; j < maxCols; j++) {
        real34_t r34Val;
//      real34Copy(&matrix->matrixElements[(i+sRow)*cols+j+sCol], &r34Val);

        // Measure one element
        uint8_t toBeAngle = amNone;
        displayVectorAngle(matrix, j, rows, cols, &toBeAngle);
        uint16_t calcDigits = (displayFormat == DF_ALL && !allElementsInColAreIntegers[j]) ? k : (fixPage ? 33 : (showPage ? 34 : 15));   // integer columns at 15 digits, SHOW at 34; the FIX page window is 33
        extractVectorElement34(matrix, j, (i+sRow)*actualCols+j+sCol, rows, cols, &r34Val, &toBeAngle, calcDigits, &aa, &bb, &cc);   // a row steps by the matrix width, not by the count of columns on screen

        bool_t r34sign = real34IsNegative(&r34Val);

        if(allElementsInColAreIntegers[j]){ // && !(maxRows == 1 && (maxCols == 2 || maxCols == 3))) {  //no integers needed in vector
          displayFormat = DF_FIX;
          displayFormatDigits = 0;
        }
        else {
          displayFormat = displayFormat1;
          displayFormatDigits = displayFormatDigits1;
        }


        if(displayFormat == DF_SF && verticalVector) {
          if(sigFitReal34(&r34Val, toBeAngle, tmpString, font, maxWidth - 4, FRONTSPACE)) {   // per-row SIG self-fit
            sfShed = true;
          }
        }
        else {
          // no internal shrink under SHOW: it hides shed digits and flips SIG to sci; the k countdown does the fitting
          real34ToDisplayString(&r34Val, toBeAngle, tmpString, font, showPage ? UNSHRUNK_WIDTH : maxWidth, calcDigits, LIMITEXP, FRONTSPACE, cols*rows > 3 ? LIMITIRFRAC : LIGHTIRFRAC);
        }
        if(r34sign) {                                                    // the signed value rounds as the cell does; its sign is measured as the front space
          char *minusSign = strchr(tmpString, '-');
          if(minusSign != NULL) {
            *minusSign = ' ';
          }
        }
        #if STRIP_INTEGER_MATRIX_RADIX
          if(allElementsInColAreIntegers[j]) {
            stripTrailingRadix(tmpString);
          }
        #endif //STRIP_INTEGER_MATRIX_RADIX
        if(displayFormat == DF_ALL && !noFix && strstr(tmpString, STD_SUB_10)) { // something like SCI
          noFix = true;
          totalWidth = 0;
          for(int p = 0; p < MATRIX_MAX_COLUMNS; ++p) {
            maxRightWidth[p] = maxLeftWidth[p] = 0;
          }
          goto begin; // redo
        }

        // Align on the radix
        int16_t width = stringWidth(tmpString, font, true, true) + 1;
        rPadWidth[i * MATRIX_MAX_COLUMNS + j] = 0;
        if((strstr(tmpString, ".") || strstr(tmpString, ",")) && !(verticalVector && displayFormat == DF_SF)) {   // vector SIG skips the line-up
          for(char *xStr = tmpString; *xStr != 0; xStr++) {
            if(((displayFormat != DF_ENG && (displayFormat != DF_ALL || !getSystemFlag(FLAG_ENGOVR))) && (*xStr == '.' || *xStr == ',')) ||
               ((displayFormat == DF_ENG || (displayFormat == DF_ALL && getSystemFlag(FLAG_ENGOVR))) && xStr[0] == (char)0x80 && (xStr[1] == (char)0x87 || xStr[1] == (char)0xd7))) {  //STD_CROSS
              rPadWidth[i * MATRIX_MAX_COLUMNS + j] = stringWidth(xStr, font, true, true) + 1;
              if(maxRightWidth[j] < rPadWidth[i * MATRIX_MAX_COLUMNS + j]) {
                maxRightWidth[j] = rPadWidth[i * MATRIX_MAX_COLUMNS + j];
              }
              break;
            }
          }
          if(maxLeftWidth[j] < (width - rPadWidth[i * MATRIX_MAX_COLUMNS + j])) {
            maxLeftWidth[j] = (width - rPadWidth[i * MATRIX_MAX_COLUMNS + j]);
          }
        }
        else {
          if(r34sign && (strstr(tmpString, "/") || strstr(tmpString, STD_ALMOST_EQUAL))) {
            width += stringWidth("-", font, true, true);
          }
          rPadWidth[i * MATRIX_MAX_COLUMNS + j] = width | exponentOutOfRange;
        }
      }
    }

    displayFormat = displayFormat1;
    displayFormatDigits = displayFormatDigits1;

    // Exponent rows widen the column
    for(int i = 0; i < maxRows; i++) {
      for(int j = 0; j < maxCols; j++) {
        if(rPadWidth[i * MATRIX_MAX_COLUMNS + j] & exponentOutOfRange) {
          if((maxLeftWidth[j] + maxRightWidth[j]) < (rPadWidth[i * MATRIX_MAX_COLUMNS + j] & (~exponentOutOfRange))) {
            maxLeftWidth[j] = (rPadWidth[i * MATRIX_MAX_COLUMNS + j] & (~exponentOutOfRange)) - maxRightWidth[j];
          }
        }
      }
    }
    for(int i = 0; i < maxRows; i++) {
      for(int j = 0; j < maxCols; j++) {
        if(rPadWidth[i * MATRIX_MAX_COLUMNS + j] & exponentOutOfRange) {
          rPadWidth[i * MATRIX_MAX_COLUMNS + j] = 0;
        }
        else {
          rPadWidth[i * MATRIX_MAX_COLUMNS + j] -= maxRightWidth[j];
          rPadWidth[i * MATRIX_MAX_COLUMNS + j] *= -1;
        }
      }
    }
    // Sum and test the fit
    for(int j = 0; j < maxCols; j++) {
      colWidth[j] = (maxLeftWidth[j] + maxRightWidth[j]);
      totalWidth += colWidth[j] + stringWidth(STD_SPACE_FIGURE, font, true, true);   // one gap per column and a 3 pixel end margin, the sum the drawing code lays out, so the fit is neither heavy nor light
    }
    totalWidth -= stringWidth(STD_SPACE_FIGURE, font, true, true) - 3;
    if(noFix) {
      displayFormat = DF_ALL;
      displayFormatDigits = dspDigits;
    }
    if(displayFormat != DF_ALL && !showPage) {
      break;
    }
    else if(totalWidth <= maxWidth) {
      *digits = (showPage && k == startDigitCountDown && !sfShed) ? 34 : k;   // nothing shed: the font retry rests
      break;
    }
    else if(k > 1) {
      totalWidth = 0;
      for(int j = 0; j < maxCols; j++) {
        maxRightWidth[j] = 0;
        maxLeftWidth[j] = 0;
      }
    }
  }
  return totalWidth * (noFix ? -1 : 1);
}


void showComplexMatrix(const complex34Matrix_t *matrix, int16_t prefixWidth, angularMode_t angleMode, bool_t polarMode, bool_t regXposition) {
  int rows = matrix->header.matrixRows;
  int cols = matrix->header.matrixColumns;
  int16_t Y_POS = Y_POSITION_OF_REGISTER_X_LINE;
  int16_t X_POS = 0;
  int16_t totalWidth = 0, width = 0;
  const font_t *font;
  int16_t fontHeight = NUMERIC_FONT_HEIGHT_;
  int16_t maxWidth = MATRIX_LINE_WIDTH - prefixWidth;
  int16_t colWidth[MATRIX_MAX_COLUMNS] = {}, colWidth_r[MATRIX_MAX_COLUMNS] = {}, colWidth_i[MATRIX_MAX_COLUMNS] = {};
  int16_t rPadWidth_r[MATRIX_MAX_ROWS_ON_SHOW * MATRIX_MAX_COLUMNS] = {}, rPadWidth_i[MATRIX_MAX_ROWS_ON_SHOW * MATRIX_MAX_COLUMNS] = {};
  const bool_t forEditor = matrix == &openMatrixMIMPointer.complexMatrix;
  const uint16_t sRow = forEditor ? scrollRow : 0;
  uint16_t sCol = forEditor ? scrollColumn : 0;
  const uint16_t tmpDisplayFormat = displayFormat;
  const int16_t tmpExponentLimit = exponentLimit;
  const uint8_t tmpDisplayFormatDigits = displayFormatDigits;
  const bool_t tmpMultX = getSystemFlag(FLAG_MULTx);

  Y_POS = Y_POSITION_OF_REGISTER_X_LINE - NUMERIC_FONT_HEIGHT_;

  // Reshape and page format
  bool_t colVector, transposedVector;
  const bool_t verticalVector = reshapeVerticalVector(&matrix->header, prefixWidth, regXposition, false, &rows, &cols, &colVector, &transposedVector);   // a complex vector has no integer form to keep
  const bool_t showPage = MX_SHOW_PAGE(prefixWidth, regXposition);   // the SHOW page, never the stack line
  const bool_t fixPage = showPage && displayFormat == DF_FIX;

  sCol = boundScrollColumn(forEditor, sCol, cols);

  int maxCols = cols > MATRIX_MAX_COLUMNS ? MATRIX_MAX_COLUMNS : cols;
  const int rowLimit = (!regXposition && prefixWidth > 0) ? (SHOWMODE ? MATRIX_MAX_ROWS_ON_SHOW : MATRIX_MAX_ROWS_ON_VIEW) : MATRIX_MAX_ROWS_ON_STACK;   // VIEW/SHOW/Stack
  const int maxRows = rows > rowLimit ? rowLimit : rows;

  // The rolled out page: every element on its own line, one blank line between the rows of the matrix it came from
  const int groupCols = (verticalVector && matrix->header.matrixRows >= 2 && matrix->header.matrixColumns >= 2) ? matrix->header.matrixColumns : 0;
  const int totalLines = groupCols ? maxRows + matrix->header.matrixRows - 1 : maxRows;

  int16_t matSelRow = colVector ? getJRegisterAsInt(true) : getIRegisterAsInt(true);
  int16_t matSelCol = colVector ? getIRegisterAsInt(true) : getJRegisterAsInt(true);

  videoMode_t vm = vmNormal;
    if(maxCols + sCol >= cols) {
      maxCols = cols - sCol;
    }

  // The font choice
  font = &numericFont;
  if(rows >= (showPage ? 6 : (forEditor ? 4 : 5)) || (verticalVector && displayFormat == DF_SF && getSystemFlag(FLAG_SIGZEROS))) {   // SHOW fits 5 numeric rows; padded SIG takes the small font
smallFont:
    font = &standardFont;
    fontHeight = STANDARD_FONT_HEIGHT_;
    Y_POS = Y_POSITION_OF_REGISTER_X_LINE - STANDARD_FONT_HEIGHT_ + 2;
    //maxWidth = MATRIX_LINE_WIDTH_SMALL * 4 - 20;
  }
  // The vertical page layout
  if(verticalVector) {
    // same budget plus figure space
    maxWidth = showsVerticalVectorMaxWidth(&matrix->header, font, prefixWidth) + stringWidth(STD_SPACE_FIGURE, font, true, true);
  }
  // The SIG part budget
  int16_t sfPartWidth = 0;
  if(verticalVector && displayFormat == DF_SF) {
    if(polarMode) {
      strcpy(tmpString, STD_SPACE_4_PER_EM STD_MEASURED_ANGLE STD_SPACE_4_PER_EM);
    }
    else {
      strcpy(tmpString, "+");
      strcat(tmpString, COMPLEX_UNIT);
      strcat(tmpString, PRODUCT_SIGN);
    }
    sfPartWidth = (maxWidth - stringWidth(tmpString, font, true, true) - stringWidth(STD_SPACE_FIGURE, font, true, true) - 2) / 2;
  }
  #if defined(OPTION_MX_SHOW)
    if(totalLines > MATRIX_MAX_ROWS) {
      fontHeight = STANDARD_FONT_HEIGHT_ - 1;                    // 11 rows at 19 px fit the screen
    }
  #endif // OPTION_MX_SHOW

    if(!forEditor) {
      Y_POS += REGISTER_LINE_HEIGHT;
    }
  bool_t rightEllipsis = (cols > maxCols) && (cols > maxCols + sCol);
  bool_t leftEllipsis = (sCol > 0);
  int16_t digits;

    if(!regXposition && prefixWidth > 0) {
      Y_POS = Y_POSITION_OF_REGISTER_T_LINE - REGISTER_LINE_HEIGHT + 1 + totalLines * fontHeight;
    }
    if(!regXposition && prefixWidth > 0 && font == &standardFont) {
      Y_POS += (totalLines == 1 ? STANDARD_FONT_HEIGHT_ : REGISTER_LINE_HEIGHT - STANDARD_FONT_HEIGHT_);
    }

    // Measure the column widths
    int16_t baseWidth = (leftEllipsis ? stringWidth(STD_ELLIPSIS " ", font, true, true) : 0) + (rightEllipsis ? stringWidth(STD_ELLIPSIS, font, true, true) : 0);
  totalWidth = baseWidth + getComplexMatrixColumnWidths(matrix, prefixWidth, regXposition, font, colWidth, colWidth_r, colWidth_i, rPadWidth_r, rPadWidth_i, &digits, maxCols, angleMode, polarMode);
  // Shed digits: retry small font
  if(totalWidth > maxWidth || leftEllipsis || (showPage && font == &numericFont && digits < 34)) {
    if(font == &numericFont) {
      goto smallFont;
    }
    else if(exponentLimit > 99) {
      exponentLimit = 99;
      goto smallFont;
    }
    else {
      if(tmpDisplayFormat == DF_ALL || getSystemFlag(FLAG_M_ALL)) { //user format used unless ALL will be biased to make it fit
        displayFormat = getSystemFlag(FLAG_ENGOVR) ? DF_ENG : DF_SCI;   // ENGOVR decides the exponent form everywhere
        displayFormatDigits = 2;
      }
      clearSystemFlag(FLAG_MULTx);
      totalWidth = baseWidth + getComplexMatrixColumnWidths(matrix, prefixWidth + baseWidth, regXposition, font, colWidth, colWidth_r, colWidth_i, rPadWidth_r, rPadWidth_i, &digits, maxCols, angleMode, polarMode);   // the ellipsis takes its width off the budget
      if(totalWidth > maxWidth && maxCols > 1) {     // the width call fits without the ellipsis the caller adds, so an unfloored shrink strips every column
        maxCols--;
        goto smallFont;
      }
    }
  }
  if(forEditor) {
    if(matSelCol < sCol) {
      scrollColumn--;
      sCol--;
      goto smallFont;
    }
    else if(matSelCol >= sCol + maxCols) {
      scrollColumn++;
      sCol++;
      goto smallFont;
    }
  }
    for(int j = 0; j < maxCols; j++) {
      baseWidth += colWidth[j] + stringWidth(STD_SPACE_FIGURE, font, true, true);
    }
  baseWidth -= stringWidth(STD_SPACE_FIGURE, font, true, true);

    if(!regXposition && prefixWidth > 0) {
      X_POS = prefixWidth;
    }
    else if(!forEditor) {
      X_POS = SCREEN_WIDTH - ((colVector ? stringWidth("[]" STD_SUP_BOLD_T, font, true, true) : stringWidth("[]", font, true, true)) + baseWidth) - (font == &standardFont ? 0 : 1);
    }

  // Blank the page area
  if(forEditor) {
    clearRegisterLine(REGISTER_X, true, true);
    clearRegisterLine(REGISTER_Y, true, true);
      if(rows >= (font == &standardFont ? 3 : 2)) {
        clearRegisterLine(REGISTER_Z, true, true);
      }
      if(rows >= (font == &standardFont ? 4 : 3)) {
        clearRegisterLine(REGISTER_T, true, true);
      }
  }
  else if(!regXposition && prefixWidth > 0) {
    clearRegisterLine(REGISTER_T, true, true);
      if(rows >= 2) {
        clearRegisterLine(REGISTER_Z, true, true);
      }
      if(rows >= (font == &standardFont ? 4 : 3)) {
        clearRegisterLine(REGISTER_Y, true, true);
      }
      if(rows == 4 && font != &standardFont) {
        clearRegisterLine(REGISTER_X, true, true);
      }
  }
  else {   // the stack position: a partial refresh leaves stale register text in the band the matrix covers
    lcd_fill_rect(X_POS, Y_POS - (totalLines - 1) * fontHeight, (colVector ? stringWidth("[]" STD_SUP_BOLD_T, font, true, true) : stringWidth("[]", font, true, true)) + baseWidth, (totalLines - 1) * fontHeight + (font == &numericFont ? NUMERIC_FONT_HEIGHT : STANDARD_FONT_HEIGHT), LCD_SET_VALUE);
  }

  // Draw the rows
  // The rolled out page labels each row after the first in the prefix column
  const int16_t outerBracket = groupCols ? stringWidth("[", font, true, true) : 0;
  if(groupCols) {
    char rowLabel[4] = {'r', '0', ':', 0};
    for(int group = 1; group < matrix->header.matrixRows; group++) {
      rowLabel[1] = '1' + group;
      showString(rowLabel, &standardFont, 1, Y_POS - (totalLines - 1 - group * (groupCols + 1)) * fontHeight, vmNormal, true, false);
    }
  }

  for(int i = 0; i < maxRows; i++) {
    const int lineFromBottom = groupCols ? (totalLines - 1 - (i + i / groupCols)) : (maxRows - 1 - i);
    int16_t colX = stringWidth("[", font, true, true);
    showString(groupCols ? "[" : (maxRows == 1) ? "[" : (i == 0) ? STD_MAT_TL : (i + 1 == maxRows) ? STD_MAT_BL : STD_MAT_ML, font, X_POS + 1 + outerBracket, Y_POS - lineFromBottom * fontHeight, vmNormal, true, false);
    if(groupCols && i == 0) {                          // the matrix bracket opens outside the row bracket of the first element
      showString("[", font, X_POS + 1, Y_POS - lineFromBottom * fontHeight, vmNormal, true, false);
    }
    if(leftEllipsis) {
      showString(STD_ELLIPSIS " ", font, X_POS + outerBracket + stringWidth("[", font, true, true), Y_POS - lineFromBottom * fontHeight, vmNormal, true, false);
      colX += stringWidth(STD_ELLIPSIS " ", font, true, true);
    }
    for(int j = 0; j < maxCols + (rightEllipsis ? 1 : 0); j++) {
      real34_t re, im;
      if(polarMode){
        real_t x, y;
        real34ToReal(VARIABLE_REAL34_DATA(&matrix->matrixElements[(i+sRow)*cols+j+sCol]), &x);
        real34ToReal(VARIABLE_IMAG34_DATA(&matrix->matrixElements[(i+sRow)*cols+j+sCol]), &y);
        realRectangularToPolar(&x, &y, &x, &y, &ctxtReal39);
        convertAngleFromTo(&y, amRadian, angleMode, &ctxtReal39);
        realToReal34(&x, &re);
        realToReal34(&y, &im);
      }
      else { // rectangular mode
        real34Copy(VARIABLE_REAL34_DATA(&matrix->matrixElements[(i+sRow)*cols+j+sCol]), &re);
        real34Copy(VARIABLE_IMAG34_DATA(&matrix->matrixElements[(i+sRow)*cols+j+sCol]), &im);
      }

      if(((i == maxRows - 1) && (rows > maxRows + sRow)) || ((j == maxCols) && rightEllipsis) || ((i == 0) && (sRow > 0))) {
        strcpy(tmpString, STD_ELLIPSIS);
        vm = vmNormal;
      }
      else {
        tmpString[0] = 0;
        if(displayFormat == DF_SF && verticalVector) {
          sigFitReal34(&re, amNone, tmpString, font, sfPartWidth, FRONTSPACE);   // same budget as measured
        }
        else {
          real34ToDisplayString(&re, amNone, tmpString, font, colWidth_r[j], displayFormat == DF_ALL ? digits : (fixPage ? 33 : (showPage ? 34 : 15)), LIMITEXP, FRONTSPACE, LIMITIRFRAC);
        }
        if(forEditor && matSelRow == (i + sRow) && matSelCol == (j + sCol)) {
          lcd_fill_rect(X_POS + outerBracket + colX, Y_POS - lineFromBottom * fontHeight, colWidth[j], font == &numericFont ? 32 : 20, LCD_EMPTY_VALUE);
          vm = vmReverse;
        }
        else {
          vm = vmNormal;
        }
      }
      width = stringWidth(tmpString, font, true, true) + 1;
      showString(tmpString, font, X_POS + outerBracket + colX + (((j == maxCols) && rightEllipsis) ? stringWidth(STD_SPACE_FIGURE, font, true, true) - width : (colWidth_r[j] - width) - rPadWidth_r[i * MATRIX_MAX_COLUMNS + j]), Y_POS - lineFromBottom * fontHeight, vm, true, false);
      if(strcmp(tmpString, STD_ELLIPSIS) != 0) {
        bool_t neg = real34IsNegative(&im);
        int16_t cpxUnitWidth;

        if(polarMode) {
          strcpy(tmpString, STD_SPACE_4_PER_EM STD_MEASURED_ANGLE STD_SPACE_4_PER_EM);
        }
        else { // rectangular mode
          strcpy(tmpString, "+");
          strcat(tmpString, COMPLEX_UNIT);
          strcat(tmpString, PRODUCT_SIGN);
        }
        cpxUnitWidth = width = stringWidth(tmpString, font, true, true);
        if(!polarMode) {
          if(neg) {
            tmpString[0] = '-';
            real34SetPositiveSign(&im);
          }
        }
        showString(tmpString, font, X_POS + outerBracket + colX + colWidth_r[j] + (width - stringWidth(tmpString, font, true, true)), Y_POS - lineFromBottom * fontHeight, vm, true, false);

        if(displayFormat == DF_SF && verticalVector) {
          sigFitReal34(&im, polarMode ? angleMode : amNone, tmpString, font, sfPartWidth, !FRONTSPACE);   // same budget as measured
        }
        else {
          real34ToDisplayString(&im, polarMode ? angleMode : amNone, tmpString, font, colWidth_i[j], displayFormat == DF_ALL ? digits : (fixPage ? 33 : (showPage ? 34 : 15)), LIMITEXP, !FRONTSPACE, LIMITIRFRAC);
        }
        width = stringWidth(tmpString, font, true, true) + 1;
        showString(tmpString, font, X_POS + outerBracket + colX + colWidth_r[j] + cpxUnitWidth + (((j == maxCols - 1) && rightEllipsis) ? 0 : (colWidth_i[j] - width) - rPadWidth_i[i * MATRIX_MAX_COLUMNS + j]), Y_POS - lineFromBottom * fontHeight, vm, true, false);
      }
      colX += colWidth[j] + stringWidth(STD_SPACE_FIGURE, font, true, true);
    }
    showString(groupCols ? "]" : (maxRows == 1) ? "]" : (i == 0) ? STD_MAT_TR : (i + 1 == maxRows) ? STD_MAT_BR : STD_MAT_MR, font, X_POS + outerBracket + stringWidth("[", font, true, true) + baseWidth - 1, Y_POS - lineFromBottom * fontHeight, vmNormal, true, false);
    if(groupCols && i + 1 == maxRows) {                // the matrix bracket closes outside the row bracket of the last element
      showString("]", font, X_POS + outerBracket + stringWidth("[]", font, true, true) + baseWidth - 1, Y_POS - lineFromBottom * fontHeight, vmNormal, true, false);
    }
    if(colVector == true) {
      showString(STD_SUP_BOLD_T, font, X_POS + stringWidth("[]", font, true, true) + baseWidth, Y_POS - lineFromBottom * fontHeight, vmNormal, true, false);
    }
  }

  if(transposedVector) {                               // the ^T on the closing bracket
    showString(STD_SUP_BOLD_T, font, X_POS + stringWidth("[", font, true, true) + stringWidth(STD_MAT_BR, font, true, true) + baseWidth - 1, Y_POS, vmNormal, true, false);
  }

  // Restore the format globals
  displayFormat = tmpDisplayFormat;
  displayFormatDigits = tmpDisplayFormatDigits;
  exponentLimit = tmpExponentLimit;
    if(tmpMultX) {
      setSystemFlag(FLAG_MULTx);
    }
}

int16_t getComplexMatrixColumnWidths(const complex34Matrix_t *matrix, int16_t prefixWidth, bool_t regXposition, const font_t *font, int16_t *colWidth, int16_t *colWidth_r, int16_t *colWidth_i, int16_t *rPadWidth_r, int16_t *rPadWidth_i, int16_t *digits, uint16_t maxCols, angularMode_t angleMode, bool_t polarMode) {
  // The measured page shape
  char tmpString[200];
  const bool_t showPage = MX_SHOW_PAGE(prefixWidth, regXposition);   // the SHOW page, never the stack line
  const bool_t fixPage = showPage && displayFormat == DF_FIX;
  const bool_t verticalVector = showsVerticalVector(&matrix->header, prefixWidth, regXposition, false);   // one shared column under SHOW; a complex vector has no integer form to keep
  const bool_t colVector = !verticalVector && matrix->header.matrixColumns == 1 && matrix->header.matrixRows > 1;
  const int rows = verticalVector ? matrix->header.matrixRows * matrix->header.matrixColumns : colVector ? 1 : matrix->header.matrixRows;
  const int actualCols = verticalVector ? 1 : colVector ? matrix->header.matrixRows : matrix->header.matrixColumns;
  const int cols = (actualCols > maxCols) ? maxCols : actualCols;   // clamp for safety
  const int rowLimit = showPage ? MATRIX_MAX_ROWS_ON_SHOW : MATRIX_MAX_ROWS;   // SHOW takes more rows
  const int maxRows = rows > rowLimit ? rowLimit : rows;
  const bool_t forEditor = matrix == &openMatrixMIMPointer.complexMatrix;
  const uint16_t sRow = forEditor ? scrollRow : 0;
  const uint16_t sCol = forEditor ? scrollColumn : 0;
  // The width budget
  // widened by one figure space
  const int16_t maxWidth = verticalVector ? showsVerticalVectorMaxWidth(&matrix->header, font, prefixWidth) + stringWidth(STD_SPACE_FIGURE, font, true, true) : MATRIX_LINE_WIDTH - prefixWidth;
  int16_t totalWidth = 0;
  int16_t maxRightWidth_r[MATRIX_MAX_COLUMNS] = {};
  int16_t maxLeftWidth_r[MATRIX_MAX_COLUMNS] = {};
  int16_t maxRightWidth_i[MATRIX_MAX_COLUMNS] = {};
  int16_t maxLeftWidth_i[MATRIX_MAX_COLUMNS] = {};
  const int16_t exponentOutOfRange = 0x4000;
  bool_t sfShed = false;                               // a part shed digits

  // The complex unit width
  uint16_t cpxUnitWidth;
  if(polarMode) {
    strcpy(tmpString, STD_SPACE_4_PER_EM STD_MEASURED_ANGLE STD_SPACE_4_PER_EM);
  }
  else { // rectangular mode
    strcpy(tmpString, "+");
    strcat(tmpString, COMPLEX_UNIT);
    strcat(tmpString, PRODUCT_SIGN);
  }
  cpxUnitWidth = stringWidth(tmpString, font, true, true);
  // half the line per part
  const int16_t sfPartWidth = (maxWidth - cpxUnitWidth - stringWidth(STD_SPACE_FIGURE, font, true, true) - 2) / 2;

  // The SHOW starting count
  int startDigitCountDown = showPage ? 34 :   // start at full precision
                            max(min(displayFormatDigits*(displayFormat == DF_ALL ? 2 : 1), max((50/cols-2), 0) ), 10);
  if(showPage && displayFormat == DF_SF) {
    startDigitCountDown = 33;                          // SIG n shows n+1 digits
  }
  else if(showPage && displayFormat == DF_FIX) {   // cap FIX at 34 total digits
    int16_t maxE = 0;
    for(int i = 0; i < maxRows; i++) {
      for(int j = 0; j < maxCols; j++) {
        const complex34_t *c34 = &matrix->matrixElements[(i+sRow)*actualCols+j+sCol];
        const int16_t eRe = real34IsZero(VARIABLE_REAL34_DATA(c34)) ? 0 : real34GetExponent(VARIABLE_REAL34_DATA(c34)) + real34Digits(VARIABLE_REAL34_DATA(c34)) - 1;
        const int16_t eIm = real34IsZero(VARIABLE_IMAG34_DATA(c34)) ? 0 : real34GetExponent(VARIABLE_IMAG34_DATA(c34)) + real34Digits(VARIABLE_IMAG34_DATA(c34)) - 1;
        if(eRe > maxE && eRe <= 32) {   // plain-renderable parts only; e33 up takes the sci form through the 33 window
          maxE = eRe;
        }
        if(eIm > maxE && eIm <= 32) {
          maxE = eIm;
        }
      }
    }
    startDigitCountDown = max(min(33 - maxE, 99), 1);
  }
  for(int k = startDigitCountDown; k >= 1; k--) {                                    //HERE IS THE TIME WASTER - CYCLING THROUGH 15 PRECISIONS !! REDUCE SIGNIFICANTLY from 15 to settingx2 or setting
      if(displayFormat == DF_ALL) {
        *digits = k;
      }
      else if(showPage) {   // the shared maximised count
        displayFormatDigits = (displayFormat == DF_SF && verticalVector) ? 34 : k;
        *digits = k;
      }
    for(int i = 0; i < maxRows; i++) {
      for(int j = 0; j < maxCols; j++) {
        complex34_t c34Val;
        complex34Copy(&matrix->matrixElements[(i+sRow)*actualCols+j+sCol], &c34Val);
        if(polarMode) {
          real_t x, y;
          real34ToReal(VARIABLE_REAL34_DATA(&c34Val), &x);
          real34ToReal(VARIABLE_IMAG34_DATA(&c34Val), &y);
          realRectangularToPolar(&x, &y, &x, &y, &ctxtReal39);
          convertAngleFromTo(&y, amRadian, angleMode, &ctxtReal39);
          realToReal34(&x, VARIABLE_REAL34_DATA(&c34Val));
          realToReal34(&y, VARIABLE_IMAG34_DATA(&c34Val));
        }

        // Measure the real part
        rPadWidth_r[i * MATRIX_MAX_COLUMNS + j] = 0;
        real34SetPositiveSign(VARIABLE_REAL34_DATA(&c34Val));
        bool_t c34sign = real34IsNegative(&matrix->matrixElements[(i+sRow)*actualCols+j+sCol]);
        if(displayFormat == DF_SF && verticalVector) {
          if(sigFitReal34(VARIABLE_REAL34_DATA(&c34Val), amNone, tmpString, font, sfPartWidth, FRONTSPACE)) {   // per-part SIG self-fit
            sfShed = true;
          }
        }
        else {
          // no internal shrink under SHOW: it hides shed digits and flips SIG to sci; the k countdown does the fitting
          real34ToDisplayString(VARIABLE_REAL34_DATA(&c34Val), amNone, tmpString, font, showPage ? UNSHRUNK_WIDTH : maxWidth, displayFormat == DF_ALL ? k : (fixPage ? 33 : (showPage ? 34 : 15)), LIMITEXP, FRONTSPACE, LIMITIRFRAC);
        }
        int16_t width = stringWidth(tmpString, font, true, true) + 1;
        if((strstr(tmpString, ".") || strstr(tmpString, ",")) && !(verticalVector && displayFormat == DF_SF)) {   // vector SIG skips the line-up
          for(char *xStr = tmpString; *xStr != 0; xStr++) {
            if(((displayFormat != DF_ENG && (displayFormat != DF_ALL || !getSystemFlag(FLAG_ENGOVR))) && (*xStr == '.' || *xStr == ',')) ||
               ((displayFormat == DF_ENG || (displayFormat == DF_ALL && getSystemFlag(FLAG_ENGOVR))) && xStr[0] == (char)0x80 && (xStr[1] == (char)0x87 || xStr[1] == (char)0xd7))) {  //STD_CROSS
              rPadWidth_r[i * MATRIX_MAX_COLUMNS + j] = stringWidth(xStr, font, true, true) + 1;
                if(maxRightWidth_r[j] < rPadWidth_r[i * MATRIX_MAX_COLUMNS + j]) {
                  maxRightWidth_r[j] = rPadWidth_r[i * MATRIX_MAX_COLUMNS + j];
                }
              break;
            }
          }
            if(maxLeftWidth_r[j] < (width - rPadWidth_r[i * MATRIX_MAX_COLUMNS + j])) {
              maxLeftWidth_r[j] = (width - rPadWidth_r[i * MATRIX_MAX_COLUMNS + j]);
            }
        }
        else {
          if(c34sign && (strstr(tmpString, "/") || strstr(tmpString, STD_ALMOST_EQUAL))) {
            width += stringWidth("-", font, true, true);
          }
          rPadWidth_r[i * MATRIX_MAX_COLUMNS + j] = width | exponentOutOfRange;
        }

        // Measure the imaginary part
        rPadWidth_i[i * MATRIX_MAX_COLUMNS + j] = 0;
        c34sign = false;
        if(!polarMode) {
          c34sign = real34IsNegative(&matrix->matrixElements[(i+sRow)*actualCols+j+sCol]);
          real34SetPositiveSign(VARIABLE_IMAG34_DATA(&c34Val));
        }
        if(displayFormat == DF_SF && verticalVector) {
          if(sigFitReal34(VARIABLE_IMAG34_DATA(&c34Val), polarMode ? angleMode : amNone, tmpString, font, sfPartWidth, !FRONTSPACE)) {
            sfShed = true;
          }
        }
        else {
          // no internal shrink under SHOW: it hides shed digits and flips SIG to sci; the k countdown does the fitting
          real34ToDisplayString(VARIABLE_IMAG34_DATA(&c34Val), polarMode ? angleMode : amNone, tmpString, font, showPage ? UNSHRUNK_WIDTH : maxWidth, displayFormat == DF_ALL ? k : (fixPage ? 33 : (showPage ? 34 : 15)), LIMITEXP, !FRONTSPACE, LIMITIRFRAC);
        }
        width = stringWidth(tmpString, font, true, true) + 1;
        if((strstr(tmpString, ".") || strstr(tmpString, ",")) && !(verticalVector && displayFormat == DF_SF)) {   // vector SIG skips the line-up
          for(char *xStr = tmpString; *xStr != 0; xStr++) {
            if(((displayFormat != DF_ENG && (displayFormat != DF_ALL || !getSystemFlag(FLAG_ENGOVR))) && (*xStr == '.' || *xStr == ',')) ||
               ((displayFormat == DF_ENG || (displayFormat == DF_ALL && getSystemFlag(FLAG_ENGOVR))) && xStr[0] == (char)0x80 && (xStr[1] == (char)0x87 || xStr[1] == (char)0xd7))) {  //STD_CROSS
              rPadWidth_i[i * MATRIX_MAX_COLUMNS + j] = stringWidth(xStr, font, true, true) + 1;
                if(maxRightWidth_i[j] < rPadWidth_i[i * MATRIX_MAX_COLUMNS + j]) {
                  maxRightWidth_i[j] = rPadWidth_i[i * MATRIX_MAX_COLUMNS + j];
                }
              break;
            }
          }
            if(maxLeftWidth_i[j] < (width - rPadWidth_i[i * MATRIX_MAX_COLUMNS + j])) {
              maxLeftWidth_i[j] = (width - rPadWidth_i[i * MATRIX_MAX_COLUMNS + j]);
            }
        }
        else {
          if(c34sign && strstr(tmpString, "/")) {
            width += stringWidth("-", font, true, true);
          }
          rPadWidth_i[i * MATRIX_MAX_COLUMNS + j] = width | exponentOutOfRange;
        }
      }
    }
    // Exponent rows widen the column
    for(int i = 0; i < maxRows; i++) {
      for(int j = 0; j < maxCols; j++) {
        if(rPadWidth_r[i * MATRIX_MAX_COLUMNS + j] & exponentOutOfRange) {
          if((maxLeftWidth_r[j] + maxRightWidth_r[j]) < (rPadWidth_r[i * MATRIX_MAX_COLUMNS + j] & (~exponentOutOfRange))) {
            maxLeftWidth_r[j] = (rPadWidth_r[i * MATRIX_MAX_COLUMNS + j] & (~exponentOutOfRange)) - maxRightWidth_r[j];
          }
        }
        if(rPadWidth_i[i * MATRIX_MAX_COLUMNS + j] & exponentOutOfRange) {
          if((maxLeftWidth_i[j] + maxRightWidth_i[j]) < (rPadWidth_i[i * MATRIX_MAX_COLUMNS + j] & (~exponentOutOfRange))) {
            maxLeftWidth_i[j] = (rPadWidth_i[i * MATRIX_MAX_COLUMNS + j] & (~exponentOutOfRange)) - maxRightWidth_i[j];
          }
        }
      }
    }
    for(int i = 0; i < maxRows; i++) {
      for(int j = 0; j < maxCols; j++) {
        if(rPadWidth_r[i * MATRIX_MAX_COLUMNS + j] & exponentOutOfRange) {
          rPadWidth_r[i * MATRIX_MAX_COLUMNS + j] = 0;
        }
        else {
          rPadWidth_r[i * MATRIX_MAX_COLUMNS + j] -= maxRightWidth_r[j];
          rPadWidth_r[i * MATRIX_MAX_COLUMNS + j] *= -1;
        }
        if(rPadWidth_i[i * MATRIX_MAX_COLUMNS + j] & exponentOutOfRange) {
          rPadWidth_i[i * MATRIX_MAX_COLUMNS + j] = 0;
        }
        else {
          rPadWidth_i[i * MATRIX_MAX_COLUMNS + j] -= maxRightWidth_i[j];
          rPadWidth_i[i * MATRIX_MAX_COLUMNS + j] *= -1;
        }
      }
    }
    // Sum and test the fit
    for(int j = 0; j < maxCols; j++) {
      colWidth_r[j] = maxLeftWidth_r[j] + maxRightWidth_r[j];
      colWidth_i[j] = maxLeftWidth_i[j] + maxRightWidth_i[j];
      colWidth[j] = colWidth_r[j] + (colWidth_i[j] > 0 ? (cpxUnitWidth + colWidth_i[j]) : 0);
      totalWidth += colWidth[j] + stringWidth(STD_SPACE_FIGURE, font, true, true) * 2;
    }
    totalWidth -= stringWidth(STD_SPACE_FIGURE, font, true, true);
    if(displayFormat != DF_ALL && !showPage) {
      break;
    }
    else if(totalWidth <= maxWidth) {
      *digits = (showPage && k == startDigitCountDown && !sfShed) ? 34 : k;   // nothing shed: the font retry rests
      break;
    }
    else if(k > 1) {
      totalWidth = 0;
      for(int j = 0; j < maxCols; j++) {
        maxRightWidth_r[j] = 0;
        maxLeftWidth_r[j] = 0;
        maxRightWidth_i[j] = 0;
        maxLeftWidth_i[j] = 0;
      }
    }
  }
  return totalWidth;
}

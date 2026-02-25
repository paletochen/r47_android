// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ui/matrixEditor.c
 ***********************************************/

#include "c47.h"

#define addFlag true

#if !defined(TESTSUITE_BUILD)
  any34Matrix_t         openMatrixMIMPointer;
  bool_t                matEditMode;
  uint16_t              scrollRow;
  uint16_t              scrollColumn;
  uint16_t              tmpRow;
  uint16_t              matrixIndex = INVALID_VARIABLE;

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
#endif // !defined(TESTSUITE_BUILD)

void fnEditMatrix(uint16_t regist) {
  #if !defined(TESTSUITE_BUILD)
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
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
    sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(reg));
    moreInfoOnError("In function fnEditMatrix:", errorMessage, "is not a matrix.", "");
      #endif // PC_BUILD
    }
  #endif // !TESTSUITE_BUILD
}


void fnOldMatrix(uint16_t unusedParamButMandatory) {
  #if !defined(TESTSUITE_BUILD)
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
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
    sprintf(errorMessage, "works in MIM only");
    moreInfoOnError("In function fnOldMatrix:", errorMessage, NULL, NULL);
      #endif // PC_BUILD
    }
  #endif // !TESTSUITE_BUILD
}


void fnGoToElement(uint16_t unusedParamButMandatory) {
  #if !defined(TESTSUITE_BUILD)
  if(calcMode == CM_MIM) {
    mimEnter(false);
    runFunction(ITM_M_GOTO_ROW);
  }
  else {
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function fnGoToElement:", errorMessage, NULL, NULL);
      #endif // PC_BUILD
    }
  #endif // !TESTSUITE_BUILD
}


void fnGoToRow(uint16_t row) {
  #if !defined(TESTSUITE_BUILD)
  if(calcMode == CM_MIM) {
    tmpRow = row;
  }
  else {
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function fnGoToRow:", errorMessage, NULL, NULL);
      #endif // PC_BUILD
    }
  #endif // !TESTSUITE_BUILD
}


void fnGoToColumn(uint16_t col) {
  #if !defined(TESTSUITE_BUILD)
  if(calcMode == CM_MIM) {
    if(tmpRow == 0 || tmpRow > openMatrixMIMPointer.header.matrixRows || col == 0 || col > openMatrixMIMPointer.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
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
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function fnGoToColumn:", errorMessage, NULL, NULL);
      #endif // PC_BUILD
    }
  #endif // !TESTSUITE_BUILD
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
  #if !defined(TESTSUITE_BUILD)
  callByIndexedMatrix((mode == DEC_FLAG) ? decIReal : incIReal, (mode == DEC_FLAG) ? decIComplex : incIComplex);
  #endif // !TESTSUITE_BUILD
}


void fnIncDecJ(uint16_t mode) {
  #if !defined(TESTSUITE_BUILD)
  callByIndexedMatrix((mode == DEC_FLAG) ? decJReal : incJReal, (mode == DEC_FLAG) ? decJComplex : incJComplex);
  #endif // !TESTSUITE_BUILD
}



void _fnInsRow(bool_t add) {
  #if !defined(TESTSUITE_BUILD)
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
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function _fnInsRow:", errorMessage, NULL, NULL);
      #endif // PC_BUILD
    }
  #endif // !TESTSUITE_BUILD
}


void fnInsRow(uint16_t unusedParamButMandatory) {
  _fnInsRow(!addFlag);
}
void fnAddRow(uint16_t unusedParamButMandatory) {
  _fnInsRow(addFlag);
}


void _fnInsCol(bool_t add) {
  #if !defined(TESTSUITE_BUILD)
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
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function _fnInsCol:", errorMessage, NULL, NULL);
      #endif // PC_BUILD
    }
  #endif // !TESTSUITE_BUILD
}

void fnInsCol(uint16_t unusedParamButMandatory) {
  _fnInsCol(!addFlag);
}
void fnAddCol(uint16_t unusedParamButMandatory) {
  _fnInsCol(addFlag);
}


void fnDelRow(uint16_t unusedParamButMandatory) {
  #if !defined(TESTSUITE_BUILD)
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
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function fnDelRow:", errorMessage, NULL, NULL);
      #endif // PC_BUILD
    }
  #endif // !TESTSUITE_BUILD
}


void fnDelCol(uint16_t unusedParamButMandatory) {
  #if !defined(TESTSUITE_BUILD)
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
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
      sprintf(errorMessage, "works in MIM only");
      moreInfoOnError("In function fnDelCol:", errorMessage, NULL, NULL);
      #endif // PC_BUILD
    }
  #endif // !TESTSUITE_BUILD
}


#if !defined(TESTSUITE_BUILD)
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

//Row of Matrix
int16_t getIRegisterAsInt(bool_t asArrayPointer) {
  return getRegisterAsInt(asArrayPointer, REGISTER_I);
}

//Col of Matrix
int16_t getJRegisterAsInt(bool_t asArrayPointer) {
  return getRegisterAsInt(asArrayPointer, REGISTER_J);

}

//Row of Matrix
void setIRegisterAsInt(bool_t asArrayPointer, int16_t toStore) {
  setRegisterAsInt(asArrayPointer, toStore, REGISTER_I);
}

//ColOfMatrix
void setJRegisterAsInt(bool_t asArrayPointer, int16_t toStore) {
  setRegisterAsInt(asArrayPointer, toStore, REGISTER_J);
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
      insRowRealMatrix(&openMatrixMIMPointer.realMatrix, rows, !addFlag);
      convertReal34MatrixToReal34MatrixRegister(&openMatrixMIMPointer.realMatrix, matrixIndex);
    }
    else {
      insRowComplexMatrix(&openMatrixMIMPointer.complexMatrix, rows, !addFlag);
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
      showRealMatrix(&openMatrixMIMPointer.realMatrix, 0, toDisplayVectorMatrix);
    }
    else {
      showComplexMatrix(&openMatrixMIMPointer.complexMatrix, 0, currentAngularMode, getSystemFlag(FLAG_POLAR));
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
  int cols = openMatrixMIMPointer.header.matrixColumns;
  int16_t row = getIRegisterAsInt(true);
  int16_t col = getJRegisterAsInt(true);

  if(aimBuffer[0] != 0) {
    if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
      real34_t *real34Ptr = &openMatrixMIMPointer.realMatrix.matrixElements[row * cols + col];

      switch(nimNumberPart) {
        case NP_FRACTION_DENOMINATOR:
        case NP_HP32SII_DENOMINATOR:
          closeNimWithFraction(real34Ptr);
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
          stringToReal34(aimBuffer, real34Ptr);
      }
    }
    else {
      complex34_t *complex34Ptr = &openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col];

      switch(nimNumberPart) {
        case NP_FRACTION_DENOMINATOR:
        case NP_HP32SII_DENOMINATOR:
          closeNimWithFraction(VARIABLE_REAL34_DATA(complex34Ptr));
          real34Zero(VARIABLE_IMAG34_DATA(complex34Ptr));
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
          real34Zero(VARIABLE_IMAG34_DATA(complex34Ptr));
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
    }
    else {
      convertComplex34MatrixToComplex34MatrixRegister(&openMatrixMIMPointer.complexMatrix, matrixIndex);
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
          real34Zero(&openMatrixMIMPointer.realMatrix.matrixElements[row * cols + col]);
        }
        else {
          real34Zero(VARIABLE_REAL34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col]));
          real34Zero(VARIABLE_IMAG34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col]));
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
              realCopy(const_piOn2, &theta);
              convertAngleFromTo(&theta, amRadian, currentAngularMode, &ctxtReal39);
              real34Copy(const34_1, VARIABLE_REAL34_DATA(elm));
              real34Copy(&theta, VARIABLE_IMAG34_DATA(elm));
            }
            else {
              real34Zero(VARIABLE_REAL34_DATA(elm));
              real34Copy(const34_1, VARIABLE_IMAG34_DATA(elm));
            }
          }
          return;
        }
      break;
      }

      case ITM_CONSTpi: {
      if(aimBuffer[0] == 0) {
        if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
          realToReal34(const_pi, &openMatrixMIMPointer.realMatrix.matrixElements[row * cols + col]);
        }
        else {
          realToReal34(const_pi, VARIABLE_REAL34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col]));
          real34Zero(VARIABLE_IMAG34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[row * cols + col]));
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

  if(isComplex) {
    real34Copy(VARIABLE_REAL34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]), &re1);
    real34Copy(VARIABLE_IMAG34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]), &im1);
  }
  else {
    real34Copy(&openMatrixMIMPointer.realMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j], &re1);
    real34Zero(&im1);
  }

  mimEnter(true);
  clearSystemFlag(FLAG_ASLIFT);
  lastErrorCode = ERROR_NONE;

  if(isComplex) {
    real34Copy(VARIABLE_REAL34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]), &re);
    real34Copy(VARIABLE_IMAG34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]), &im);
  }
  else {
    real34Copy(&openMatrixMIMPointer.realMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j], &re);
    real34Zero(&im);
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

  reallyRunFunction(func, param);

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
      real34Zero(                                  VARIABLE_IMAG34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]));
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

#define NUMERIC_FONT_HEIGHT_ (NUMERIC_FONT_HEIGHT - 4)        // reduce font spacing to easily bind the matrix lines without any complicated pixel manipulation
#define STANDARD_FONT_HEIGHT_ (STANDARD_FONT_HEIGHT - 2)      // reduce font spacing to easily bind the matrix lines without any complicated pixel manipulation

void showRealMatrix(const real34Matrix_t *matrix, int16_t prefixWidth, bool_t toDisplay) {
  int rows = matrix->header.matrixRows;
  int cols = matrix->header.matrixColumns;
  int16_t Y_POS = Y_POSITION_OF_REGISTER_X_LINE;
  int16_t X_POS = 0;
  int16_t totalWidth = 0, width = 0;
  const font_t *font;
  int16_t fontHeight = NUMERIC_FONT_HEIGHT_;
  int16_t maxWidth = MATRIX_LINE_WIDTH - prefixWidth;
  int16_t colWidth[MATRIX_MAX_COLUMNS] = {}, rPadWidth[MATRIX_MAX_ROWS * MATRIX_MAX_COLUMNS] = {};
  bool_t allElementsInColAreIntegers[MATRIX_MAX_COLUMNS] = {};
  const bool_t forEditor = matrix == &openMatrixMIMPointer.realMatrix;
  const uint16_t sRow = forEditor ? scrollRow : 0;
  uint16_t sCol = forEditor ? scrollColumn : 0;
  const uint16_t tmpDisplayFormat = displayFormat;
  const uint8_t tmpDisplayFormatDigits = displayFormatDigits;

  Y_POS = Y_POSITION_OF_REGISTER_X_LINE - NUMERIC_FONT_HEIGHT_;

  bool_t colVector = false;
  if(cols == 1 && rows > 1) {
    colVector = true;
    cols = rows;
    rows = 1;
  }

  toDisplay |= forEditor || rows > 1;
  strcpy(errorMessage,"[");

  uint16_t maxCols = cols > MATRIX_MAX_COLUMNS ? MATRIX_MAX_COLUMNS : cols;
  const uint16_t maxRows = rows > MATRIX_MAX_ROWS ? MATRIX_MAX_ROWS : rows;
    if(maxCols + sCol >= cols) {
      maxCols = cols - sCol;
    }

  int16_t matSelRow = colVector ? getJRegisterAsInt(true) : getIRegisterAsInt(true);
  int16_t matSelCol = colVector ? getIRegisterAsInt(true) : getJRegisterAsInt(true);

  videoMode_t vm = vmNormal;

  font = &numericFont;
  if(rows >= (forEditor ? 4 : 5)){
smallFont:
    font = &standardFont;
    fontHeight = STANDARD_FONT_HEIGHT_;
    Y_POS = Y_POSITION_OF_REGISTER_X_LINE - STANDARD_FONT_HEIGHT_;
    //maxWidth = MATRIX_LINE_WIDTH_SMALL * 4 - 20;
  }

  if(!forEditor) {
    Y_POS += REGISTER_LINE_HEIGHT;
  }
  const bool_t rightEllipsis = (cols > maxCols) && (cols > maxCols + sCol);
  const bool_t leftEllipsis = (sCol > 0);
  int16_t digits;

  if(prefixWidth > 0) {
    Y_POS = Y_POSITION_OF_REGISTER_T_LINE - REGISTER_LINE_HEIGHT + 1 + maxRows * fontHeight;
  }
  if(prefixWidth > 0 && font == &standardFont) {
    Y_POS += (maxRows == 1 ? STANDARD_FONT_HEIGHT_ : REGISTER_LINE_HEIGHT - STANDARD_FONT_HEIGHT_);
  }

  for(int j = 0; j < maxCols; j++) {
    allElementsInColAreIntegers[j]=true;
    for(int i = 0; i < maxRows; i++) {
      if(!real34IsAnInteger(&matrix->matrixElements[i*cols+j])) {
        allElementsInColAreIntegers[j]=false;
        break;
      }
    }
  }

  int16_t baseWidth = (leftEllipsis ? stringWidth(STD_ELLIPSIS " ", font, true, true) : 0) + (rightEllipsis ? stringWidth(" " STD_ELLIPSIS, font, true, true) : 0);
  int16_t mtxWidth = getRealMatrixColumnWidths(matrix, prefixWidth, font, colWidth, rPadWidth, &digits, maxCols, allElementsInColAreIntegers);
  bool_t noFix = (mtxWidth < 0);
  mtxWidth = abs(mtxWidth);
  totalWidth = baseWidth + mtxWidth;

  if(displayFormat == DF_ALL && noFix) {
    displayFormat = getSystemFlag(FLAG_ENGOVR) ? DF_ENG : DF_SCI;
    displayFormatDigits = digits;
  }
  if(totalWidth > maxWidth || leftEllipsis) {
    if(font == &numericFont) {
      displayFormat = tmpDisplayFormat;
      displayFormatDigits = tmpDisplayFormatDigits;
      goto smallFont;
    }
    else {
      displayFormat = DF_SCI;
      displayFormatDigits = 3;
      mtxWidth = getRealMatrixColumnWidths(matrix, prefixWidth, font, colWidth, rPadWidth, &digits, maxCols, allElementsInColAreIntegers);
      noFix = (mtxWidth < 0);
      mtxWidth = abs(mtxWidth);
      totalWidth = baseWidth + mtxWidth;
      if(totalWidth > maxWidth) {
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

  if(prefixWidth > 0) {
    X_POS = prefixWidth;
  }
  else if(!forEditor) {
    X_POS = SCREEN_WIDTH - ((colVector ? stringWidth("[]" STD_SUP_BOLD_T, font, true, true) : stringWidth("[]", font, true, true)) + baseWidth) - (font == &standardFont ? 0 : 1);
  }

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
  else if(prefixWidth > 0) {
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
}
  const uint16_t displayFormat1 = displayFormat;
  const uint8_t displayFormatDigits1 = displayFormatDigits;

int16_t colX = 0;

  for(int i = 0; i < maxRows; i++) {
    if(toDisplay) {
      colX = stringWidth("[", font, true, true);
      showString((maxRows == 1) ? "[" : (i == 0) ? STD_MAT_TL : (i + 1 == maxRows) ? STD_MAT_BL : STD_MAT_ML, font, X_POS + 1, Y_POS - (maxRows -1 - i) * fontHeight, vmNormal, true, false);
      if(leftEllipsis) {
        showString(STD_ELLIPSIS " ", font, X_POS + stringWidth("[", font, true, true), Y_POS - (maxRows -1 -i) * fontHeight, vmNormal, true, false);
        colX += stringWidth(STD_ELLIPSIS " ", font, true, true);
      }
    }

//from here, convert to use a single string
    for(int j = 0; j < maxCols + (rightEllipsis ? 1 : 0); j++) {

      if(allElementsInColAreIntegers[j]) {
        displayFormat = DF_FIX;
        displayFormatDigits = 0;
      } else {
        displayFormat = displayFormat1;
        displayFormatDigits = displayFormatDigits1;
      }

      if(((i == maxRows - 1) && (rows > maxRows + sRow)) || ((j == maxCols) && rightEllipsis) || ((i == 0) && (sRow > 0))) {
        strcpy(tmpString, " " STD_ELLIPSIS);
        vm = vmNormal;
      }
      else {
        real34ToDisplayString(&matrix->matrixElements[(i+sRow)*cols+j+sCol], amNone, tmpString, font, colWidth[j], displayFormat == DF_ALL ? digits : 15, LIMITEXP, FRONTSPACE, cols*rows > 3 ? LIMITIRFRAC : LIGHTIRFRAC);
        if(toDisplay) {
          if(forEditor && matSelRow == (i + sRow) && matSelCol == (j + sCol)) {
            lcd_fill_rect(X_POS + colX, Y_POS - (maxRows -1 -i) * fontHeight, colWidth[j], font == &numericFont ? 32 : 20, LCD_EMPTY_VALUE);
            vm = vmReverse;
          }
          else {
            vm = vmNormal;
          }
        }
      }
      if(toDisplay) {
        width = stringWidth(tmpString, font, true, true) + 1;
        showString(tmpString, font, X_POS + colX + (((j == maxCols) && rightEllipsis) ? -stringWidth(" ", font, true, true) : (colWidth[j] - width) - rPadWidth[i * MATRIX_MAX_COLUMNS + j]), Y_POS - (maxRows -1 -i) * fontHeight, vm, true, false);
        colX += colWidth[j] + stringWidth(STD_SPACE_FIGURE, font, true, true);
      } else {
        if(j > 0) {
          strcat(errorMessage," ");
        }
      strcat(errorMessage,tmpString);
      }
    }
//end string creation

    if(toDisplay) {
      showString((maxRows == 1) ? "]" : (i == 0) ? STD_MAT_TR : (i + 1 == maxRows) ? STD_MAT_BR : STD_MAT_MR, font, X_POS + stringWidth("[", font, true, true) + baseWidth, Y_POS - (maxRows -1 -i) * fontHeight, vmNormal, true, false);
      if(colVector == true) {
        showString(STD_SUP_BOLD_T, font, X_POS + stringWidth("[]", font, true, true) + baseWidth, Y_POS - (maxRows -1 -i) * fontHeight, vmNormal, true, false);
      }
    } else {
      strcat(errorMessage,"]");
      if(colVector == true) {
        strcat(errorMessage, STD_SUP_BOLD_T);
      }
    }

  }

// I suspect strongly this is test code previously not removed. Keeping in here until we are sure. JM 2025-05-16
// It interferes by printing vectors in X while the vector is locaed in say T
// why do we have this ????  if(!toDisplay) {
// why do we have this ????    //printf("sss:%s\n", errorMessage);
// why do we have this ????    showString(errorMessage, font, X_POS, Y_POS - (maxRows -1) * fontHeight, vm, true, false);
// why do we have this ????  }

  displayFormat = tmpDisplayFormat;
  displayFormatDigits = tmpDisplayFormatDigits;

}

int16_t getRealMatrixColumnWidths(const real34Matrix_t *matrix, int16_t prefixWidth, const font_t *font, int16_t *colWidth, int16_t *rPadWidth, int16_t *digits, uint16_t maxCols, bool_t *allElementsInColAreIntegers) {
  char tmpString[200];
  const bool_t colVector = matrix->header.matrixColumns == 1 && matrix->header.matrixRows > 1;
  const int rows = colVector ? 1 : matrix->header.matrixRows;
  const int actualCols = colVector ? matrix->header.matrixRows : matrix->header.matrixColumns;
  const int cols = (actualCols > maxCols) ? maxCols : actualCols;   // clamp for safety
  const int maxRows = rows > MATRIX_MAX_ROWS ? MATRIX_MAX_ROWS : rows;
  const bool_t forEditor = matrix == &openMatrixMIMPointer.realMatrix;
  const uint16_t sRow = forEditor ? scrollRow : 0;
  const uint16_t sCol = forEditor ? scrollColumn : 0;
  const int16_t maxWidth = MATRIX_LINE_WIDTH - prefixWidth;
  int16_t totalWidth = 0;
  int16_t maxRightWidth[MATRIX_MAX_COLUMNS] = {};
  int16_t maxLeftWidth[MATRIX_MAX_COLUMNS] = {};
  const int16_t exponentOutOfRange = 0x4000;
  bool_t noFix = false; const int16_t dspDigits = displayFormatDigits;

  begin:
  for(int k = max(min(displayFormatDigits*(displayFormat == DF_ALL ? 2 : 1), max((50/cols-2),0) ), 10); k >= 1; k--) {                                    //HERE IS THE TIME WASTER - CYCLING THROUGH 15 PRECISIONS !! REDUCE SIGNIFICANTLY from 15 to settingx2 or setting
      if(displayFormat == DF_ALL) {
        *digits = k;
      }
    if(displayFormat == DF_ALL && noFix) { // something like SCI
      displayFormat = getSystemFlag(FLAG_ENGOVR) ? DF_ENG : DF_SCI;
      displayFormatDigits = k;
    }

    const uint16_t displayFormat1 = displayFormat;
    const uint8_t displayFormatDigits1 = displayFormatDigits;

    for(int i = 0; i < maxRows; i++) {
      for(int j = 0; j < maxCols; j++) {
        real34_t r34Val;
        bool_t r34sign = real34IsNegative(&matrix->matrixElements[(i+sRow)*actualCols+j+sCol]);
        real34Copy(&matrix->matrixElements[(i+sRow)*actualCols+j+sCol], &r34Val);
        real34SetPositiveSign(&r34Val);

        if(allElementsInColAreIntegers[j]) {
          displayFormat = DF_FIX;
          displayFormatDigits = 0;
        } else {
          displayFormat = displayFormat1;
          displayFormatDigits = displayFormatDigits1;
        }

        real34ToDisplayString(&r34Val, amNone, tmpString, font, maxWidth, displayFormat == DF_ALL ? k : 15, LIMITEXP, FRONTSPACE, cols*rows > 3 ? LIMITIRFRAC : LIGHTIRFRAC);
        if(displayFormat == DF_ALL && !noFix && strstr(tmpString, STD_SUB_10)) { // something like SCI
          noFix = true;
          totalWidth = 0;
          for(int p = 0; p < MATRIX_MAX_COLUMNS; ++p) {
            maxRightWidth[p] = maxLeftWidth[p] = 0;
          }
          goto begin; // redo
        }

        int16_t width = stringWidth(tmpString, font, true, true) + 1;
        rPadWidth[i * MATRIX_MAX_COLUMNS + j] = 0;
        if(strstr(tmpString, ".") || strstr(tmpString, ",")) {
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
          if(r34sign && strstr(tmpString, "/")) {
            width += stringWidth("-", font, true, true);
          }
          rPadWidth[i * MATRIX_MAX_COLUMNS + j] = width | exponentOutOfRange;
        }
      }
    }

    displayFormat = displayFormat1;
    displayFormatDigits = displayFormatDigits1;

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
    for(int j = 0; j < maxCols; j++) {
      colWidth[j] = (maxLeftWidth[j] + maxRightWidth[j]);
      totalWidth += colWidth[j] + stringWidth(STD_SPACE_FIGURE, font, true, true) * 2;
    }
    totalWidth -= stringWidth(STD_SPACE_FIGURE, font, true, true);
    if(noFix) {
      displayFormat = DF_ALL;
      displayFormatDigits = dspDigits;
    }
    if(displayFormat != DF_ALL) {
      break;
    }
    else if(totalWidth <= maxWidth) {
      *digits = k;
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


void showComplexMatrix(const complex34Matrix_t *matrix, int16_t prefixWidth, angularMode_t angleMode, bool_t polarMode) {
  int rows = matrix->header.matrixRows;
  int cols = matrix->header.matrixColumns;
  int16_t Y_POS = Y_POSITION_OF_REGISTER_X_LINE;
  int16_t X_POS = 0;
  int16_t totalWidth = 0, width = 0;
  const font_t *font;
  int16_t fontHeight = NUMERIC_FONT_HEIGHT_;
  int16_t maxWidth = MATRIX_LINE_WIDTH - prefixWidth;
  int16_t colWidth[MATRIX_MAX_COLUMNS] = {}, colWidth_r[MATRIX_MAX_COLUMNS] = {}, colWidth_i[MATRIX_MAX_COLUMNS] = {};
  int16_t rPadWidth_r[MATRIX_MAX_ROWS * MATRIX_MAX_COLUMNS] = {}, rPadWidth_i[MATRIX_MAX_ROWS * MATRIX_MAX_COLUMNS] = {};
  const bool_t forEditor = matrix == &openMatrixMIMPointer.complexMatrix;
  const uint16_t sRow = forEditor ? scrollRow : 0;
  uint16_t sCol = forEditor ? scrollColumn : 0;
  const uint16_t tmpDisplayFormat = displayFormat;
  const int16_t tmpExponentLimit = exponentLimit;
  const uint8_t tmpDisplayFormatDigits = displayFormatDigits;
  const bool_t tmpMultX = getSystemFlag(FLAG_MULTx);

  Y_POS = Y_POSITION_OF_REGISTER_X_LINE - NUMERIC_FONT_HEIGHT_;

  bool_t colVector = false;
  if(cols == 1 && rows > 1) {
    colVector = true;
    cols = rows;
    rows = 1;
  }

  int maxCols = cols > MATRIX_MAX_COLUMNS ? MATRIX_MAX_COLUMNS : cols;
  const int maxRows = rows > MATRIX_MAX_ROWS ? MATRIX_MAX_ROWS : rows;

  int16_t matSelRow = colVector ? getJRegisterAsInt(true) : getIRegisterAsInt(true);
  int16_t matSelCol = colVector ? getIRegisterAsInt(true) : getJRegisterAsInt(true);

  videoMode_t vm = vmNormal;
    if(maxCols + sCol >= cols) {
      maxCols = cols - sCol;
    }

  font = &numericFont;
  if(rows >= (forEditor ? 4 : 5)) {
smallFont:
    font = &standardFont;
    fontHeight = STANDARD_FONT_HEIGHT_;
    Y_POS = Y_POSITION_OF_REGISTER_X_LINE - STANDARD_FONT_HEIGHT_ + 2;
    //maxWidth = MATRIX_LINE_WIDTH_SMALL * 4 - 20;
  }

    if(!forEditor) {
      Y_POS += REGISTER_LINE_HEIGHT;
    }
  bool_t rightEllipsis = (cols > maxCols) && (cols > maxCols + sCol);
  bool_t leftEllipsis = (sCol > 0);
  int16_t digits;

    if(prefixWidth > 0) {
      Y_POS = Y_POSITION_OF_REGISTER_T_LINE - REGISTER_LINE_HEIGHT + 1 + maxRows * fontHeight;
    }
    if(prefixWidth > 0 && font == &standardFont) {
      Y_POS += (maxRows == 1 ? STANDARD_FONT_HEIGHT_ : REGISTER_LINE_HEIGHT - STANDARD_FONT_HEIGHT_);
    }

    int16_t baseWidth = (leftEllipsis ? stringWidth(STD_ELLIPSIS " ", font, true, true) : 0) + (rightEllipsis ? stringWidth(STD_ELLIPSIS, font, true, true) : 0);
  totalWidth = baseWidth + getComplexMatrixColumnWidths(matrix, prefixWidth, font, colWidth, colWidth_r, colWidth_i, rPadWidth_r, rPadWidth_i, &digits, maxCols, angleMode, polarMode);
  if(totalWidth > maxWidth || leftEllipsis) {
    if(font == &numericFont) {
      goto smallFont;
    }
    else if(exponentLimit > 99) {
      exponentLimit = 99;
      goto smallFont;
    }
    else {
      displayFormat = DF_SCI;
      displayFormatDigits = 2;
      clearSystemFlag(FLAG_MULTx);
      totalWidth = baseWidth + getComplexMatrixColumnWidths(matrix, prefixWidth, font, colWidth, colWidth_r, colWidth_i, rPadWidth_r, rPadWidth_i, &digits, maxCols, angleMode, polarMode);
      if(totalWidth > maxWidth) {
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

    if(prefixWidth > 0) {
      X_POS = prefixWidth;
    }
    else if(!forEditor) {
      X_POS = SCREEN_WIDTH - ((colVector ? stringWidth("[]" STD_SUP_BOLD_T, font, true, true) : stringWidth("[]", font, true, true)) + baseWidth) - (font == &standardFont ? 0 : 1);
    }

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
  else if(prefixWidth > 0) {
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

  for(int i = 0; i < maxRows; i++) {
    int16_t colX = stringWidth("[", font, true, true);
    showString((maxRows == 1) ? "[" : (i == 0) ? STD_MAT_TL : (i + 1 == maxRows) ? STD_MAT_BL : STD_MAT_ML, font, X_POS + 1, Y_POS - (maxRows -1 - i) * fontHeight, vmNormal, true, false);
    if(leftEllipsis) {
      showString(STD_ELLIPSIS " ", font, X_POS + stringWidth("[", font, true, true), Y_POS - (maxRows -1 -i) * fontHeight, vmNormal, true, false);
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
        real34ToDisplayString(&re, amNone, tmpString, font, colWidth_r[j], displayFormat == DF_ALL ? digits : 15, LIMITEXP, FRONTSPACE, LIMITIRFRAC);
        if(forEditor && matSelRow == (i + sRow) && matSelCol == (j + sCol)) {
          lcd_fill_rect(X_POS + colX, Y_POS - (maxRows -1 -i) * fontHeight, colWidth[j], font == &numericFont ? 32 : 20, LCD_EMPTY_VALUE);
          vm = vmReverse;
        }
        else {
          vm = vmNormal;
        }
      }
      width = stringWidth(tmpString, font, true, true) + 1;
      showString(tmpString, font, X_POS + colX + (((j == maxCols) && rightEllipsis) ? stringWidth(STD_SPACE_FIGURE, font, true, true) - width : (colWidth_r[j] - width) - rPadWidth_r[i * MATRIX_MAX_COLUMNS + j]), Y_POS - (maxRows -1 -i) * fontHeight, vm, true, false);
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
        showString(tmpString, font, X_POS + colX + colWidth_r[j] + (width - stringWidth(tmpString, font, true, true)), Y_POS - (maxRows -1 -i) * fontHeight, vm, true, false);

        real34ToDisplayString(&im, polarMode ? angleMode : amNone, tmpString, font, colWidth_i[j], displayFormat == DF_ALL ? digits : 15, LIMITEXP, !FRONTSPACE, LIMITIRFRAC);
        width = stringWidth(tmpString, font, true, true) + 1;
        showString(tmpString, font, X_POS + colX + colWidth_r[j] + cpxUnitWidth + (((j == maxCols - 1) && rightEllipsis) ? 0 : (colWidth_i[j] - width) - rPadWidth_i[i * MATRIX_MAX_COLUMNS + j]), Y_POS - (maxRows -1 -i) * fontHeight, vm, true, false);
      }
      colX += colWidth[j] + stringWidth(STD_SPACE_FIGURE, font, true, true);
    }
    showString((maxRows == 1) ? "]" : (i == 0) ? STD_MAT_TR : (i + 1 == maxRows) ? STD_MAT_BR : STD_MAT_MR, font, X_POS + stringWidth("[", font, true, true) + baseWidth, Y_POS - (maxRows -1 -i) * fontHeight, vmNormal, true, false);
    if(colVector == true) {
      showString(STD_SUP_BOLD_T, font, X_POS + stringWidth("[]", font, true, true) + baseWidth, Y_POS - (maxRows -1 -i) * fontHeight, vmNormal, true, false);
    }
  }

  displayFormat = tmpDisplayFormat;
  displayFormatDigits = tmpDisplayFormatDigits;
  exponentLimit = tmpExponentLimit;
    if(tmpMultX) {
      setSystemFlag(FLAG_MULTx);
    }
}

int16_t getComplexMatrixColumnWidths(const complex34Matrix_t *matrix, int16_t prefixWidth, const font_t *font, int16_t *colWidth, int16_t *colWidth_r, int16_t *colWidth_i, int16_t *rPadWidth_r, int16_t *rPadWidth_i, int16_t *digits, uint16_t maxCols, angularMode_t angleMode, bool_t polarMode) {
  char tmpString[200];
  const bool_t colVector = matrix->header.matrixColumns == 1 && matrix->header.matrixRows > 1;
  const int rows = colVector ? 1 : matrix->header.matrixRows;
  const int actualCols = colVector ? matrix->header.matrixRows : matrix->header.matrixColumns;
  const int cols = (actualCols > maxCols) ? maxCols : actualCols;   // clamp for safety
  const int maxRows = rows > MATRIX_MAX_ROWS ? MATRIX_MAX_ROWS : rows;
  const bool_t forEditor = matrix == &openMatrixMIMPointer.complexMatrix;
  const uint16_t sRow = forEditor ? scrollRow : 0;
  const uint16_t sCol = forEditor ? scrollColumn : 0;
  const int16_t maxWidth = MATRIX_LINE_WIDTH - prefixWidth;
  int16_t totalWidth = 0;
  int16_t maxRightWidth_r[MATRIX_MAX_COLUMNS] = {};
  int16_t maxLeftWidth_r[MATRIX_MAX_COLUMNS] = {};
  int16_t maxRightWidth_i[MATRIX_MAX_COLUMNS] = {};
  int16_t maxLeftWidth_i[MATRIX_MAX_COLUMNS] = {};
  const int16_t exponentOutOfRange = 0x4000;

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

  for(int k = max(min(displayFormatDigits*(displayFormat == DF_ALL ? 2 : 1), max((50/cols-2),0) ), 10); k >= 1; k--) {                                    //HERE IS THE TIME WASTER - CYCLING THROUGH 15 PRECISIONS !! REDUCE SIGNIFICANTLY from 15 to settingx2 or setting
      if(displayFormat == DF_ALL) {
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

        rPadWidth_r[i * MATRIX_MAX_COLUMNS + j] = 0;
        real34SetPositiveSign(VARIABLE_REAL34_DATA(&c34Val));
        bool_t c34sign = real34IsNegative(&matrix->matrixElements[(i+sRow)*actualCols+j+sCol]);
        real34ToDisplayString(VARIABLE_REAL34_DATA(&c34Val), amNone, tmpString, font, maxWidth, displayFormat == DF_ALL ? k : 15, LIMITEXP, FRONTSPACE, LIMITIRFRAC);
        int16_t width = stringWidth(tmpString, font, true, true) + 1;
        if(strstr(tmpString, ".") || strstr(tmpString, ",")) {
          for(char *xStr = tmpString; *xStr != 0; xStr++) {
            if(((displayFormat != DF_ENG && (displayFormat != DF_ALL || !getSystemFlag(FLAG_ENGOVR))) && (*xStr == '.' || *xStr == ',')) ||
               ((displayFormat == DF_ENG || (displayFormat == DF_ALL && getSystemFlag(FLAG_ENGOVR))) && xStr[0] == (char)0x80 && (xStr[1] == (char)0x87 || xStr[1] == (char)0xd7))) {
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
          if(c34sign && strstr(tmpString, "/")) {
            width += stringWidth("-", font, true, true);
          }
          rPadWidth_r[i * MATRIX_MAX_COLUMNS + j] = width | exponentOutOfRange;
        }

        rPadWidth_i[i * MATRIX_MAX_COLUMNS + j] = 0;
        c34sign = false;
        if(!polarMode) {
          c34sign = real34IsNegative(&matrix->matrixElements[(i+sRow)*actualCols+j+sCol]);
          real34SetPositiveSign(VARIABLE_IMAG34_DATA(&c34Val));
        }
        real34ToDisplayString(VARIABLE_IMAG34_DATA(&c34Val), polarMode ? angleMode : amNone, tmpString, font, maxWidth, displayFormat == DF_ALL ? k : 15, LIMITEXP, !FRONTSPACE, LIMITIRFRAC);
        width = stringWidth(tmpString, font, true, true) + 1;
        if(strstr(tmpString, ".") || strstr(tmpString, ",")) {
          for(char *xStr = tmpString; *xStr != 0; xStr++) {
            if(((displayFormat != DF_ENG && (displayFormat != DF_ALL || !getSystemFlag(FLAG_ENGOVR))) && (*xStr == '.' || *xStr == ',')) ||
               ((displayFormat == DF_ENG || (displayFormat == DF_ALL && getSystemFlag(FLAG_ENGOVR))) && xStr[0] == (char)0x80 && (xStr[1] == (char)0x87 || xStr[1] == (char)0xd7))) {
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
    for(int j = 0; j < maxCols; j++) {
      colWidth_r[j] = maxLeftWidth_r[j] + maxRightWidth_r[j];
      colWidth_i[j] = maxLeftWidth_i[j] + maxRightWidth_i[j];
      colWidth[j] = colWidth_r[j] + (colWidth_i[j] > 0 ? (cpxUnitWidth + colWidth_i[j]) : 0);
      totalWidth += colWidth[j] + stringWidth(STD_SPACE_FIGURE, font, true, true) * 2;
    }
    totalWidth -= stringWidth(STD_SPACE_FIGURE, font, true, true);
    if(displayFormat != DF_ALL) {
      break;
    }
    else if(totalWidth <= maxWidth) {
      *digits = k;
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
#endif // !defined(TESTSUITE_BUILD)

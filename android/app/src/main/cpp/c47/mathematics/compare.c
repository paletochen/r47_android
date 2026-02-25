// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file compare.c
 ***********************************************/

#include "c47.h"


void compareTypeError(calcRegister_t regist) {
  temporaryInformation = TI_FALSE;
  badTypeError(regist);
}

void compareTypeErrorX(void) {
  compareTypeError(REGISTER_X);
}

bool_t registerCmp(calcRegister_t regist1, calcRegister_t regist2, int8_t *res) {
  const uint32_t type1 = getRegisterDataType(regist1);
  const uint32_t type2 = getRegisterDataType(regist2);

  if(type1 == dtString && type2 == dtString) {
    *res = compareString(REGISTER_STRING_DATA(regist1), REGISTER_STRING_DATA(regist2), CMP_EXTENSIVE);
  }
  else if((type1 == dtShortInteger || type1 == dtLongInteger) && (type2 == dtShortInteger || type2 == dtLongInteger)) {
    longInteger_t int1, int2;

    /* These should never fail */
    if(getRegisterAsLongIntQuiet(regist1, int1, NULL) != ERROR_NONE) {
      goto end;
    }

    if(getRegisterAsLongIntQuiet(regist2, int2, NULL)) {
      longIntegerFree(int2);
end:
      longIntegerFree(int1);
      return false;
    }

    *res = longIntegerCompare(int1, int2);
    longIntegerFree(int1);
    longIntegerFree(int2);
  }
  else {
    real_t rcmp, real1, real2;

    if(!getRegisterAsAnyRealQuiet(regist1, &real1) || !getRegisterAsAnyRealQuiet(regist2, &real2)) {
      return false;
    }
    realCompare(&real1, &real2, &rcmp, &ctxtReal39);
    *res = realIsZero(&rcmp) ? 0 : realIsPositive(&rcmp) ? 1 : -1;
  }
  return true;
}

static void registerMinMax(calcRegister_t regist1, calcRegister_t regist2, calcRegister_t dest, int maximum) {
    int8_t cmp;
    calcRegister_t rMin = regist1, rMax = regist2, where;

    if (registerCmp(rMin, rMax, &cmp)) {
      if (cmp == 0 && (dest == regist1 || dest == regist2))
        return;
      if (cmp > 0) {
        rMin = regist2;
        rMax = regist1;
      }
      where = maximum ? rMax : rMin;
      if (where != dest)
        copySourceRegisterToDestRegister(where, dest);
    } else
      badTypeError(regist1);
}

void registerMax(calcRegister_t regist1, calcRegister_t regist2, calcRegister_t dest) {
  registerMinMax(regist1, regist2, dest, 1);
}

void registerMin(calcRegister_t regist1, calcRegister_t regist2, calcRegister_t dest) {
  registerMinMax(regist1, regist2, dest, 0);
}

#define COMPARE_MODE_LESS_THAN     0x1
#define COMPARE_MODE_EQUAL         0x2
#define COMPARE_MODE_LESS_EQUAL    0x3
#define COMPARE_MODE_GREATER_THAN  0x4
#define COMPARE_MODE_NOT_EQUAL     0x5
#define COMPARE_MODE_GREATER_EQUAL 0x6

static void compareMatrix01(uint16_t regist, uint8_t mode, uint32_t typeX) {
  int i, j, k;
  complex34Matrix_t mCplx;
  real34Matrix_t mReal;
  uint8_t actualMode = COMPARE_MODE_NOT_EQUAL;
  const real34_t *const r = REGISTER_REAL34_DATA(regist);

  /* Check for zero and identity matricies */
  if (typeX == dtReal34Matrix) {
    linkToRealMatrixRegister(REGISTER_X, &mReal);
    for (i=k=0; i<mReal.header.matrixRows; i++)
      for (j=0; j<mReal.header.matrixColumns; j++, k++)
        if (!real34CompareEqual(&mReal.matrixElements[k], i==j ? r : const34_0))
          goto different;
  } else {
    linkToComplexMatrixRegister(REGISTER_X, &mCplx);
    for (i=k=0; i<mCplx.header.matrixRows; i++)
      for (j=0; j<mCplx.header.matrixColumns; j++, k++)
        if (!real34CompareEqual(&mCplx.matrixElements[k].real, i==j ? r : const34_0)
            || !real34IsZero((&mCplx.matrixElements[k].imag)))
          goto different;
  }
  actualMode = COMPARE_MODE_EQUAL;

different:
  SET_TI_TRUE_FALSE(mode == actualMode);
}

/* In matrices, we consider NaNs to be equal which is counter to normal */
static int matrixCompareRealEqual(const real34_t *a, const real34_t *b) {
  return real34CompareEqual(a, b) || (real34IsNaN(a) && real34IsNaN(b));
}

static void compareMatrices(uint16_t regist, uint8_t mode, uint32_t typeX, uint32_t typeR) {
  complex34Matrix_t x, r;

  if(typeX == dtComplex34Matrix) {
    linkToComplexMatrixRegister(REGISTER_X, &x);
  }
  else {
    convertReal34MatrixRegisterToComplex34Matrix(REGISTER_X, &x);
  }
  if(typeR == dtComplex34Matrix) {
    linkToComplexMatrixRegister(regist, &r);
  }
  else {
    convertReal34MatrixRegisterToComplex34Matrix(regist, &r);
  }
  if(x.header.matrixRows == r.header.matrixRows && x.header.matrixColumns == r.header.matrixColumns) {
    temporaryInformation = TI_TRUE;
    for(int i = 0; i < x.header.matrixRows * x.header.matrixColumns; ++i) {
      if((!matrixCompareRealEqual(VARIABLE_REAL34_DATA(&x.matrixElements[i]), VARIABLE_REAL34_DATA(&r.matrixElements[i])))
          || (!matrixCompareRealEqual(VARIABLE_IMAG34_DATA(&x.matrixElements[i]), VARIABLE_IMAG34_DATA(&r.matrixElements[i])))) {
        temporaryInformation = TI_FALSE;
        break;
      }
    }
  }
  else {
    temporaryInformation = TI_FALSE;
  }
  if(mode == COMPARE_MODE_NOT_EQUAL) {
    if(temporaryInformation == TI_TRUE) {
      temporaryInformation = TI_FALSE;
    }
    else {
     temporaryInformation = TI_TRUE;
    }
  }
  if(typeX == dtReal34Matrix) {
    complexMatrixFree(&x);
  }
  if(typeR == dtReal34Matrix) {
    complexMatrixFree(&r);
  }
}

static void cmpToResult(int result, uint8_t mode) {
  if(result < 0) {
    SET_TI_TRUE_FALSE((mode & COMPARE_MODE_LESS_THAN) != 0);
  }
  else if(result > 0) {
    SET_TI_TRUE_FALSE((mode & COMPARE_MODE_GREATER_THAN) != 0);
  }
  else {
    SET_TI_TRUE_FALSE((mode & COMPARE_MODE_EQUAL) != 0);
  }
}

static void compareRegisters(uint16_t regist, uint8_t mode) {
  if((regist < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) || (FIRST_NAMED_VARIABLE <= regist && regist <= FIRST_NAMED_VARIABLE + numberOfNamedVariables) || (FIRST_RESERVED_VARIABLE <= regist && regist <= LAST_RESERVED_VARIABLE) || (regist == TEMP_REGISTER_1)) {
    const uint32_t typeX = getRegisterDataType(REGISTER_X);
    const uint32_t typeR = getRegisterDataType(regist);

    if(typeX == dtString || typeR == dtString) {
      if(typeX != typeR) {
        compareTypeError(regist);
      }
      else {
        cmpToResult(compareString(REGISTER_STRING_DATA(REGISTER_X), REGISTER_STRING_DATA(regist), CMP_EXTENSIVE), mode);
      }
    }
    else if(typeX == dtConfig || typeR == dtConfig) {
      if(typeX != typeR) {
        compareTypeError(regist);
      }
      else if(mode != COMPARE_MODE_EQUAL && mode != COMPARE_MODE_NOT_EQUAL) {
        compareTypeError(regist);
      }
      else {
        cmpToResult(memcmp(REGISTER_CONFIG_DATA(REGISTER_X), REGISTER_CONFIG_DATA(regist), CONFIG_SIZE_IN_BLOCKS), mode);
      }
    }
    else if(typeX == dtReal34Matrix || typeX == dtComplex34Matrix || typeR == dtReal34Matrix || typeR == dtComplex34Matrix) {
      if(mode != COMPARE_MODE_EQUAL && mode != COMPARE_MODE_NOT_EQUAL) {
        compareTypeError(regist);
      }
      else if(regist == TEMP_REGISTER_1) {
        compareMatrix01(regist, mode, typeX);
      }
      else if((typeX != dtReal34Matrix && typeX != dtComplex34Matrix) || (typeR != dtReal34Matrix && typeR != dtComplex34Matrix)) {
        compareTypeError(regist);
      }
      else {
        compareMatrices(regist, mode, typeX, typeR);
      }
    }
    else if((typeX == dtShortInteger || typeX == dtLongInteger) && (typeR == dtShortInteger || typeR == dtLongInteger)) {
      longInteger_t xInt, rInt;

      if(getRegisterAsLongInt(REGISTER_X, xInt, NULL) && getRegisterAsLongInt(regist, rInt, NULL)) {
        cmpToResult(longIntegerCompare(xInt, rInt), mode);
      }
      longIntegerFree(xInt);
      longIntegerFree(rInt);
    }
    else {
      real_t xReal, xImag, rReal, rImag;
      bool cmplx = false;

      if(!getRegisterAsComplexOrAnyReal(REGISTER_X, &xReal, &xImag, &cmplx) || !getRegisterAsComplexOrAnyReal(regist, &rReal, &rImag, &cmplx)) {
        return;
      }
      if(cmplx) {
        if(mode != COMPARE_MODE_EQUAL && mode != COMPARE_MODE_NOT_EQUAL) {
          compareTypeError(regist);
        }
        else if(realIsNaN(&xReal) || realIsNaN(&rReal) || realIsNaN(&xImag) || realIsNaN(&rImag)) {
          temporaryInformation = TI_FALSE;
        }
        else {
          SET_TI_TRUE_FALSE((realCompareEqual(&xReal, &rReal) && realCompareEqual(&xImag, &rImag)) == (mode == COMPARE_MODE_EQUAL));
        }
      }
      else {
        if(realIsNaN(&xReal) || realIsNaN(&rReal)) {
          temporaryInformation = TI_FALSE;
        }
        else {
          realCompare(&xReal, &rReal, &xImag, &ctxtReal39);
          cmpToResult(realIsZero(&xImag) ? 0 : realIsNegative(&xImag) ? -1 : 1, mode);
        }
      }
    }
  }
  #if defined(PC_BUILD)
    else {
      sprintf(errorMessage, "local register .%02d", regist - FIRST_LOCAL_REGISTER);
      moreInfoOnError("In function compareRegisters:", errorMessage, "is not defined!", NULL);
    }
  #endif // PC_BUILD
}

void fnXLessThan(uint16_t regist) {
  compareRegisters(regist, COMPARE_MODE_LESS_THAN);
}

void fnXLessEqual(uint16_t regist) {
  compareRegisters(regist, COMPARE_MODE_LESS_EQUAL);
}

void fnXGreaterThan(uint16_t regist) {
  compareRegisters(regist, COMPARE_MODE_GREATER_THAN);
}

void fnXGreaterEqual(uint16_t regist) {
  compareRegisters(regist, COMPARE_MODE_GREATER_EQUAL);
}

void fnXEqualsTo(uint16_t regist) {
  compareRegisters(regist, COMPARE_MODE_EQUAL);
}

void fnXNotEqual(uint16_t regist) {
  compareRegisters(regist, COMPARE_MODE_NOT_EQUAL);
}

static void almostEqualMatrix(uint16_t regist) {
  #if !defined(TESTSUITE_BUILD)
    if(getRegisterDataType(REGISTER_X) == dtReal34Matrix && getRegisterDataType(regist) == dtReal34Matrix) {
      real34Matrix_t x, r;
      convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &x);
      convertReal34MatrixRegisterToReal34Matrix(regist, &r);
      roundRema();
      fnSwapX(regist);
      roundRema();
      fnSwapX(regist);
      compareRegisters(regist, COMPARE_MODE_EQUAL);
      convertReal34MatrixToReal34MatrixRegister(&r, regist);
      convertReal34MatrixToReal34MatrixRegister(&x, REGISTER_X);
      realMatrixFree(&r);
      realMatrixFree(&x);
    }
    else {
      const bool_t xIsCxma = getRegisterDataType(REGISTER_X) == dtComplex34Matrix;
      const bool_t rIsCxma = getRegisterDataType(regist)     == dtComplex34Matrix;
      complex34Matrix_t x, r;
      real34Matrix_t m;

      if(xIsCxma) {
        convertComplex34MatrixRegisterToComplex34Matrix(REGISTER_X, &x);
      }
      else {
        convertReal34MatrixRegisterToComplex34Matrix(REGISTER_X, &x);
        convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &m);
      }
      if(rIsCxma) {
        convertComplex34MatrixRegisterToComplex34Matrix(regist, &r);
      }
      else {
        convertReal34MatrixRegisterToComplex34Matrix(regist, &r);
        convertReal34MatrixRegisterToReal34Matrix(regist, &m);
      }

      if(xIsCxma) {
        roundCxma();
      }
      else {
        roundRema();
      }
      fnSwapX(regist);
      if(rIsCxma) {
        roundCxma();
      }
      else {
        roundRema();
      }
      fnSwapX(regist);

      compareRegisters(regist, COMPARE_MODE_EQUAL);

      if(rIsCxma) {
        convertComplex34MatrixToComplex34MatrixRegister(&r, regist);
      }
      else {
        convertReal34MatrixToReal34MatrixRegister(&m, regist);
      }
      if(xIsCxma) {
        convertComplex34MatrixToComplex34MatrixRegister(&x, REGISTER_X);
      }
      else {
        convertReal34MatrixToReal34MatrixRegister(&m, REGISTER_X);
      }

      complexMatrixFree(&r);
      complexMatrixFree(&x);
      if(!xIsCxma || !rIsCxma) {
        realMatrixFree(&m);
      }
    }
  #endif // !TESTSUITE_BUILD
}

static void almostEqualScalar(uint16_t regist) {
  real34_t val1r, val1i, val2r, val2i;

  real34Copy(const34_0, &val1i);
  real34Copy(const34_0, &val2i);

  if(getRegisterDataType(REGISTER_X) == dtComplex34) {
    real34Copy(REGISTER_IMAG34_DATA(REGISTER_X), &val1i);
  }
  real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &val1r);
  if(getRegisterDataType(regist) == dtComplex34) {
    real34Copy(REGISTER_IMAG34_DATA(regist), &val2i);
  }
  real34Copy(REGISTER_REAL34_DATA(regist), &val2r);

  switch(getRegisterDataType(REGISTER_X)) {
    case dtComplex34: {
      roundCplx();
      break;
    }
    case dtReal34: {
      roundReal();
      break;
    }
    default: { /* dtTime */
     roundTime();
     break;
    }
  }
  if(regist != TEMP_REGISTER_1) {
    fnSwapX(regist);
    switch(getRegisterDataType(REGISTER_X)) {
      case dtComplex34: {
        roundCplx();
        break;
      }
      case dtReal34: {
        roundReal();
        break;
      }
      default: { /* dtTime */
        roundTime();
        break;
      }
    }
    fnSwapX(regist);
  }

  compareRegisters(regist, COMPARE_MODE_EQUAL);

  if(getRegisterDataType(REGISTER_X) == dtComplex34) {
    real34Copy(&val1i, REGISTER_IMAG34_DATA(REGISTER_X));
  }
  real34Copy(&val1r, REGISTER_REAL34_DATA(REGISTER_X));
  if(getRegisterDataType(regist) == dtComplex34) {
    real34Copy(&val2i, REGISTER_IMAG34_DATA(regist));
  }
  real34Copy(&val2r, REGISTER_REAL34_DATA(regist));
}

void fnXAlmostEqual(uint16_t regist) {
  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix || getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    if(getRegisterDataType(regist) == dtReal34Matrix || getRegisterDataType(regist) == dtComplex34Matrix) {
      almostEqualMatrix(regist);
    }
    else {
      compareTypeError(regist);
    }
  }
  else if(getRegisterDataType(REGISTER_X) == dtReal34 || getRegisterDataType(REGISTER_X) == dtComplex34 || getRegisterDataType(REGISTER_X) == dtTime) {
    if(getRegisterDataType(regist) == dtReal34 || getRegisterDataType(regist) == dtComplex34 || getRegisterDataType(regist) == dtTime) {
      almostEqualScalar(regist);
    }
    else {
      compareTypeError(regist);
    }
  }
  else {
    compareTypeError(regist);
  }
}


#undef COMPARE_MODE_LESS_THAN
#undef COMPARE_MODE_EQUAL
#undef COMPARE_MODE_LESS_EQUAL
#undef COMPARE_MODE_GREATER_THAN
#undef COMPARE_MODE_NOT_EQUAL
#undef COMPARE_MODE_GREATER_EQUAL

void fnIsConverged(uint16_t mode) {
  real_t xReal, xImag, yReal, yImag, tol;
  bool_t isComplex = false;

  convergenceTolerence(&tol);
  if(!getRegisterAsComplexOrReal(REGISTER_X, &xReal, &xImag, &isComplex) || !getRegisterAsComplexOrReal(REGISTER_Y, &yReal, &yImag, &isComplex)) {
    compareTypeError(REGISTER_Y);
    return;
  }

  if(realIsNaN(&xReal) || realIsNaN(&yReal) || realIsNaN(&xImag) || realIsNaN(&yImag)) {
    SET_TI_TRUE_FALSE((mode & 0x4) != 0);
  }
  else if(realIsInfinite(&xReal) || realIsInfinite(&yReal) || realIsInfinite(&xImag) || realIsInfinite(&yImag)) {
    SET_TI_TRUE_FALSE((mode & 0x2) != 0);
  }
  else if(mode & 0x01) {
    SET_TI_TRUE_FALSE(isComplex ? WP34S_ComplexAbsError(&xReal, &xImag, &yReal, &yImag, &tol, &ctxtReal39) : WP34S_AbsoluteError(&xReal, &yReal, &tol, &ctxtReal39));
  }
  else {
    SET_TI_TRUE_FALSE(isComplex ? WP34S_ComplexRelativeError(&xReal, &xImag, &yReal, &yImag, &tol, &ctxtReal39) : WP34S_RelativeError(&xReal, &yReal, &tol, &ctxtReal39));
  }
}


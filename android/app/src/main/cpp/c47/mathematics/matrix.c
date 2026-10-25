// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file matrix.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_EIGEN)                                                    // stubs for the eigen suite (real code is guarded below): EIGVAL EIGVEC M.QR MSQRT
  void fnMatrixSquareRoot(uint16_t unusedParamButMandatory) {}
  void fnQrDecomposition (uint16_t unusedParamButMandatory) {}
  void fnEigenvalues     (uint16_t unusedParamButMandatory) {}
  void fnEigenvectors    (uint16_t unusedParamButMandatory) {}
#endif // !OPTION_EIGEN


#if defined(OPTION_EIGEN)
static void sqrtRealMatrix     (const real34Matrix_t *matrix, real34Matrix_t *res);
static void sqrtComplexMatrix  (const complex34Matrix_t *matrix, complex34Matrix_t *res);
static void realEigenvalues    (const real34Matrix_t *matrix, real34Matrix_t *res, real34Matrix_t *ires);
static void complexEigenvalues (const complex34Matrix_t *matrix, complex34Matrix_t *res);
static void realEigenvectors   (const real34Matrix_t *matrix, real34Matrix_t *res, real34Matrix_t *ires);
static void complexEigenvectors(const complex34Matrix_t *matrix, complex34Matrix_t *res);
#endif // OPTION_EIGEN

static void matrixAccumulateValue(const real_t *value, real_t *sum, real_t *correction, real_t *absoluteSum, realContext_t *realContext);
static bool_t matrixNeedsExtendedAccumulator(const real_t *absoluteSum, const real_t *result);
static void matrixFinishAccumulator(const real_t *sum, const real_t *correction, real_t *result, realContext_t *realContext);
static bool_t realMatExactlySingular(const real34Matrix_t *matrix);


static void real34CopyWrapper(void *src, void *dst) {
  real34Copy((real34_t*)src, (real34_t*)dst);
}

static void real34ZeroWrapper(void *dst) {
  real34SetZero((real34_t*)dst);
}

static void complex34CopyWrapper(void *src, void *dst) {
  complex34Copy((complex34_t*)src, (complex34_t*)dst);
}

static void complex34ZeroWrapper(void *dst) {
  real34SetZero(VARIABLE_REAL34_DATA((real34_t*)dst));
  real34SetZero(VARIABLE_IMAG34_DATA((real34_t*)dst));
}

static void real34OneWrapper(void *dst) {
  real34SetOne((real34_t*)dst);
}

static void complex34OneWrapper(void *dst) {
  real34SetOne( VARIABLE_REAL34_DATA((real34_t*)dst));
  real34SetZero(VARIABLE_IMAG34_DATA((real34_t*)dst));
}

static bool_t real34EqualWrapper(void *a, void *b) {
  return real34CompareEqual((real34_t*)a, (real34_t*)b);
}

static bool_t complex34EqualWrapper(void *a, void *b) {
  return real34CompareEqual(VARIABLE_REAL34_DATA((real34_t*)a), VARIABLE_REAL34_DATA((real34_t*)b))
      && real34CompareEqual(VARIABLE_IMAG34_DATA((real34_t*)a), VARIABLE_IMAG34_DATA((real34_t*)b));
}

static bool_t realMatrixInitWrapper(void *matrix, uint16_t rows, uint16_t cols) {
  return realMatrixInit((real34Matrix_t*)matrix, rows, cols);
}

static void realMatrixFreeWrapper(void *matrix) {
  realMatrixFree((real34Matrix_t*)matrix);
}

static void realMatrixCopyWrapper(const void *matrix, void *res) {
  copyRealMatrix((const real34Matrix_t*)matrix, (real34Matrix_t*)res);
}

static void realMatrixLinkWrapper(calcRegister_t regist, void *linkedMatrix) {
  linkToRealMatrixRegister(regist, (real34Matrix_t*)linkedMatrix);
}

static bool_t complexMatrixInitWrapper(void *matrix, uint16_t rows, uint16_t cols) {
  return complexMatrixInit((complex34Matrix_t*)matrix, rows, cols);
}

static void complexMatrixFreeWrapper(void *matrix) {
  complexMatrixFree((complex34Matrix_t*)matrix);
}

static void complexMatrixCopyWrapper(const void *matrix, void *res) {
  copyComplexMatrix((const complex34Matrix_t*)matrix, (complex34Matrix_t*)res);
}

static void complexMatrixLinkWrapper(calcRegister_t regist, void *linkedMatrix) {
  linkToComplexMatrixRegister(regist, (complex34Matrix_t*)linkedMatrix);
}

// real34Matrix_t and complex34Matrix_t take the same header and differ only in the size of an element, so a routine that moves elements about has one body that takes
// the element size and the services that differ. matrixKindReal and matrixKindComplex are the two of them.
typedef struct {
  size_t  elementSize;
  uint8_t registerDataType;
  bool_t  isComplex;
  bool_t (*initMatrix)(void *matrix, uint16_t rows, uint16_t cols);
  void   (*freeMatrix)(void *matrix);
  void   (*copyMatrix)(const void *matrix, void *res);
  void   (*linkToRegister)(calcRegister_t regist, void *linkedMatrix);
  void   (*copyElement)(void *src, void *dst);
  void   (*zeroElement)(void *dst);
  void   (*oneElement)(void *dst);
  bool_t (*equalElement)(void *a, void *b);
} matrixKind_t;

static const matrixKind_t matrixKindReal    = {REAL34_SIZE_IN_BYTES,    dtReal34Matrix,    false, realMatrixInitWrapper,    realMatrixFreeWrapper,
                                               realMatrixCopyWrapper,    realMatrixLinkWrapper,    real34CopyWrapper,    real34ZeroWrapper,
                                               real34OneWrapper,    real34EqualWrapper};
static const matrixKind_t matrixKindComplex = {COMPLEX34_SIZE_IN_BYTES, dtComplex34Matrix, true,  complexMatrixInitWrapper, complexMatrixFreeWrapper,
                                               complexMatrixCopyWrapper, complexMatrixLinkWrapper, complex34CopyWrapper, complex34ZeroWrapper,
                                               complex34OneWrapper, complex34EqualWrapper};

// The element block of either matrix, as bytes. any34Matrix_t declares both members over the same storage, so the pointer is taken through the union.
#define MATRIX_BYTES(m) ((uint8_t*)(((const any34Matrix_t*)(m))->realMatrix.matrixElements))

// The new matrix replaces the old one in place: the header is taken over and the old element block is released.
static void adoptNewMatrix(any34Matrix_t *matrix, any34Matrix_t *newMat, const matrixKind_t *kind) {
  kind->freeMatrix(matrix);
  matrix->header.matrixRows         = newMat->header.matrixRows;
  matrix->header.matrixColumns      = newMat->header.matrixColumns;
  matrix->realMatrix.matrixElements = newMat->realMatrix.matrixElements;
}



// Eigenvalue setup
//unlikely to change:
#undef  CONV_SUM_159       //DEFAULT NOT SET  //use for higher accuracy on the converge sums. Need need seen for the higher accuracy.
#define ABS_SUMS           //DEFAULT  //use absulate value sums instead of sum of squares, to preserve the digits better

//can change:
  #if defined(PC_BUILD)
    #define maxEigenIter 10000
    #define EIGENDEBUG
    #undef  EIGENDEBUG1
    #undef  EIGENDEBUG2
    #define EIGENDEBUGMINIMAL
    #define EIGEN_TESTOUT
    #undef  EIGENDEBUG_QR
    //
    #undef EIGENDEBUG         //comment this undef to switch eigenvalue debug on
    #undef EIGENDEBUGMINIMAL  //comment this undef to switch eigenvalue debug on
    #undef EIGEN_TESTOUT      //comment this undef to switch eigenvalue debug on
  #else
    #define maxEigenIter 10000
    #undef EIGENDEBUG
    #undef EIGENDEBUG1
    #undef EIGENDEBUG2
    #undef EIGENDEBUGMINIMAL
    #undef EIGEN_TESTOUT
    #undef EIGENDEBUG_QR
  #endif //PC_BUILD

  #if defined(TESTSUITE_BUILD)
    #define extraDigits           3
    #define toleranceDigits       (34 + extraDigits)
    #undef EIGENDEBUG
  #else //TESTSUITE_BUILD
    #define extraDigits           3
    #define toleranceDigits       ((significantDigits == 0 ? 34 : significantDigits) + extraDigits)
  #endif //TESTSUITE_BUILD

  #define eigenTolerance          (min(70, toleranceDigits*2))
  #define blockDetectionTolerance 40
  #define finalBlockTolerance     30
  #define POST_QR_RELATIVE_BLOCK_CHECK
  #define symmetricTolerance      30
  #define eigenNoiseThreshold     70                                              // clamp rediculously small numbers to zero if smaller than 10^-eigenZeroing
  #define eigenContext            &ctxtReal75
  #define ctxtTruncate            &ctxtReal51  //ctxtReal34
  #define ctxtDeNoise             &ctxtReal39                                     // tested, no benfit on 51
// End Eigenvalue setup


bool_t getMatrixDims(calcRegister_t regist, const char *funcName, uint16_t *rows, uint16_t *cols) {
  uint32_t dt = getRegisterDataType(regist);
  if(dt == dtReal34Matrix) {
    real34Matrix_t x;
    linkToRealMatrixRegister(regist, &x);
    *rows = x.header.matrixRows;
    *cols = x.header.matrixColumns;
    return true;
  }
  else if(dt == dtComplex34Matrix) {
    complex34Matrix_t x;
    linkToComplexMatrixRegister(regist, &x);
    *rows = x.header.matrixRows;
    *cols = x.header.matrixColumns;
    return true;
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, dt);
      moreInfoOnError(funcName, errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
}

  static bool_t getArg(calcRegister_t regist, real_t *arg) {
    if(getRegisterDataType(regist) == dtLongInteger) {
      convertLongIntegerRegisterToReal(regist, arg, &ctxtReal39);
    }
    else if(getRegisterDataType(regist) == dtReal34) {
      real34ToReal(REGISTER_REAL34_DATA(regist), arg);
      realToIntegralValue(arg, arg, DEC_ROUND_DOWN, &ctxtReal39);
    }
    else {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot accept %s as the argument", getRegisterDataTypeName(regist, true, false));
        moreInfoOnError("In function getArg:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }

bool_t getSingleDimension(calcRegister_t reg, uint32_t *d) {
  longInteger_t tmp;
  bool_t res = false;

  if(!getRegisterAsLongInt(reg, tmp, NULL)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "invalid data type %s for %s", getRegisterDataTypeName(reg, true, false), reg == REGISTER_X ? "columns" : "rows");
      moreInfoOnError("In function getSingleDimension:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    longIntegerFree(tmp);
    return false;
  }

  if(longIntegerIsNegativeOrZero(tmp) || longIntegerCompareInt(tmp, 4096) > 0) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "invalid number of %s", reg == REGISTER_X ? "columns" : "rows");
      moreInfoOnError("In function getSingleDimension:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    longIntegerToUInt32(tmp, *d);
    res = true;
  }

  longIntegerFree(tmp);
  return res;
}

bool_t getDimensionArg(uint32_t *rows, uint32_t *cols) {
  //Get Size or I&J for STOIJ from REGISTER_X and REGISTER_Y
  return getSingleDimension(REGISTER_X, cols) && getSingleDimension(REGISTER_Y, rows);
}



  static bool_t _swapReal(real34Matrix_t *matrix, bool_t isRow) {
    real_t ry, rx, rlimit;
    uint16_t a, b;

    int32ToReal(isRow ? matrix->header.matrixRows : matrix->header.matrixColumns, &rlimit);

    if((!getArg(REGISTER_Y, &ry)) || (!getArg(REGISTER_X, &rx))) {
      return false;
    }

    a = realToInt32C47(&ry, NULL);
    b = realToInt32C47(&rx, NULL);
    if(realIsPositive(&rx) && realIsPositive(&ry) && realCompareLessEqual(&rx, &rlimit) && realCompareLessEqual(&ry, &rlimit)) {
      if(!realCompareEqual(&ry, &rx)) {
        if(isRow) {
          realMatrixSwapRows(matrix, matrix, a - 1, b - 1);
        }
        else {
          realMatrixSwapColumns(matrix, matrix, a - 1, b - 1);
        }
      }
    }
    else {
        displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "%s %" PRIu16 " and/or %" PRIu16 " out of range", isRow ? "rows" : "columns", a, b);
          moreInfoOnError("In function _swapReal:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }

    return true;
  }

  static bool_t _swapComplex(complex34Matrix_t *matrix, bool_t isRow) {
    real_t ry, rx, rlimit;
    uint16_t a, b;

    int32ToReal(isRow ? matrix->header.matrixRows : matrix->header.matrixColumns, &rlimit);

    if((!getArg(REGISTER_Y, &ry)) || (!getArg(REGISTER_X, &rx))) {
      return false;
    }

    a = realToInt32C47(&ry, NULL);
    b = realToInt32C47(&rx, NULL);
    if(realIsPositive(&rx) && realIsPositive(&ry) && realCompareLessEqual(&rx, &rlimit) && realCompareLessEqual(&ry, &rlimit)) {
      if(!realCompareEqual(&ry, &rx)) {
        if(isRow) {
          complexMatrixSwapRows(matrix, matrix, a - 1, b - 1);
        }
        else {
          complexMatrixSwapColumns(matrix, matrix, a - 1, b - 1);
        }
      }
    }
    else {
        displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "%s %" PRIu16 " and/or %" PRIu16 " out of range", isRow ? "rows" : "columns", a, b);
          moreInfoOnError("In function swapComplex:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }

    return true;
  }

  static bool_t swapRowsReal      (real34Matrix_t *matrix)    { return _swapReal   (matrix, true);  }
  static bool_t swapColumnsReal   (real34Matrix_t *matrix)    { return _swapReal   (matrix, false); }
  static bool_t swapRowsComplex   (complex34Matrix_t *matrix) { return _swapComplex(matrix, true);  }
  static bool_t swapColumnsComplex(complex34Matrix_t *matrix) { return _swapComplex(matrix, false); }


  static bool_t getMatrixAny(any34Matrix_t *matrix, const matrixKind_t *kind) {
    real_t ry, rx, rrows, rcols;
    uint16_t a, b, r, c;

    const int16_t i = getIRegisterAsInt(true);
    const int16_t j = getJRegisterAsInt(true);

    int32ToReal(matrix->header.matrixRows    - i, &rrows);
    int32ToReal(matrix->header.matrixColumns - j, &rcols);

    if((!getArg(REGISTER_Y, &ry)) || (!getArg(REGISTER_X, &rx))) {
      return false;
    }

    a = realToInt32C47(&ry, NULL);
    b = realToInt32C47(&rx, NULL);
    if(realIsPositive(&rx) && realIsPositive(&ry) && realCompareLessEqual(&rx, &rcols) && realCompareLessEqual(&ry, &rrows)) {
      any34Matrix_t mat;
      fnDropY(NOPARAM);
      if(lastErrorCode == ERROR_NONE) {
        if(initMatrixRegister(REGISTER_X, a, b, kind->isComplex)) {
          const size_t size = kind->elementSize;
          uint8_t *src, *dst;
          kind->linkToRegister(REGISTER_X, &mat);
          src = MATRIX_BYTES(matrix);
          dst = MATRIX_BYTES(&mat);
          for(r = 0; r < a; ++r) {
            for(c = 0; c < b; ++c) {
              kind->copyElement(src + ((r + i) * matrix->header.matrixColumns + c + j)*size, dst + (r * b + c)*size);
            }
          }
        }
        else {
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "Ram full");
            moreInfoOnError("In function getMatrixAny:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return false;
        }
      }
    }
    else {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "%" PRIu16 " " STD_CROSS " %" PRIu16 " out of range", a, b);
        moreInfoOnError("In function getMatrixAny:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }

    return false;
  }

  static bool_t getMatrixReal(real34Matrix_t *matrix) {
    return getMatrixAny((any34Matrix_t*)matrix, &matrixKindReal);
  }

  static bool_t getMatrixComplex(complex34Matrix_t *matrix) {
    return getMatrixAny((any34Matrix_t*)matrix, &matrixKindComplex);
  }



  static bool_t putMatrixAny(any34Matrix_t *matrix, const matrixKind_t *kind) {
    uint16_t r, c;
    any34Matrix_t mat;
    const size_t size = kind->elementSize;
    uint8_t *src, *dst;

    const int16_t i = getIRegisterAsInt(true);
    const int16_t j = getJRegisterAsInt(true);

    if(getRegisterDataType(REGISTER_X) != kind->registerDataType) {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "%s is not a %s matrix", getRegisterDataTypeName(REGISTER_X, true, false), kind->isComplex ? "complex" : "real");
        moreInfoOnError("In function putMatrixAny:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }

    kind->linkToRegister(REGISTER_X, &mat);
    if((mat.header.matrixRows + i) <= matrix->header.matrixRows && (mat.header.matrixColumns + j) <= matrix->header.matrixColumns) {
      src = MATRIX_BYTES(&mat);
      dst = MATRIX_BYTES(matrix);
      for(r = 0; r < mat.header.matrixRows; ++r) {
        for(c = 0; c < mat.header.matrixColumns; ++c) {
          kind->copyElement(src + (r * mat.header.matrixColumns + c)*size, dst + ((r + i) * matrix->header.matrixColumns + c + j)*size);
        }
      }
    }
    else {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "%" PRIu16 " " STD_CROSS " %" PRIu16 " out of range", mat.header.matrixRows, mat.header.matrixColumns);
        moreInfoOnError("In function putMatrixAny:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
    }

    return true;
  }

  static bool_t putMatrixReal(real34Matrix_t *matrix) {
    return putMatrixAny((any34Matrix_t*)matrix, &matrixKindReal);
  }

  static bool_t putMatrixComplex(complex34Matrix_t *matrix) {
    return putMatrixAny((any34Matrix_t*)matrix, &matrixKindComplex);
  }


void fnNewMatrix(uint16_t unusedParamButMandatory) {
  uint32_t rows, cols;

  if(!getDimensionArg(&rows, &cols)) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  //Initialize Memory for Matrix
  if(initMatrixRegister(REGISTER_X, rows, cols, false)) {
    setSystemFlag(FLAG_ASLIFT);
  }
  else {
    displayCalcErrorMessage(ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Not enough memory for a %" PRIu32 STD_CROSS "%" PRIu32 " matrix", rows, cols);
      moreInfoOnError("In function fnNewMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  adjustResult(REGISTER_X, true, false, REGISTER_X, REGISTER_Y, -1);
}


bool_t saveStatsMatrix(void) {
  uint32_t rows, cols;
  calcRegister_t regStats = findNamedVariable("STATS");

  #if defined(DEBUGUNDO)
    printf(">>> saveStatsMatrix\n");
    printRegisterToConsole(regStats, "...STATS:\n", "\n");
    printRegisterToConsole(TEMP_REGISTER_2_SAVED_STATS, "...pre-save: TEMP_REGISTER_2_SAVED_STATS:\n", "\n");
  #endif // DEBUGUNDO

  if(regStats != INVALID_VARIABLE) {
    if(getRegisterDataType(regStats) == dtReal34Matrix) {
      rows = REGISTER_MATRIX_HEADER(regStats)->matrixRows;
      cols = REGISTER_MATRIX_HEADER(regStats)->matrixColumns;

      //Initialize Memory for Matrix
      if(initMatrixRegister(TEMP_REGISTER_2_SAVED_STATS, rows, cols, false)) {
        copySourceRegisterToDestRegister(regStats, TEMP_REGISTER_2_SAVED_STATS);
        #if defined(DEBUGUNDO)
          printf(">>>saveStatsMatrix:    backing up STATS matrix containing %i rows and %i columns\n", rows, cols);
          regStats = findNamedVariable("STATS");
          printRegisterToConsole(regStats, "...STATS:\n", "\n");
          printRegisterToConsole(TEMP_REGISTER_2_SAVED_STATS, "post save: ...TEMP_REGISTER_2_SAVED_STATS:\n", "\n");
        #endif // DEBUGUNDO
        return true; //backed up
      }
      else {
        displayCalcErrorMessage(ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Not enough memory for STATS undo matrix: rows=%i, cols=%i", rows, cols);
          moreInfoOnError("In function saveStatsMatrix:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
    }
    else {
      return true; //nothing to backup
    }
  }
  else {
    return true; //nothing to backup
  }
}


bool_t recallStatsMatrix(void) {
  #if defined(DEBUGUNDO)
    printf(">>> recallStatsMatrix ...");
    printRegisterToConsole(TEMP_REGISTER_2_SAVED_STATS, "From: recallStatsMatrix: TEMP_REGISTER_2_SAVED_STATS\n", "");
  #endif //DEBUGUNDO
  uint32_t rows, cols;

  calcRegister_t regStats = TEMP_REGISTER_2_SAVED_STATS;

  if(getRegisterDataType(regStats) == dtReal34Matrix) {
    rows = REGISTER_MATRIX_HEADER(regStats)->matrixRows;
    cols = REGISTER_MATRIX_HEADER(regStats)->matrixColumns;
    if(cols == 2 && rows >= 1) {
      regStats = findNamedVariable("STATS");
      if(regStats == INVALID_VARIABLE) {
        allocateNamedVariable("STATS", dtReal34, REAL34_SIZE_IN_BLOCKS);
        regStats = findNamedVariable("STATS");
      }
      #if defined(DEBUGUNDO)
        printf("Clearing STATS\n");
      #endif // DEBUGUNDO
      clearRegister(regStats);

      #if defined(DEBUGUNDO)
        printf("Trying to initialize matrix: R=%d C=%d into register: %d\n", rows, cols, regStats);
      #endif // DEBUGUNDO
      //Initialize Memory for Matrix
      if(initMatrixRegister(regStats, rows, cols, false)) {
        #if defined(DEBUGUNDO)
          printf(">>>    restoring STATS matrix containing %i rows and %i columns\n", rows, cols);
        #endif // DEBUGUNDO
        copySourceRegisterToDestRegister(TEMP_REGISTER_2_SAVED_STATS, regStats);
        clearRegister(TEMP_REGISTER_2_SAVED_STATS);
        return true; //restored
      }
      else {
        displayCalcErrorMessage(ERROR_TI_UNDO_FAILED, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Creation of STATS matrix from TEMP2 failed, likely due to insufficient memory: rows=%i, cols=%i", rows, cols);
          moreInfoOnError("In function recallStatsMatrix:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false; //not enough memory
      }
    }
    else {
      return false; //TEMP_REGISTER_2_SAVED_STATS is a non-STATS dimensioned register
    }
  }
  else {
    return false; //TEMP_REGISTER_2_SAVED_STATS is no real 34 matrix
  }
}


static void _SetMatrixDimensions(uint16_t regist, uint16_t dimMode) {
  uint32_t y, x;

  if(!getDimensionArg(&y, &x)) {
  }
  else if(redimMatrixRegister(regist, y, x, dimMode)) {
  }
  else {
    displayCalcErrorMessage(ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Not enough memory for a %" PRIu32 STD_CROSS "%" PRIu32 " matrix", y, x);
      moreInfoOnError("In function _SetMatrixDimensions:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
}


void fnSetMatrixDimensions(uint16_t regist) {
  _SetMatrixDimensions(regist, ITM_M_DIM);
}

void fnSetMatrixDimensionsGr(uint16_t regist) {
  _SetMatrixDimensions(regist, ITM_M_DIM_GR);
}


static void getMatrixDimensionsToStack(uint16_t regist, bool_t consumeX) {
  if(getRegisterDataType(regist) == dtReal34Matrix || getRegisterDataType(regist) == dtComplex34Matrix) {
    const uint16_t rows = REGISTER_MATRIX_HEADER(regist)->matrixRows;
    const uint16_t cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
    longInteger_t li;

    if(consumeX) {
      fnDrop(NOPARAM);
    }
    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
    longIntegerInit(li);
    uInt32ToLongInteger(rows, li);
    convertLongIntegerToLongIntegerRegister(li, REGISTER_Y);
    uInt32ToLongInteger(cols, li);
    convertLongIntegerToLongIntegerRegister(li, REGISTER_X);
    longIntegerFree(li);
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(regist));
      moreInfoOnError("In function fnGetMatrixDimensions:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

void fnGetMatrixDimensions(uint16_t regist) {
  if(regist == REGISTER_X && !saveLastX()) {
    return;
  }
  getMatrixDimensionsToStack(regist, false);
}

void fnGetMatrixDimensions42(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }
  getMatrixDimensionsToStack(REGISTER_X, true);
}


void fnTranspose(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    real34Matrix_t x;

    linkToRealMatrixRegister(REGISTER_X, &x);
    transposeRealMatrix(&x, &x);
    REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows    = x.header.matrixRows;
    REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns = x.header.matrixColumns;
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complex34Matrix_t x;

    linkToComplexMatrixRegister(REGISTER_X, &x);
    transposeComplexMatrix(&x, &x);
    REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows    = x.header.matrixRows;
    REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns = x.header.matrixColumns;
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
      moreInfoOnError("In function fnTranspose:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}


void fnLuDecomposition(uint16_t unusedParamButMandatory) {
  if(!saveLastX()) {
    return;
  }

  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    real34Matrix_t x, l, u;
    uint16_t *p, i, j;

    convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a square matrix (%d" STD_CROSS "%d)", x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnLuDecomposition:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      if((p = allocC47Blocks(x.header.matrixRows * sizeof(uint16_t)))) {
        WP34S_LU_decomposition(&x, &l, p);
        if(l.matrixElements) {
          copyRealMatrix(&l, &u);
          if(u.matrixElements) {
            for(i = 0; i < l.header.matrixRows; ++i) {
              for(j = i; j < l.header.matrixColumns; ++j) {
                real34Copy(i == j ? const34_1 : const34_0, l.matrixElements + (i * l.header.matrixColumns + j));
              }
            }
            for(i = 1; i < u.header.matrixRows; ++i) {
              for(j = 0; j < i; ++j) {
                real34SetZero(u.matrixElements + (i * u.header.matrixColumns + j));
              }
            }
            realMatrixFree(&x);
            realMatrixIdentity(&x, l.header.matrixColumns);
            for(uint16_t i = 0; i < l.header.matrixColumns; ++i) {
              realMatrixSwapRows(&x, &x, i, p[i]);
            }
            transposeRealMatrix(&x, &x);
            liftStack();
            setSystemFlag(FLAG_ASLIFT);
            liftStack();
            convertReal34MatrixToReal34MatrixRegister(&x, REGISTER_Z);
            if(lastErrorCode == ERROR_NONE) {
              convertReal34MatrixToReal34MatrixRegister(&l, REGISTER_Y);
              if(lastErrorCode == ERROR_NONE) {
                convertReal34MatrixToReal34MatrixRegister(&u, REGISTER_X);
              }
            }
            setSystemFlag(FLAG_ASLIFT);
            realMatrixFree(&u);
          }
          realMatrixFree(&l);
        }
        else {
          displayCalcErrorMessage(ERROR_SINGULAR_MATRIX, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "attempt to LU-decompose a singular matrix");
            moreInfoOnError("In function fnLuDecomposition:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
        freeC47Blocks(p, x.header.matrixRows * sizeof(uint16_t));
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 1a");
          moreInfoOnError("In function fnLuDecomposition:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }

    realMatrixFree(&x);
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complex34Matrix_t x, l, u;
    real34Matrix_t pivot;
    uint16_t *p, i, j;

    convertComplex34MatrixRegisterToComplex34Matrix(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a square matrix (%d" STD_CROSS "%d)", x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnLuDecomposition:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      if((p = allocC47Blocks(x.header.matrixRows * sizeof(uint16_t)))) {
        complex_LU_decomposition(&x, &l, p);
        if(l.matrixElements) {
          copyComplexMatrix(&l, &u);
          if(u.matrixElements) {
            for(i = 0; i < l.header.matrixRows; ++i) {
              for(j = i; j < l.header.matrixColumns; ++j) {
                real34Copy(i == j ? const34_1 : const34_0, VARIABLE_REAL34_DATA(l.matrixElements + (i * l.header.matrixColumns + j)));
                real34SetZero(                                VARIABLE_IMAG34_DATA(l.matrixElements + (i * l.header.matrixColumns + j)));
              }
            }
            for(i = 1; i < u.header.matrixRows; ++i) {
              for(j = 0; j < i; ++j) {
                real34SetZero(VARIABLE_REAL34_DATA(u.matrixElements + (i * u.header.matrixColumns + j)));
                real34SetZero(VARIABLE_IMAG34_DATA(u.matrixElements + (i * u.header.matrixColumns + j)));
              }
            }
            realMatrixIdentity(&pivot, l.header.matrixColumns);
            if(lastErrorCode == ERROR_NONE) {
              for(uint16_t i = 0; i < l.header.matrixColumns; ++i) {
                realMatrixSwapRows(&pivot, &pivot, i, p[i]);
              }
              transposeRealMatrix(&pivot, &pivot);
              liftStack();
              setSystemFlag(FLAG_ASLIFT);
              liftStack();
              setSystemFlag(FLAG_ASLIFT);
              convertReal34MatrixToReal34MatrixRegister(&pivot, REGISTER_Z);
              if(lastErrorCode == ERROR_NONE) {
                convertComplex34MatrixToComplex34MatrixRegister(&l, REGISTER_Y);
                if(lastErrorCode == ERROR_NONE) {
                  convertComplex34MatrixToComplex34MatrixRegister(&u, REGISTER_X);
                }
              }
              realMatrixFree(&pivot);
            }
            else {
              displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                sprintf(errorMessage, "Ram full, 2b");
                moreInfoOnError("In function fnLuDecomposition:", errorMessage, NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            }
            complexMatrixFree(&u);
          }
          else {
            displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Ram full, 3c");
              moreInfoOnError("In function fnLuDecomposition:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
          complexMatrixFree(&l);
        }
        else {
          displayCalcErrorMessage(ERROR_SINGULAR_MATRIX, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "attempt to LU-decompose a singular matrix");
            moreInfoOnError("In function fnLuDecomposition:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
        freeC47Blocks(p, x.header.matrixRows * sizeof(uint16_t));
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 4d");
          moreInfoOnError("In function fnLuDecomposition:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }

    complexMatrixFree(&x);
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
      moreInfoOnError("In function fnLuDecomposition:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}


void fnDeterminant(uint16_t unusedParamButMandatory) {
  if(!saveLastX()) {
    return;
  }

  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    real34Matrix_t x;
    real34_t res;

    linkToRealMatrixRegister(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a square matrix (%d" STD_CROSS "%d)",
                x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnDeterminant:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      detRealMatrix(&x, &res);
      if(lastErrorCode == ERROR_NONE) {
        convertReal34ToReal34Register(&res, REGISTER_X);
      }
    }
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complex34Matrix_t x;
    real34_t res_r, res_i;

    linkToComplexMatrixRegister(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a square matrix (%d" STD_CROSS "%d)", x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnDeterminant:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      detComplexMatrix(&x, &res_r, &res_i);
      if(lastErrorCode == ERROR_NONE) {
        convertReal34ToComplex34Register(&res_r, &res_i, REGISTER_X);
      }
    }
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
      moreInfoOnError("In function fnLuDecomposition:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}


void fnInvertMatrix(uint16_t unusedParamButMandatory) {
  if(!saveLastX()) {
    return;
  }

  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    real34Matrix_t x, res;

    linkToRealMatrixRegister(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a square matrix (%d" STD_CROSS "%d)", x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnInvertMatrix:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      invertRealMatrix(&x, &res);
      if(lastErrorCode == ERROR_NONE) {
        if(res.matrixElements) {
          convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
          realMatrixFree(&res);
          setSystemFlag(FLAG_ASLIFT);
        }
        else {
          displayCalcErrorMessage(ERROR_SINGULAR_MATRIX, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "attempt to invert a singular matrix");
            moreInfoOnError("In function fnInvertMatrix:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
      }
      else {
        temporaryInformation = TI_NO_INFO;
        if(programRunStop == PGM_WAITING) {
          programRunStop = PGM_STOPPED;
        }
      }
    }
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complex34Matrix_t x, res;

    linkToComplexMatrixRegister(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a square matrix (%d" STD_CROSS "%d)", x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnInvertMatrix:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      invertComplexMatrix(&x, &res);
      if(lastErrorCode == ERROR_NONE) {
        if(res.matrixElements) {
          convertComplex34MatrixToComplex34MatrixRegister(&res, REGISTER_X);
          complexMatrixFree(&res);
          setSystemFlag(FLAG_ASLIFT);
        }
        else {
          displayCalcErrorMessage(ERROR_SINGULAR_MATRIX, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "attempt to invert a singular matrix");
            moreInfoOnError("In function fnInvertMatrix:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
      }
      else {
        temporaryInformation = TI_NO_INFO;
        if(programRunStop == PGM_WAITING) {
          programRunStop = PGM_STOPPED;
        }
      }
    }
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
      moreInfoOnError("In function fnInvertMatrix:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}


#if defined(OPTION_EIGEN)
void fnMatrixSquareRoot(uint16_t unusedParamButMandatory) {
  if(!saveLastX()) {
    return;
  }

  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    real34Matrix_t x, res;
    linkToRealMatrixRegister(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a square matrix (%d" STD_CROSS "%d)", x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnMatrixSquareRoot:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      sqrtRealMatrix(&x, &res);
      if(lastErrorCode == ERROR_NONE) {
        if(res.matrixElements) {
          convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
          realMatrixFree(&res);
          setSystemFlag(FLAG_ASLIFT);
        }
        else if(getSystemFlag(FLAG_CPXRES)) {  // Real path failed, promote to complex and solve
          complex34Matrix_t cx, cres;
          convertReal34MatrixToComplex34Matrix(&x, &cx);
          if(cx.matrixElements) {
            sqrtComplexMatrix(&cx, &cres);
            complexMatrixFree(&cx);
            if(lastErrorCode == ERROR_NONE) {
              if(cres.matrixElements) {                                                      // Auto-promote produced a complex result. If all imag parts are zero store as real matrix
                bool_t allRealResult = true;
                const uint16_t total = cres.header.matrixRows * cres.header.matrixColumns;
                for(uint16_t i = 0; i < total; ++i) {
                  if(!real34IsZero(VARIABLE_IMAG34_DATA(cres.matrixElements + i))) {
                    allRealResult = false;
                    break;
                  }
                }
                if(allRealResult) {
                  real34Matrix_t rres;
                  if(realMatrixInit(&rres, cres.header.matrixRows, cres.header.matrixColumns)) {
                    for(uint16_t i = 0; i < total; ++i) {
                      real34Copy(VARIABLE_REAL34_DATA(cres.matrixElements + i), rres.matrixElements + i);
                    }
                    convertReal34MatrixToReal34MatrixRegister(&rres, REGISTER_X);
                    realMatrixFree(&rres);
                  }
                  else {
                    convertComplex34MatrixToComplex34MatrixRegister(&cres, REGISTER_X);      // Insufficient RAM for downgrade - keep complex
                  }
                }
                else {
                  convertComplex34MatrixToComplex34MatrixRegister(&cres, REGISTER_X);
                }
                complexMatrixFree(&cres);
                setSystemFlag(FLAG_ASLIFT);
              }
              else {
                displayCalcErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE);
                #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                  sprintf(errorMessage, "matrix has no square root, or iteration failed to converge");
                  moreInfoOnError("In function fnMatrixSquareRoot:", errorMessage, NULL, NULL);
                #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
              }
            }
            else {
              temporaryInformation = TI_NO_INFO;
              if(programRunStop == PGM_WAITING) {
                programRunStop = PGM_STOPPED;
              }
            }
          }
          // else: convert failed and already raised RAM_FULL
        }
        else {
          displayCalcErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "matrix has no real square root, or iteration failed to converge");
            moreInfoOnError("In function fnMatrixSquareRoot:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
      }
      else {
        temporaryInformation = TI_NO_INFO;
        if(programRunStop == PGM_WAITING) {
          programRunStop = PGM_STOPPED;
        }
      }
    }
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complex34Matrix_t x, res;
    linkToComplexMatrixRegister(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a square matrix (%d" STD_CROSS "%d)", x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnMatrixSquareRoot:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      sqrtComplexMatrix(&x, &res);
      if(lastErrorCode == ERROR_NONE) {
        if(res.matrixElements) {
          convertComplex34MatrixToComplex34MatrixRegister(&res, REGISTER_X);
          complexMatrixFree(&res);
          setSystemFlag(FLAG_ASLIFT);
        }
        else {
          displayCalcErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "matrix has no square root, or iteration failed to converge");
            moreInfoOnError("In function fnMatrixSquareRoot:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
      }
      else {
        temporaryInformation = TI_NO_INFO;
        if(programRunStop == PGM_WAITING) {
          programRunStop = PGM_STOPPED;
        }
      }
    }
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
      moreInfoOnError("In function fnMatrixSquareRoot:", errorMessage, "is not a (real or complex) matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}


#endif // OPTION_EIGEN


static void _fnEuclideanNorm(uint16_t pParam) {
  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    real34Matrix_t matrix;
    real34_t sum;

    linkToRealMatrixRegister(REGISTER_X, &matrix);

    euclideanNormRealMatrix(&matrix, pParam, &sum);

    // `matrix` invalidates here
    convertReal34ToReal34Register(&sum, REGISTER_X);
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complex34Matrix_t matrix;
    real34_t sum;

    linkToComplexMatrixRegister(REGISTER_X, &matrix);

    euclideanNormComplexMatrix(&matrix, pParam, &sum);

    // `matrix` invalidates here
    convertReal34ToReal34Register(&sum, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
      moreInfoOnError("In function _fnEuclideanNorm:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}


/**
 * \brief Calculate the p39 magnitude of one matrix-element difference.
 *
 * Real matrices supply an exact zero imaginary component.  Keeping both component differences wide avoids a Real34 conversion between subtraction and magnitude.
 *
 * \param[in] yType Data type of the minuend matrix.
 * \param[in] xType Data type of the subtrahend matrix.
 * \param[in] element Zero-based element index.
 * \param[out] magnitude Magnitude of the element difference.
 */
static void matrixElementDifferenceMagnitude(uint32_t yType, uint32_t xType, uint16_t element, real_t *magnitude) {
  cplx_t yElement, xElement;

  if(yType == dtReal34Matrix) {
    real34ToReal(REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_Y) + element, &yElement.Real);
    realSetZero(&yElement.Imag);
  }
  else {
    const complex34_t *y = REGISTER_COMPLEX34_MATRIX_ELEMENTS(REGISTER_Y) + element;
    real34ToReal(VARIABLE_REAL34_DATA(y), &yElement.Real);
    real34ToReal(VARIABLE_IMAG34_DATA(y), &yElement.Imag);
  }

  if(xType == dtReal34Matrix) {
    real34ToReal(REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_X) + element, &xElement.Real);
    realSetZero(&xElement.Imag);
  }
  else {
    const complex34_t *x = REGISTER_COMPLEX34_MATRIX_ELEMENTS(REGISTER_X) + element;
    real34ToReal(VARIABLE_REAL34_DATA(x), &xElement.Real);
    real34ToReal(VARIABLE_IMAG34_DATA(x), &xElement.Imag);
  }

  realSubtract(&yElement.Real, &xElement.Real, &yElement.Real, &ctxtReal39);
  if(yType == dtReal34Matrix && xType == dtReal34Matrix) {
    realSetPositiveSign(&yElement.Real);
    realCopy(&yElement.Real, magnitude);
  }
  else {
    realSubtract(&yElement.Imag, &xElement.Imag, &yElement.Imag, &ctxtReal39);
    complexMagnitude(&yElement, magnitude, &ctxtReal39);
  }
}


/**
 * \brief Calculate the Euclidean distance between two equally shaped real or complex matrices.
 *
 * Component differences, scaling, the non-negative sum, and the closing square root remain at p39 until the public Real34 conversion.
 *
 * \param[in] yType Data type of the minuend matrix.
 * \param[in] xType Data type of the subtrahend matrix.
 * \param[in] elements Number of elements in each matrix.
 * \param[out] distance P39 Euclidean distance.
 */
static void matrixEuclideanDistance(uint32_t yType, uint32_t xType, uint16_t elements, real_t *distance) {
  REAL_T_PTR(elementMagnitude, 39);
  REAL_T_PTR(scale, 39);
  REAL_T_PTR(sum, 39);

  realSetZero(scale);
  for(uint16_t i = 0; i < elements; ++i) {
    matrixElementDifferenceMagnitude(yType, xType, i, elementMagnitude);
    if(realIsNaN(elementMagnitude)) {
      realCopy(elementMagnitude, distance);
      return;
    }
    if(realCompareGreaterThan(elementMagnitude, scale)) {
      realCopy(elementMagnitude, scale);
    }
  }
  if(realIsInfinite(scale) || realIsZero(scale)) {
    realCopy(scale, distance);
    return;
  }

  realSetZero(sum);
  for(uint16_t i = 0; i < elements; ++i) {
    matrixElementDifferenceMagnitude(yType, xType, i, elementMagnitude);
    realDivide(elementMagnitude, scale, elementMagnitude, &ctxtReal39);
    realFMA(elementMagnitude, elementMagnitude, sum, sum, &ctxtReal39);
  }
  realSquareRoot(sum, distance, &ctxtReal39);
  realMultiply(scale, distance, distance, &ctxtReal39);
}


/**
 * \brief Return the Euclidean distance between equally shaped real or complex matrices.
 *
 * \param[in] unusedParamButMandatory Unused catalogue parameter.
 */
void fnVectorDist(uint16_t unusedParamButMandatory) {
  const uint32_t xType = getRegisterDataType(REGISTER_X);
  const uint32_t yType = getRegisterDataType(REGISTER_Y);
  REAL_T_PTR(distance, 39);

  if((xType != dtReal34Matrix && xType != dtComplex34Matrix) || (yType != dtReal34Matrix && yType != dtComplex34Matrix)) {
    fnSubtract(NOPARAM);
    _fnEuclideanNorm(2);
    return;
  }

  if(!saveLastX()) {
    return;
  }

  const matrixHeader_t *xHeader = REGISTER_MATRIX_HEADER(REGISTER_X);
  const matrixHeader_t *yHeader = REGISTER_MATRIX_HEADER(REGISTER_Y);
  if(xHeader->matrixRows != yHeader->matrixRows || xHeader->matrixColumns != yHeader->matrixColumns) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate the distance between %d" STD_CROSS "%d and %d" STD_CROSS "%d matrices",
              yHeader->matrixRows, yHeader->matrixColumns, xHeader->matrixRows, xHeader->matrixColumns);
      moreInfoOnError("In function fnVectorDist:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    matrixEuclideanDistance(yType, xType, xHeader->matrixRows * xHeader->matrixColumns, distance);
    convertRealToResultRegister(distance, REGISTER_X, amNone);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}




// isRow == 1: row sum (result column vector); isRow == 0: column sum (result row vector)
void fnRowColSum(uint16_t isRow) {
  if(!saveLastX()) {
    return;
  }

  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    real34Matrix_t x, res;
    realContext_t *sumContext;
    REAL_T_PTR(sum, 75);
    REAL_T_PTR(correction, 75);
    REAL_T_PTR(absoluteSum, 75);
    REAL_T_PTR(elem, 75);
    uint16_t outerLimit, innerLimit, cols;
    linkToRealMatrixRegister(REGISTER_X, &x);
    cols       = x.header.matrixColumns;
    outerLimit = isRow ? x.header.matrixRows : cols;
    innerLimit = isRow ? cols                : x.header.matrixRows;
    if(realMatrixInit(&res, isRow ? outerLimit : 1, isRow ? 1 : outerLimit)) {
      for(uint16_t i = 0; i < outerLimit; ++i) {
        sumContext = &ctxtReal39;
        do {
          realSetZero(sum);
          realSetZero(correction);
          realSetZero(absoluteSum);
          for(uint16_t j = 0; j < innerLimit; ++j) {
            real34ToReal(x.matrixElements + (isRow ? i * cols + j : j * cols + i), elem);
            matrixAccumulateValue(elem, sum, correction, absoluteSum, sumContext);
          }
          matrixFinishAccumulator(sum, correction, sum, sumContext);
          if(sumContext == &ctxtReal39 && matrixNeedsExtendedAccumulator(absoluteSum, sum)) {
            sumContext = &ctxtReal75;
          }
          else {
            break;
          }
        } while(true);
        realToReal34(sum, res.matrixElements + i);
      }

      convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
      realMatrixFree(&res);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 1e");
        moreInfoOnError("In function fnRowColSum:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complex34Matrix_t x, res;
    realContext_t *sumContext;
    REAL_T_PTR(sumr, 75);
    REAL_T_PTR(sumi, 75);
    REAL_T_PTR(correctionr, 75);
    REAL_T_PTR(correctioni, 75);
    REAL_T_PTR(absoluteSumr, 75);
    REAL_T_PTR(absoluteSumi, 75);
    REAL_T_PTR(elem, 75);
    uint16_t outerLimit, innerLimit, cols, idx;
    linkToComplexMatrixRegister(REGISTER_X, &x);
    cols       = x.header.matrixColumns;
    outerLimit = isRow ? x.header.matrixRows : cols;
    innerLimit = isRow ? cols                : x.header.matrixRows;
    if(complexMatrixInit(&res, isRow ? outerLimit : 1, isRow ? 1 : outerLimit)) {
      for(uint16_t i = 0; i < outerLimit; ++i) {
        sumContext = &ctxtReal39;
        do {
          realSetZero(sumr);
          realSetZero(sumi);
          realSetZero(correctionr);
          realSetZero(correctioni);
          realSetZero(absoluteSumr);
          realSetZero(absoluteSumi);
          for(uint16_t j = 0; j < innerLimit; ++j) {
            idx = isRow ? i * cols + j : j * cols + i;
            real34ToReal(VARIABLE_REAL34_DATA(x.matrixElements + idx), elem);
            matrixAccumulateValue(elem, sumr, correctionr, absoluteSumr, sumContext);
            real34ToReal(VARIABLE_IMAG34_DATA(x.matrixElements + idx), elem);
            matrixAccumulateValue(elem, sumi, correctioni, absoluteSumi, sumContext);
          }
          matrixFinishAccumulator(sumr, correctionr, sumr, sumContext);
          matrixFinishAccumulator(sumi, correctioni, sumi, sumContext);
          if(sumContext == &ctxtReal39 && (matrixNeedsExtendedAccumulator(absoluteSumr, sumr) || matrixNeedsExtendedAccumulator(absoluteSumi, sumi))) {
            sumContext = &ctxtReal75;
          }
          else {
            break;
          }
        } while(true);
        realToReal34(sumr, VARIABLE_REAL34_DATA(res.matrixElements + i));
        realToReal34(sumi, VARIABLE_IMAG34_DATA(res.matrixElements + i));
      }

      convertComplex34MatrixToComplex34MatrixRegister(&res, REGISTER_X);
      complexMatrixFree(&res);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 2f");
        moreInfoOnError("In function fnRowColSum:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
      moreInfoOnError("In function fnRowColSum:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}



// pParam == pNorm_0_NNZ     : count of non-zero elements (whole matrix/vector)
// pParam == pNorm_inf_RNORM : rowNorm;
// pParam == pNorm_1_CNORM   : columnNorm
static void _row_columnNorm(uint16_t pParam) {
  if(pParam != pNorm_inf_RNORM && pParam != pNorm_1_CNORM && pParam != pNorm_0_NNZ) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "pParam %" PRIu32, pParam);
      moreInfoOnError("In function _row_columnNorm:", errorMessage, "incorrect operating option.", "");
    #endif // EXTRA_INFO_ON_CALC_ERROR
    return;
  }

  if(pParam == pNorm_0_NNZ) {
    if(!saveLastX()) {
      return;
    }
    if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
      real34Matrix_t x;
      real_t nnz;
      uint16_t nnzi = 0;
      linkToRealMatrixRegister(REGISTER_X, &x);
      uint16_t n = x.header.matrixRows * x.header.matrixColumns;
      for(uint16_t k = 0; k < n; ++k) {
        if(!real34IsZero(x.matrixElements + k)) {
          nnzi++;
        }
      }
      uInt32ToReal(nnzi, &nnz);
      convertRealToResultRegister(&nnz, REGISTER_X, amNone);
    }
    else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
      complex34Matrix_t x;
      real_t nnz;
      uint16_t nnzi = 0;
      linkToComplexMatrixRegister(REGISTER_X, &x);
      uint16_t n = x.header.matrixRows * x.header.matrixColumns;
      for(uint16_t k = 0; k < n; ++k) {
        if(!real34IsZero(VARIABLE_REAL34_DATA(x.matrixElements + k)) || !real34IsZero(VARIABLE_IMAG34_DATA(x.matrixElements + k))) {
          nnzi++;
        }
      }
      uInt32ToReal(nnzi, &nnz);
      convertRealToResultRegister(&nnz, REGISTER_X, amNone);
    }
    else {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
        moreInfoOnError("In function _row_columnNorm:", errorMessage, "is not a matrix.", "");
      #endif // EXTRA_INFO_ON_CALC_ERROR
    }
    adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
    return;
  }

  bool_t isRow = (pParam == pNorm_inf_RNORM);
  if(!saveLastX()) {
    return;
  }
  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    real34Matrix_t x;
    real_t norm, sum, elem;
    uint16_t outerLimit, innerLimit, cols, idx;
    linkToRealMatrixRegister(REGISTER_X, &x);
    cols = x.header.matrixColumns;
    outerLimit = isRow ? x.header.matrixRows : cols;
    innerLimit = isRow ? cols                : x.header.matrixRows;
    realSetZero(&norm);
    for(uint16_t i = 0; i < outerLimit; ++i) {
      realSetZero(&sum);
      for(uint16_t j = 0; j < innerLimit; ++j) {
        idx = isRow ? i * cols + j : j * cols + i;
        real34ToReal(x.matrixElements + idx, &elem);
        realSetPositiveSign(&elem);
        realAdd(&sum, &elem, &sum, &ctxtReal39);
      }
      if(realIsNaN(&sum)) {
        realCopy(&sum, &norm);
        break;
      }
      else if(realCompareGreaterThan(&sum, &norm)) {
        realCopy(&sum, &norm);
      }
    }

    convertRealToResultRegister(&norm, REGISTER_X, amNone);
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complex34Matrix_t x;
    real_t norm, sum;
    cplx_t elem;
    uint16_t outerLimit, innerLimit, cols, idx;
    linkToComplexMatrixRegister(REGISTER_X, &x);
    cols = x.header.matrixColumns;
    outerLimit = isRow ? x.header.matrixRows : cols;
    innerLimit = isRow ? cols                : x.header.matrixRows;
    realSetZero(&norm);
    for(uint16_t i = 0; i < outerLimit; ++i) {
      realSetZero(&sum);
      for(uint16_t j = 0; j < innerLimit; ++j) {
        idx = isRow ? i * cols + j : j * cols + i;
        real34ToReal(VARIABLE_REAL34_DATA(x.matrixElements + idx), &elem.Real);
        real34ToReal(VARIABLE_IMAG34_DATA(x.matrixElements + idx), &elem.Imag);
        complexMagnitude(&elem, &elem.Real, &ctxtReal39);
        realAdd(&sum, &elem.Real, &sum, &ctxtReal39);
      }
      if(realIsNaN(&sum)) {
        realCopy(&sum, &norm);
        break;
      }
      else if(realCompareGreaterThan(&sum, &norm)) {
        realCopy(&sum, &norm);
      }
    }

    convertRealToResultRegister(&norm, REGISTER_X, amNone);
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
      moreInfoOnError("In function _row_columnNorm:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}



void fnPNorm(uint16_t param) {
  switch(param) {
    case pNorm_0_NNZ    :
    case pNorm_1_CNORM  :
    case pNorm_inf_RNORM: _row_columnNorm(param); break;
    case pNorm_2_ENORM  :
    default: if(saveLastX()) {
               _fnEuclideanNorm(param);
             }
  }
}




void fnVectorAngle(uint16_t unusedParamButMandatory) {
  if(!saveLastX()) {
    return;
  }

  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix && getRegisterDataType(REGISTER_Y) == dtReal34Matrix) {
    real34Matrix_t x, y;
    REAL_T_PTR(res, 39);
    linkToRealMatrixRegister(REGISTER_X, &x);
    linkToRealMatrixRegister(REGISTER_Y, &y);

    if((realVectorSize(&y) < 2) || (realVectorSize(&x) < 2) || (realVectorSize(&y) > 3) || (realVectorSize(&x) > 3) || (realVectorSize(&y) != realVectorSize(&x))) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "invalid numbers of elements of %d" STD_CROSS "%d-matrix to %d" STD_CROSS "%d-matrix", x.header.matrixRows, x.header.matrixColumns, y.header.matrixRows, y.header.matrixColumns);
        moreInfoOnError("In function fnVectorAngle:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      vectorAngle(&y, &x, res);
      convertAngleFromTo(res, amRadian, currentAngularMode, &ctxtReal39);
      reallocateRegister(REGISTER_X, dtReal34, 0, currentAngularMode);
      convertRealToReal34ResultRegister(res, REGISTER_X);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
      moreInfoOnError("In function fnVectorAngle:", errorMessage, "is not a real matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}


bool_t isMatrixIndexed(void) {
  return ((matrixIndex != INVALID_VARIABLE) && (isRegInRange(matrixIndex)) && (getRegisterDataType(matrixIndex) == dtReal34Matrix || getRegisterDataType(matrixIndex) == dtComplex34Matrix));
}


void fnIndexMatrix(uint16_t regist) {
  if((getRegisterDataType(regist) == dtReal34Matrix) || (getRegisterDataType(regist) == dtComplex34Matrix)) {
    matrixIndex = regist;
    setIRegisterAsInt(false, 1);
    setJRegisterAsInt(false, 1);
    clearSystemFlag(FLAG_WRAPEDG);
    clearSystemFlag(FLAG_WRAPEND);
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(regist));
    moreInfoOnError("In function fnIndexMatrix:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnGetMatrix(uint16_t unusedParamButMandatory) {
  callByIndexedMatrix(getMatrixReal, getMatrixComplex);
}


void fnPutMatrix(uint16_t unusedParamButMandatory) {
  callByIndexedMatrix(putMatrixReal, putMatrixComplex);
}


void fnSwapRows(uint16_t unusedParamButMandatory) {
  callByIndexedMatrix(swapRowsReal, swapRowsComplex);
}

void fnSwapColumns(uint16_t unusedParamButMandatory) {
  callByIndexedMatrix(swapColumnsReal, swapColumnsComplex);
}


void fnSimultaneousLinearEquation(uint16_t numberOfUnknowns) {
  if(allocateNamedMatrix("Mat_A", numberOfUnknowns, numberOfUnknowns) != INVALID_VARIABLE) {
    if(allocateNamedMatrix("Mat_B", numberOfUnknowns, 1) != INVALID_VARIABLE) {
      if(allocateNamedMatrix("Mat_X", numberOfUnknowns, 1) != INVALID_VARIABLE) {
        popSoftmenu();
        showSoftmenu(-MNU_SIMQ);
        showSoftmenu(-MNU_TAM);
        numberOfTamMenusToPop = 1;
      }
    }
  }
}


void fnEditLinearEquationMatrixA(uint16_t unusedParamButMandatory) {
  fnEditMatrix(findNamedVariable("Mat_A"));
}


void fnEditLinearEquationMatrixB(uint16_t unusedParamButMandatory) {
  fnEditMatrix(findNamedVariable("Mat_B"));
}


void fnEditLinearEquationMatrixX(uint16_t unusedParamButMandatory) {
  if(findNamedVariable("Mat_A") == INVALID_VARIABLE || findNamedVariable("Mat_B") == INVALID_VARIABLE || findNamedVariable("Mat_X") == INVALID_VARIABLE) {
    displayCalcErrorMessage(ERROR_UNDEF_SOURCE_VAR, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "At least one of Mat_A, Mat_B or Mat_X is missing");
      moreInfoOnError("In function fnEditLinearEquationMatrixX:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else if(getRegisterDataType(findNamedVariable("Mat_A")) == dtReal34Matrix && getRegisterDataType(findNamedVariable("Mat_B")) == dtReal34Matrix) {
    real34Matrix_t a, b, x;
    linkToRealMatrixRegister(findNamedVariable("Mat_A"), &a);
    linkToRealMatrixRegister(findNamedVariable("Mat_B"), &b);
      real_matrix_linear_eqn(&a, &b, &x);
    if(x.matrixElements) {
      convertReal34MatrixToReal34MatrixRegister(&x, findNamedVariable("Mat_X"));
      realMatrixFree(&x);
    }
  }
  else if(getRegisterDataType(findNamedVariable("Mat_A")) == dtReal34Matrix && getRegisterDataType(findNamedVariable("Mat_B")) == dtComplex34Matrix) {
    complex34Matrix_t a, b, x;
    convertReal34MatrixRegisterToComplex34Matrix(findNamedVariable("Mat_A"), &a);
    linkToComplexMatrixRegister(findNamedVariable("Mat_B"), &b);
    complex_matrix_linear_eqn(&a, &b, &x);
    if(x.matrixElements) {
      convertComplex34MatrixToComplex34MatrixRegister(&x, findNamedVariable("Mat_X"));
      complexMatrixFree(&x);
    }
    complexMatrixFree(&a);
  }
  else if(getRegisterDataType(findNamedVariable("Mat_A")) == dtComplex34Matrix && getRegisterDataType(findNamedVariable("Mat_B")) == dtReal34Matrix) {
    complex34Matrix_t a, b, x;
    linkToComplexMatrixRegister(findNamedVariable("Mat_A"), &a);
    convertReal34MatrixRegisterToComplex34Matrix(findNamedVariable("Mat_B"), &b);
    complex_matrix_linear_eqn(&a, &b, &x);
    if(x.matrixElements) {
      convertComplex34MatrixToComplex34MatrixRegister(&x, findNamedVariable("Mat_X"));
      complexMatrixFree(&x);
    }
    complexMatrixFree(&b);
  }
  else if(getRegisterDataType(findNamedVariable("Mat_A")) == dtComplex34Matrix && getRegisterDataType(findNamedVariable("Mat_B")) == dtComplex34Matrix) {
    complex34Matrix_t a, b, x;
    linkToComplexMatrixRegister(findNamedVariable("Mat_A"), &a);
    linkToComplexMatrixRegister(findNamedVariable("Mat_B"), &b);
    complex_matrix_linear_eqn(&a, &b, &x);
    if(x.matrixElements) {
      convertComplex34MatrixToComplex34MatrixRegister(&x, findNamedVariable("Mat_X"));
      complexMatrixFree(&x);
    }
  }
  if(lastErrorCode == ERROR_NONE) {
    liftStack();
    copySourceRegisterToDestRegister(findNamedVariable("Mat_X"), REGISTER_X);
    popSoftmenu();
    //printf("Popped\n");
  }
}


#if defined(OPTION_EIGEN)
void fnQrDecomposition(uint16_t unusedParamButMandatory) {
  if(!saveLastX()) {
    return;
  }

  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    real34Matrix_t x, q, r;

    linkToRealMatrixRegister(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a square matrix (%d" STD_CROSS "%d)", x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnQrDecomposition:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      real_QR_decomposition(&x, &q, &r);
      setSystemFlag(FLAG_ASLIFT);
      liftStack();
      convertReal34MatrixToReal34MatrixRegister(&q, REGISTER_Y);
      convertReal34MatrixToReal34MatrixRegister(&r, REGISTER_X);
      realMatrixFree(&q);
      realMatrixFree(&r);
    }
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complex34Matrix_t x, q, r;

    linkToComplexMatrixRegister(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a square matrix (%d" STD_CROSS "%d)", x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnQrDecomposition:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      complex_QR_decomposition(&x, &q, &r);
      setSystemFlag(FLAG_ASLIFT);
      liftStack();
      convertComplex34MatrixToComplex34MatrixRegister(&q, REGISTER_Y);
      convertComplex34MatrixToComplex34MatrixRegister(&r, REGISTER_X);
      complexMatrixFree(&q);
      complexMatrixFree(&r);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
      moreInfoOnError("In function fnQrDecomposition:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}


static void extractDiagonalToRowReal34Matrix(const real34Matrix_t *source, real34Matrix_t *dest) {
  uint32_t size = source->header.matrixRows;
  #if defined(EIGEN_TESTOUT)
    char ts[100];
    printf("\nReMa:\"M%1d, %1d[", 1, size);
  #endif //EIGEN_TESTOUT
  if(realMatrixInit(dest, 1, size)) {
    for(uint32_t i = 0; i < size; i++) {
      real34Plus(source->matrixElements + (i * size + i), dest->matrixElements + i);
      #if defined(EIGEN_TESTOUT)
        real34ToString(dest->matrixElements + i, ts);
        printf("%s%s", ts, (int32_t)i == (int32_t)(size-1) ? "" : ",");
      #endif //EIGEN_TESTOUT
    }
    #if defined(EIGEN_TESTOUT)
      printf("]\"\n\n");
    #endif //EIGEN_TESTOUT
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function extractDiagonalToRowReal34Matrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

static void extractDiagonalToRowComplex34Matrix(const complex34Matrix_t *source, complex34Matrix_t *dest) {
  uint32_t size = source->header.matrixRows;
  #if defined(EIGEN_TESTOUT)
    char ts[100];
    printf("\nCxMa:\"M%1d, %1d[", 1, size);
  #endif //EIGEN_TESTOUT
  if(complexMatrixInit(dest, 1, size)) {
    for(uint32_t i = 0; i < size; i++) {
      real34Plus(VARIABLE_REAL34_DATA(source->matrixElements + (i * size + i)), VARIABLE_REAL34_DATA(dest->matrixElements + i));
      real34Plus(VARIABLE_IMAG34_DATA(source->matrixElements + (i * size + i)), VARIABLE_IMAG34_DATA(dest->matrixElements + i));
      #if defined(EIGEN_TESTOUT)
        real34ToString(VARIABLE_REAL34_DATA(dest->matrixElements + i), ts);
        printf("%s", ts);
        real34ToString(VARIABLE_IMAG34_DATA(dest->matrixElements + i), ts);
        printf("i%s%s", ts, (int32_t)i == (int32_t)(size-1) ? "" : ",");
      #endif //EIGEN_TESTOUT
    }
    #if defined(EIGEN_TESTOUT)
      printf("]\"\n\n");
    #endif //EIGEN_TESTOUT
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function extractDiagonalToRowComplex34Matrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnEigenvalues(uint16_t unusedParamButMandatory) {
  bool_t doneAdjusting = false;
  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    real34Matrix_t x, res, ires;

    linkToRealMatrixRegister(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "rectangular or single-element matrix or (%d" STD_CROSS "%d)", x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnEigenvalues:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto ErrorExit;
    }

    if(!saveLastX()) {
      goto ErrorExit;
    }

    if(x.header.matrixRows == 1 && x.header.matrixColumns == 1) {
      fnRecallVElement(1);
    }
    else {
      setSystemFlag(FLAG_ASLIFT);
      liftStack();
      linkToRealMatrixRegister(REGISTER_Y, &x);
      ires.header.matrixRows = ires.header.matrixColumns = 0;
      ires.matrixElements = NULL;
      res.matrixElements = NULL;
      realEigenvalues(&x, &res, &ires);
      if(lastErrorCode == ERROR_NONE || lastErrorCode == ERROR_SOLVER_ABORT) {
        if(ires.matrixElements) {
          complex34Matrix_t cres;
          if(complexMatrixInit(&cres, res.header.matrixRows, res.header.matrixColumns)) {
            for(uint32_t i = 0; i < x.header.matrixRows * x.header.matrixColumns; i++) {
              real34Copy(res.matrixElements + i,  VARIABLE_REAL34_DATA(cres.matrixElements + i));
              real34Copy(ires.matrixElements + i, VARIABLE_IMAG34_DATA(cres.matrixElements + i));
            }
            convertComplex34MatrixToComplex34MatrixRegister(&cres, REGISTER_X);
            adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
            doneAdjusting = true;

            //provide additional matrix for the eigenvalue outputs in a vector
            complex34Matrix_t cresRow;
            extractDiagonalToRowComplex34Matrix(&cres, &cresRow);
            if(cresRow.matrixElements) {
              setSystemFlag(FLAG_ASLIFT);
              liftStack();
              convertComplex34MatrixToComplex34MatrixRegister(&cresRow, REGISTER_X);
              adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
              complexMatrixFree(&cresRow);
            }

            realMatrixFree(&ires);
            complexMatrixFree(&cres);
          }
          else {
            displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Ram full");
              moreInfoOnError("In function fnEigenvalues:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
        }
        else {
          convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
          adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
          doneAdjusting = true;

          //provide additional matrix for the eigenvalue outputs in a vector
          real34Matrix_t resRow;
          extractDiagonalToRowReal34Matrix(&res, &resRow);
          if(resRow.matrixElements) {
            setSystemFlag(FLAG_ASLIFT);
            liftStack();
            convertReal34MatrixToReal34MatrixRegister(&resRow, REGISTER_X);
            adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
            realMatrixFree(&resRow);
          }

        }
        realMatrixFree(&res);
      }
      // When the solver leaves an error code other than ERROR_NONE or ERROR_SOLVER_ABORT (e.g. complex eigenvalues with FL_CPXRES clear), the
      // block above is skipped, but realEigenvalues already allocated res and ires. Release them here; on the handled paths they are already freed and NULL, so these are no-ops.
      if(res.matrixElements)  {
        realMatrixFree(&res);
      }
      if(ires.matrixElements) {
        realMatrixFree(&ires);
      }
    }
    goto Success;
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complex34Matrix_t x, res;

    linkToComplexMatrixRegister(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "rectangular or single-element matrix or (%d" STD_CROSS "%d)", x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnEigenvalues:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto ErrorExit;
    }

    if(!saveLastX()) {
      goto ErrorExit;
    }

    if(x.header.matrixRows == 1 && x.header.matrixColumns == 1) {
      fnRecallVElement(1);
    }
    else {
      setSystemFlag(FLAG_ASLIFT);
      liftStack();
      res.matrixElements = NULL;
      complexEigenvalues(&x, &res);
      if(res.matrixElements && lastErrorCode != ERROR_NONE && lastErrorCode != ERROR_SOLVER_ABORT) {   // after an error no result is written, as on the real path
        complexMatrixFree(&res);
      }
      if(res.matrixElements) {
        convertComplex34MatrixToComplex34MatrixRegister(&res, REGISTER_X);
        adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
        doneAdjusting = true;

        //provide additional matrix for the eigenvalue outputs in a vector
        complex34Matrix_t resRow;
        extractDiagonalToRowComplex34Matrix(&res, &resRow);
        if(resRow.matrixElements) {
          setSystemFlag(FLAG_ASLIFT);
          liftStack();
          convertComplex34MatrixToComplex34MatrixRegister(&resRow, REGISTER_X);
          adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
          complexMatrixFree(&resRow);
        }

        complexMatrixFree(&res);
      }
    }
    goto Success;
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
      moreInfoOnError("In function fnEigenvalues:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

ErrorExit:
return;

Success:
  if(!doneAdjusting) {
    adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
  }
return;


}



static uint8_t createEigenVectorIf1x1(uint16_t Rows, uint16_t Columns, bool_t isComplex) {
  real34Matrix_t    rmatrix;
  complex34Matrix_t cmatrix;
  if(Rows == 1 && Columns == 1) {
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
    if(!initMatrixRegister(REGISTER_X, 1, 1, isComplex)) {
      fnDrop(NOPARAM);
      displayCalcErrorMessage(ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Not enough memory for a %" PRIu32 STD_CROSS "%" PRIu32 " matrix", 1, 1);
        moreInfoOnError("In function createEigenVectorIf1x1:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return 255;
    }
    if(isComplex) {
      // consume incoming X = [n+mi]
      fnDrop(NOPARAM);
      linkToComplexMatrixRegister(REGISTER_X, &cmatrix);
      realToReal34(const_1, VARIABLE_REAL34_DATA(cmatrix.matrixElements));
      real34SetZero(VARIABLE_IMAG34_DATA(cmatrix.matrixElements));
    }
    else {
      // consume incoming X = [n]
      fnDrop(NOPARAM);
      linkToRealMatrixRegister(REGISTER_X,  &rmatrix);
      realToReal34(const_1, rmatrix.matrixElements);
    }
    adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
    return 1;
  }
  else {
    return 0;
  }
}



void fnEigenvectors(uint16_t unusedParamButMandatory) {
  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    real34Matrix_t x, res, ires;

    linkToRealMatrixRegister(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "rectangular or single-element matrix or (%d" STD_CROSS "%d)",
                x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnEigenvectors:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto ErrorExit;
    }

    if(!saveLastX()) {
      goto ErrorExit;
    }

    switch(createEigenVectorIf1x1(x.header.matrixRows, x.header.matrixColumns, false)) {
      case 1  : break;
      case 255: return;
      default:
      ires.header.matrixRows = ires.header.matrixColumns = 0;
      ires.matrixElements = NULL;
      realEigenvectors(&x, &res, &ires);
      if(res.matrixElements) {
        // Success: lift the stack (removed)
        //        setSystemFlag(FLAG_ASLIFT);
        //        liftStack();
        // Install the result.
        if(ires.matrixElements) {
          complex34Matrix_t cres;
          if(complexMatrixInit(&cres, res.header.matrixRows, res.header.matrixColumns)) {
            for(uint32_t i = 0; i < x.header.matrixRows * x.header.matrixColumns; i++) {
              real34Copy(res.matrixElements + i,  VARIABLE_REAL34_DATA(cres.matrixElements + i));
              real34Copy(ires.matrixElements + i, VARIABLE_IMAG34_DATA(cres.matrixElements + i));
            }
            convertComplex34MatrixToComplex34MatrixRegister(&cres, REGISTER_X);
            realMatrixFree(&ires);
            complexMatrixFree(&cres);
          }
          else {
            realMatrixFree(&ires);
            displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Ram full");
              moreInfoOnError("In function fnEigenvectors:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
        }
        else {
          convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
        }
        realMatrixFree(&res);
      }
      else {
        if(lastErrorCode == ERROR_NO_ROOT_FOUND || lastErrorCode == ERROR_RAM_FULL) {            // the error is already displayed
          goto ErrorExit;
        }
        displayCalcErrorMessage(ERROR_SINGULAR_MATRIX, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "matrix is defective: no full set of linearly independent eigenvectors");
          moreInfoOnError("In function fnEigenvectors:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        goto ErrorExit;
      }
    }
    goto Success;
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complex34Matrix_t x, res;

    linkToComplexMatrixRegister(REGISTER_X, &x);

    if(x.header.matrixRows != x.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "rectangular or single-element matrix or (%d" STD_CROSS "%d)", x.header.matrixRows, x.header.matrixColumns);
        moreInfoOnError("In function fnEigenvectors:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto ErrorExit;
    }

    if(!saveLastX()) {
      goto ErrorExit;
    }

    switch(createEigenVectorIf1x1(x.header.matrixRows, x.header.matrixColumns, true)) {
      case 1  : break;
      case 255: return;
      default:
      complexEigenvectors(&x, &res);
      if(res.matrixElements) {
        // Success: lift the stack (removed)
        //        setSystemFlag(FLAG_ASLIFT);
        //        liftStack();
        // Install the result.
        convertComplex34MatrixToComplex34MatrixRegister(&res, REGISTER_X);
        complexMatrixFree(&res);
      }
      else {
        if(lastErrorCode == ERROR_NO_ROOT_FOUND || lastErrorCode == ERROR_RAM_FULL) {            // the error is already displayed
          goto ErrorExit;
        }
        displayCalcErrorMessage(ERROR_SINGULAR_MATRIX, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "matrix is defective: no full set of linearly independent eigenvectors");
          moreInfoOnError("In function fnEigenvectors:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        goto ErrorExit;
      }
    }
    goto Success;
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
      moreInfoOnError("In function fnEigenvectors:", errorMessage, "is not a matrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto ErrorExit;
  }

ErrorExit:
return;

Success:
adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
return;

}


#endif // OPTION_EIGEN


bool_t realMatrixInit(real34Matrix_t *matrix, uint16_t rows, uint16_t cols) {
  //Allocate Memory for Matrix
  const size_t neededSize = rows * cols * REAL34_SIZE_IN_BLOCKS;
  matrix->header.mtag = amNone;
  if(!isMemoryBlockAvailable(neededSize, 2, 0.1f)) {
    matrix->header.matrixColumns = matrix->header.matrixRows = 0;
    matrix->matrixElements = NULL;
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Ram full");
              moreInfoOnError("In function realMatrixInit:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  matrix->matrixElements = allocC47Blocks(neededSize);

  matrix->header.matrixColumns = cols;
  matrix->header.matrixRows = rows;

  //Initialize with 0.
  for(uint32_t i = 0; i < rows * cols; i++) {
    real34SetZero(matrix->matrixElements + i);
  }
  return true;
}


void realMatrixFree(real34Matrix_t *matrix) {
  freeC47Blocks(matrix->matrixElements, matrix->header.matrixRows * matrix->header.matrixColumns * REAL34_SIZE_IN_BLOCKS);
  matrix->matrixElements = NULL;
  matrix->header.matrixRows = matrix->header.matrixColumns = 0;
}


static void anyMatrixIdentity(any34Matrix_t *matrix, uint16_t size, const matrixKind_t *kind) {
  if(kind->initMatrix(matrix, size, size)) {
    uint8_t *e = MATRIX_BYTES(matrix);
    for(uint16_t i = 0; i < size; ++i) {
      kind->oneElement(e + (i * size + i)*kind->elementSize);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function anyMatrixIdentity:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

void realMatrixIdentity(real34Matrix_t *matrix, uint16_t size) {
  anyMatrixIdentity((any34Matrix_t*)matrix, size, &matrixKindReal);
}


void fnMatrixIdentity(uint16_t unusedButMandatoryParameter) {
  uint32_t rows, cols;
  if(!getDimensionArg(&rows, &cols)) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  //Initialize Memory for Matrix (zero-filled by realMatrixInit)
  if(initMatrixRegister(REGISTER_X, rows, cols, false)) {
    real34Matrix_t res;
    const uint32_t n = rows < cols ? rows : cols;
    linkToRealMatrixRegister(REGISTER_X, &res);
    for(uint32_t i = 0; i < n; ++i) {
      real34SetOne(res.matrixElements + (i * cols + i));
    }
    setSystemFlag(FLAG_ASLIFT);
  }
  else {
    displayCalcErrorMessage(ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Not enough memory for a %" PRIu32 STD_CROSS "%" PRIu32 " identity matrix", rows, cols);
      moreInfoOnError("In function fnMatrixIdentity:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  adjustResult(REGISTER_X, true, false, REGISTER_X, REGISTER_Y, -1);
}



bool_t complexMatrixInit(complex34Matrix_t *matrix, uint16_t rows, uint16_t cols) {
  //Allocate Memory for Matrix
  const size_t neededSize = rows * cols * COMPLEX34_SIZE_IN_BLOCKS;
  matrix->header.mtag = amNone;
  if(!isMemoryBlockAvailable(neededSize, 2, 0.1f)) {
    matrix->header.matrixColumns = matrix->header.matrixRows = 0;
    matrix->matrixElements = NULL;
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Ram full 01");
              moreInfoOnError("In function complexMatrixInit:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  matrix->matrixElements = allocC47Blocks(neededSize);
  if(matrix->matrixElements == NULL) {
    matrix->header.matrixColumns = matrix->header.matrixRows = 0;
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Ram full 02");
              moreInfoOnError("In function complexMatrixInit:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  matrix->header.matrixColumns = cols;
  matrix->header.matrixRows = rows;

  //Initialize with 0.
  for(uint32_t i = 0; i < rows * cols; i++) {
    real34SetZero(VARIABLE_REAL34_DATA(matrix->matrixElements + i));
    real34SetZero(VARIABLE_IMAG34_DATA(matrix->matrixElements + i));
  }
  return true;
}


void complexMatrixFree(complex34Matrix_t *matrix) {
  freeC47Blocks(matrix->matrixElements, matrix->header.matrixRows * matrix->header.matrixColumns * COMPLEX34_SIZE_IN_BLOCKS);
  matrix->matrixElements = NULL;
  matrix->header.matrixRows = matrix->header.matrixColumns = 0;
}


void complexMatrixIdentity(complex34Matrix_t *matrix, uint16_t size) {
  anyMatrixIdentity((any34Matrix_t*)matrix, size, &matrixKindComplex);
}



/*
void storeMatrixToXRegister(real34Matrix_t *matrix) {
  setSystemFlag(FLAG_ASLIFT);
  liftStack();
  clearSystemFlag(FLAG_ASLIFT);

  //WORKING//
  //openMatrixMIMPointer = matrix;
  // END WORKING//

  convertReal34MatrixToReal34MatrixRegister(matrix, REGISTER_X);
}
*/

void getMatrixFromRegister(calcRegister_t regist) {
  if(getRegisterDataType(regist) == dtReal34Matrix) {
    real34Matrix_t matrix;

    if(openMatrixMIMPointer.realMatrix.matrixElements) {
      realMatrixFree(&openMatrixMIMPointer.realMatrix);
    }
    convertReal34MatrixRegisterToReal34Matrix(regist, &matrix);

    matrix.header.mtag = getRegisterTag(regist);
    openMatrixMIMPointer.realMatrix = matrix;
  }
  else if(getRegisterDataType(regist) == dtComplex34Matrix) {
    complex34Matrix_t matrix;

    if(openMatrixMIMPointer.complexMatrix.matrixElements) {
      complexMatrixFree(&openMatrixMIMPointer.complexMatrix);
    }
    convertComplex34MatrixRegisterToComplex34Matrix(regist, &matrix);

    matrix.header.mtag = getRegisterTag(regist);
    openMatrixMIMPointer.complexMatrix = matrix;
  }
  else {
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(regist));
    moreInfoOnError("In function getMatrixFromRegister:", errorMessage, "is not dataType dtRealMatrix.", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    return;
  }
}


bool_t initMatrixRegister(calcRegister_t regist, uint16_t rows, uint16_t cols, bool_t complex) {
  #if defined(PC_BUILD)
    if(lastErrorCode != ERROR_NONE) {
      errorf("initMatrixRegister(): Entered initMatrixRegister with pre-existing error.");
      printf("  Error code: %d:%s\n", lastErrorCode, errorMessageOf(lastErrorCode));
    }
  #endif //PC_BUILD

  const size_t neededSize = (rows * cols) * (complex ? COMPLEX34_SIZE_IN_BLOCKS : REAL34_SIZE_IN_BLOCKS);
  reallocateRegister(regist, complex ? dtComplex34Matrix : dtReal34Matrix, neededSize, amNone);
  if(regist == INVALID_VARIABLE) {
    return false;
  }
  else if(lastErrorCode == ERROR_NONE) {
    REGISTER_MATRIX_HEADER(regist)->matrixRows    = rows;
    REGISTER_MATRIX_HEADER(regist)->matrixColumns = cols;
    REGISTER_MATRIX_HEADER(regist)->mtag          = amNone;
    if(complex) {
      for(uint16_t i = 0; i < rows * cols; ++i) {
        real34SetZero(VARIABLE_REAL34_DATA(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist) + i));
        real34SetZero(VARIABLE_IMAG34_DATA(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist) + i));
      }
    }
    else {
      for(uint16_t i = 0; i < rows * cols; ++i) {
        real34SetZero(REGISTER_REAL34_MATRIX_ELEMENTS(regist) + i);
      }
    }
    return true;
  }
  else {
    #if defined(PC_BUILD)
      printf("  initMatrixRegister(): Error number %d:%s\n", lastErrorCode, errorMessageOf(lastErrorCode));
    #endif //PC_BUILD
    return false;
  }
}




//Matrix redimension

  // KEEPRECT
  // Keeping rectangles, adding zero lines below and zero columns to the right, and when reducing, removing those lines and columns so the elements in the new shape is kept as is. See examples:
  // 1 2 3
  // 4 5 6
  // 7 8 9
  // reduced to 2x2, will become
  // 1 2
  // 4 5
  // --------
  // 1 2
  // 3 4
  // redimmed to 3x3 will become
  // 1 2 0
  // 3 4 0
  // 0 0 0


  // Reflow elements: all elements are kept, but re-listed from element 1. If it overruns the new matrix, the end elements are deleted: examples:
  // 1 2 3
  // 4 5 6
  // 7 8 9
  // reduced to 2x2, will become
  // 1 2
  // 3 4
  // --------
  // 1 2
  // 3 4
  // redimmed to 3x3 will become
  // 1 2 3
  // 4 0 0
  // 0 0 0




static void copyAndZeroMatrixElements(void *src, void *dst, uint16_t srcRows, uint16_t srcCols, uint16_t dstRows, uint16_t dstCols, size_t elementSize, void (*copyFunc)(void*, void*), void (*zeroFunc)(void*), bool_t keepRect) {
  if(keepRect) {
    // Copy preserving rectangular structure
    const uint16_t copyRows = min(srcRows, dstRows);
    const uint16_t copyCols = min(srcCols, dstCols);
    for(uint16_t row = 0; row < copyRows; ++row) {
      for(uint16_t col = 0; col < copyCols; ++col) {
        copyFunc((uint8_t*)src + (row * srcCols + col) * elementSize,
                 (uint8_t*)dst + (row * dstCols + col) * elementSize);
      }
    }
    // Zero new columns in existing rows
    for(uint16_t row = 0; row < min(srcRows, dstRows); ++row) {
      for(uint16_t col = srcCols; col < dstCols; ++col) {
        zeroFunc((uint8_t*)dst + (row * dstCols + col) * elementSize);
      }
    }
    // Zero entire new rows
    for(uint16_t row = srcRows; row < dstRows; ++row) {
      for(uint16_t col = 0; col < dstCols; ++col) {
        zeroFunc((uint8_t*)dst + (row * dstCols + col) * elementSize);
      }
    }
  }
  else {
    // Linear copy
    for(uint16_t i = 0; i < min(srcRows * srcCols, dstRows * dstCols); ++i) {
      copyFunc((uint8_t*)src + i * elementSize, (uint8_t*)dst + i * elementSize);
    }
    // Zero new elements
    if(srcRows * srcCols < dstRows * dstCols) {
      for(uint16_t i = srcRows * srcCols; i < dstRows * dstCols; ++i) {
        zeroFunc((uint8_t*)dst + i * elementSize);
      }
    }
  }
}




bool_t redimMatrixRegister(calcRegister_t regist, uint16_t rows, uint16_t cols, uint16_t dimMode) {
  const uint16_t origRows = REGISTER_MATRIX_HEADER(regist)->matrixRows, origCols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
  if(regist == INVALID_VARIABLE) {
    return false;
  }
  else if(getRegisterDataType(regist) == dtReal34Matrix) {
    const size_t newSize = (rows * cols) * REAL34_SIZE_IN_BLOCKS;
    real34Matrix_t newMatrix;
    convertReal34MatrixRegisterToReal34Matrix(regist, &newMatrix);
    if(lastErrorCode == ERROR_NONE) {
      reallocateRegister(regist, dtReal34Matrix, newSize, amNone);
      if(lastErrorCode == ERROR_NONE) {
        REGISTER_MATRIX_HEADER(regist)->matrixRows    = rows;
        REGISTER_MATRIX_HEADER(regist)->matrixColumns = cols;
        copyAndZeroMatrixElements(newMatrix.matrixElements, REGISTER_REAL34_MATRIX_ELEMENTS(regist), origRows, origCols, rows, cols, REAL34_SIZE_IN_BYTES, real34CopyWrapper, real34ZeroWrapper, dimMode == ITM_M_DIM_GR);
        realMatrixFree(&newMatrix);
        return true;
      }
      else {
        realMatrixFree(&newMatrix);
        return false;
      }
    }
    else {
      return false;
    }
  }
  else if(getRegisterDataType(regist) == dtComplex34Matrix) {
    const size_t newSize = (rows * cols) * COMPLEX34_SIZE_IN_BLOCKS;
    complex34Matrix_t newMatrix;
    convertComplex34MatrixRegisterToComplex34Matrix(regist, &newMatrix);
    if(lastErrorCode == ERROR_NONE) {
      reallocateRegister(regist, dtComplex34Matrix, newSize, amNone);
      if(lastErrorCode == ERROR_NONE) {
        REGISTER_MATRIX_HEADER(regist)->matrixRows    = rows;
        REGISTER_MATRIX_HEADER(regist)->matrixColumns = cols;
        copyAndZeroMatrixElements(newMatrix.matrixElements, REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist), origRows, origCols, rows, cols, COMPLEX34_SIZE_IN_BYTES, complex34CopyWrapper, complex34ZeroWrapper, dimMode == ITM_M_DIM_GR);
        complexMatrixFree(&newMatrix);
        return true;
      }
      else {
        complexMatrixFree(&newMatrix);
        return false;
      }
    }
    else {
      return false;
    }
  }
  else {
    return initMatrixRegister(regist, rows, cols, false);
  }
}

calcRegister_t allocateNamedMatrix(const char *name, uint16_t rows, uint16_t cols) {
  const calcRegister_t regist = findOrAllocateNamedVariable(name);
  if(regist == INVALID_VARIABLE) {
    return INVALID_VARIABLE;
  }
  else if(redimMatrixRegister(regist, rows, cols, ITM_M_DIM)) {
    return regist;
  }
  else {
    return INVALID_VARIABLE;
  }
}

bool_t appendRowAtMatrixRegister(calcRegister_t regist) {
  if(regist == INVALID_VARIABLE) {
    return false;
  }
  else if(getRegisterDataType(regist) == dtReal34Matrix || getRegisterDataType(regist) == dtComplex34Matrix) {
    const uint16_t rows = REGISTER_MATRIX_HEADER(regist)->matrixRows, cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
    return redimMatrixRegister(regist, rows + 1, cols, ITM_M_DIM);
  }
  else {
    return false;
  }
}


/* Duplicate */
static void copyAnyMatrix(const any34Matrix_t *matrix, any34Matrix_t *res, const matrixKind_t *kind) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  const size_t   size = kind->elementSize;
  int32_t i;

  if(kind->initMatrix(res, rows, cols)) {
    uint8_t *src = MATRIX_BYTES(matrix), *dst = MATRIX_BYTES(res);
    for(i = 0; i < cols * rows; ++i) {
      kind->copyElement(src + i*size, dst + i*size);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function copyAnyMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

void copyRealMatrix(const real34Matrix_t *matrix, real34Matrix_t *res) {
  copyAnyMatrix((const any34Matrix_t*)matrix, (any34Matrix_t*)res, &matrixKindReal);
}

void copyComplexMatrix(const complex34Matrix_t *matrix, complex34Matrix_t *res) {
  copyAnyMatrix((const any34Matrix_t*)matrix, (any34Matrix_t*)res, &matrixKindComplex);
}


/* Link to real matrix register (data not copied) */
void linkToRealMatrixRegister(calcRegister_t regist, real34Matrix_t *linkedMatrix) {
  linkedMatrix->header.matrixRows    = REGISTER_MATRIX_HEADER(regist)->matrixRows;
  linkedMatrix->header.matrixColumns = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
  linkedMatrix->header.mtag          = amNone;   // default; without it, a non-vector or non-X-T link leaves mtag uninitialised and displayVectorAngle reads it
  if(isMatrixVector(linkedMatrix->header.matrixRows, linkedMatrix->header.matrixColumns)) {
    linkedMatrix->header.mtag        = getRegisterTag(regist);   // any register class: SHOW and VIEW link registers outside X-T
  }
  linkedMatrix->matrixElements       = REGISTER_REAL34_MATRIX_ELEMENTS(regist);
}


/* Link to complex matrix register (data not copied) */
void linkToComplexMatrixRegister(calcRegister_t regist, complex34Matrix_t *linkedMatrix) {
  linkedMatrix->header.matrixRows    = REGISTER_MATRIX_HEADER(regist)->matrixRows;
  linkedMatrix->header.matrixColumns = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
  linkedMatrix->header.mtag          = amNone;   // same as the real link: keep mtag defined for readers
  linkedMatrix->matrixElements       = REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist);
}


/* Insert a row */
static void insRowAnyMatrix(any34Matrix_t *matrix, uint16_t beforeRowNo, bool_t add, const matrixKind_t *kind) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  const size_t   size = kind->elementSize;
  int32_t i;
  any34Matrix_t newMat;
  if(add) {
    beforeRowNo = rows;
  }

  if(kind->initMatrix(&newMat, rows + 1, cols)) {
    uint8_t *src = MATRIX_BYTES(matrix), *dst = MATRIX_BYTES(&newMat);
    for(i = 0; i < beforeRowNo * cols; ++i) {
      kind->copyElement(src + i*size, dst + i*size);
    }
    for(i = 0; i < cols; ++i) {
      kind->zeroElement(dst + (beforeRowNo * cols + i)*size);
    }
    for(i = beforeRowNo * cols; i < cols * rows; ++i) {
      kind->copyElement(src + i*size, dst + (i + cols)*size);
    }

    adoptNewMatrix(matrix, &newMat, kind);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function insRowAnyMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


static void insColAnyMatrix(any34Matrix_t *matrix, uint16_t beforeColNo, bool_t add, const matrixKind_t *kind) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  const size_t   size = kind->elementSize;
  int32_t i, j;
  any34Matrix_t newMat;
  if(add) {
    beforeColNo = cols;
  }

  if(kind->initMatrix(&newMat, rows, cols + 1)) {
    uint8_t *src = MATRIX_BYTES(matrix), *dst = MATRIX_BYTES(&newMat);
    for(j = 0; j < beforeColNo; ++j) {
      for( i = 0; i < rows; i++) {
        kind->copyElement(src + (j + i*cols)*size, dst + (j + i*(cols+1))*size);
      }
    }
    for(i = 0; i < rows; ++i) {
      kind->zeroElement(dst + (beforeColNo + i*(cols+1))*size);
    }
    for(j = beforeColNo; j < cols; ++j) {
      for( i = 0; i < rows; i++) {
        kind->copyElement(src + (j + i*cols)*size, dst + ((j+1) + i*(cols+1))*size);
      }
    }

    adoptNewMatrix(matrix, &newMat, kind);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function insColAnyMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void insRowRealMatrix(real34Matrix_t *matrix, uint16_t beforeRowNo, bool_t add) {
  insRowAnyMatrix((any34Matrix_t*)matrix, beforeRowNo, add, &matrixKindReal);
}


void insColRealMatrix(real34Matrix_t *matrix, uint16_t beforeColNo, bool_t add) {
  insColAnyMatrix((any34Matrix_t*)matrix, beforeColNo, add, &matrixKindReal);
}


void insRowComplexMatrix(complex34Matrix_t *matrix, uint16_t beforeRowNo, bool_t add) {
  insRowAnyMatrix((any34Matrix_t*)matrix, beforeRowNo, add, &matrixKindComplex);
}


void insColComplexMatrix(complex34Matrix_t *matrix, uint16_t beforeColNo, bool_t add) {
  insColAnyMatrix((any34Matrix_t*)matrix, beforeColNo, add, &matrixKindComplex);
}


/* Delete a row */
static void delRowAnyMatrix(any34Matrix_t *matrix, uint16_t beforeRowNo, const matrixKind_t *kind) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  const size_t   size = kind->elementSize;
  int32_t i;
  any34Matrix_t newMat;

  if(kind->initMatrix(&newMat, rows - 1, cols)) {
    uint8_t *src = MATRIX_BYTES(matrix), *dst = MATRIX_BYTES(&newMat);
    for(i = 0; i < beforeRowNo * cols; ++i) {
      kind->copyElement(src + i*size, dst + i*size);
    }
    for(i = (beforeRowNo + 1) * cols; i < cols * rows; ++i) {
      kind->copyElement(src + i*size, dst + (i - cols)*size);
    }

    adoptNewMatrix(matrix, &newMat, kind);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function delRowAnyMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


static void delColAnyMatrix(any34Matrix_t *matrix, uint16_t beforeColNo, const matrixKind_t *kind) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  const size_t   size = kind->elementSize;
  int32_t i, j;
  any34Matrix_t newMat;

  if(kind->initMatrix(&newMat, rows, cols - 1)) {
    uint8_t *src = MATRIX_BYTES(matrix), *dst = MATRIX_BYTES(&newMat);
    for(j = 0; j < beforeColNo; ++j) {
      for( i = 0; i < rows; i++) {
        kind->copyElement(src + (j + i*cols)*size, dst + (j + i*(cols-1))*size);
      }
    }
    for(j = (beforeColNo + 1); j < cols; ++j) {
      for( i = 0; i < rows; i++) {
        kind->copyElement(src + (j + i*cols)*size, dst + ((j-1) + i*(cols-1))*size);
      }
    }

    adoptNewMatrix(matrix, &newMat, kind);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function delColAnyMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void delRowRealMatrix(real34Matrix_t *matrix, uint16_t beforeRowNo) {
  delRowAnyMatrix((any34Matrix_t*)matrix, beforeRowNo, &matrixKindReal);
}

/* Delete a col */
void delColRealMatrix(real34Matrix_t *matrix, uint16_t beforeColNo) {
  delColAnyMatrix((any34Matrix_t*)matrix, beforeColNo, &matrixKindReal);
}


void delRowComplexMatrix(complex34Matrix_t *matrix, uint16_t beforeRowNo) {
  delRowAnyMatrix((any34Matrix_t*)matrix, beforeRowNo, &matrixKindComplex);
}

void delColComplexMatrix(complex34Matrix_t *matrix, uint16_t beforeColNo) {
  delColAnyMatrix((any34Matrix_t*)matrix, beforeColNo, &matrixKindComplex);
}



/* Transpose */
static void transposeAnyMatrix(const any34Matrix_t *matrix, any34Matrix_t *res, const matrixKind_t *kind) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  const size_t   size = kind->elementSize;
  int32_t i, j;

  if(matrix != res) {
    if(kind->initMatrix(res, cols, rows)) {
      uint8_t *src = MATRIX_BYTES(matrix), *dst = MATRIX_BYTES(res);
      for(i = 0; i < rows; ++i) {
        for(j = 0; j < cols; ++j) {
          kind->copyElement(src + (i * cols + j)*size, dst + (j * rows + i)*size);
        }
      }
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 1g");
        moreInfoOnError("In function transposeAnyMatrix:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    any34Matrix_t tmp;
    kind->copyMatrix(matrix, &tmp);
    if(MATRIX_BYTES(&tmp)) {
      uint8_t *src = MATRIX_BYTES(&tmp), *dst = MATRIX_BYTES(res);
      for(i = 0; i < rows; ++i) {
        for(j = 0; j < cols; ++j) {
          kind->copyElement(src + (i * cols + j)*size, dst + (j * rows + i)*size);
        }
      }
      kind->freeMatrix(&tmp);
      res->header.matrixRows    = cols;
      res->header.matrixColumns = rows;
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 2h");
        moreInfoOnError("In function transposeAnyMatrix:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
}


void transposeRealMatrix(const real34Matrix_t *matrix, real34Matrix_t *res) {
  transposeAnyMatrix((const any34Matrix_t*)matrix, (any34Matrix_t*)res, &matrixKindReal);
}


void transposeComplexMatrix(const complex34Matrix_t *matrix, complex34Matrix_t *res) {
  transposeAnyMatrix((const any34Matrix_t*)matrix, (any34Matrix_t*)res, &matrixKindComplex);
}


/* Addition and subtraction */
static void addSubRealMatrices(const real34Matrix_t *y, const real34Matrix_t *x, bool_t subtraction, real34Matrix_t *res) {
  const uint16_t rows = y->header.matrixRows;
  const uint16_t cols = y->header.matrixColumns;
  int32_t i;

  if((y->header.matrixColumns != x->header.matrixColumns) || (y->header.matrixRows != x->header.matrixRows)) {
    realMatrixFree(res); // Matrix mismatch: res aliases an owned operand, so free it (this also clears matrixElements and the dimensions)
    return;
  }

  if((y != res) && (x != res)) {
    if(!realMatrixInit(res, rows, cols)) {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full");
        moreInfoOnError("In function addSubRealMatrices:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  for(i = 0; i < cols * rows; ++i) {
    if(subtraction) {
      real34Subtract(y->matrixElements + i, x->matrixElements + i, res->matrixElements + i);
    }
    else {
      real34Add(y->matrixElements + i, x->matrixElements + i, res->matrixElements + i);
    }
  }
}


void addRealMatrices(const real34Matrix_t *y, const real34Matrix_t *x, real34Matrix_t *res) {
  addSubRealMatrices(y, x, false, res);
}


void subtractRealMatrices(const real34Matrix_t *y, const real34Matrix_t *x, real34Matrix_t *res) {
  addSubRealMatrices(y, x, true, res);
}


static void addSubComplexMatrices(const complex34Matrix_t *y, const complex34Matrix_t *x, bool_t subtraction, complex34Matrix_t *res) {
  const uint16_t rows = y->header.matrixRows;
  const uint16_t cols = y->header.matrixColumns;
  int32_t i;

  if((y->header.matrixColumns != x->header.matrixColumns) || (y->header.matrixRows != x->header.matrixRows)) {
    complexMatrixFree(res); // Matrix mismatch: res aliases an owned operand, so free it (this also clears matrixElements and the dimensions)
    return;
  }

  if((y == res) || (x == res) || complexMatrixInit(res, rows, cols)) {
    for(i = 0; i < cols * rows; ++i) {
      if(subtraction) {
        real34Subtract(VARIABLE_REAL34_DATA(y->matrixElements + i), VARIABLE_REAL34_DATA(x->matrixElements + i), VARIABLE_REAL34_DATA(res->matrixElements + i));
        real34Subtract(VARIABLE_IMAG34_DATA(y->matrixElements + i), VARIABLE_IMAG34_DATA(x->matrixElements + i), VARIABLE_IMAG34_DATA(res->matrixElements + i));
      }
      else {
        real34Add(VARIABLE_REAL34_DATA(y->matrixElements + i), VARIABLE_REAL34_DATA(x->matrixElements + i), VARIABLE_REAL34_DATA(res->matrixElements + i));
        real34Add(VARIABLE_IMAG34_DATA(y->matrixElements + i), VARIABLE_IMAG34_DATA(x->matrixElements + i), VARIABLE_IMAG34_DATA(res->matrixElements + i));
      }
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function addSubComplexMatrices:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void addComplexMatrices(const complex34Matrix_t *y, const complex34Matrix_t *x, complex34Matrix_t *res) {
  addSubComplexMatrices(y, x, false, res);
}


void subtractComplexMatrices(const complex34Matrix_t *y, const complex34Matrix_t *x, complex34Matrix_t *res) {
  addSubComplexMatrices(y, x, true, res);
}


/* Multiplication */

/**
 * \brief Add two finite values and retain the rounding residual.
 *
 * The greater-magnitude operand is selected for the FastTwoSum identity.
 */
static void matrixTwoSum(const real_t *a, const real_t *b, real_t *sum, real_t *error, realContext_t *realContext) {
  REAL_T_PTR(z, 75);
  const real_t *larger = a;
  const real_t *smaller = b;

  if(realCompareAbsLessThan(larger, smaller)) {
    larger = b;
    smaller = a;
  }
  realAdd(larger, smaller, sum, realContext);
  realSubtract(sum, larger, z, realContext);
  realSubtract(smaller, z, error, realContext);
}


/**
 * \brief Add one value to a two-term compensated accumulator.
 *
 * Special values use ordinary addition and discard a finite correction once
 * the main sum becomes non-finite.
 */
static void matrixAccumulate(const real_t *value, real_t *sum, real_t *correction, realContext_t *realContext) {
  REAL_T_PTR(next, 75);
  REAL_T_PTR(error, 75);

  if(realIsSpecial(value) || realIsSpecial(sum) || realIsSpecial(correction)) {
    realAdd(sum, value, sum, realContext);
    realSetZero(correction);
    return;
  }
  matrixTwoSum(sum, value, next, error, realContext);
  realAdd(correction, error, correction, realContext);
  realCopy(next, sum);
}


/**
 * \brief Add a value to a compensated sum and its positive magnitude sum.
 */
static void matrixAccumulateValue(const real_t *value, real_t *sum, real_t *correction, real_t *absoluteSum, realContext_t *realContext) {
  REAL_T_PTR(magnitude, 75);

  if(!realIsSpecial(value)) {
    realCopy(value, magnitude);
    realSetPositiveSign(magnitude);
    realAdd(absoluteSum, magnitude, absoluteSum, realContext);
  }
  matrixAccumulate(value, sum, correction, realContext);
}


/**
 * \brief Accumulate an optionally negated product and its fused residual.
 *
 * A product of two Real34 operands has at most 68 coefficient digits.  Its
 * residual after a p39 product has at most 29 digits and is therefore exact.
 */
static void matrixAccumulateProduct(const real_t *a, const real_t *b, bool_t subtract, real_t *sum, real_t *correction, real_t *absoluteSum, realContext_t *realContext) {
  REAL_T_PTR(product, 75);
  REAL_T_PTR(residual, 75);
  REAL_T_PTR(magnitude, 75);

  realMultiply(a, b, product, realContext);
  if(realIsSpecial(a) || realIsSpecial(b) || realIsSpecial(product)) {
    if(subtract) {
      realChangeSign(product);
    }
    matrixAccumulate(product, sum, correction, realContext);
    return;
  }
  if(absoluteSum != NULL) {
    realCopy(product, magnitude);
    realSetPositiveSign(magnitude);
    realAdd(absoluteSum, magnitude, absoluteSum, realContext);
  }
  realCopy(product, residual);
  realChangeSign(residual);
  realFMA(a, b, residual, residual, realContext);
  if(subtract) {
    realChangeSign(product);
    realChangeSign(residual);
  }
  matrixAccumulate(product, sum, correction, realContext);
  matrixAccumulate(residual, sum, correction, realContext);
}


/**
 * \brief Test whether cancellation merits a p75 accumulator replay.
 */
static bool_t matrixNeedsExtendedAccumulator(const real_t *absoluteSum, const real_t *result) {
  if(realIsZero(absoluteSum) || realIsSpecial(absoluteSum) || realIsSpecial(result)) {
    return false;
  }
  return realIsZero(result) || realGetExponent(absoluteSum) - realGetExponent(result) > 20;
}


/**
 * \brief Convert a two-term accumulator to one working result.
 */
static void matrixFinishAccumulator(const real_t *sum, const real_t *correction, real_t *result, realContext_t *realContext) {
  realAdd(sum, correction, result, realContext);
}


void multiplyRealMatrix(const real34Matrix_t *matrix, const real34_t *x, real34Matrix_t *res) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  int32_t i;

  if(matrix == res || realMatrixInit(res, rows, cols)) {
    for(i = 0; i < cols * rows; ++i) {
      real34Multiply(matrix->matrixElements + i, x, res->matrixElements + i);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function multiplyRealMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void _multiplyRealMatrix(const real34Matrix_t *matrix, const real_t *x, real34Matrix_t *res, realContext_t *realContext) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  int32_t i;
  real_t y;

  if(matrix == res || realMatrixInit(res, rows, cols)) {
    for(i = 0; i < cols * rows; ++i) {
      real34ToReal(matrix->matrixElements + i, &y);
      realMultiply(&y, x, &y, realContext);
      realToReal34(&y, res->matrixElements + i);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function _multiplyRealMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void multiplyRealMatrices(const real34Matrix_t *y, const real34Matrix_t *x, real34Matrix_t *res) {
  const uint16_t rows = y->header.matrixRows;
  const uint16_t iter = y->header.matrixColumns;
  const uint16_t cols = x->header.matrixColumns;
  int32_t i, j, k;
  real_t sum, absoluteSum, p, q;
  realContext_t *dotContext;
  REAL_T_PTR(correction, 75);

  if(y->header.matrixColumns != x->header.matrixRows) {
    res->matrixElements = NULL; // Matrix mismatch
    res->header.matrixRows = res->header.matrixColumns = 0;
    return;
  }

  if(realMatrixInit(res, rows, cols)) {
    for(i = 0; i < rows; ++i) {
      for(j = 0; j < cols; ++j) {
        dotContext = &ctxtReal39;
        do {
          realSetZero(&sum);
          realSetZero(&absoluteSum);
          realSetZero(correction);
          for(k = 0; k < iter; ++k) {
            real34ToReal(y->matrixElements + (i * iter + k), &p);
            real34ToReal(x->matrixElements + (k * cols + j), &q);
            matrixAccumulateProduct(&p, &q, false, &sum, correction, &absoluteSum, dotContext);
          }
          matrixFinishAccumulator(&sum, correction, &sum, dotContext);
          if(dotContext == &ctxtReal39 && matrixNeedsExtendedAccumulator(&absoluteSum, &sum)) {
            dotContext = &ctxtReal75;
          }
          else {
            break;
          }
        } while(true);
        realToReal34(&sum, res->matrixElements + (i * cols + j));
      }
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function multiplyRealMatrices:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void multiplyComplexMatrix(const complex34Matrix_t *matrix, const real34_t *xr, const real34_t *xi, complex34Matrix_t *res) {
  cplx_t _x;

  real34ToReal(xr, &_x.Real);
  real34ToReal(xi, &_x.Imag);
  _multiplyComplexMatrix(matrix, &_x, res, &ctxtReal39);
}


void _multiplyComplexMatrix(const complex34Matrix_t *matrix, const cplx_t *x, complex34Matrix_t *res, realContext_t *realContext) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  int32_t i;
  cplx_t y;

  if(matrix == res || complexMatrixInit(res, rows, cols)) {
    for(i = 0; i < cols * rows; ++i) {
      real34ToReal(VARIABLE_REAL34_DATA(matrix->matrixElements + i), &y.Real);
      real34ToReal(VARIABLE_IMAG34_DATA(matrix->matrixElements + i), &y.Imag);
      mulComplexComplex(&y, x, &y, &ctxtReal39);
      realToReal34(&y.Real, VARIABLE_REAL34_DATA(res->matrixElements + i));
      realToReal34(&y.Imag, VARIABLE_IMAG34_DATA(res->matrixElements + i));
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function _multiplyComplexMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


/*
//replaced due to suspicion that the real-only optimization is not working properly. I did not check in detail and removed fance stuff until the crash disappeared.
static void mulCpxMat(const real_t *y, const real_t *x, uint16_t sizeY, uint16_t sizeYX, uint16_t sizeX, real_t *res, realContext_t *realContext) {
  int32_t i, j, k;
  real_t *sumr, prodr;
  real_t *sumi, prodi;

  for(i = 0; i < sizeY; ++i) {
    for(j = 0; j < sizeX; ++j) {
      sumr = res + (i * sizeX + j) * 2;
      sumi = sumr + 1;
      realSetZero(sumr);
      realSetZero(sumi);
      realSetZero(&prodr);
      realSetZero(&prodi);
      for(k = 0; k < sizeYX; ++k) {
        if(realIsZero(y + (i * sizeYX + k) * 2 + 1) && realIsZero(x + (k * sizeX + j) * 2 + 1)) {
          realFMA(y + (i * sizeYX + k) * 2, x + (k * sizeX + j) * 2, sumr, sumr, realContext);
        }
        else {
          mulComplexComplex(y + (i * sizeYX + k) * 2, y + (i * sizeYX + k) * 2 + 1,
                            x + (k * sizeX  + j) * 2, x + (k * sizeX  + j) * 2 + 1,
                            &prodr, &prodi, realContext);
          realAdd(sumr, &prodr, sumr, realContext);
          realAdd(sumi, &prodi, sumi, realContext);
        }
      }
    }
  }
}
*/

static void mulCpxMat(const cplx_t *y, const cplx_t *x, uint16_t sizeY, uint16_t sizeYX, uint16_t sizeX, cplx_t *res, realContext_t *realContext) {
  int32_t i, j, k;
  cplx_t *sum;
  cplx_t prod;

  for(i = 0; i < sizeY; ++i) {
    for(j = 0; j < sizeX; ++j) {
      sum = &res[i * sizeX + j];
      realSetZero(&sum->Real);
      realSetZero(&sum->Imag);
      for(k = 0; k < sizeYX; ++k) {
        // Always use full complex multiplication for consistency
        mulComplexComplex(&y[i * sizeYX + k], &x[k * sizeX + j], &prod, realContext);
        realAdd(&sum->Real, &prod.Real, &sum->Real, realContext);
        realAdd(&sum->Imag, &prod.Imag, &sum->Imag, realContext);
      }
    }
  }
}


void multiplyComplexMatrices(const complex34Matrix_t *y, const complex34Matrix_t *x, complex34Matrix_t *res) {
  const uint16_t rows = y->header.matrixRows;
  const uint16_t iter = y->header.matrixColumns;
  const uint16_t cols = x->header.matrixColumns;
  int32_t i, j, k;
  cplx_t sum, absoluteSum, prod, p, q;
  realContext_t *dotContext;
  cplx_t correction;

  if(y->header.matrixColumns != x->header.matrixRows) {
    res->matrixElements = NULL; // Matrix mismatch
    res->header.matrixRows = res->header.matrixColumns = 0;
    return;
  }

  if(complexMatrixInit(res, rows, cols)) {
    for(i = 0; i < rows; ++i) {
      for(j = 0; j < cols; ++j) {
        dotContext = &ctxtReal39;
        do {
          realSetZero(&sum.Real);
          realSetZero(&sum.Imag);
          realSetZero(&absoluteSum.Real);
          realSetZero(&absoluteSum.Imag);
          realSetZero(&correction.Real);
          realSetZero(&correction.Imag);
          for(k = 0; k < iter; ++k) {
            real34ToReal(VARIABLE_REAL34_DATA(y->matrixElements + (i * iter + k)), &p.Real);
            real34ToReal(VARIABLE_IMAG34_DATA(y->matrixElements + (i * iter + k)), &p.Imag);
            real34ToReal(VARIABLE_REAL34_DATA(x->matrixElements + (k * cols + j)), &q.Real);
            real34ToReal(VARIABLE_IMAG34_DATA(x->matrixElements + (k * cols + j)), &q.Imag);
            if(realIsSpecial(&p.Real) || realIsSpecial(&p.Imag) || realIsSpecial(&q.Real) || realIsSpecial(&q.Imag)) {
              mulComplexComplex(&p, &q, &prod, dotContext);
              matrixAccumulate(&prod.Real, &sum.Real, &correction.Real, dotContext);
              matrixAccumulate(&prod.Imag, &sum.Imag, &correction.Imag, dotContext);
            }
            else {
              matrixAccumulateProduct(&p.Real, &q.Real, false, &sum.Real, &correction.Real, &absoluteSum.Real, dotContext);
              matrixAccumulateProduct(&p.Imag, &q.Imag, true,  &sum.Real, &correction.Real, &absoluteSum.Real, dotContext);
              matrixAccumulateProduct(&p.Real, &q.Imag, false, &sum.Imag, &correction.Imag, &absoluteSum.Imag, dotContext);
              matrixAccumulateProduct(&p.Imag, &q.Real, false, &sum.Imag, &correction.Imag, &absoluteSum.Imag, dotContext);
            }
          }
          matrixFinishAccumulator(&sum.Real, &correction.Real, &sum.Real, dotContext);
          matrixFinishAccumulator(&sum.Imag, &correction.Imag, &sum.Imag, dotContext);
          if(dotContext == &ctxtReal39 && (matrixNeedsExtendedAccumulator(&absoluteSum.Real, &sum.Real) || matrixNeedsExtendedAccumulator(&absoluteSum.Imag, &sum.Imag))) {
            dotContext = &ctxtReal75;
          }
          else {
            break;
          }
        } while(true);
        realToReal34(&sum.Real, VARIABLE_REAL34_DATA(res->matrixElements + (i * cols + j)));
        realToReal34(&sum.Imag, VARIABLE_IMAG34_DATA(res->matrixElements + (i * cols + j)));
      }
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function multiplyComplexMatrices:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


/**
 * \brief Raise a finite value in [0,1] to a positive integer power.
 */
static void matrixPositiveIntegerPower(const real_t *base, uint16_t exponent, real_t *result, realContext_t *realContext) {
  REAL_T_PTR(factor, 75);
  REAL_T_PTR(product, 75);

  realCopy(base, factor);
  realSetOne(result);
  while(exponent != 0) {
    if(exponent & 1) {
      realMultiply(result, factor, product, realContext);
      realCopy(product, result);
    }
    exponent >>= 1;
    if(exponent != 0) {
      realMultiply(factor, factor, product, realContext);
      realCopy(product, factor);
    }
  }
}


/**
 * \brief Take the positive p-th root used to close a scaled matrix norm.
 */
static void matrixPositiveIntegerRoot(const real_t *value, const real_t *pReal, uint16_t pParam, real_t *result, realContext_t *realContext) {
  if(pParam == 1) {
    realCopy(value, result);
  }
  else if(pParam == 2) {
    realSquareRoot(value, result, realContext);
  }
  else {
    WP34S_Ln(value, result, realContext);
    realDivide(result, pReal, result, realContext);
    realExp(result, result, realContext);
  }
}


/* Euclidean (Frobenius) norm */
static void _euclideanNormRealMatrix(const real34Matrix_t *matrix, uint16_t pParam, real_t *res, realContext_t *realContext) {
  REAL_T_PTR(elem, 75);
  REAL_T_PTR(scale, 75);
  REAL_T_PTR(pReal, 75);

  realSetZero(scale);
  for(int i = 0; i < matrix->header.matrixRows * matrix->header.matrixColumns; ++i) {
    real34ToReal(matrix->matrixElements + i, elem);
    realSetPositiveSign(elem);
    if(realIsNaN(elem)) {
      realCopy(elem, res);
      return;
    }
    if(realCompareGreaterThan(elem, scale)) {
      realCopy(elem, scale);
    }
  }
  if(realIsInfinite(scale) || realIsZero(scale)) {
    realCopy(scale, res);
    return;
  }

  uInt32ToReal(pParam, pReal);
  realSetZero(res);
  for(int i = 0; i < matrix->header.matrixRows * matrix->header.matrixColumns; ++i) {
    real34ToReal(matrix->matrixElements + i, elem);
    realSetPositiveSign(elem);
    realDivide(elem, scale, elem, realContext);
    matrixPositiveIntegerPower(elem, pParam, elem, realContext);
    realAdd(res, elem, res, realContext);
  }
  matrixPositiveIntegerRoot(res, pReal, pParam, res, realContext);
  realMultiply(scale, res, res, realContext);
}


void euclideanNormRealMatrix(const real34Matrix_t *matrix, uint16_t pParam, real34_t *res) {
  real_t sum;

  _euclideanNormRealMatrix(matrix, pParam, &sum, &ctxtReal39);
  realToReal34(&sum, res);
}


void euclideanNormComplexMatrix(const complex34Matrix_t *matrix, uint16_t pParam, real34_t *res) {
  cplx_t elem;
  REAL_T_PTR(scale, 75);
  REAL_T_PTR(pReal, 75);
  REAL_T_PTR(sum, 75);

  realSetZero(scale);
  for(int i = 0; i < matrix->header.matrixRows * matrix->header.matrixColumns; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(matrix->matrixElements + i), &elem.Real);
    real34ToReal(VARIABLE_IMAG34_DATA(matrix->matrixElements + i), &elem.Imag);
    complexMagnitude(&elem, &elem.Real, &ctxtReal39);
    if(realIsNaN(&elem.Real)) {
      realToReal34(&elem.Real, res);
      return;
    }
    if(realCompareGreaterThan(&elem.Real, scale)) {
      realCopy(&elem.Real, scale);
    }
  }
  if(realIsInfinite(scale) || realIsZero(scale)) {
    realToReal34(scale, res);
    return;
  }

  uInt32ToReal(pParam, pReal);
  realSetZero(sum);
  for(int i = 0; i < matrix->header.matrixRows * matrix->header.matrixColumns; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(matrix->matrixElements + i), &elem.Real);
    real34ToReal(VARIABLE_IMAG34_DATA(matrix->matrixElements + i), &elem.Imag);
    complexMagnitude(&elem, &elem.Real, &ctxtReal39);
    realDivide(&elem.Real, scale, &elem.Real, &ctxtReal39);
    matrixPositiveIntegerPower(&elem.Real, pParam, &elem.Real, &ctxtReal39);
    realAdd(sum, &elem.Real, sum, &ctxtReal39);
  }
  matrixPositiveIntegerRoot(sum, pReal, pParam, sum, &ctxtReal39);
  realMultiply(scale, sum, sum, &ctxtReal39);
  realToReal34(sum, res);
}


/* Vectors */
uint16_t realVectorSize(const real34Matrix_t *matrix) {
  if((matrix->header.matrixColumns != 1) && (matrix->header.matrixRows != 1)) {
    return 0;
  }
  else {
    return matrix->header.matrixColumns * matrix->header.matrixRows;
  }
}


static void _dotRealVectors(const real34Matrix_t *y, const real34Matrix_t *x, real_t *res, realContext_t *realContext) {
  const uint16_t elements = realVectorSize(y);
  int32_t i;
  real_t sum, absoluteSum, p, q;
  realContext_t *dotContext = realContext;
  REAL_T_PTR(correction, 75);

  do {
    realSetZero(&sum);
    realSetZero(&absoluteSum);
    realSetZero(correction);
    for(i = 0; i < elements; ++i) {
      real34ToReal(y->matrixElements + i, &p);
      real34ToReal(x->matrixElements + i, &q);
      matrixAccumulateProduct(&p, &q, false, &sum, correction, &absoluteSum, dotContext);
    }
    matrixFinishAccumulator(&sum, correction, res, dotContext);
    if(dotContext->digits <= 39 && matrixNeedsExtendedAccumulator(&absoluteSum, res)) {
      dotContext = &ctxtReal75;
    }
    else {
      break;
    }
  } while(true);
}


void dotRealVectors(const real34Matrix_t *y, const real34Matrix_t *x, real34_t *res) {
  real_t p;

  if((realVectorSize(y) == 0) || (realVectorSize(x) == 0) || (realVectorSize(y) != realVectorSize(x))) {
    realToReal34(const_NaN, res); // Not a vector or mismatched
    return;
  }

  _dotRealVectors(y, x, &p, &ctxtReal39);
  realToReal34(&p, res);
}


void crossRealVectors(const real34Matrix_t *y, const real34Matrix_t *x, real34Matrix_t *res) {
  const uint16_t elementsY = realVectorSize(y);
  const uint16_t elementsX = realVectorSize(x);
  real_t a1, a2, a3, b1, b2, b3, p, q;

  if((elementsY == 0) || (elementsX == 0) || (elementsY > 3) || (elementsX > 3)) {
    return; // Not a vector or mismatched
  }

  real34ToReal(                 y->matrixElements            , &a1);
  real34ToReal(elementsY >= 2 ? y->matrixElements + 1 : const34_0, &a2);
  real34ToReal(elementsY >= 3 ? y->matrixElements + 2 : const34_0, &a3);

  real34ToReal(                 x->matrixElements            , &b1);
  real34ToReal(elementsX >= 2 ? x->matrixElements + 1 : const34_0, &b2);
  real34ToReal(elementsX >= 3 ? x->matrixElements + 2 : const34_0, &b3);

  if(realMatrixInit(res, 1, 3)) {
    realCopy(&b2, &q);
    realChangeSign(&q);
    dotCplx(&a2, &a3, &b3, &q, &p, &ctxtReal39);
    realToReal34(&p, res->matrixElements);

    realCopy(&b3, &q);
    realChangeSign(&q);
    dotCplx(&a3, &a1, &b1, &q, &p, &ctxtReal39);
    realToReal34(&p, res->matrixElements + 1);

    realCopy(&b1, &q);
    realChangeSign(&q);
    dotCplx(&a1, &a2, &b2, &q, &p, &ctxtReal39);
    realToReal34(&p, res->matrixElements + 2);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function crossRealVectors:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


uint16_t complexVectorSize(const complex34Matrix_t *matrix) {
  return realVectorSize((const real34Matrix_t *)matrix);
}


/**
 * \brief Evaluate the difference of two complex products with compensated component sums.
 *
 * P39 is used first. A component that loses more than twenty decimal places is evaluated again at p75.
 *
 * \param[in] a1 First left factor.
 * \param[in] b1 First right factor.
 * \param[in] a2 Second left factor.
 * \param[in] b2 Second right factor.
 * \param[out] res Difference.
 */
static void matrixComplexProductDifference(const cplx_t *a1, const cplx_t *b1, const cplx_t *a2, const cplx_t *b2, cplx_t *res) {
  cplx_t correction, second, sum, absoluteSum;
  realContext_t *workContext = &ctxtReal39;

  if(realIsSpecial(&a1->Real) || realIsSpecial(&a1->Imag) || realIsSpecial(&b1->Real) || realIsSpecial(&b1->Imag)
     || realIsSpecial(&a2->Real) || realIsSpecial(&a2->Imag) || realIsSpecial(&b2->Real) || realIsSpecial(&b2->Imag)) {
    mulComplexComplex(a1, b1, res, &ctxtReal39);
    mulComplexComplex(a2, b2, &second, &ctxtReal39);
    realSubtract(&res->Real, &second.Real, &res->Real, &ctxtReal39);
    realSubtract(&res->Imag, &second.Imag, &res->Imag, &ctxtReal39);
    return;
  }

  do {
    realSetZero(&sum.Real);
    realSetZero(&sum.Imag);
    realSetZero(&absoluteSum.Real);
    realSetZero(&absoluteSum.Imag);
    realSetZero(&correction.Real);
    realSetZero(&correction.Imag);

    matrixAccumulateProduct(&a1->Real, &b1->Real, false, &sum.Real, &correction.Real, &absoluteSum.Real, workContext);
    matrixAccumulateProduct(&a1->Imag, &b1->Imag, true,  &sum.Real, &correction.Real, &absoluteSum.Real, workContext);
    matrixAccumulateProduct(&a2->Real, &b2->Real, true,  &sum.Real, &correction.Real, &absoluteSum.Real, workContext);
    matrixAccumulateProduct(&a2->Imag, &b2->Imag, false, &sum.Real, &correction.Real, &absoluteSum.Real, workContext);

    matrixAccumulateProduct(&a1->Real, &b1->Imag, false, &sum.Imag, &correction.Imag, &absoluteSum.Imag, workContext);
    matrixAccumulateProduct(&a1->Imag, &b1->Real, false, &sum.Imag, &correction.Imag, &absoluteSum.Imag, workContext);
    matrixAccumulateProduct(&a2->Real, &b2->Imag, true,  &sum.Imag, &correction.Imag, &absoluteSum.Imag, workContext);
    matrixAccumulateProduct(&a2->Imag, &b2->Real, true,  &sum.Imag, &correction.Imag, &absoluteSum.Imag, workContext);

    matrixFinishAccumulator(&sum.Real, &correction.Real, &res->Real, workContext);
    matrixFinishAccumulator(&sum.Imag, &correction.Imag, &res->Imag, workContext);
    if(workContext == &ctxtReal39 && (matrixNeedsExtendedAccumulator(&absoluteSum.Real, &res->Real) || matrixNeedsExtendedAccumulator(&absoluteSum.Imag, &res->Imag))) {
      workContext = &ctxtReal75;
    }
    else {
      break;
    }
  } while(true);
}


void dotComplexVectors(const complex34Matrix_t *y, const complex34Matrix_t *x, real34_t *res_r, real34_t *res_i) {
  const uint16_t elements = complexVectorSize(y);
  int32_t i;
  cplx_t sum, absoluteSum, prod, p, q;
  realContext_t *dotContext = &ctxtReal39;
  cplx_t correction;

  if((complexVectorSize(y) == 0) || (complexVectorSize(x) == 0) || (complexVectorSize(y) != complexVectorSize(x))) {
    realToReal34(const_NaN, res_r);
    realToReal34(const_NaN, res_i); // Not a vector or mismatched
    return;
  }

  do {
    realSetZero(&sum.Real);
    realSetZero(&sum.Imag);
    realSetZero(&absoluteSum.Real);
    realSetZero(&absoluteSum.Imag);
    realSetZero(&correction.Real);
    realSetZero(&correction.Imag);
    for(i = 0; i < elements; ++i) {
      real34ToReal(VARIABLE_REAL34_DATA(y->matrixElements + i), &p.Real);
      real34ToReal(VARIABLE_IMAG34_DATA(y->matrixElements + i), &p.Imag);
      real34ToReal(VARIABLE_REAL34_DATA(x->matrixElements + i), &q.Real);
      real34ToReal(VARIABLE_IMAG34_DATA(x->matrixElements + i), &q.Imag);
      if(realIsSpecial(&p.Real) || realIsSpecial(&p.Imag) || realIsSpecial(&q.Real) || realIsSpecial(&q.Imag)) {
        mulComplexComplex(&p, &q, &prod, dotContext);
        matrixAccumulate(&prod.Real, &sum.Real, &correction.Real, dotContext);
        matrixAccumulate(&prod.Imag, &sum.Imag, &correction.Imag, dotContext);
      }
      else {
        matrixAccumulateProduct(&p.Real, &q.Real, false, &sum.Real, &correction.Real, &absoluteSum.Real, dotContext);
        matrixAccumulateProduct(&p.Imag, &q.Imag, true,  &sum.Real, &correction.Real, &absoluteSum.Real, dotContext);
        matrixAccumulateProduct(&p.Real, &q.Imag, false, &sum.Imag, &correction.Imag, &absoluteSum.Imag, dotContext);
        matrixAccumulateProduct(&p.Imag, &q.Real, false, &sum.Imag, &correction.Imag, &absoluteSum.Imag, dotContext);
      }
    }
    matrixFinishAccumulator(&sum.Real, &correction.Real, &sum.Real, dotContext);
    matrixFinishAccumulator(&sum.Imag, &correction.Imag, &sum.Imag, dotContext);
    if(dotContext == &ctxtReal39 && (matrixNeedsExtendedAccumulator(&absoluteSum.Real, &sum.Real) || matrixNeedsExtendedAccumulator(&absoluteSum.Imag, &sum.Imag))) {
      dotContext = &ctxtReal75;
    }
    else {
      break;
    }
  } while(true);
  realToReal34(&sum.Real, res_r);
  realToReal34(&sum.Imag, res_i);
}


void crossComplexVectors(const complex34Matrix_t *y, const complex34Matrix_t *x, complex34Matrix_t *res) {
  const uint16_t elementsY = complexVectorSize(y);
  const uint16_t elementsX = complexVectorSize(x);
  cplx_t a1, a2, a3, b1, b2, b3, p;

  if((elementsY == 0) || (elementsX == 0) || (elementsY > 3) || (elementsX > 3)) {
    return; // Not a vector or mismatched
  }

  real34ToReal(                 VARIABLE_REAL34_DATA(y->matrixElements)            , &a1.Real);
  real34ToReal(                 VARIABLE_IMAG34_DATA(y->matrixElements)            , &a1.Imag);
  real34ToReal(elementsY >= 2 ? VARIABLE_REAL34_DATA(y->matrixElements + 1) : const34_0, &a2.Real);
  real34ToReal(elementsY >= 2 ? VARIABLE_IMAG34_DATA(y->matrixElements + 1) : const34_0, &a2.Imag);
  real34ToReal(elementsY >= 3 ? VARIABLE_REAL34_DATA(y->matrixElements + 2) : const34_0, &a3.Real);
  real34ToReal(elementsY >= 3 ? VARIABLE_IMAG34_DATA(y->matrixElements + 2) : const34_0, &a3.Imag);

  real34ToReal(                 VARIABLE_REAL34_DATA(x->matrixElements)            , &b1.Real);
  real34ToReal(                 VARIABLE_IMAG34_DATA(x->matrixElements)            , &b1.Imag);
  real34ToReal(elementsX >= 2 ? VARIABLE_REAL34_DATA(x->matrixElements + 1) : const34_0, &b2.Real);
  real34ToReal(elementsX >= 2 ? VARIABLE_IMAG34_DATA(x->matrixElements + 1) : const34_0, &b2.Imag);
  real34ToReal(elementsX >= 3 ? VARIABLE_REAL34_DATA(x->matrixElements + 2) : const34_0, &b3.Real);
  real34ToReal(elementsX >= 3 ? VARIABLE_IMAG34_DATA(x->matrixElements + 2) : const34_0, &b3.Imag);

  if(complexMatrixInit(res, 1, 3)) {
    matrixComplexProductDifference(&a2, &b3, &a3, &b2, &p);
    realToReal34(&p.Real, VARIABLE_REAL34_DATA(res->matrixElements));
    realToReal34(&p.Imag, VARIABLE_IMAG34_DATA(res->matrixElements));

    matrixComplexProductDifference(&a3, &b1, &a1, &b3, &p);
    realToReal34(&p.Real, VARIABLE_REAL34_DATA(res->matrixElements + 1));
    realToReal34(&p.Imag, VARIABLE_IMAG34_DATA(res->matrixElements + 1));

    matrixComplexProductDifference(&a1, &b2, &a2, &b1, &p);
    realToReal34(&p.Real, VARIABLE_REAL34_DATA(res->matrixElements + 2));
    realToReal34(&p.Imag, VARIABLE_IMAG34_DATA(res->matrixElements + 2));
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function crossComplexVectors:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


/**
 * \brief Calculate one compensated component of a real-vector cross product.
 */
static void matrixRealVectorCrossComponent(const real34_t *a, const real34_t *b, const real34_t *c, const real34_t *d, real_t *component) {
  REAL_T_PTR(aReal, 39);
  REAL_T_PTR(bReal, 39);
  REAL_T_PTR(cReal, 39);
  REAL_T_PTR(dReal, 39);

  real34ToReal(a, aReal);
  real34ToReal(b, bReal);
  real34ToReal(c, cReal);
  real34ToReal(d, dReal);
  if(realIsSpecial(aReal) || realIsSpecial(bReal) || realIsSpecial(cReal) || realIsSpecial(dReal)) {
    realSetNaN(component);
    return;
  }
  realChangeSign(dReal);
  dotCplx(aReal, bReal, cReal, dReal, component, &ctxtReal39);
}


/**
 * \brief Calculate the magnitude of a two- or three-dimensional real-vector cross product.
 *
 * Each component uses the compensated two-product kernel at p39.  Keeping the components wide preserves a small angle which would disappear if a cross product
 * matrix were converted to Real34 before its norm was calculated.
 */
static void matrixRealVectorCrossMagnitude(const real34Matrix_t *y, const real34Matrix_t *x, real_t *magnitude) {
  const uint16_t elements = realVectorSize(y);
  cplx_t c;

  if(elements == 2) {
    matrixRealVectorCrossComponent(y->matrixElements, y->matrixElements + 1, x->matrixElements + 1, x->matrixElements, magnitude);
    realSetPositiveSign(magnitude);
    return;
  }

  matrixRealVectorCrossComponent(y->matrixElements + 1, y->matrixElements + 2, x->matrixElements + 2, x->matrixElements + 1, &c.Real);
  matrixRealVectorCrossComponent(y->matrixElements + 2, y->matrixElements,     x->matrixElements,     x->matrixElements + 2, &c.Imag);
  complexMagnitude(&c, &c.Real, &ctxtReal39);
  matrixRealVectorCrossComponent(y->matrixElements,     y->matrixElements + 1, x->matrixElements + 1, x->matrixElements,     &c.Imag);
  complexMagnitude(&c, magnitude, &ctxtReal39);
}


/**
 * \brief Calculate the angle between two real vectors in radians.
 *
 * \param[in] y A two- or three-dimensional Real34 vector.
 * \param[in] x A Real34 vector with the same dimension as y.
 * \param[out] radians The p39 radian result, or NaN for invalid vectors.
 */
void vectorAngle(const real34Matrix_t *y, const real34Matrix_t *x, real_t *radians) {
  const uint16_t elements = realVectorSize(y);
  REAL_T_PTR(crossMagnitude, 39);
  REAL_T_PTR(dot, 75);

  if((elements == 0) || (realVectorSize(x) == 0) || (elements != realVectorSize(x))) {
    realSetNaN(radians); // Not a vector or mismatched
    return;
  }

  if(elements == 2 || elements == 3) {
    matrixRealVectorCrossMagnitude(y, x, crossMagnitude);
    if(realIsNaN(crossMagnitude)) {
      realCopy(crossMagnitude, radians);
      return;
    }
    _dotRealVectors(y, x, dot, &ctxtReal39);
    if(realIsZero(crossMagnitude) && realIsZero(dot)) {
      realSetNaN(radians);
      return;
    }
    C47_WP34S_Atan2(crossMagnitude, dot, radians, &ctxtReal39);
  }
  else {
    realSetNaN(radians);
  }
}


/* LU decomposition routine borrowed from WP 34s */
void WP34S_LU_decomposition(const real34Matrix_t *matrix, real34Matrix_t *lu, uint16_t *p) {
  int i, j, k;
  int pvt;
  real_t max, t, u, tolerance;
  real_t *tmpMat;

  const uint16_t m = matrix->header.matrixRows;
  const uint16_t n = matrix->header.matrixColumns;

  if(matrix->header.matrixRows != matrix->header.matrixColumns) {
    if(matrix != lu) {
      lu->matrixElements = NULL; // Matrix is not square
      lu->header.matrixRows = lu->header.matrixColumns = 0;
    }
    return;
  }

  if(realMatExactlySingular(matrix)) {
    if(matrix != lu) {
      lu->matrixElements = NULL;
      lu->header.matrixRows = lu->header.matrixColumns = 0;
    }
    return;
  }

  if((tmpMat = allocC47Blocks(m * n * REAL_SIZE_IN_BLOCKS(75)))) {
    if(matrix != lu) {
      copyRealMatrix(matrix, lu);
    }

    if(lu->matrixElements) {
      realCopy(const_1, &tolerance);
      tolerance.exponent = 2 - ctxtReal75.digits;
      for(i = 0; i < n; i++) {
        for(j = 0; j < n; j++) {
          real34ToReal(lu->matrixElements + (i * n + j), &tmpMat[i * n + j]);
        }
      }

      for(k = 0; k < n; k++) {
        /* Find the pivot row */
        pvt = k;
        realCopy(&tmpMat[k * n + k], &u);
        realCopyAbs(&u, &max);
        for(j = k + 1; j < n; j++) {
          realCopy(&tmpMat[j * n + k], &t);
          realCopyAbs(&t, &u);
          if(realCompareGreaterThan(&u, &max)) {
            realCopy(&u, &max);
            pvt = j;
          }
        }
        if(p != NULL) {
          *p++ = pvt;
        }

        /* pivot if required */
        if(pvt != k) {
          for(i = 0; i < n; i++) {
            realCopy(&tmpMat[k   * n + i], &t                  );
            realCopy(&tmpMat[pvt * n + i], &tmpMat[k   * n + i]);
            realCopy(&t,                   &tmpMat[pvt * n + i]);
          }
        }

        /* Check for singular */
        realCopy(&tmpMat[k * n + k], &t);
        if(realIsZero(&t)) {
          realMatrixFree(lu);
          freeC47Blocks(tmpMat, m * n * REAL_SIZE_IN_BLOCKS(75));
          return;
        }

        /* Find the lower triangular elements for column k */
        for(i = k + 1; i < n; i++) {
          realCopy(&tmpMat[k * n + k], &t);
          realCopy(&tmpMat[i * n + k], &u);
          realDivide(&u, &t, &max, &ctxtReal75);
          realCopy(&max, &tmpMat[i * n + k]);
        }
        /* Update the upper triangular elements */
        for(i = k + 1; i < n; i++) {
          for(j = k + 1; j < n; j++) {
            realCopy(&tmpMat[i * n + k], &t);
            realCopy(&tmpMat[k * n + j], &u);
            realMultiply(&t, &u, &max, &ctxtReal75);
            realCopy(&tmpMat[i * n + j], &t);
            realSubtract(&t, &max, &u, &ctxtReal75);
            realDivide(&u, &t, &max, &ctxtReal75); // condition number
            if(realCompareAbsLessThan(&max, &tolerance)) {
              realSetZero(&tmpMat[i * n + j]); // prevent ill-conditionedness (likely singular)
            }
            else {
              realCopy(&u, &tmpMat[i * n + j]);
            }
          }
        }
      }

      for(i = 0; i < n; i++) {
        for(j = 0; j < n; j++) {
          realToReal34(&tmpMat[i * n + j], lu->matrixElements + (i * n + j));
        }
      }
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 1k");
        moreInfoOnError("In function WP34S_LU_decomposition:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }

    freeC47Blocks(tmpMat, m * n * REAL_SIZE_IN_BLOCKS(75));
  }
  else {
    if(matrix != lu) {
      lu->matrixElements = NULL;
      lu->header.matrixRows = lu->header.matrixColumns = 0;
    }
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full, 2l");
      moreInfoOnError("In function WP34S_LU_decomposition:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


static bool_t luCpxMat(cplx_t *tmpMat, uint16_t size, uint16_t *p, int32_t toleranceExponent, realContext_t *realContext) {
  int i, j, k;
  int pvt;
  real_t max, t, u, tolerance;
  cplx_t prod, previous;

  const uint16_t n = size;

  realCopy(const_1, &tolerance);
  tolerance.exponent = toleranceExponent != 0 ? toleranceExponent : 2 - realContext->digits;  // an update cancelling below this fraction of its value is set to zero
  for(k = 0; k < n; k++) {
    /* Find the pivot row */
    pvt = k;
    complexMagnitude(&tmpMat[k * n + k], &max, realContext);
    for(j = k + 1; j < n; j++) {
      complexMagnitude(&tmpMat[j * n + k], &u, realContext);
      if(realCompareGreaterThan(&u, &max)) {
        realCopy(&u, &max);
        pvt = j;
      }
    }
    if(p != NULL) {
      *p++ = pvt;
    }

    /* pivot if required */
    if(pvt != k) {
      for(i = 0; i < n; i++) {
        realCopy(&tmpMat[k   * n + i].Real, &t                        );
        realCopy(&tmpMat[pvt * n + i].Real, &tmpMat[k   * n + i].Real);
        realCopy(&t,                        &tmpMat[pvt * n + i].Real);
        realCopy(&tmpMat[k   * n + i].Imag, &t                        );
        realCopy(&tmpMat[pvt * n + i].Imag, &tmpMat[k   * n + i].Imag);
        realCopy(&t,                        &tmpMat[pvt * n + i].Imag);
      }
    }

    /* Check for singular */
    if(realIsZero(&tmpMat[k * n + k].Real) && realIsZero(&tmpMat[k * n + k].Imag)) {
      return false;
    }

    /* Find the lower triangular elements for column k */
    for(i = k + 1; i < n; i++) {
      divComplexComplex(&tmpMat[i * n + k], &tmpMat[k * n + k], &tmpMat[i * n + k], realContext);
    }
    /* Update the upper triangular elements */
    for(i = k + 1; i < n; i++) {
      for(j = k + 1; j < n; j++) {
        mulComplexComplex(&tmpMat[i * n + k], &tmpMat[k * n + j], &prod, realContext);
        realCopy(&tmpMat[i * n + j].Real, &previous.Real);
        realCopy(&tmpMat[i * n + j].Imag, &previous.Imag);
        realSubtract(&previous.Real, &prod.Real, &tmpMat[i * n + j].Real, realContext);
        realSubtract(&previous.Imag, &prod.Imag, &tmpMat[i * n + j].Imag, realContext);
        complexMagnitude(&tmpMat[i * n + j], &t, realContext);
        complexMagnitude(&previous, &u, realContext);
        realDivide(&t, &u, &t, realContext); // condition number
        if(realCompareAbsLessThan(&t, &tolerance)) {
          realSetZero(&tmpMat[i * n + j].Real);      // prevent ill-conditionedness
          realSetZero(&tmpMat[i * n + j].Imag);
        }
      }
    }
  }

  return true;
}


void complex_LU_decomposition(const complex34Matrix_t *matrix, complex34Matrix_t *lu, uint16_t *p) {
  int i, j;
  cplx_t *tmpMat;

  const uint16_t m = matrix->header.matrixRows;
  const uint16_t n = matrix->header.matrixColumns;

  if(matrix->header.matrixRows != matrix->header.matrixColumns) {
    if(matrix != lu) {
      lu->matrixElements = NULL; // Matrix is not square
      lu->header.matrixRows = lu->header.matrixColumns = 0;
    }
    return;
  }

  if((tmpMat = allocC47Blocks(m * n * REAL_SIZE_IN_BLOCKS(75) * 2))) {
    if(matrix != lu) {
      copyComplexMatrix(matrix, lu);
    }

    if(lu->matrixElements) {
      for(i = 0; i < n; i++) {
        for(j = 0; j < n; j++) {
          real34ToReal(VARIABLE_REAL34_DATA(lu->matrixElements + (i * n + j)), &tmpMat[i * n + j].Real);
          real34ToReal(VARIABLE_IMAG34_DATA(lu->matrixElements + (i * n + j)), &tmpMat[i * n + j].Imag);
        }
      }

      if(luCpxMat(tmpMat, n, p, 0, &ctxtReal75)) {
        for(i = 0; i < n; i++) {
          for(j = 0; j < n; j++) {
            realToReal34(&tmpMat[i * n + j].Real, VARIABLE_REAL34_DATA(lu->matrixElements + (i * n + j)));
            realToReal34(&tmpMat[i * n + j].Imag, VARIABLE_IMAG34_DATA(lu->matrixElements + (i * n + j)));
          }
        }
      }
      else {
        complexMatrixFree(lu);
      }
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 1m");
        moreInfoOnError("In function complex_LU_decomposition:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }

    freeC47Blocks(tmpMat, m * n * REAL_SIZE_IN_BLOCKS(75) * 2);
  }
  else {
    if(matrix != lu) {
      lu->matrixElements = NULL; // Matrix is not square
      lu->header.matrixRows = lu->header.matrixColumns = 0;
    }
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full, 2n");
      moreInfoOnError("In function complex_LU_decomposition:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


/* Swap 2 rows / cols*/
static void _anyMatrixSwap(const any34Matrix_t *matrix, any34Matrix_t *res, uint16_t a, uint16_t b, bool_t isRow, const matrixKind_t *kind) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  const uint16_t limit = isRow ? rows : cols;
  const size_t   size = kind->elementSize;
  uint16_t i;
  complex34_t t;                                                              // large enough for an element of either kind

  if(matrix != res) {
    kind->copyMatrix(matrix, res);
  }
  if(MATRIX_BYTES(res)) {
    if((a < limit) && (b < limit) && (a != b)) {
      uint8_t *e = MATRIX_BYTES(res);
      for(i = 0; i < (isRow ? cols : rows); i++) {
        uint16_t ia = isRow ? a * cols + i : i * cols + a;
        uint16_t ib = isRow ? b * cols + i : i * cols + b;
        kind->copyElement(e + ia*size, &t);
        kind->copyElement(e + ib*size, e + ia*size);
        kind->copyElement(&t,          e + ib*size);
      }
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function _anyMatrixSwap:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}




void realMatrixSwapRows      (const real34Matrix_t    *matrix, real34Matrix_t    *res, uint16_t a, uint16_t b) { _anyMatrixSwap((const any34Matrix_t*)matrix, (any34Matrix_t*)res, a, b, true,  &matrixKindReal);    }
void realMatrixSwapColumns   (const real34Matrix_t    *matrix, real34Matrix_t    *res, uint16_t a, uint16_t b) { _anyMatrixSwap((const any34Matrix_t*)matrix, (any34Matrix_t*)res, a, b, false, &matrixKindReal);    }
void complexMatrixSwapRows   (const complex34Matrix_t *matrix, complex34Matrix_t *res, uint16_t a, uint16_t b) { _anyMatrixSwap((const any34Matrix_t*)matrix, (any34Matrix_t*)res, a, b, true,  &matrixKindComplex); }
void complexMatrixSwapColumns(const complex34Matrix_t *matrix, complex34Matrix_t *res, uint16_t a, uint16_t b) { _anyMatrixSwap((const any34Matrix_t*)matrix, (any34Matrix_t*)res, a, b, false, &matrixKindComplex); }


/* Determinant */
static void detCpxMat(const cplx_t *matrix, uint16_t size, cplx_t *res, realContext_t *realContext) {
  uint16_t *p;
  cplx_t *lu;
  cplx_t t;

  if((lu = allocC47Blocks(size * size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
    xcopy(lu, matrix, TO_BYTES(size * size * REAL_SIZE_IN_BLOCKS(75) * 2));
    if((p = allocC47Blocks(TO_BLOCKS(size * sizeof(uint16_t))))) {
      realSetOne(&t.Real);
      realSetZero(&t.Imag);
      if(luCpxMat(lu, size, p, 0, realContext)) {
        for(uint16_t i = 0; i < size; ++i) {
          mulComplexComplex(&t, &lu[i * size + i], &t, realContext);
        }
        for(uint16_t i = 0; i < size; ++i) {
          if(p[i] != i) {
            realChangeSign(&t.Real);
            realChangeSign(&t.Imag);
          }
        }
        realCopy(&t.Real, &res->Real);
        realCopy(&t.Imag, &res->Imag);
      }
      else { // singular
        realSetZero(&res->Real);
        realSetZero(&res->Imag);
      }

      freeC47Blocks(p, TO_BLOCKS(size * sizeof(uint16_t)));
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 1o");
        moreInfoOnError("In function detCpxMat:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      realSetNaN(&res->Real);
      realSetNaN(&res->Imag);
    }

    freeC47Blocks(lu, size * size * REAL_SIZE_IN_BLOCKS(75) * 2);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full, 2p");
      moreInfoOnError("In function detCpxMat:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    realSetNaN(&res->Real);
    realSetNaN(&res->Imag);
  }
}


/**
 * \brief Calculate the determinant of a real matrix by fraction-free (Bareiss) elimination, and report whether it is exact.
 *
 * After step k each element below and right of the pivot is a minor of order k + 2, and the division by the previous pivot is exact. The difference of two products
 * before it is the new minor times the previous pivot, at most 150 digits in the 159 of its context, so no step rounds while every minor fits in 75 digits, and an
 * exactly singular matrix gives exactly zero. A zero pivot is replaced by adding the first later row with a nonzero element in its column, which changes the minors
 * but not the determinant.
 *
 * \param[in,out] matrix Complex work matrix with zero imaginary parts. The real parts are overwritten.
 * \param[in] size Order of the matrix, at least two.
 * \param[out] res Determinant, set when the return value is true.
 * \return false for a NaN or infinite element, or as soon as a step rounds.
 */
static bool_t detRealMatExact(cplx_t *matrix, uint16_t size, real_t *res) {
  realContext_t context = ctxtReal75;
  REAL_T_PTR(difference, 159);
  const real_t *pivot = const_1;
  cplx_t *const end = matrix + size * size;

  for(cplx_t *element = matrix; element < end; element++) {
    if(realIsSpecial(&element->Real)) {
      return false;
    }
  }
  context.status = 0;
  for(int k = 0; k < size; k++) {  // k is the offset of the pivot column in a row
    cplx_t *rowK = matrix + k * size;
    cplx_t *rowP = rowK;
    while(realIsZero(&rowP[k].Real)) {
      rowP += size;
      if(rowP == end) {
        realSetZero(res);
        return true;
      }
    }
    if(rowP != rowK) {
      for(int j = k; j < size; j++) {
        realAdd(&rowK[j].Real, &rowP[j].Real, &rowK[j].Real, &context);
      }
    }
    for(cplx_t *rowI = rowK + size; rowI < end; rowI += size) {
      realChangeSign(&rowI[k].Real);
      for(int j = k + 1; j < size; j++) {
        context.digits = 159;
        realMultiply(&rowI[j].Real, &rowK[k].Real, difference, &context);
        realFMA(&rowI[k].Real, &rowK[j].Real, difference, difference, &context);
        context.digits = 75;
        realDivide(difference, pivot, &rowI[j].Real, &context);
      }
    }
    if(context.status & DEC_Inexact) {
      return false;
    }
    pivot = &rowK[k].Real;
  }
  realCopy(pivot, res);
  return true;
}


/**
 * \brief Report whether a real matrix is exactly singular, by the fraction-free elimination of detRealMatExact.
 *
 * An LU of a matrix of 34-digit numbers that is singular to the last digit can leave a residue where the pivot is zero, about 1E-72 of the matrix at 75 digits,
 * and then accepts the matrix. Where every minor fits in 75 digits the elimination is exact and gives exactly zero for it. Where a step rounds, the LU takes over.
 *
 * \param[in] matrix The square matrix.
 * \return true where the elimination reaches zero without rounding; false for a matrix of order one, where a step rounds, or where its work matrix cannot be
 * allocated.
 */
static bool_t realMatExactlySingular(const real34Matrix_t *matrix) {
  const uint16_t n = matrix->header.matrixColumns;
  cplx_t *work;
  real_t det;
  bool_t singular = false;

  if(n > 1 && (work = allocC47Blocks(n * n * REAL_SIZE_IN_BLOCKS(75) * 2))) {
    for(int i = 0; i < n * n; ++i) {
      real34ToReal(matrix->matrixElements + i, &work[i].Real);
      realSetZero(&work[i].Imag);
    }
    singular = detRealMatExact(work, n, &det) && realIsZero(&det);
    freeC47Blocks(work, n * n * REAL_SIZE_IN_BLOCKS(75) * 2);
  }
  return singular;
}


void detRealMatrix(const real34Matrix_t *matrix, real34_t *res) {
  const uint16_t n = matrix->header.matrixColumns;
  cplx_t *lu;
  cplx_t det;

  if(matrix->header.matrixRows != n) {
    realToReal34(const_NaN, res);
    return;
  }
  if(n == 1) {
    real34Copy(matrix->matrixElements, res);
    return;
  }

  if((lu = allocC47Blocks(n * n * REAL_SIZE_IN_BLOCKS(75) * 2))) {
    for(int i = 0; i < n * n; ++i) {
      real34ToReal(matrix->matrixElements + i, &lu[i].Real);
      realSetZero(&lu[i].Imag);
    }
    if(!detRealMatExact(lu, n, &det.Real)) {
      for(int i = 0; i < n * n; ++i) {
        real34ToReal(matrix->matrixElements + i, &lu[i].Real);
      }
      detCpxMat(lu, n, &det, &ctxtReal75);
    }
    freeC47Blocks(lu, n * n * REAL_SIZE_IN_BLOCKS(75) * 2);
    realToReal34(&det.Real, res);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function detRealMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    realToReal34(const_NaN, res);
  }
}


void detComplexMatrix(const complex34Matrix_t *matrix, real34_t *res_r, real34_t *res_i) {
  const uint16_t n = matrix->header.matrixColumns;
  cplx_t *lu;
  cplx_t det;

  if(matrix->header.matrixRows != n) {
    realToReal34(const_NaN, res_r);
    realToReal34(const_NaN, res_i);
    return;
  }
  if(n == 1) {
    real34Copy(VARIABLE_REAL34_DATA(matrix->matrixElements), res_r);
    real34Copy(VARIABLE_IMAG34_DATA(matrix->matrixElements), res_i);
    return;
  }

  if((lu = allocC47Blocks(n * n * REAL_SIZE_IN_BLOCKS(75) * 2))) {
    for(int i = 0; i < n * n; ++i) {
      real34ToReal(VARIABLE_REAL34_DATA(matrix->matrixElements + i), &lu[i].Real);
      real34ToReal(VARIABLE_IMAG34_DATA(matrix->matrixElements + i), &lu[i].Imag);
    }
    detCpxMat(lu, n, &det, &ctxtReal75);
    freeC47Blocks(lu, n * n * REAL_SIZE_IN_BLOCKS(75) * 2);
    realToReal34(&det.Real, res_r);
    realToReal34(&det.Imag, res_i);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function detComplexMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    realToReal34(const_NaN, res_r);
    realToReal34(const_NaN, res_i);
  }
}


/* Solve the linear equation Ax = b.
 * We do this by utilising the LU decomposition passed in in A and solving
 * the linear equation Ly = b for y, where L is the lower diagonal triangular
 * matrix with unity along the diagonal.  Then we solve the linear system
 * Ux = y, where U is the upper triangular matrix.
 */
static void complex_matrix_pivoting_solve(const cplx_t *LU, uint16_t n, cplx_t *b, uint16_t *pivot, cplx_t *x, realContext_t *realContext) {
  uint16_t i, k;
  cplx_t r, t;

  /* Solve the first linear equation Ly = b */
  for(k = 0; k < n; k++) {
    if(k != pivot[k]) {
      real_t swap;
      realCopy(&b[k].Real, &swap);
      realCopy(&b[pivot[k]].Real, &b[k].Real);
      realCopy(&swap, &b[pivot[k]].Real);
      realCopy(&b[k].Imag, &swap);
      realCopy(&b[pivot[k]].Imag, &b[k].Imag);
      realCopy(&swap, &b[pivot[k]].Imag);
    }
    realCopy(&b[k].Real, &x[k].Real);
    realCopy(&b[k].Imag, &x[k].Imag);
    for(i = 0; i < k; i++) {
      realCopy(&LU[k * n + i].Real, &r.Real);
      realCopy(&LU[k * n + i].Imag, &r.Imag);
      mulComplexComplex(&r, &x[i], &t, realContext);
      realSubtract(&x[k].Real, &t.Real, &x[k].Real, realContext);
      realSubtract(&x[k].Imag, &t.Imag, &x[k].Imag, realContext);
    }
  }

  /* Solve the second linear equation Ux = y */
  for(k = n; k > 0; k--) {
    --k;
    for(i = k + 1; i < n; i++) {
      realCopy(&LU[k * n + i].Real, &r.Real);
      realCopy(&LU[k * n + i].Imag, &r.Imag);
      mulComplexComplex(&r, &x[i], &t, realContext);
      realSubtract(&x[k].Real, &t.Real, &x[k].Real, realContext);
      realSubtract(&x[k].Imag, &t.Imag, &x[k].Imag, realContext);
    }
    realCopy(&LU[k * n + k].Real, &r.Real);
    realCopy(&LU[k * n + k].Imag, &r.Imag);
    divComplexComplex(&x[k], &r, &x[k], realContext);
    ++k;
  }
}


/* Invert a matrix
 * Do this by calculating the LU decomposition and solving lots of systems
 * of linear equations.
 */
static bool_t invCpxMat(cplx_t *matrix, uint16_t n, int32_t toleranceExponent, realContext_t *realContext) {
  cplx_t *x;
  cplx_t *lu;
  uint16_t *pivots;
  uint16_t i, j;
  cplx_t *b;

  if((lu = allocC47Blocks(n * n * REAL_SIZE_IN_BLOCKS(75) * 2))) {
    if((pivots = allocC47Blocks(TO_BLOCKS(n * sizeof(uint16_t))))) {
      for(i = 0; i < n * n; i++) {
        realCopy(&matrix[i].Real, &lu[i].Real);
        realCopy(&matrix[i].Imag, &lu[i].Imag);
      }
      if(!luCpxMat(lu, n, pivots, toleranceExponent, realContext)) {
          freeC47Blocks(lu, n * n * REAL_SIZE_IN_BLOCKS(75) * 2);
          freeC47Blocks(pivots, TO_BLOCKS(n * sizeof(uint16_t)));
        return false;
      }

      {
        real_t maxVal, minVal;
        real_t p, q;
        complexMagnitude(&lu[0], &p, realContext);
        realCopy(&p, &maxVal);
        realCopy(&p, &minVal);
        for(i = 1; i < n; ++i) {
          complexMagnitude(&lu[i * n + i], &p, realContext);
          if(realCompareLessThan(&p, &minVal)) {
            realCopy(&p, &minVal);
          }
          if(realCompareGreaterThan(&p, &maxVal)) {
            realCopy(&p, &maxVal);
          }
        }
        WP34S_Log10(&maxVal, &p, realContext);
        WP34S_Log10(&minVal, &q, realContext);
        realSubtract(&p, &q, &p, realContext);
        int32ToReal(33 - displayFormatDigits, &q);
        if(realCompareLessEqual(&q, &p)) {
          temporaryInformation = TI_INACCURATE;
        }
      }

      if((x = allocC47Blocks(n * REAL_SIZE_IN_BLOCKS(75) * 2))) {
        if((b = allocC47Blocks(n * REAL_SIZE_IN_BLOCKS(75) * 2))) {
          for(i = 0; i < n; i++) {
            for(j = 0; j < n; j++) {
              if(i == j) {
                realSetOne(&b[j].Real);
              }
              else {
                realSetZero(&b[j].Real);
              }
              realSetZero(&b[j].Imag);
            }
            complex_matrix_pivoting_solve(lu, n, b, pivots, x, realContext);
            for(j = 0; j < n; j++) {
              realCopy(&x[j].Real, &matrix[j * n + i].Real    );
              realCopy(&x[j].Imag, &matrix[j * n + i].Imag);
            }
          }
          freeC47Blocks(b, n * REAL_SIZE_IN_BLOCKS(75) * 2);
        }
        else {
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "Ram full, 1q");
            moreInfoOnError("In function invCpxMat:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
        freeC47Blocks(x, n * REAL_SIZE_IN_BLOCKS(75) * 2);
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 2r");
          moreInfoOnError("In function invCpxMat:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      freeC47Blocks(pivots, TO_BLOCKS(n * sizeof(uint16_t)));
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 3s");
        moreInfoOnError("In function invCpxMat:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    freeC47Blocks(lu, n * n * REAL_SIZE_IN_BLOCKS(75) * 2);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full, 4t");
      moreInfoOnError("In function invCpxMat:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  return lastErrorCode == ERROR_NONE;
}


void invertRealMatrix(const real34Matrix_t *matrix, real34Matrix_t *res) {
  const uint16_t n = matrix->header.matrixColumns;
  cplx_t *tmpMat;
  uint16_t i, j;

  if(matrix->header.matrixRows != matrix->header.matrixColumns) {
    if(matrix != res) {
      res->matrixElements = NULL; // Matrix is not square
      res->header.matrixRows = res->header.matrixColumns = 0;
    }
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    return;
  }

  if((tmpMat = allocC47Blocks(n * n * REAL_SIZE_IN_BLOCKS(75) * 2))) {
    for(i = 0; i < n; i++) {
      for(j = 0; j < n; j++) {
        real34ToReal(matrix->matrixElements + (i * n + j), &tmpMat[i * n + j].Real);
        realSetZero(&tmpMat[i * n + j].Imag);
      }
    }

    if(!realMatExactlySingular(matrix) && invCpxMat(tmpMat, n, 0, n == 1 ? &ctxtReal39 : &ctxtReal75)) {
      if(matrix != res) {
        copyRealMatrix(matrix, res);
      }
      if(res->matrixElements) {
        for(i = 0; i < n; i++) {
          for(j = 0; j < n; j++) {
            realToReal34(&tmpMat[i * n + j].Real, res->matrixElements + (i * n + j));
          }
        }
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 1u");
          moreInfoOnError("In function invertRealMatrix:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
    else { // singular matrix
      if(matrix != res) {
        res->matrixElements = NULL;
        res->header.matrixRows = res->header.matrixColumns = 0;
      }
    }

    freeC47Blocks(tmpMat, n * n * REAL_SIZE_IN_BLOCKS(75) * 2);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full, 2v");
      moreInfoOnError("In function invertRealMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



// ============================================================================
// Matrix square root
#if !defined(MATRIX_SQRT_USE_EIGEN)
//   0 = Denman-Beavers iteration (iterative)
//   1 = Eigendecomposition: Y = Q * sqrt(Lambda) * Q^-1 (single-shot, real eigenvalues, default
  #define MATRIX_SQRT_USE_EIGEN 1
#endif // !MATRIX_SQRT_USE_EIGEN


#if (MATRIX_SQRT_USE_EIGEN == 0)
// Element-wise (a + b) / 2 at the given context precision; supports in-place when a == res or b == res.  For Denman-Beavers.
static void halfSumRealMatrices(const real34Matrix_t *a, const real34Matrix_t *b, real34Matrix_t *res, realContext_t *realContext) {
  const uint16_t rows = a->header.matrixRows;
  const uint16_t cols = a->header.matrixColumns;
  real_t ae, be;

  if((a != res) && (b != res)) {
    if(!realMatrixInit(res, rows, cols)) {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full");
        moreInfoOnError("In function halfSumRealMatrices:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

  for(uint32_t i = 0; i < (uint32_t)rows * cols; ++i) {
    real34ToReal(a->matrixElements + i, &ae);
    real34ToReal(b->matrixElements + i, &be);
    realAdd(&ae, &be, &ae, realContext);
    realMultiply(&ae, const_1on2, &ae, realContext);
    realToReal34(&ae, res->matrixElements + i);
  }
}
#endif //MATRIX_SQRT_USE_EIGEN


#if defined(OPTION_EIGEN)
static bool_t isRealMatrixDiagonal(const real34Matrix_t *matrix) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  for(uint16_t i = 0; i < rows; ++i) {
    for(uint16_t j = 0; j < cols; ++j) {
      if(i != j && !real34IsZero(matrix->matrixElements + (i * cols + j))) {
        return false;
      }
    }
  }
  return true;
}


#if (MATRIX_SQRT_USE_EIGEN == 0)
// Denman-Beavers iteration
//   Y_0 = A,  Z_0 = I
//   Y_{k+1} = (Y_k + inv(Z_k)) / 2
//   Z_{k+1} = (Z_k + inv(Y_k)) / 2
// Y converges to sqrt(A), Z converges to inv(sqrt(A))
// On failure (singular intermediate or non-convergence), res->matrixElements set to NULL
static void sqrtRealMatrixDB(const real34Matrix_t *matrix, real34Matrix_t *res) {
  const uint16_t n = matrix->header.matrixRows;
  const uint16_t MAX_ITER = 30;
  real_t normVal;
  real34Matrix_t Y = {0}, Z = {0}, Yinv = {0}, Zinv = {0}, YY = {0};
  bool_t converged = false;
  bool_t failed = false;
  uint16_t k;

  copyRealMatrix(matrix, &Y);                      // Y_0 = A
  if(!Y.matrixElements) {
    failed = true;
    goto cleanup;
  }
  realMatrixIdentity(&Z, n);                       // Z_0 = I
  if(!Z.matrixElements) {
    failed = true;
    goto cleanup;
  }

  for(k = 0; k < MAX_ITER; ++k) {
    if(Yinv.matrixElements) {
      realMatrixFree(&Yinv);
    }
    if(Zinv.matrixElements) {
      realMatrixFree(&Zinv);
    }

    invertRealMatrix(&Y, &Yinv);
    if(!Yinv.matrixElements) {
      failed = true;
      goto cleanup;
    }
    invertRealMatrix(&Z, &Zinv);
    if(!Zinv.matrixElements) {
      failed = true;
      goto cleanup;
    }

    halfSumRealMatrices(&Y, &Zinv, &Y, &ctxtReal39);
    halfSumRealMatrices(&Z, &Yinv, &Z, &ctxtReal39);

    if((k & 1) == 1) {
      if(YY.matrixElements) {
        realMatrixFree(&YY);
      }
      multiplyRealMatrices(&Y, &Y, &YY);
      if(!YY.matrixElements) {
        failed = true;
        goto cleanup;
      }
      subtractRealMatrices(&YY, matrix, &YY);
      _euclideanNormRealMatrix(&YY, 2, &normVal, &ctxtReal39);
      if(realCompareLessThan(&normVal, const_1e_30)) {    // convergence check, using const_1e_30
        if(converged) {
          break;
        ]
        converged = true;
      }
    }
  }

cleanup:
  if(Yinv.matrixElements) {
    realMatrixFree(&Yinv);
  }
  if(Zinv.matrixElements) {
    realMatrixFree(&Zinv);
  }
  if(YY.matrixElements) {
    realMatrixFree(&YY);
  }
  if(Z.matrixElements) {
    realMatrixFree(&Z);
  }

  if(converged && !failed) {
    res->header        = Y.header;
    res->matrixElements = Y.matrixElements;
  }
  else {
    if(Y.matrixElements) {
      realMatrixFree(&Y);
    }
    res->matrixElements = NULL;
    res->header.matrixRows = res->header.matrixColumns = 0;
  }
}
#endif // MATRIX_SQRT_USE_EIGEN == 0


#if (MATRIX_SQRT_USE_EIGEN == 1)
// Eigendecomposition: A = Q * Lambda * inv(Q)
// sqrt(A) = Q * sqrt(Lambda) * inv(Q)
// On failure (complex or negative eigenvalues, eigensolver error), res->matrixElements is set to NULL
static void sqrtRealMatrixEigen(const real34Matrix_t *matrix, real34Matrix_t *res) {
  const uint16_t n = matrix->header.matrixRows;
  real34Matrix_t Lambda = {0}, LambdaImag = {0};
  real34Matrix_t Q = {0}, QImag = {0}, Qinv = {0}, tmp = {0};
  real_t a;
  bool_t failed = false;

  // Eigenvalues. Lambda gets the (size x size) diagonal-eigenvalue matrix. LambdaImag allocated only if at least one eigenvalue is complex.
  realEigenvalues(matrix, &Lambda, &LambdaImag);
  if(!Lambda.matrixElements) {
    failed = true;
    goto cleanup;
  }
  if(LambdaImag.matrixElements) {
    // Reject genuine complex eigenvalues; tolerate QR roundoff noise (||A||_F * 1e-34).
    real34_t inputNorm34;
    real_t tol, val, scale;
    bool_t hasGenuineComplex = false;

    euclideanNormRealMatrix(matrix, 2, &inputNorm34);
    real34ToReal(&inputNorm34, &scale);
    realMultiply(&scale, const_1e_34, &tol, &ctxtReal39);

    for(uint16_t i = 0; i < n; ++i) {
      real34ToReal(LambdaImag.matrixElements + (i * n + i), &val);
      if(realIsNegative(&val)) {
        realChangeSign(&val);
      }
      if(realCompareGreaterThan(&val, &tol)) {
        hasGenuineComplex = true;
        break;
      }
    }

    if(hasGenuineComplex) {
      failed = true;
      goto cleanup;
    }
  }

  // Element-wise sqrt of diagonal eigenvalue matrix; bail on negatives.
  for(uint16_t i = 0; i < n; ++i) {
    real34ToReal(Lambda.matrixElements + (i * n + i), &a);
    if(realIsNegative(&a)) {
      failed = true;
      goto cleanup;
    }
    realSquareRoot(&a, &a, &ctxtReal39);
    realToReal34(&a, Lambda.matrixElements + (i * n + i));
  }

  // Eigenvectors. Q's columns are the eigenvectors of A.
  realEigenvectors(matrix, &Q, &QImag);
  if(!Q.matrixElements) {
    failed = true;
    goto cleanup;
  }
  if(QImag.matrixElements) {
    // Reject genuine complex eigenvectors; tolerate QR roundoff noise (||A||_F * 1e-34).
    real34_t inputNorm34;
    real_t tol, val, scale;
    bool_t hasGenuineComplex = false;

    euclideanNormRealMatrix(matrix, 2, &inputNorm34);
    real34ToReal(&inputNorm34, &scale);
    realMultiply(&scale, const_1e_34, &tol, &ctxtReal39);

    for(uint32_t k = 0; k < (uint32_t)n * n; ++k) {
      real34ToReal(QImag.matrixElements + k, &val);
      if(realIsNegative(&val)) {
        realChangeSign(&val);
      }
      if(realCompareGreaterThan(&val, &tol)) {
        hasGenuineComplex = true;
        break;
      }
    }

    if(hasGenuineComplex) {
      failed = true;
      goto cleanup;
    }
  }

  // inv(Q). For symmetric input Q is orthogonal so Q^T would suffice and be faster, but using the general inverse covers the non-symmetric case at modest cost.
  invertRealMatrix(&Q, &Qinv);
  if(!Qinv.matrixElements) {    // singular Q
    failed = true;
    goto cleanup;
  }

  // tmp = Q * sqrt(Lambda)
  multiplyRealMatrices(&Q, &Lambda, &tmp);
  if(!tmp.matrixElements) {
    failed = true;
    goto cleanup;
  }

  // res = tmp * inv(Q) = Q * sqrt(Lambda) * inv(Q)
  multiplyRealMatrices(&tmp, &Qinv, res);
  if(!res->matrixElements) {
    failed = true;
    goto cleanup;
  }

cleanup:
  if(Lambda.matrixElements) {
    realMatrixFree(&Lambda);
  }
  if(LambdaImag.matrixElements) {
    realMatrixFree(&LambdaImag);
  }
  if(Q.matrixElements) {
    realMatrixFree(&Q);
  }
  if(QImag.matrixElements) {
    realMatrixFree(&QImag);
  }
  if(Qinv.matrixElements) {
    realMatrixFree(&Qinv);
  }
  if(tmp.matrixElements) {
    realMatrixFree(&tmp);
  }

  if(failed) {
    res->matrixElements = NULL;
    res->header.matrixRows = res->header.matrixColumns = 0;
  }
}
#endif // MATRIX_SQRT_USE_EIGEN == 1


// Verify Y^2 == matrix to catch cases where the algorithm produces a non-existent sqrt like [[0,0],[1,0]]).
// Operates in complex for both real and complex paths, with NULL for the unused type.
// Returns true if Y^2 - matrix is below ||matrix||_F * 1e-30
static bool_t verifySqrtMatrix(const real34Matrix_t *inputReal,    const real34Matrix_t *resultReal, const complex34Matrix_t *inputComplex, const complex34Matrix_t *resultComplex) {
  complex34Matrix_t inputCopy = {0}, resultCopy = {0}, residual = {0};
  real_t residualNorm, inputNorm, tolerance;
  real34_t residualNorm34, inputNorm34;
  const bool_t isComplex = (inputComplex != NULL);
  const uint16_t rows = isComplex ? inputComplex->header.matrixRows    : inputReal->header.matrixRows;
  const uint16_t cols = isComplex ? inputComplex->header.matrixColumns : inputReal->header.matrixColumns;
  const uint32_t total = (uint32_t)rows * cols;
  bool_t verified = false;

  if(complexMatrixInit(&inputCopy, rows, cols) && complexMatrixInit(&resultCopy, rows, cols)) {
    for(uint32_t i = 0; i < total; ++i) {
      if(isComplex) {
        real34Copy(VARIABLE_REAL34_DATA(inputComplex->matrixElements + i),  VARIABLE_REAL34_DATA(inputCopy.matrixElements + i));
        real34Copy(VARIABLE_IMAG34_DATA(inputComplex->matrixElements + i),  VARIABLE_IMAG34_DATA(inputCopy.matrixElements + i));
        real34Copy(VARIABLE_REAL34_DATA(resultComplex->matrixElements + i), VARIABLE_REAL34_DATA(resultCopy.matrixElements + i));
        real34Copy(VARIABLE_IMAG34_DATA(resultComplex->matrixElements + i), VARIABLE_IMAG34_DATA(resultCopy.matrixElements + i));
      } else {
        real34Copy(inputReal->matrixElements + i,  VARIABLE_REAL34_DATA(inputCopy.matrixElements + i));
        real34Copy(resultReal->matrixElements + i, VARIABLE_REAL34_DATA(resultCopy.matrixElements + i));
        real34SetZero(VARIABLE_IMAG34_DATA(inputCopy.matrixElements + i));
        real34SetZero(VARIABLE_IMAG34_DATA(resultCopy.matrixElements + i));
      }
    }

    multiplyComplexMatrices(&resultCopy, &resultCopy, &residual);
    if(residual.matrixElements) {
      subtractComplexMatrices(&residual, &inputCopy, &residual);
      euclideanNormComplexMatrix(&residual, 2, &residualNorm34);
      euclideanNormComplexMatrix(&inputCopy, 2, &inputNorm34);
      real34ToReal(&residualNorm34, &residualNorm);
      real34ToReal(&inputNorm34, &inputNorm);
      realMultiply(&inputNorm, const_1e_30, &tolerance, &ctxtReal39);
      verified = realCompareLessEqual(&residualNorm, &tolerance);
      complexMatrixFree(&residual);
    }
  }

  if(inputCopy.matrixElements)  {
    complexMatrixFree(&inputCopy);
  }
  if(resultCopy.matrixElements) {
    complexMatrixFree(&resultCopy);
  }

  return verified;
}


/**
 * \brief Evaluate the principal square root of a symmetric positive-semidefinite 2 by 2 real matrix.
 *
 * For A with determinant d, the result is (A + sqrt(d) I) / sqrt(trace(A) + 2 sqrt(d)).
 *
 * \param[in] matrix Input matrix.
 * \param[out] res Result matrix.
 * \return True when the direct formula is applicable and storage succeeds.
 */
static bool_t sqrtSymmetricRealMatrix2x2(const real34Matrix_t *matrix, real34Matrix_t *res) {
  real_t a, b, d, determinant, negativeB, rootDeterminant, denominator, value;

  if(!real34CompareEqual(matrix->matrixElements + 1, matrix->matrixElements + 2)) {
    return false;
  }

  real34ToReal(matrix->matrixElements, &a);
  real34ToReal(matrix->matrixElements + 1, &b);
  real34ToReal(matrix->matrixElements + 3, &d);
  if(realIsNegative(&a) || realIsNegative(&d) || realIsSpecial(&a) || realIsSpecial(&b) || realIsSpecial(&d)) {
    return false;
  }

  realCopy(&b, &negativeB);
  realChangeSign(&negativeB);
  dotCplx(&a, &b, &d, &negativeB, &determinant, &ctxtReal39);
  if(realIsNegative(&determinant) || realIsSpecial(&determinant)) {
    return false;
  }
  realSquareRoot(&determinant, &rootDeterminant, &ctxtReal39);
  realAdd(&a, &d, &denominator, &ctxtReal39);
  realAdd(&rootDeterminant, &rootDeterminant, &value, &ctxtReal39);
  realAdd(&denominator, &value, &denominator, &ctxtReal39);
  if(!realCompareGreaterThan(&denominator, const_0)) {
    return false;
  }
  realSquareRoot(&denominator, &denominator, &ctxtReal39);
  if(!realMatrixInit(res, 2, 2)) {
    return false;
  }

  realAdd(&a, &rootDeterminant, &value, &ctxtReal39);
  realDivide(&value, &denominator, &value, &ctxtReal39);
  realToReal34(&value, res->matrixElements);
  realDivide(&b, &denominator, &value, &ctxtReal39);
  realToReal34(&value, res->matrixElements + 1);
  realToReal34(&value, res->matrixElements + 2);
  realAdd(&d, &rootDeterminant, &value, &ctxtReal39);
  realDivide(&value, &denominator, &value, &ctxtReal39);
  realToReal34(&value, res->matrixElements + 3);
  return true;
}



// Compute Y such that Y*Y == matrix. Matrix must be square. On failure res->matrixElements is NULL. Fast paths handle 1x1 and diagonal inputs;
// otherwise dispatches to the eigendecomposition or Denman-Beavers iteration based on MATRIX_SQRT_USE_EIGEN == 1 or 0.
void sqrtRealMatrix(const real34Matrix_t *matrix, real34Matrix_t *res) {
  const uint16_t n = matrix->header.matrixRows;
  real_t a;

  res->matrixElements = NULL;
  res->header.matrixRows = res->header.matrixColumns = 0;

  // Fast path: 1x1 matrix
  if(n == 1) {
    real34ToReal(matrix->matrixElements, &a);
    if(realIsNegative(&a)) {
      goto fail;
    }
    if(!realMatrixInit(res, 1, 1)) {
      goto fail;
    }
    realSquareRoot(&a, &a, &ctxtReal39);
    realToReal34(&a, res->matrixElements);
    return;
  }

  // Fast path: diagonal A (per-element diagonal sqrt)
  if(isRealMatrixDiagonal(matrix)) {
    if(!realMatrixInit(res, n, n)) {
      goto fail;
    }
    for(uint16_t i = 0; i < n; ++i) {
      real34ToReal(matrix->matrixElements + (i * n + i), &a);
      if(realIsNegative(&a)) {
        goto fail;
      }
      realSquareRoot(&a, &a, &ctxtReal39);
      realToReal34(&a, res->matrixElements + (i * n + i));
    }
    return;
  }

  if(n == 2 && sqrtSymmetricRealMatrix2x2(matrix, res)) {
    if(!verifySqrtMatrix(matrix, res, NULL, NULL)) {
      goto fail;
    }
    return;
  }

  #if (MATRIX_SQRT_USE_EIGEN == 1)
    sqrtRealMatrixEigen(matrix, res);
  #else
    sqrtRealMatrixDB(matrix, res);
  #endif

  if(res->matrixElements && !verifySqrtMatrix(matrix, res, NULL, NULL)) {
    goto fail;
  }
  return;

fail:
  if(res->matrixElements) {
    realMatrixFree(res);
  }
  res->matrixElements = NULL;
  res->header.matrixRows = res->header.matrixColumns = 0;
}


static bool_t isComplexMatrixDiagonal(const complex34Matrix_t *matrix) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  for(uint16_t i = 0; i < rows; ++i) {
    for(uint16_t j = 0; j < cols; ++j) {
      if(i != j) {
        if(!real34IsZero(VARIABLE_REAL34_DATA(matrix->matrixElements + (i * cols + j))) ||
           !real34IsZero(VARIABLE_IMAG34_DATA(matrix->matrixElements + (i * cols + j)))) {
          return false;
        }
      }
    }
  }
  return true;
}


#if (MATRIX_SQRT_USE_EIGEN == 0)
// Denman-Beavers iteration on a complex matrix. Same structure as real DB,
// but with complex primitives. addComplexMatrices and _multiplyComplexMatrix
// both support in-place (matrix == res), so we inline the (a+b)/2 step.
static void sqrtComplexMatrixDB(const complex34Matrix_t *matrix, complex34Matrix_t *res) {
  const uint16_t n = matrix->header.matrixRows;
  const uint16_t MAX_ITER = 30;
  real34_t normVal34;
  real_t normVal;
  complex34Matrix_t Y = {0}, Z = {0}, Yinv = {0}, Zinv = {0}, YY = {0};
  bool_t converged = false;
  bool_t failed = false;
  uint16_t k;

  copyComplexMatrix(matrix, &Y);                   // Y_0 = A
  if(!Y.matrixElements) {
    failed = true;
    goto cleanup;
  }
  complexMatrixIdentity(&Z, n);                    // Z_0 = I
  if(!Z.matrixElements) {
    failed = true;
    goto cleanup;
  }

  for(k = 0; k < MAX_ITER; ++k) {
    if(Yinv.matrixElements) {
      complexMatrixFree(&Yinv);
    }
    if(Zinv.matrixElements) {
      complexMatrixFree(&Zinv);
    }

    invertComplexMatrix(&Y, &Yinv);
    if(!Yinv.matrixElements) {
      failed = true;
      goto cleanup;
    }
    invertComplexMatrix(&Z, &Zinv);
    if(!Zinv.matrixElements) {
      failed = true;
      goto cleanup;
    }

    // Y = (Y + Zinv) * 0.5
    addComplexMatrices(&Y, &Zinv, &Y);
    _multiplyComplexMatrix(&Y, const_1on2, const_0, &Y, &ctxtReal39);

    // Z = (Z + Yinv) * 0.5
    addComplexMatrices(&Z, &Yinv, &Z);
    _multiplyComplexMatrix(&Z, const_1on2, const_0, &Z, &ctxtReal39);

    if((k & 1) == 1) {
      if(YY.matrixElements) {
        complexMatrixFree(&YY);
      }
      multiplyComplexMatrices(&Y, &Y, &YY);
      if(!YY.matrixElements) {
        failed = true;
        goto cleanup;
      }
      subtractComplexMatrices(&YY, matrix, &YY);
      euclideanNormComplexMatrix(&YY, 2, &normVal34);
      real34ToReal(&normVal34, &normVal);
      if(realCompareLessThan(&normVal, const_1e_30)) {    // convergence check, using const_1e_30
        if(converged) {
          break;
        }
        converged = true;
      }
    }
  }

cleanup:
  if(Yinv.matrixElements) {
    complexMatrixFree(&Yinv);
  }
  if(Zinv.matrixElements) {
    complexMatrixFree(&Zinv);
  }
  if(YY.matrixElements) {
    complexMatrixFree(&YY);
  }
  if(Z.matrixElements) {
    complexMatrixFree(&Z);
  }

  if(converged && !failed) {
    res->header        = Y.header;
    res->matrixElements = Y.matrixElements;
  }
  else {
    if(Y.matrixElements) {
      complexMatrixFree(&Y);
    }
    res->matrixElements = NULL;
    res->header.matrixRows = res->header.matrixColumns = 0;
  }
}
#endif // MATRIX_SQRT_USE_EIGEN == 0


#if (MATRIX_SQRT_USE_EIGEN == 1)
// Eigendecomposition: A = Q * Lambda * inv(Q), so sqrt(A) = Q * sqrt(Lambda) * inv(Q).
// Scalar complex sqrt of each eigenvalue computed inline using the rectangular
// formula sqrt(a+bi) = sqrt((|z|+a)/2) + sign(b)*sqrt((|z|-a)/2)*i (principal branch).
static void sqrtComplexMatrixEigen(const complex34Matrix_t *matrix, complex34Matrix_t *res) {
  const uint16_t n = matrix->header.matrixRows;
  complex34Matrix_t Lambda = {0}, Q = {0}, Qinv = {0}, tmp = {0};
  bool_t failed = false;

  // Eigenvalues - complex matrix already holds both real and imag parts
  complexEigenvalues(matrix, &Lambda);
  if(!Lambda.matrixElements) {
    failed = true;
    goto cleanup;
  }

  // Element-wise complex sqrt of diagonal eigenvalue matrix
  for(uint16_t i = 0; i < n; ++i) {
    cplx_t a, sq;
    real34ToReal(VARIABLE_REAL34_DATA(Lambda.matrixElements + (i * n + i)), &a.Real);
    real34ToReal(VARIABLE_IMAG34_DATA(Lambda.matrixElements + (i * n + i)), &a.Imag);
    sqrtComplex(&a, &sq, &ctxtReal39);
    realToReal34(&sq.Real, VARIABLE_REAL34_DATA(Lambda.matrixElements + (i * n + i)));
    realToReal34(&sq.Imag, VARIABLE_IMAG34_DATA(Lambda.matrixElements + (i * n + i)));
  }

  // Eigenvectors
  complexEigenvectors(matrix, &Q);
  if(!Q.matrixElements) {
    failed = true;
    goto cleanup;
  }

  // inv(Q)
  invertComplexMatrix(&Q, &Qinv);
  if(!Qinv.matrixElements) {
    failed = true;
    goto cleanup;
  }

  // tmp = Q * sqrt(Lambda)
  multiplyComplexMatrices(&Q, &Lambda, &tmp);
  if(!tmp.matrixElements) {
    failed = true;
    goto cleanup;
  }

  // res = tmp * inv(Q) = Q * sqrt(Lambda) * inv(Q)
  multiplyComplexMatrices(&tmp, &Qinv, res);
  if(!res->matrixElements) {
    failed = true;
    goto cleanup;
  }

cleanup:
  if(Lambda.matrixElements) {
    complexMatrixFree(&Lambda);
  }
  if(Q.matrixElements) {
    complexMatrixFree(&Q);
  }
  if(Qinv.matrixElements) {
    complexMatrixFree(&Qinv);
  }
  if(tmp.matrixElements) {
    complexMatrixFree(&tmp);
  }

  if(failed) {
    res->matrixElements = NULL;
    res->header.matrixRows = res->header.matrixColumns = 0;
  }
}
#endif // MATRIX_SQRT_USE_EIGEN == 1


// Compute Y such that Y*Y == matrix (matrix product), complex case.
// Caller must ensure matrix is square. On failure res->matrixElements is NULL.
void sqrtComplexMatrix(const complex34Matrix_t *matrix, complex34Matrix_t *res) {
  const uint16_t n = matrix->header.matrixRows;
  cplx_t a, sq;

  res->matrixElements = NULL;
  res->header.matrixRows = res->header.matrixColumns = 0;

  // Fast path: diagonal complex matrix. Avoids the eigensolver, which is ill-conditioned for degenerate eigenvalues (e.g. k*I).
  // Not needed in diagonal form.
  if(isComplexMatrixDiagonal(matrix)) {
    if(!complexMatrixInit(res, n, n)) {
      goto fail;
    }
    for(uint16_t i = 0; i < n; ++i) {
      real34ToReal(VARIABLE_REAL34_DATA(matrix->matrixElements + (i * n + i)), &a.Real);
      real34ToReal(VARIABLE_IMAG34_DATA(matrix->matrixElements + (i * n + i)), &a.Imag);
      sqrtComplex(&a, &sq, &ctxtReal39);
      realToReal34(&sq.Real, VARIABLE_REAL34_DATA(res->matrixElements + (i * n + i)));
      realToReal34(&sq.Imag, VARIABLE_IMAG34_DATA(res->matrixElements + (i * n + i)));
    }
    return;
  }

  #if (MATRIX_SQRT_USE_EIGEN == 1)
    sqrtComplexMatrixEigen(matrix, res);
  #else
    sqrtComplexMatrixDB(matrix, res);
  #endif

  if(res->matrixElements && !verifySqrtMatrix(NULL, NULL, matrix, res)) {
    goto fail;
  }
  return;

fail:
  if(res->matrixElements) {
    complexMatrixFree(res);
  }
  res->matrixElements = NULL;
  res->header.matrixRows = res->header.matrixColumns = 0;
}


#endif // OPTION_EIGEN

void invertComplexMatrix(const complex34Matrix_t *matrix, complex34Matrix_t *res) {
  const uint16_t n = matrix->header.matrixColumns;
  cplx_t *tmpMat;
  uint16_t i, j;

  if(matrix->header.matrixRows != matrix->header.matrixColumns) {
    if(matrix != res) {
      res->matrixElements = NULL; // Matrix is not square
      res->header.matrixRows = res->header.matrixColumns = 0;
    }
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    return;
  }

  if((tmpMat = allocC47Blocks(n * n * REAL_SIZE_IN_BLOCKS(75) * 2))) {
    for(i = 0; i < n; i++) {
      for(j = 0; j < n; j++) {
        real34ToReal(VARIABLE_REAL34_DATA(matrix->matrixElements + (i * n + j)), &tmpMat[i * n + j].Real);
        real34ToReal(VARIABLE_IMAG34_DATA(matrix->matrixElements + (i * n + j)), &tmpMat[i * n + j].Imag);
      }
    }

    if(invCpxMat(tmpMat, n, 0, n == 1 ? &ctxtReal39 : &ctxtReal75)) {
      if(matrix != res) {
        copyComplexMatrix(matrix, res);
      }
      if(res->matrixElements) {
        for(i = 0; i < n; i++) {
          for(j = 0; j < n; j++) {
            realToReal34(&tmpMat[i * n + j].Real, VARIABLE_REAL34_DATA(res->matrixElements + (i * n + j)));
            realToReal34(&tmpMat[i * n + j].Imag, VARIABLE_IMAG34_DATA(res->matrixElements + (i * n + j)));
          }
        }
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 1w");
          moreInfoOnError("In function invertComplexMatrix:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
    else { // singular matrix
      if(matrix != res) {
        res->matrixElements = NULL;
        res->header.matrixRows = res->header.matrixColumns = 0;
      }
    }

    freeC47Blocks(tmpMat, n * n * REAL_SIZE_IN_BLOCKS(75) * 2);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full, 2x");
      moreInfoOnError("In function invertComplexMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


/* Division */
void divideRealMatrix(const real34Matrix_t *matrix, const real34_t *x, real34Matrix_t *res) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  int32_t i;

  if(matrix == res || realMatrixInit(res, rows, cols)) {
    for(i = 0; i < cols * rows; ++i) {
      real34Divide(matrix->matrixElements + i, x, res->matrixElements + i);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function divideRealMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void divideByRealMatrix(const real34_t *y, const real34Matrix_t *matrix, real34Matrix_t *res) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  int32_t i;

  if(matrix == res || realMatrixInit(res, rows, cols)) {
    for(i = 0; i < cols * rows; ++i) {
      real34Divide(y, matrix->matrixElements + i, res->matrixElements + i);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function divideByRealMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void _divideRealMatrix(const real34Matrix_t *matrix, const real_t *x, real34Matrix_t *res, realContext_t *realContext) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  int32_t i;
  real_t y;

  if(matrix == res || realMatrixInit(res, rows, cols)) {
    for(i = 0; i < cols * rows; ++i) {
      real34ToReal(matrix->matrixElements + i, &y);
      realDivide(&y, x, &y, realContext);
      realToReal34(&y, res->matrixElements + i);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function _divideRealMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void _divideByRealMatrix(const real_t *y, const real34Matrix_t *matrix, real34Matrix_t *res, realContext_t *realContext) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  int32_t i;
  real_t x;

  if(matrix == res || realMatrixInit(res, rows, cols)) {
    for(i = 0; i < cols * rows; ++i) {
      real34ToReal(matrix->matrixElements + i, &x);
      realDivide(y, &x, &x, realContext);
      realToReal34(&x, res->matrixElements + i);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function _divideByRealMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void divideRealMatrices(const real34Matrix_t *y, const real34Matrix_t *x, real34Matrix_t *res) {
  const uint16_t sizeY = y->header.matrixRows;
  const uint16_t size  = x->header.matrixRows;
  cplx_t *yy, *xx, *rr;

  if(y->header.matrixColumns != x->header.matrixRows || x->header.matrixRows != x->header.matrixColumns) {
    res->matrixElements = NULL; // Matrix mismatch
    res->header.matrixRows = res->header.matrixColumns = 0;
    return;
  }

  if((yy = allocC47Blocks(sizeY * size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
    if((xx = allocC47Blocks(size * size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
      if((rr = allocC47Blocks(sizeY * size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
        for(int i = 0; i < size * size; ++i) {
          real34ToReal(x->matrixElements + i, &xx[i].Real);
          realSetZero(&xx[i].Imag);
        }
        if(!realMatExactlySingular(x) && invCpxMat(xx, size, 0, &ctxtReal39)) {
          for(int i = 0; i < sizeY * size; ++i) {
            real34ToReal(y->matrixElements + i, &yy[i].Real);
            realSetZero(&yy[i].Imag);
          }
          mulCpxMat(yy, xx, sizeY, size, size, rr, &ctxtReal39);

          if(realMatrixInit(res, sizeY, size)) {
            for(int i = 0; i < sizeY * size; ++i) {
              realToReal34(&rr[i].Real, res->matrixElements + i);
            }
          }
          else {
            if(y != res && x != res) {
              res->matrixElements = NULL;
              res->header.matrixRows = res->header.matrixColumns = 0;
            }
            displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Ram full, 1y");
              moreInfoOnError("In function divideRealMatrices:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
        }
        else { // singular matrix
          if(y != res && x != res) {
            res->matrixElements = NULL;
            res->header.matrixRows = res->header.matrixColumns = 0;
          }
        }

        freeC47Blocks(rr, sizeY * size * REAL_SIZE_IN_BLOCKS(75) * 2);
      }
      else {
        if(y != res && x != res) {
          res->matrixElements = NULL;
          res->header.matrixRows = res->header.matrixColumns = 0;
        }
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 2z");
          moreInfoOnError("In function divideRealMatrices:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      freeC47Blocks(xx, size * size * REAL_SIZE_IN_BLOCKS(75) * 2);
    }
    else {
      if(y != res && x != res) {
        res->matrixElements = NULL;
        res->header.matrixRows = res->header.matrixColumns = 0;
      }
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 3aa");
        moreInfoOnError("In function divideRealMatrices:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    freeC47Blocks(yy, sizeY * size * REAL_SIZE_IN_BLOCKS(75) * 2);
  }
  else {
    if(y != res && x != res) {
      res->matrixElements = NULL;
      res->header.matrixRows = res->header.matrixColumns = 0;
    }
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full, 4ab");
      moreInfoOnError("In function divideRealMatrices:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



void divideComplexMatrix(const complex34Matrix_t *matrix, const real34_t *xr, const real34_t *xi, complex34Matrix_t *res) {
  cplx_t _x;

  real34ToReal(xr, &_x.Real);
  real34ToReal(xi, &_x.Imag);
  _divideComplexMatrix(matrix, &_x, res, &ctxtReal39);
}


void divideByComplexMatrix(const real34_t *yr, const real34_t *yi, const complex34Matrix_t *matrix, complex34Matrix_t *res) {
  cplx_t _y;

  real34ToReal(yr, &_y.Real);
  real34ToReal(yi, &_y.Imag);
  _divideByComplexMatrix(&_y, matrix, res, &ctxtReal39);
}


void _divideComplexMatrix(const complex34Matrix_t *matrix, const cplx_t *x, complex34Matrix_t *res, realContext_t *realContext) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  int32_t i;
  cplx_t y;

  if(matrix == res || complexMatrixInit(res, rows, cols)) {
    for(i = 0; i < cols * rows; ++i) {
      real34ToReal(VARIABLE_REAL34_DATA(matrix->matrixElements + i), &y.Real);
      real34ToReal(VARIABLE_IMAG34_DATA(matrix->matrixElements + i), &y.Imag);
      divComplexComplex(&y, x, &y, realContext);
      realToReal34(&y.Real, VARIABLE_REAL34_DATA(res->matrixElements + i));
      realToReal34(&y.Imag, VARIABLE_IMAG34_DATA(res->matrixElements + i));
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function divideComplexMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void _divideByComplexMatrix(const cplx_t *y, const complex34Matrix_t *matrix, complex34Matrix_t *res, realContext_t *realContext) {
  const uint16_t rows = matrix->header.matrixRows;
  const uint16_t cols = matrix->header.matrixColumns;
  int32_t i;
  cplx_t x;

  if(matrix == res || complexMatrixInit(res, rows, cols)) {
    for(i = 0; i < cols * rows; ++i) {
      real34ToReal(VARIABLE_REAL34_DATA(matrix->matrixElements + i), &x.Real);
      real34ToReal(VARIABLE_IMAG34_DATA(matrix->matrixElements + i), &x.Imag);
      divComplexComplex(y, &x, &x, realContext);
      realToReal34(&x.Real, VARIABLE_REAL34_DATA(res->matrixElements + i));
      realToReal34(&x.Imag, VARIABLE_IMAG34_DATA(res->matrixElements + i));
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function _divideByComplexMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void divideComplexMatrices(const complex34Matrix_t *y, const complex34Matrix_t *x, complex34Matrix_t *res) {
  const uint16_t sizeY = y->header.matrixRows;
  const uint16_t size  = x->header.matrixRows;
  cplx_t *yy, *xx, *rr;

  if(y->header.matrixColumns != x->header.matrixRows || x->header.matrixRows != x->header.matrixColumns) {
    res->matrixElements = NULL; // Matrix mismatch
    res->header.matrixRows = res->header.matrixColumns = 0;
    return;
  }

  if((yy = allocC47Blocks(sizeY * size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
    if((xx = allocC47Blocks(size * size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
      if((rr = allocC47Blocks(sizeY * size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
        for(int i = 0; i < size * size; ++i) {
          real34ToReal(VARIABLE_REAL34_DATA(x->matrixElements + i), &xx[i].Real);
          real34ToReal(VARIABLE_IMAG34_DATA(x->matrixElements + i), &xx[i].Imag);
        }
        if(invCpxMat(xx, size, 0, &ctxtReal39)) {
          for(int i = 0; i < sizeY * size; ++i) {
            real34ToReal(VARIABLE_REAL34_DATA(y->matrixElements + i), &yy[i].Real);
            real34ToReal(VARIABLE_IMAG34_DATA(y->matrixElements + i), &yy[i].Imag);
          }
          mulCpxMat(yy, xx, sizeY, size, size, rr, &ctxtReal39);

          if(complexMatrixInit(res, sizeY, size)) {
            for(int i = 0; i < sizeY * size; ++i) {
              realToReal34(&rr[i].Real, VARIABLE_REAL34_DATA(res->matrixElements + i));
              realToReal34(&rr[i].Imag, VARIABLE_IMAG34_DATA(res->matrixElements + i));
            }
          }
          else {
            if(y != res && x != res) {
              res->matrixElements = NULL;
              res->header.matrixRows = res->header.matrixColumns = 0;
            }
            displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Ram full, 1ac");
              moreInfoOnError("In function divideComplexMatrices:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
        }
        else { // Singular matrix
          if(y != res && x != res) {
            res->matrixElements = NULL;
            res->header.matrixRows = res->header.matrixColumns = 0;
          }
        }

        freeC47Blocks(rr, sizeY * size * REAL_SIZE_IN_BLOCKS(75) * 2);
      }
      else {
        if(y != res && x != res) {
          res->matrixElements = NULL;
          res->header.matrixRows = res->header.matrixColumns = 0;
        }
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 2ad");
          moreInfoOnError("In function divideComplexMatrices:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      freeC47Blocks(xx, size * size * REAL_SIZE_IN_BLOCKS(75) * 2);
    }
    else {
      if(y != res && x != res) {
        res->matrixElements = NULL;
        res->header.matrixRows = res->header.matrixColumns = 0;
      }
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 3ae");
        moreInfoOnError("In function divideComplexMatrices:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    freeC47Blocks(yy, sizeY * size * REAL_SIZE_IN_BLOCKS(75) * 2);
  }
  else {
    if(y != res && x != res) {
      res->matrixElements = NULL;
      res->header.matrixRows = res->header.matrixColumns = 0;
    }
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full, 4af");
      moreInfoOnError("In function divideComplexMatrices:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


// Solve a system of linear equations Ac = b
static void cpxLinearEqn(const cplx_t *a, const cplx_t *b, cplx_t *r, uint16_t size, int32_t toleranceExponent, realContext_t *realContext) {
  cplx_t *inv_a;

  if((inv_a = allocC47Blocks(size * size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
    xcopy(inv_a, a, TO_BYTES(size * size * REAL_SIZE_IN_BLOCKS(75) * 2));
    if(invCpxMat(inv_a, size, toleranceExponent, realContext)) {
      mulCpxMat(inv_a, b, size, size, 1, r, realContext);
    }
    else if(lastErrorCode != ERROR_RAM_FULL) {
      displayCalcErrorMessage(ERROR_SINGULAR_MATRIX, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "attempt to invert a singular matrix");
        moreInfoOnError("In function cpxLinearEqn:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    freeC47Blocks(inv_a, size * size * REAL_SIZE_IN_BLOCKS(75) * 2);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function cpxLinearEqn:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


  void real_matrix_linear_eqn(const real34Matrix_t *a, const real34Matrix_t *b, real34Matrix_t *r) {
    const uint16_t size = a->header.matrixColumns;
    cplx_t *aa, *bb, *rr;

    if(size != a->header.matrixRows) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a square matrix (%d" STD_CROSS "%d)", a->header.matrixRows, a->header.matrixColumns);
        moreInfoOnError("In function real_matrix_linear_eqn:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    if(b->header.matrixRows != size || b->header.matrixColumns != 1) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a column vector or size mismatch (%d" STD_CROSS "%d)", b->header.matrixRows, b->header.matrixColumns);
        moreInfoOnError("In function real_matrix_linear_eqn:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }

    if((aa = allocC47Blocks(size * size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
      if((bb = allocC47Blocks(size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
        if((rr = allocC47Blocks(size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
          for(int i = 0; i < size * size; ++i) {
            real34ToReal(a->matrixElements + i, &aa[i].Real);
            realSetZero(&aa[i].Imag);
          }
          for(int i = 0; i < size; ++i) {
            real34ToReal(b->matrixElements + i, &bb[i].Real);
            realSetZero(&bb[i].Imag);
          }
          if(realMatExactlySingular(a)) {
            displayCalcErrorMessage(ERROR_SINGULAR_MATRIX, ERR_REGISTER_LINE);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "attempt to solve with a singular matrix");
              moreInfoOnError("In function real_matrix_linear_eqn:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
          else {
            cpxLinearEqn(aa, bb, rr, size, 0, &ctxtReal51);
          }
          if(lastErrorCode == ERROR_NONE) {
            if(realMatrixInit(r, size, 1)) {
              for(int i = 0; i < size; ++i) {
                realToReal34(&rr[i].Real, r->matrixElements + i);
              }
            }
            else {
              if(a != r && b != r) {
                r->matrixElements = NULL;
                r->header.matrixRows = r->header.matrixColumns = 0;
              }
              displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                sprintf(errorMessage, "Ram full, 1ag");
                moreInfoOnError("In function real_matrix_linear_eqn:", errorMessage, NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            }
          }
          else { // singular
            if(a != r && b != r) {
              r->matrixElements = NULL;
              r->header.matrixRows = r->header.matrixColumns = 0;
            }
          }
          freeC47Blocks(rr, size * REAL_SIZE_IN_BLOCKS(75) * 2);
        }
        else {
          if(a != r && b != r) {
            r->matrixElements = NULL;
            r->header.matrixRows = r->header.matrixColumns = 0;
          }
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "Ram full, 2ah");
            moreInfoOnError("In function real_matrix_linear_eqn:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
        freeC47Blocks(bb, size * REAL_SIZE_IN_BLOCKS(75) * 2);
      }
      else {
        if(a != r && b != r) {
          r->matrixElements = NULL;
          r->header.matrixRows = r->header.matrixColumns = 0;
        }
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 3ai");
          moreInfoOnError("In function real_matrix_linear_eqn:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      freeC47Blocks(aa, size * size * REAL_SIZE_IN_BLOCKS(75) * 2);
    }
    else {
      if(a != r && b != r) {
        r->matrixElements = NULL;
        r->header.matrixRows = r->header.matrixColumns = 0;
      }
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 4aj");
        moreInfoOnError("In function real_matrix_linear_eqn:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }


  void complex_matrix_linear_eqn(const complex34Matrix_t *a, const complex34Matrix_t *b, complex34Matrix_t *r) {
    const uint16_t size = a->header.matrixColumns;
    cplx_t *aa, *bb, *rr;

    if(size != a->header.matrixRows) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a square matrix (%d" STD_CROSS "%d)", a->header.matrixRows, a->header.matrixColumns);
        moreInfoOnError("In function complex_matrix_linear_eqn:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    if(b->header.matrixRows != size || b->header.matrixColumns != 1) {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not a column vector or size mismatch (%d" STD_CROSS "%d)", b->header.matrixRows, b->header.matrixColumns);
        moreInfoOnError("In function complex_matrix_linear_eqn:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }

    if((aa = allocC47Blocks(size * size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
      if((bb = allocC47Blocks(size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
        if((rr = allocC47Blocks(size * REAL_SIZE_IN_BLOCKS(75) * 2))) {
          for(int i = 0; i < size * size; ++i) {
            real34ToReal(VARIABLE_REAL34_DATA(a->matrixElements + i), &aa[i].Real);
            real34ToReal(VARIABLE_IMAG34_DATA(a->matrixElements + i), &aa[i].Imag);
          }
          for(int i = 0; i < size; ++i) {
            real34ToReal(VARIABLE_REAL34_DATA(b->matrixElements + i), &bb[i].Real);
            real34ToReal(VARIABLE_IMAG34_DATA(b->matrixElements + i), &bb[i].Imag);
          }
          cpxLinearEqn(aa, bb, rr, size, 0, &ctxtReal51);
          if(lastErrorCode == ERROR_NONE) {
            if(complexMatrixInit(r, size, 1)) {
              for(int i = 0; i < size; ++i) {
                realToReal34(&rr[i].Real, VARIABLE_REAL34_DATA(r->matrixElements + i));
                realToReal34(&rr[i].Imag, VARIABLE_IMAG34_DATA(r->matrixElements + i));
              }
            }
            else {
              if(a != r && b != r) {
                r->matrixElements = NULL;
                r->header.matrixRows = r->header.matrixColumns = 0;
              }
              displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                sprintf(errorMessage, "Ram full, 1ak");
                moreInfoOnError("In function complex_matrix_linear_eqn:", errorMessage, NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            }
          }
          else { // singular
            if(a != r && b != r) {
              r->matrixElements = NULL;
              r->header.matrixRows = r->header.matrixColumns = 0;
            }
          }
          freeC47Blocks(rr, size * REAL_SIZE_IN_BLOCKS(75) * 2);
        }
        else {
          if(a != r && b != r) {
            r->matrixElements = NULL;
            r->header.matrixRows = r->header.matrixColumns = 0;
          }
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "Ram full, 2al");
            moreInfoOnError("In function complex_matrix_linear_eqn:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
        freeC47Blocks(bb, size * REAL_SIZE_IN_BLOCKS(75) * 2);
      }
      else {
        if(a != r && b != r) {
          r->matrixElements = NULL;
          r->header.matrixRows = r->header.matrixColumns = 0;
        }
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 3am");
          moreInfoOnError("In function complex_matrix_linear_eqn:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      freeC47Blocks(aa, size * size * REAL_SIZE_IN_BLOCKS(75) * 2);
    }
    else {
      if(a != r && b != r) {
        r->matrixElements = NULL;
        r->header.matrixRows = r->header.matrixColumns = 0;
      }
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 4an");
        moreInfoOnError("In function complex_matrix_linear_eqn:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }


/* Routines for calculating eigenpairs */
#if defined(OPTION_EIGEN)
static void adjCpxMat(const cplx_t *x, uint16_t size, cplx_t *res) {
  int32_t i, j;
  for(i = 0; i < size; ++i) {
    for(j = 0; j < size; ++j) {
      realCopy(&x[i * size + j].Real, &res[j * size + i].Real    );
      realCopy(&x[i * size + j].Imag, &res[j * size + i].Imag);
      realChangeSign(&res[j * size + i].Imag);
    }
  }
}



static void QR_decomposition_householder(const cplx_t *mat, uint16_t size, cplx_t *q, cplx_t *r, realContext_t *realContext) {
  uint32_t i, j, k;

  cplx_t *bulk;
  cplx_t *matq, *matr;

  cplx_t *v, *qq, *qt, *newMat;
  real_t sum;
  cplx_t mt;
  size_t bulkSize = (size_t)(size * size * 5 + size) * REAL_SIZE_IN_BLOCKS(75) * 2;

  // Allocate
  if((bulk = allocC47Blocks(bulkSize))) {
    // ZERO THE ENTIRE BULK ALLOCATION
    for(uint32_t i = 0; i < (uint32_t)(size * size * 5 + size); i++) {
      realSetZero(&bulk[i].Real);
      realSetZero(&bulk[i].Imag);
    }

    matr = bulk;
    matq = bulk + (size * size);

    qq     = bulk + 2 * (size * size);
    qt     = bulk + 3 * (size * size);
    newMat = bulk + 4 * (size * size);
    v      = bulk + 5 * (size * size);

    // Copy mat to matr
    for(i = 0; i < size * size; i++) {
      realCopy(&mat[i].Real, &matr[i].Real    );
      realCopy(&mat[i].Imag, &matr[i].Imag);
    }

    // Initialize Q
    for(i = 0; i < size * size; i++) {
      realSetZero(&matq[i].Real    );
      realSetZero(&matq[i].Imag);
    }
    for(i = 0; i < size; i++) {
      realSetOne(&matq[i * size + i].Real);
    }

    // Calculate
    for(j = 0; j < (size - 1u); ++j) {
      // Column vector of submatrix
      realSetZero(&sum);
      for(i = 0; i < (size - j); i++) {
        realCopy(&matr[(i + j) * size + j].Real, &v[i].Real    );
        realCopy(&matr[(i + j) * size + j].Imag, &v[i].Imag);
        real_t temp_v1, temp_v2;
        realFMA(&v[i].Real, &v[i].Real, &sum, &temp_v1, realContext);
        realCopy(&temp_v1, &sum);
        realFMA(&v[i].Imag, &v[i].Imag, &sum, &temp_v2, realContext);
        realCopy(&temp_v2, &sum);
      }
      realSquareRoot(&sum, &sum, realContext);

      // Calculate u = x - alpha e1
      // using STABLE sign choice to avoid cancellation When v[0] and ||v|| have same sign, catastrophic cancellation occurs.
      // change to alpha sign being OPPOSITE to v[0] sign, making subtraction effectively addition.
      if(realIsZero(&v[0].Imag)) {
        // v is real only
        if(!realIsNegative(&v[0].Real)) {
          // v[0] >= 0: alpha = -||v|| so u = v[0] - (-||v||) = v[0] + ||v|| (safe addition)
          realChangeSign(&sum);
        }
        // v[0] < 0: alpha = +||v|| so u = v[0] - ||v|| (more negative, safe)
        realSubtract(&v[0].Real, &sum, &v[0].Real, realContext);
      }
      else {
        // alpha = ||v|| * exp(i*arg(v[0]))
        blockMonitoring = true;
        realRectangularToPolar(&v[0], &mt.Mag, &mt.Arg, realContext);
        blockMonitoring = true;
        realPolarToRectangular(&sum, &mt.Arg, &mt, realContext);
        blockMonitoring = false;
        realAdd(&v[0].Real, &mt.Real, &v[0].Real, realContext);
        realAdd(&v[0].Imag, &mt.Imag, &v[0].Imag, realContext);
      }

      // Euclidean norm
      realSetZero(&sum);
      for(i = 0; i < (size - j); i++) {
        real_t temp_v1, temp_v2;
        realFMA(&v[i].Real, &v[i].Real, &sum, &temp_v1, realContext);
        realCopy(&temp_v1, &sum);
        realFMA(&v[i].Imag, &v[i].Imag, &sum, &temp_v2, realContext);
        realCopy(&temp_v2, &sum);
      }
      realSquareRoot(&sum, &sum, realContext);

      // Calculate v = u / ||u|| with a precision-dependent zero threshold.
      real_t min_norm;
      realSetOne(&min_norm);
      min_norm.exponent -= realContext->digits;

      for(i = 0; i < (size - j); i++) {
        if(realCompareLessThan(&sum, &min_norm)) {
          // Norm too small relative to precision - use original vector, no normalization; this effectively skips this Householder step
          realCopy(&v[i].Real, &mt.Real);
          realCopy(&v[i].Imag, &mt.Imag);
        }
        else if(realIsZero(&v[i].Imag)) {
          realDivide(&v[i].Real, &sum, &mt.Real, realContext);
          realSetZero(&mt.Imag);
        }
        else {
          realCopy(&sum, &mt.Real);
          realSetZero(&mt.Imag);
          divComplexComplex(&v[i], &mt, &mt, realContext);
        }
        realCopy(&mt.Real, &v[i].Real    );
        realCopy(&mt.Imag, &v[i].Imag);
      }

      // Initialize Q
      for(i = 0; i < size * size; i++) {
        realSetZero(&qq[i].Real    );
        realSetZero(&qq[i].Imag);
      }
      for(i = 0; i < size; i++) {
        realSetOne(&qq[i * size + i].Real);
      }

      // Q -= 2vv*
      for(i = 0; i < (size - j); i++) {
        for(k = 0; k < (size - j); k++) {
          const uint32_t qe = (i + j) * size + k + j;
          realSubtract(const_0, (&v[k].Imag), &sum, realContext);
          if(realIsZero(&v[i].Imag) && realIsZero(&sum)) {
            realMultiply(&v[i].Real, &v[k].Real, &mt.Real, realContext);
            realSetZero(&mt.Imag);
          }
          else {
            realCopy(&v[k].Real, &mt.Real);                        // mt = conj(v[k]), sum takes -Im v[k]
            realCopy(&sum, &mt.Imag);
            mulComplexComplex(&v[i], &mt, &mt, realContext);
          }
          realMultiply(&mt.Real, const_2, &mt.Real, realContext);
          realMultiply(&mt.Imag, const_2, &mt.Imag, realContext);
          realSubtract(&qq[qe].Real, &mt.Real, &qq[qe].Real, realContext);
          realSubtract(&qq[qe].Imag, &mt.Imag, &qq[qe].Imag, realContext);
        }
      }

      // Calculate R
      mulCpxMat(qq, matr, size, size, size, newMat, realContext);
      for(i = 0; i < size * size; i++) {
        realCopy(&newMat[i].Real, &matr[i].Real    );
        realCopy(&newMat[i].Imag, &matr[i].Imag);
      }

      // Calculate Q
      adjCpxMat(qq, size, qt);
      mulCpxMat(matq, qt, size, size, size, newMat, realContext);
      for(i = 0; i < size * size; i++) {
        realCopy(&newMat[i].Real, &matq[i].Real    );
        realCopy(&newMat[i].Imag, &matq[i].Imag);
      }
    }

    // Make sure R is triangular
    for(j = 0; j < (size - 1u); j++) {
      for(i = j + 1; i < size; i++) {
        realSetZero(&matr[i * size + j].Real    );
        realSetZero(&matr[i * size + j].Imag);
      }
    }

    // Copy results
    for(i = 0; i < size * size; i++) {
      realCopy(&matq[i].Real, &q[i].Real    );
      realCopy(&matq[i].Imag, &q[i].Imag);
      realCopy(&matr[i].Real, &r[i].Real    );
      realCopy(&matr[i].Imag, &r[i].Imag);
    }

    // Cleanup
    freeC47Blocks(bulk, bulkSize);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Ram full");
      moreInfoOnError("In function QR_decomposition_householder:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void real_QR_decomposition(const real34Matrix_t *matrix, real34Matrix_t *q, real34Matrix_t *r) {
  if(matrix->header.matrixRows == matrix->header.matrixColumns) {
    cplx_t *mat, *matq, *matr;
    uint32_t i;

    // Allocate
    if((mat = allocC47Blocks(matrix->header.matrixRows * matrix->header.matrixColumns * REAL_SIZE_IN_BLOCKS(75) * 2 * 3))) {
      matq = mat + matrix->header.matrixRows * matrix->header.matrixColumns;
      matr = mat + matrix->header.matrixRows * matrix->header.matrixColumns * 2;

      // Convert real34 to real
      for(i = 0; i < matrix->header.matrixRows * matrix->header.matrixColumns; i++) {
        real34ToReal(matrix->matrixElements + i, &mat[i].Real);
        realSetZero(&mat[i].Imag);
      }

      // Calculate
      QR_decomposition_householder(mat, matrix->header.matrixRows, matq, matr, &ctxtReal75);
      if(lastErrorCode == ERROR_NONE) {
        // Write back
        if(realMatrixInit(q, matrix->header.matrixRows, matrix->header.matrixRows)) {
          for(i = 0; i < matrix->header.matrixRows * matrix->header.matrixRows; i++) {
            realToReal34(&matq[i].Real, q->matrixElements + i);
          }
          if(realMatrixInit(r, matrix->header.matrixRows, matrix->header.matrixRows)) {
            for(i = 0; i < matrix->header.matrixRows * matrix->header.matrixRows; i++) {
              realToReal34(&matr[i].Real, r->matrixElements + i);
            }
          }
          else {
            displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Ram full, 1ao");
              moreInfoOnError("In function real_QR_decomposition:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
        }
        else {
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "Ram full, 2ap");
            moreInfoOnError("In function real_QR_decomposition:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 2aq");
          moreInfoOnError("In function real_QR_decomposition:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }

      // Cleanup
      freeC47Blocks(mat, matrix->header.matrixRows * matrix->header.matrixColumns * REAL_SIZE_IN_BLOCKS(75) * 2 * 3);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 3ar");
        moreInfoOnError("In function real_QR_decomposition:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
}


void complex_QR_decomposition(const complex34Matrix_t *matrix, complex34Matrix_t *q, complex34Matrix_t *r) {
  if(matrix->header.matrixRows == matrix->header.matrixColumns) {
    cplx_t *mat, *matq, *matr;
    uint32_t i;

    // Allocate
    if((mat = allocC47Blocks(matrix->header.matrixRows * matrix->header.matrixColumns * REAL_SIZE_IN_BLOCKS(75) * 2 * 3))) {
      matq = mat + matrix->header.matrixRows * matrix->header.matrixColumns;
      matr = mat + matrix->header.matrixRows * matrix->header.matrixColumns * 2;

      for(i = 0; i < matrix->header.matrixRows * matrix->header.matrixColumns; i++) {
        real34ToReal(VARIABLE_REAL34_DATA(matrix->matrixElements + i), &mat[i].Real    );
        real34ToReal(VARIABLE_IMAG34_DATA(matrix->matrixElements + i), &mat[i].Imag);
      }

      // Calculate
      QR_decomposition_householder(mat, matrix->header.matrixRows, matq, matr, &ctxtReal75);

      // Write back
      if(complexMatrixInit(q, matrix->header.matrixRows, matrix->header.matrixRows)) {
        for(i = 0; i < matrix->header.matrixRows * matrix->header.matrixColumns; i++) {
          realToReal34(&matq[i].Real, VARIABLE_REAL34_DATA(q->matrixElements + i));
          realToReal34(&matq[i].Imag, VARIABLE_IMAG34_DATA(q->matrixElements + i));
        }
        if(complexMatrixInit(r, matrix->header.matrixRows, matrix->header.matrixRows)) {
          for(i = 0; i < matrix->header.matrixRows * matrix->header.matrixColumns; i++) {
            realToReal34(&matr[i].Real, VARIABLE_REAL34_DATA(r->matrixElements + i));
            realToReal34(&matr[i].Imag, VARIABLE_IMAG34_DATA(r->matrixElements + i));
          }
        }
        else {
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "Ram full, 1as");
            moreInfoOnError("In function complex_QR_decomposition:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 2at");
          moreInfoOnError("In function complex_QR_decomposition:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }

      // Cleanup
      freeC47Blocks(mat, matrix->header.matrixRows * matrix->header.matrixColumns * REAL_SIZE_IN_BLOCKS(75) * 2 * 3);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 3au");
        moreInfoOnError("In function complex_QR_decomposition:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
}



// The 2x2 and 3x3 helpers mix 75 digit matrix elements with 159 digit intermediates when OPTION_EIGEN_159 is set, so a factor given by its parts is copied into a
// cplx159_t first. EIGEN_MUL takes each factor as a pointer to a cplx_t or a cplx159_t, and both types name their parts Real and Imag.
#if defined(OPTION_EIGEN_159)
  static void eigenMul159(const real_t *f1Real, const real_t *f1Imag, const real_t *f2Real, const real_t *f2Imag, cplx159_t *product, realContext_t *realContext) {
    cplx159_t f1, f2;

    realCopy(f1Real, &f1.Real);
    realCopy(f1Imag, &f1.Imag);
    realCopy(f2Real, &f2.Real);
    realCopy(f2Imag, &f2.Imag);
    mulComplexComplex159(&f1, &f2, product, realContext);
  }

  #define EIGEN_MUL(f1, f2, product) eigenMul159(&(f1)->Real, &(f1)->Imag, &(f2)->Real, &(f2)->Imag, product, &ctx159)
#else // OPTION_EIGEN_159
  #define EIGEN_MUL(f1, f2, product) mulComplexComplex(f1, f2, product, &ctx159)
#endif // OPTION_EIGEN_159

static void calculateEigenvalues22(const cplx_t *mat, uint16_t size, cplx_t *t1, cplx_t *t2, bool_t is_real_symmetric, realContext_t *realContext) {
// This re-write is needed because the 2x2 solver result in about 35 digits MAX accuracy if 75 digits internally is used. This is not acceptable.
// In order to increase this to around 39 diits, estimated 120 digits is needed. A 159 digit type was created for this to be used in the temprary variables in the helpers.
// The 159 digit budget results in typically maybe half.
// This allows zero noise in the eigenvalues
// The mixing of decNumber types cannot deal with the real_t temporary variable withinn mulComplexComplex().
//
  // Calculate eigenvalues of 2x2 (bottom right sub-)matrix using 159-digit precision
  // Characteristic equation of A = [[a b] [c d]]:
  //   det(A - λI) = λ² - trace(A)·λ + det(A) = 0
  // Standard form: λ² + p·λ + q = 0 where:
  //   p = -(a + d)  [negative trace]
  //   q = ad - bc   [determinant]
  // Solution: λ = (-(a+d) ± √((a+d)² - 4(ad-bc))) / 2
  //            = (-(a+d) ± √(a² + d² - 2ad + 4bc)) / 2

  #if defined(EIGENDEBUGMINIMAL)
    printf("== calculateEigenvalues22\n");
  #endif //EIGENDEBUGMINIMAL
  realContext_t ctx159 = ctxtReal75;

  #if defined(OPTION_EIGEN_159)
    ctx159.digits = 159;
    cplx159_t tr, det, discr, one;
  #else
    ctx159.digits = 75;
    cplx_t tr, det, discr, one;
  #endif //OPTION_EIGEN_159
  real_t *const trR = &tr.Real, *const trI = &tr.Imag, *const detR = &det.Real, *const detI = &det.Imag, *const discrR = &discr.Real, *const discrI = &discr.Imag;


  // Initialize all
  realSetZero(trR);
  realSetZero(trI);
  realSetZero(detR);
  realSetZero(detI);
  realSetZero(discrR);
  realSetZero(discrI);

  const cplx_t *a, *b, *c, *d;
  a = &mat[(size - 2) * size + (size - 2)];
  b = &mat[(size - 2) * size + (size - 1)];
  c = &mat[(size - 1) * size + (size - 2)];
  d = &mat[(size - 1) * size + (size - 1)];

  // determinant
  if(realIsZero(&a->Imag) && realIsZero(&b->Imag) && realIsZero(&c->Imag) && realIsZero(&d->Imag)) {
      // All real - compute ad - bc directly
      realMultiply(&a->Real, &d->Real, detR, &ctx159);
      realMultiply(&b->Real, &c->Real, trR, &ctx159);  // Reuse trR as temp
      realSubtract(detR, trR, detR, &ctx159);
      realSetZero(detI);
  }
  else {
      EIGEN_MUL(a, d, &det);
      EIGEN_MUL(b, c, &tr);
      realSubtract(detR, trR, detR, &ctx159);
      realSubtract(detI, trI, detI, &ctx159);
  }

  // Trace in high precision
  realAdd(&a->Real, &d->Real, trR, &ctx159);
  realAdd(&a->Imag, &d->Imag, trI, &ctx159);
  realChangeSign(trR);
  realChangeSign(trI);

  blockMonitoring = true;
  realSetOne(&one.Real);
  realSetZero(&one.Imag);
  #if defined(OPTION_EIGEN_159)
    cplx159_t t1H, t2H;
    realSetZero(&t1H.Real);
    realSetZero(&t1H.Imag);
    realSetZero(&t2H.Real);
    realSetZero(&t2H.Imag);
    solveQuadraticEquation159(&one, &tr, &det, &discr, &t1H, &t2H, &ctx159);
    // Convert back to 75-digit precision
    realPlus(&t1H.Real, &t1->Real, realContext);
    realPlus(&t1H.Imag, &t1->Imag, realContext);
    realPlus(&t2H.Real, &t2->Real, realContext);
    realPlus(&t2H.Imag, &t2->Imag, realContext);
  #else //OPTION_EIGEN_159
    solveQuadraticEquation(&one, &tr, &det, &discr, t1, t2, &ctx159);
  #endif //OPTION_EIGEN_159
  blockMonitoring = false;


  if(is_real_symmetric) {
    #if defined(EIGENDEBUG)
      printf("clear calculateEigenvalues22 imag output due to symmetric flag\n");
    #endif //defined(EIGENDEBUG)
    realSetZero(&t1->Imag);
    realSetZero(&t2->Imag);
  }
}



static void calculateEigenvalues33(const cplx_t *mat, uint16_t size, cplx_t *t1, cplx_t *t2, cplx_t *t3, bool_t is_real_symmetric, realContext_t *realContext) {
// This re-write is needed because the 3x3 solver result in 25 digits MAX accuracy if 75 digits internally is used. This is not acceptable.
// In order to increase this to around 39 diits, estimated 120 digits is needed. A 159 digit type was created for this to be used in the temprary variables in the helpers.
// The 159 digit budget results in typically 40 digits, so a lot more digit loss is exeprienced, but 40 exceeds 39 and also exceeds 34.
// This allows zero noise in the eigenvalues
// The mixing of decNumber types cannot deal with the real_t temporary variable withinn mulComplexComplex().
//
  // Calculate eigenvalues of 3x3 (bottom right sub-)matrix using 159-digit precision
  // Characteristic equation of A = [[a b c] [d e f] [g h k]]:
  //   det(A - λI) = -λ³ + trace(A)·λ² - (sum of 2×2 principal minors)·λ + det(A) = 0
  // Standard form: λ³ + c2·λ² + c1·λ + c0 = 0 where:
  //   c2 = -(a + e + k)
  //   c1 = (ae - bd) + (ak - cg) + (ek - fh)  [sum of principal 2×2 minors]
  //   c0 = -(aek + bfg + cdh - ceg - bdk - afh)  [negative determinant]

  #if defined(EIGENDEBUGMINIMAL)
    printf("== calculateEigenvalues33\n");
  #endif //EIGENDEBUGMINIMAL
  const cplx_t *m[9];
  realContext_t ctx159 = ctxtReal75;

  #if defined(OPTION_EIGEN_159)
    cplx159_t aek, bfg, cdh, ceg, bdk, afh, b, c, d, discr;
    ctx159.digits = 159;
  #else
    cplx_t aek, bfg, cdh, ceg, bdk, afh, b, c, d, discr;
    ctx159.digits = 75;
  #endif //OPTION_EIGEN_159
  real_t *const aekr = &aek.Real, *const aeki = &aek.Imag, *const bfgr = &bfg.Real, *const bfgi = &bfg.Imag, *const cdhr = &cdh.Real, *const cdhi = &cdh.Imag;
  real_t *const cegr = &ceg.Real, *const cegi = &ceg.Imag, *const bdkr = &bdk.Real, *const bdki = &bdk.Imag, *const afhr = &afh.Real, *const afhi = &afh.Imag;
  real_t *const br = &b.Real, *const bi = &b.Imag, *const cr = &c.Real, *const ci = &c.Imag, *const dr = &d.Real, *const di = &d.Imag;
  real_t *const discrR = &discr.Real, *const discrI = &discr.Imag;

  {
    m[0] = &mat[(size - 3) * size + (size - 3)];
    m[1] = m[0] + 1;
    m[2] = m[1] + 1;
    m[3] = &mat[(size - 2) * size + (size - 3)];
    m[4] = m[3] + 1;
    m[5] = m[4] + 1;
    m[6] = &mat[(size - 1) * size + (size - 3)];
    m[7] = m[6] + 1;
    m[8] = m[7] + 1;

    // Initialize all 159-digit variables
    realSetZero(br);
    realSetZero(bi);
    realSetZero(cr);
    realSetZero(ci);
    realSetZero(dr);
    realSetZero(di);
    realSetZero(discrR);
    realSetZero(discrI);
    realSetZero(aekr);
    realSetZero(aeki);
    realSetZero(bfgr);
    realSetZero(bfgi);
    realSetZero(cdhr);
    realSetZero(cdhi);
    realSetZero(cegr);
    realSetZero(cegi);
    realSetZero(bdkr);
    realSetZero(bdki);
    realSetZero(afhr);
    realSetZero(afhi);

    // quadratic coefficient: trace
    realAdd(&m[0]->Real, &m[4]->Real, br, &ctx159);
    realAdd(&m[0]->Imag, &m[4]->Imag, bi, &ctx159);
    realAdd(br, &m[8]->Real, br, &ctx159);
    realAdd(bi, &m[8]->Imag, bi, &ctx159);
    realChangeSign(br);
    realChangeSign(bi);

    // linear coefficient: sum of determinant of principal minors
    EIGEN_MUL(m[0], m[4], &aek);
    EIGEN_MUL(m[1], m[3], &bdk);
    EIGEN_MUL(m[0], m[8], &cdh);
    EIGEN_MUL(m[2], m[6], &ceg);
    EIGEN_MUL(m[4], m[8], &bfg);
    EIGEN_MUL(m[5], m[7], &afh);
    realAdd(aekr, cdhr, cr, &ctx159);
    realAdd(aeki, cdhi, ci, &ctx159);
    realAdd(cr, bfgr, cr, &ctx159);
    realAdd(ci, bfgi, ci, &ctx159);
    realSubtract(cr, bdkr, cr, &ctx159);
    realSubtract(ci, bdki, ci, &ctx159);
    realSubtract(cr, cegr, cr, &ctx159);
    realSubtract(ci, cegi, ci, &ctx159);
    realSubtract(cr, afhr, cr, &ctx159);
    realSubtract(ci, afhi, ci, &ctx159);

    // constant term: determinant
    EIGEN_MUL(&aek, m[8], &aek);
    EIGEN_MUL(m[1], m[5], &bfg);
    EIGEN_MUL(&bfg, m[6], &bfg);
    EIGEN_MUL(m[2], m[3], &cdh);
    EIGEN_MUL(&cdh, m[7], &cdh);
    EIGEN_MUL(&ceg, m[4], &ceg);
    EIGEN_MUL(&bdk, m[8], &bdk);
    EIGEN_MUL(&afh, m[0], &afh);
    realAdd(aekr, bfgr, dr, &ctx159);
    realAdd(aeki, bfgi, di, &ctx159);
    realAdd(dr, cdhr, dr, &ctx159);
    realAdd(di, cdhi, di, &ctx159);
    realSubtract(dr, cegr, dr, &ctx159);
    realSubtract(di, cegi, di, &ctx159);
    realSubtract(dr, bdkr, dr, &ctx159);
    realSubtract(di, bdki, di, &ctx159);
    realSubtract(dr, afhr, dr, &ctx159);
    realSubtract(di, afhi, di, &ctx159);
    realChangeSign(dr);
    realChangeSign(di);
  }

  blockMonitoring = true;
  #if defined(OPTION_EIGEN_159)
    cplx159_t t1H, t2H, t3H;
    realSetZero(&t1H.Real);
    realSetZero(&t1H.Imag);
    realSetZero(&t2H.Real);
    realSetZero(&t2H.Imag);
    realSetZero(&t3H.Real);
    realSetZero(&t3H.Imag);

    solveCubicEquation159(&b, &c, &d, &discr, &t1H, &t2H, &t3H, &ctx159);

    // Convert back to real_t
    realPlus(&t1H.Real, &t1->Real, realContext);
    realPlus(&t1H.Imag, &t1->Imag, realContext);
    realPlus(&t2H.Real, &t2->Real, realContext);
    realPlus(&t2H.Imag, &t2->Imag, realContext);
    realPlus(&t3H.Real, &t3->Real, realContext);
    realPlus(&t3H.Imag, &t3->Imag, realContext);
  #else //OPTION_EIGEN_159
    solveCubicEquation(&b, &c, &d, &discr, t1, t2, t3, &ctx159);
  #endif //OPTION_EIGEN_159
  blockMonitoring = false;

  if(is_real_symmetric) {
    realSetZero(&t1->Imag);
    realSetZero(&t2->Imag);
    realSetZero(&t3->Imag);
  }
}

#undef EIGEN_MUL

/* old shifter routine. Still operable
static void calculateQrShiftOld(const real_t *mat, uint16_t size, real_t *re, real_t *im, bool_t is_real_symmetric, realContext_t *realContext) {
  real_t t1r, t1i, t2r, t2i;
  real_t tmp, tmpR, tmpI;
  const real_t *dr, *di;

  dr = mat + ((size - 1) * size + (size - 1)) * 2; di = dr + 1;

  calculateEigenvalues22(mat, size, &t1r, &t1i, &t2r, &t2i, realContext);

  // Choose shift parameter
  realSubtract(&t1r, dr, &tmpR, realContext), realSubtract(&t1i, di, &tmpI, realContext);
  complexMagnitude(&tmpR, &tmpI, &tmpR, realContext);
  realSubtract(&t2r, dr, &tmp, realContext), realSubtract(&t2i, di, &tmpI, realContext);
  complexMagnitude(&tmp, &tmpI, &tmp, realContext);

  if(realCompareEqual(&t1r, &t2r) && realCompareEqual(&t1i, &t2i)) {
    realSetZero(re);
    realSetZero(im); // disable shift
  }
  else if(realCompareLessThan(&tmpR, &tmp)) {
    realCopy(&t1r, re);
    realCopy(&t1i, im);
  }
  else {
    realCopy(&t2r, re);
    realCopy(&t2i, im);
  }
}
*/





static void sortEigenvalues(cplx_t *eig, uint16_t size, uint16_t begin_a, uint16_t begin_b, uint16_t end_b, realContext_t *realContext) {
  const uint16_t end_a = begin_b - 1;

  if(size < 2) { // ... trivial
    return;
  }
  else if(begin_a == end_b) { // ... trivial
    return;
  }
  else if(size == 2) { // simply compare
    complexMagnitude(&eig[0], &eig[1].Real, realContext);
    complexMagnitude(&eig[3], &eig[2].Real, realContext);
    if(realCompareLessThan(&eig[1].Real, &eig[2].Real)) {
      realCopy(&eig[0].Real, &eig[1].Real);
      realCopy(&eig[0].Imag, &eig[1].Imag);
      realCopy(&eig[3].Real, &eig[0].Real);
      realCopy(&eig[3].Imag, &eig[0].Imag);
      realCopy(&eig[1].Real, &eig[3].Real);
      realCopy(&eig[1].Imag, &eig[3].Imag);
    }
  }
  else {
    uint16_t a = begin_a, b = begin_b;
    sortEigenvalues(eig, size, begin_a, (begin_a + end_a + 2) / 2, end_a, realContext);
    sortEigenvalues(eig, size, begin_b, (begin_b + end_b + 2) / 2, end_b, realContext);
    for(uint16_t i = begin_a; i <= end_b; i++) {
      complexMagnitude(&eig[i * size + i], &eig[i * size + (i + 1) % size].Real, realContext);
    }
    for(uint16_t i = begin_a; i <= end_b; i++) {
      if(a > end_a) {
        realCopy(&eig[b * size + b].Real, &eig[i * size + (i + 2) % size].Real    );
        realCopy(&eig[b * size + b].Imag, &eig[i * size + (i + 2) % size].Imag);
        ++b;
      }
      else if(b > end_b) {
        realCopy(&eig[a * size + a].Real, &eig[i * size + (i + 2) % size].Real    );
        realCopy(&eig[a * size + a].Imag, &eig[i * size + (i + 2) % size].Imag);
        ++a;
      }
      else if(realCompareLessThan(&eig[a * size + (a + 1) % size].Real, &eig[b * size + (b + 1) % size].Real)) {
        realCopy(&eig[b * size + b].Real, &eig[i * size + (i + 2) % size].Real    );
        realCopy(&eig[b * size + b].Imag, &eig[i * size + (i + 2) % size].Imag);
        ++b;
      }
      else {
        realCopy(&eig[a * size + a].Real, &eig[i * size + (i + 2) % size].Real    );
        realCopy(&eig[a * size + a].Imag, &eig[i * size + (i + 2) % size].Imag);
        ++a;
      }
    }
    for(uint16_t i = begin_a; i <= end_b; i++) {
      realCopy(&eig[i * size + (i + 2) % size].Real, &eig[i * size + i].Real    );
      realCopy(&eig[i * size + (i + 2) % size].Imag, &eig[i * size + i].Imag);
    }
  }
}



static bool_t isMatrixDiagonal(cplx_t *matrix, uint16_t size, real_t *tol, realContext_t *realContext) {
  for(uint16_t i = 0; i < size; i++) {
    for(uint16_t j = 0; j < size; j++) {
      if(i != j) {
        real_t offdiag_mag;
        complexMagnitude(&matrix[i * size + j], &offdiag_mag, realContext);
        if(!realCompareLessThan(&offdiag_mag, tol)) {
          return false;
        }
      }
    }
  }
  return true;
}



/**
 * sumOfSubSupDiagonalAll - compute sum of selected elements in a square matrix
 *
 * This function computes a sum over elements of the top-left `activeSize x activeSize` submatrix of a square matrix, depending on the selected mode:
 *   DIAG        - sum of diagonal elements
 *   SUPSUBDIAG  - sum of superdiagonal and subdiagonal elements
 *   NONDIAG     - sum of all non-diagonal elements
 *   CHDIAG      - sum of changes in the diagonal since previous call
 *
 * For CHDIAG, a bool_t `firstCall` must be provided: if true, the function initializes `previousDiagonal` and returns sum = 0. Otherwise, it sums the absolute change from `previousDiagonal`.
 *
 * Parameters:
 *   heading          - debug print heading
 *   matrix           - pointer to square matrix (size x size, complex stored as [re,im])
 *   previousDiagonal - array to store previous diagonal (only used in CHDIAG)
 *   size             - full matrix size
 *   activeSize       - size of submatrix to process
 *   mode             - mode - which elements to sum
 *   sum              - output sum (squares or abs depending on compile flags)
 *   firstCall        - only for CHDIAG, indicates array reset
 *   realContext
 */

typedef enum {
  DIAG,
  SUPSUBDIAG,
  NONDIAG,
  CHDIAG
} diagMode_t;


static void sumOfSubSupDiagonalAll(const char *heading, const cplx_t *matrix, cplx_t *previousDiagonal /*only CHDIAG*/, uint16_t size, uint16_t activeSize, diagMode_t mode, real_t *sum, bool_t firstCall /*only CHDIAG*/, realContext_t *realContext) {
  #if defined(EIGENDEBUG) || defined(EIGENDEBUGMINIMAL)
    printf("%s\n", heading);                 // print heading for debug
  #endif //EIGENDEBUG
  #if defined(CONV_SUM_159)
    REAL_T_PTR(sum159, 159); // 159-digit intermediate sum
    realSetZero(sum159);
    realContext_t ctx159 = ctxtReal75;
    ctx159.digits = 159;
  #endif //CONV_SUM_159

  real_t elemRe, elemIm;
  realSetZero(sum);                          // initialize sum to 0

  for(uint16_t i = 0; i < activeSize; i++) {
    for(uint16_t j = 0; j < activeSize; j++) {
      bool include = false;
      switch(mode) {
        case DIAG:
          include = (i == j);               // only diagonal elements
          break;
        case SUPSUBDIAG:
          include = (i == j + 1) || (j == i + 1);  // only super/subdiagonal
          break;
        case NONDIAG:
          include = (i != j);               // all non-diagonal elements
          break;
        case CHDIAG:
          include = (i == j);               // only diagonal, changes measured
          break;
      }
      if(include) {
        realCopy(&matrix[i * size + j].Real, &elemRe);
        realCopy(&matrix[i * size + j].Imag, &elemIm);

        #if defined(EIGENDEBUG) || defined(EIGENDEBUGMINIMAL)
          printf("Calculate element [%2d,%2d] = ", i, j);  // debug print
          printRealToConsole(&elemRe, "[", ",");
          printRealToConsole(&elemIm, "", "]\n");
        #endif

        if(mode == CHDIAG) {
          if(firstCall) {
            // store the initial diagonal and skip summing
            realCopy(&elemRe, &previousDiagonal[i].Real);
            realCopy(&elemIm, &previousDiagonal[i].Imag);
            continue;
          }
          else {
            // calculate change = abs(current - previous)
            real_t prevRe, prevIm;
            real_t changeRe, changeIm;
            realCopy(&previousDiagonal[i].Real, &prevRe);
            realCopy(&previousDiagonal[i].Imag, &prevIm);
            realSubtract(&elemRe, &prevRe, &changeRe, realContext);
            realSubtract(&elemIm, &prevIm, &changeIm, realContext);
            realSetPositiveSign(&changeRe);    // absolute value
            realSetPositiveSign(&changeIm);
            // update previous diagonal
            realCopy(&elemRe, &previousDiagonal[i].Real);
            realCopy(&elemIm, &previousDiagonal[i].Imag);
            elemRe = changeRe;                 // sum the change
            elemIm = changeIm;
          }
        }
        else {
          realSetPositiveSign(&elemRe);    // absolute value
          realSetPositiveSign(&elemIm);
        }

        #if defined(CONV_SUM_159)
          #if defined(ABS_SUMS)
            realAdd(&elemRe, sum159, sum159, &ctx159);   // sum abs
            realAdd(&elemIm, sum159, sum159, &ctx159);
          #else
            realFMA(&elemRe, &elemRe, sum159, sum159, &ctx159);  // sum squares
            realFMA(&elemIm, &elemIm, sum159, sum159, &ctx159);
          #endif
        #else //CONV_SUM_159
          #if defined(ABS_SUMS)
            realAdd(&elemRe, sum, sum, realContext);   // sum abs
            realAdd(&elemIm, sum, sum, realContext);
          #else
            realFMA(&elemRe, &elemRe, sum, sum, realContext);  // sum squares
            realFMA(&elemIm, &elemIm, sum, sum, realContext);
          #endif
        #endif //CONV_SUM_159
      }
    }
  }
  #if defined(CONV_SUM_159)
    realPlus(sum159, sum, realContext);    // move sum159 to output
  #endif //CONV_SUM_159
  #if defined(EIGENDEBUG) || defined(EIGENDEBUGMINIMAL)
    printf("Sum%s", mode == CHDIAG ? " of |changes|:" : "|diagonal values|:");
    printRealToConsole(sum, "", "\n");
  #endif
}



static void dropNoise(cplx_t *eig, uint16_t size, uint16_t dig) {
  int i;
  cplx_t *ptr;
  realContext_t c = *ctxtDeNoise;
  c.digits = dig;
  c.round = DEC_ROUND_HALF_UP;
  for(i = 0; i < size; i++) {
    ptr = &eig[i * size + i];
    realPlus(&ptr->Real, &ptr->Real, &c);
    realPlus(&ptr->Imag, &ptr->Imag, &c);
  }
}


// Check if element magnitude is within tolerance
static inline bool_t isElementWithinTolerance(const real_t *value_re, const real_t *value_im, const real_t *tol, realContext_t *realContext) {
  real_t mag;
  realMultiply(value_re, value_re, &mag, realContext);                     // |value| with the operations of complexMagnitude(), on separate parts
  realFMA(value_im, value_im, &mag, &mag, realContext);
  realSquareRoot(&mag, &mag, realContext);
  return realCompareLessThan(&mag, tol);
}

// check symmetrical; and check tridiagonal where the main diagonal and the diagonals immediately above and below it contain non-zero values; everything else is zero
static bool_t checkMatrixProperties(const cplx_t *a, uint16_t size, realContext_t *realContext) {
  real_t tol;
  realSetOne(&tol);
  tol.exponent -= symmetricTolerance;

  for(uint16_t i = 0; i < size; i++) {
    for(uint16_t j = i; j < size; j++) {  // Only upper triangle
      const cplx_t *a_ij = &a[i * size + j];
      const cplx_t *a_ji = &a[j * size + i];

      // Check imaginary part is zero (real matrix)
      if(!isElementWithinTolerance(&a_ij->Imag, const_0, &tol, realContext)) {
        return false;
      }

      // Check symmetry: a[i][j] == a[j][i]
      if(i < j) {  // Skip diagonal
        real_t diff_val;
        realSubtract(&a_ij->Real, &a_ji->Real, &diff_val, realContext);
        if(!isElementWithinTolerance(&diff_val, const_0, &tol, realContext)) {
          return false;
        }
      }
    }
  }
  return true;
}



static bool_t isRealSymmetric(const cplx_t *a, uint16_t size, realContext_t *realContext) {
  return checkMatrixProperties(a, size, realContext);
}

#if defined(EIGENDEBUG_QR) || defined(EIGENDEBUG)
static void printComplexMatrix(const char *heading, const cplx_t *m, uint16_t size, uint16_t activeSize, realContext_t *ctxt) {
  printf("\n%s (size=%u, activeSize=%u)\n", heading, size, activeSize);
  real_t tmp;
  realContext_t c = *ctxt;
  c.digits = 4;
  char reStr[80], imStr[80];
  for(uint16_t i = 0; i < activeSize; i++) {
    for(uint16_t j = 0; j < activeSize; j++) {
      uint32_t offset = i * size + j;
      //printf("[%u,%u] = ", i, j);
      realPlus(&m[offset].Real, &tmp, &c);  // Real part
      if(realGetExponent(&tmp) < -50) {
        printf("[≈0 ");
      }
      else {
        realToString(&tmp, reStr);
        printf("[%s", reStr);
      }
      realPlus(&m[offset].Imag, &tmp, &c);     // Imag part
      if(realGetExponent(&tmp) < -50) {
        printf(" i≈0] ");
      }
      else {
        realToString(&tmp, imStr);
        printf(" i%s] ", imStr);
      }
    }
    if(i < activeSize - 1) {
      printf("\n");
    }
  }
  printf("\n");
}
#endif //EIGENDEBUG_QR || defined(EIGENDEBUG)

#if defined(EIGENDEBUG)
static void printEigenvalues(const char *heading, const cplx_t *matrix, uint16_t size) {
  printf("%s\n", heading);
  for(int i = 0; i < size; i++) {
    real_t tmpRe, tmpIm;
    char tmpReStr[80], tmpImStr[80];
    realPlus(&matrix[i * size + i].Real, &tmpRe, &ctxtReal39);
    realPlus(&matrix[i * size + i].Imag, &tmpIm, &ctxtReal39);
    realToString(&tmpRe, tmpReStr);
    realToString(&tmpIm, tmpImStr);
    printf("eigenvalue[%d] = %s + %si\n", i, tmpReStr, tmpImStr);
  }
}

static void printEigenvaluesComparison(const char *heading, const cplx_t *a, const cplx_t *eig, uint16_t size, int converged) {
  printf("\n\n%s, converged = %d\n", heading, converged);
  printf("    Template  = 1.234567890123456789012345678901234\n");
  printEigenvalues("  eigenvalues a:", a, size);
  printEigenvalues("  eigenvalues eig:", eig, size);
}
#endif //EIGENDEBUG



static void solveEigenBlock(cplx_t *a, cplx_t *eig, uint16_t size, int first_unconverged, int last_unconverged, bool_t is_real_symmetric, realContext_t *realContext) {
    int n = last_unconverged - first_unconverged + 1;
    if(n < 2 || n > 3) {
      return;
    }

    #if defined(EIGENDEBUG)
        printf("Solving %dx%d block from position %d to %d\n", n, n, first_unconverged, last_unconverged);
    #endif
    // Allocate enough for max size 3x3
    cplx_t block[9];    // 9 complex numbers = 18 reals
    // Copy block from eig into local block
    for(int row = 0; row < n; row++) {
        for(int col = 0; col < n; col++) {
            int src_row = first_unconverged + row;
            int src_col = first_unconverged + col;
            realCopy(&eig[src_row * size + src_col].Real, &block[row * n + col].Real);
            realCopy(&eig[src_row * size + src_col].Imag, &block[row * n + col].Imag);
        }
    }
    #if defined(EIGENDEBUG)
        printf("%dx%d block contents before solving:\n", n, n);
        for(int row = 0; row < n; row++) {
            for(int col = 0; col < n; col++) {
                printf("  [%d][%d] = ", row, col);
                printRealToConsole(&block[row * n + col].Real, "", " + ");
                printRealToConsole(&block[row * n + col].Imag, "", "i\n");
            }
        }
    #endif // EIGENDEBUG

    // Storage for eigenvalues
    cplx_t ev[3];
    // Dispatch correct solver
    if(n == 2) {
        calculateEigenvalues22(block, 2, &ev[0], &ev[1], is_real_symmetric, realContext);
        #if defined(EIGENDEBUG)
           printf("Eigenvalues from 2x2 solver:\n");
           printRealToConsole(&ev[0].Real, "  ev1 = ", " + ");
           printRealToConsole(&ev[0].Imag, "", "i\n");
           printRealToConsole(&ev[1].Real, "  ev2 = ", " + ");
           printRealToConsole(&ev[1].Imag, "", "i\n");
        #endif
    }
    else if(n == 3) {
        calculateEigenvalues33(block, 3, &ev[0], &ev[1], &ev[2], is_real_symmetric, realContext);
        #if defined(EIGENDEBUG)
           printf("Eigenvalues from 3x3 solver:\n");
           for(int i = 0; i < 3; i++) {
               printf("  ev%d = ", i+1);
               printRealToConsole(&ev[i].Real, "", " + ");
               printRealToConsole(&ev[i].Imag, "", "i\n");
           }
        #endif
    }
    // Write results back to diagonal of a
    for(int i = 0; i < n; i++) {
        int pos = first_unconverged + i;
        realCopy(&ev[i].Real, &a[pos * size + pos].Real);
        realCopy(&ev[i].Imag, &a[pos * size + pos].Imag);
    }
}


/**
 * calculateSymmetricEigenvalues - compute the eigenvalues of an exact real symmetric matrix with cyclic Jacobi rotations.
 */
static bool_t calculateSymmetricEigenvalues(cplx_t *matrix, uint16_t size, realContext_t *realContext) {
  int32_t smallestExponent = 999999;

  for(uint32_t i = 0; i < (uint32_t)size * size; i++) {
    if(realIsSpecial(&matrix[i].Real) || realIsSpecial(&matrix[i].Imag)) {
      return false;
    }
    if(!realIsZero(&matrix[i].Real)) {
      smallestExponent = min(smallestExponent, realGetExponent(&matrix[i].Real));
    }
    if(!realIsZero(&matrix[i].Imag)) {
      smallestExponent = min(smallestExponent, realGetExponent(&matrix[i].Imag));
    }
  }
  for(uint16_t row = 0; row < size; row++) {
    for(uint16_t column = row; column < size; column++) {
      if(!realIsZero(&matrix[row * size + column].Imag) ||
         !realIsZero(&matrix[column * size + row].Imag) ||
         !realCompareEqual(&matrix[row * size + column].Real, &matrix[column * size + row].Real)) {
        return false;
      }
    }
  }

  const uint32_t iterationLimit = (uint32_t)size * 12 + 24;
  for(uint32_t iteration = 0; iteration < iterationLimit; iteration++) {
    for(uint16_t p = 0; p + 1 < size; p++) {
      for(uint16_t q = p + 1; q < size; q++) {
        real_t *apq = &matrix[p * size + q].Real;
        if(realIsZero(apq)) {
          continue;
        }

        real_t tau, denominator, magnitude, t, c, s, term;
        realSubtract(&matrix[q * size + q].Real, &matrix[p * size + p].Real, &tau, realContext);
        realMultiply(apq, const_2, &denominator, realContext);
        realDivide(&tau, &denominator, &tau, realContext);
        realCopyAbs(&tau, &magnitude);
        realFMA(&magnitude, &magnitude, const_1, &denominator, realContext);
        realSquareRoot(&denominator, &denominator, realContext);
        realAdd(&magnitude, &denominator, &denominator, realContext);
        realDivide(const_1, &denominator, &t, realContext);
        if(realIsNegative(&tau)) {
          realChangeSign(&t);
        }

        realFMA(&t, &t, const_1, &denominator, realContext);
        realSquareRoot(&denominator, &denominator, realContext);
        realDivide(const_1, &denominator, &c, realContext);
        realMultiply(&t, &c, &s, realContext);

        realMultiply(&t, apq, &term, realContext);
        realSubtract(&matrix[p * size + p].Real, &term, &matrix[p * size + p].Real, realContext);
        realAdd(&matrix[q * size + q].Real, &term, &matrix[q * size + q].Real, realContext);
        realSetZero(apq);
        realSetZero(&matrix[q * size + p].Real);

        for(uint16_t k = 0; k < size; k++) {
          if(k == p || k == q) {
            continue;
          }
          real_t oldP, oldQ, newP, newQ;
          realCopy(&matrix[k * size + p].Real, &oldP);
          realCopy(&matrix[k * size + q].Real, &oldQ);
          realMultiply(&c, &oldP, &newP, realContext);
          realMultiply(&s, &oldQ, &term, realContext);
          realSubtract(&newP, &term, &newP, realContext);
          realMultiply(&c, &oldQ, &newQ, realContext);
          realFMA(&s, &oldP, &newQ, &newQ, realContext);
          realCopy(&newP, &matrix[k * size + p].Real);
          realCopy(&newP, &matrix[p * size + k].Real);
          realCopy(&newQ, &matrix[k * size + q].Real);
          realCopy(&newQ, &matrix[q * size + k].Real);
        }
      }
    }

    real_t offDiagonalMax, diagonalMax, magnitude, tolerance;
    realSetZero(&offDiagonalMax);
    realSetZero(&diagonalMax);
    for(uint16_t row = 0; row < size; row++) {
      realCopyAbs(&matrix[row * size + row].Real, &magnitude);
      if(realCompareLessThan(&diagonalMax, &magnitude)) {
        realCopy(&magnitude, &diagonalMax);
      }
      for(uint16_t column = row + 1; column < size; column++) {
        realCopyAbs(&matrix[row * size + column].Real, &magnitude);
        if(realCompareLessThan(&offDiagonalMax, &magnitude)) {
          realCopy(&magnitude, &offDiagonalMax);
        }
      }
    }
    realCopy(&diagonalMax, &tolerance);
    if(realIsZero(&tolerance)) {                                                              // a zero diagonal gives no scale, so the test falls back to one
      realSetOne(&tolerance);
    }
    tolerance.exponent -= 60;
    if(realCompareLessThan(&offDiagonalMax, &tolerance)) {
      for(uint16_t row = 0; row < size; row++) {                                              // rounding noise of the rotations, below the smallest input element
        if(realGetExponent(&matrix[row * size + row].Real) < smallestExponent - eigenNoiseThreshold) {
          realSetZero(&matrix[row * size + row].Real);
        }
      }
      return true;
    }
  }
  return false;
}



/* Eigenvalues of a general complex matrix of size > 3, by the scheme LAPACK's dlahqr uses: reduce to upper Hessenberg once, then run shifted QR on that form with Givens
   rotations. A sweep costs O(n^2) and neither Q nor R is formed. The shift is a single complex one, so a complex eigenvalue deflates as a 1x1 and no real 2x2 block
   machinery is needed. */

/* The engine walks the caller's pool bulk as cplx_t. The bulk is untyped memory from allocC47Blocks sized in real_t, so the two views have to agree exactly:
   a cplx_t is two real_t with nothing between them and nothing after them. */
_Static_assert(sizeof(cplx_t) == 2 * sizeof(real_t), "cplx_t must be exactly two real_t");
_Static_assert(offsetof(cplx_t, Imag) == sizeof(real_t), "cplx_t must not pad between Real and Imag");

#define CX(m, n, i, j) ((m) + (size_t)(i) * (n) + (j))

static void cxCopy(const cplx_t *src, cplx_t *dst) {
  realCopy(&src->Real, &dst->Real);
  realCopy(&src->Imag, &dst->Imag);
}

static void cxSetZero(cplx_t *z) {
  realSetZero(&z->Real);
  realSetZero(&z->Imag);
}


static void cxConj(const cplx_t *x, cplx_t *z) {
  realCopy(&x->Real, &z->Real);
  realCopy(&x->Imag, &z->Imag);
  if(!realIsZero(&z->Imag)) {
    realChangeSign(&z->Imag);
  }
}

/* LAPACK CABS1: |Re x| + |Im x|, the norm zlahqr's deflation tests use, with no square root */
static void cxAbs1(const cplx_t *x, real_t *r, realContext_t *ctx) {
  real_t t;
  realCopyAbs(&x->Real, r);
  realCopyAbs(&x->Imag, &t);
  realAdd(r, &t, r, ctx);
}

static bool_t cxIsZero(const cplx_t *x) {
  return realIsZero(&x->Real) && realIsZero(&x->Imag);
}

/* principal square root through polar form */
static void cxSqrt(const cplx_t *x, cplx_t *z, realContext_t *ctx) {
  real_t m, t;

  if(cxIsZero(x)) {
    cxSetZero(z);
    return;
  }
  blockMonitoring = true;
  realRectangularToPolar(x, &m, &t, ctx);
  blockMonitoring = false;
  realSquareRoot(&m, &m, ctx);
  realMultiply(&t, const_1on2, &t, ctx);
  blockMonitoring = true;
  realPolarToRectangular(&m, &t, z, ctx);
  blockMonitoring = false;
}

/* ------------------------------------------------------------------------ *
 * Reduction to upper Hessenberg by complex Householder similarity.
 * In place on a (n x n complex). Scratch: v and w, n complex each.
 * ------------------------------------------------------------------------ */
static void hessenbergReduce(cplx_t *a, uint16_t n, cplx_t *v, cplx_t *w, realContext_t *ctx) {
  uint16_t k, i, j;
  real_t nrm, alpha, tmp, two;

  realAdd(const_1, const_1, &two, ctx);

  for(k = 0; k + 2 < n; k++) {
    /* x = a[k+1..n-1][k]; nothing to do when it is already a single element */
    realSetZero(&nrm);
    for(i = k + 1; i < n; i++) {
      complexMagnitude(CX(a, n, i, k), &tmp, ctx);
      realFMA(&tmp, &tmp, &nrm, &nrm, ctx);
    }
    realSquareRoot(&nrm, &nrm, ctx);
    if(realIsZero(&nrm)) {
      continue;
    }

    /* is the column already reduced? */
    {
      bool_t reduced = true;
      for(i = k + 2; reduced && i < n; i++) {
        reduced = cxIsZero(CX(a, n, i, k));
      }
      if(reduced) {
        continue;
      }
    }

    /* v = x + e^{i arg(x0)} * ||x|| * e1, then normalise */
    for(i = k + 1; i < n; i++) {
      cxCopy(CX(a, n, i, k), v + i);
    }
    complexMagnitude(v + k + 1, &alpha, ctx);
    if(realIsZero(&alpha)) {
      realAdd(&(v + k + 1)->Real, &nrm, &(v + k + 1)->Real, ctx);
    }
    else {
      cplx_t phase;
      realDivide(&nrm, &alpha, &tmp, ctx);                       /* ||x|| / |x0| */
      mulComplexReal(v + k + 1, &tmp, &phase, ctx);
      addComplex(v + k + 1, &phase, v + k + 1, ctx);
    }

    realSetZero(&nrm);
    for(i = k + 1; i < n; i++) {
      complexMagnitude(v + i, &tmp, ctx);
      realFMA(&tmp, &tmp, &nrm, &nrm, ctx);
    }
    realSquareRoot(&nrm, &nrm, ctx);
    if(realIsZero(&nrm)) {
      continue;
    }
    for(i = k + 1; i < n; i++) {
      divComplexReal(v + i, &nrm, v + i, ctx);
    }

    /* A[k+1:, :] -= 2 v (v^H A[k+1:, :]) */
    for(j = 0; j < n; j++) {
      cplx_t acc, cv, prod;
      cxSetZero(&acc);
      for(i = k + 1; i < n; i++) {
        cxConj(v + i, &cv);
        mulComplexComplex(&cv, CX(a, n, i, j), &prod, ctx);
        addComplex(&acc, &prod, &acc, ctx);
      }
      mulComplexReal(&acc, &two, &acc, ctx);
      cxCopy(&acc, w + j);
    }
    for(i = k + 1; i < n; i++) {
      for(j = 0; j < n; j++) {
        cplx_t prod;
        mulComplexComplex(v + i, w + j, &prod, ctx);
        subComplex(CX(a, n, i, j), &prod, CX(a, n, i, j), ctx);
      }
    }

    /* A[:, k+1:] -= 2 (A[:, k+1:] v) v^H */
    for(i = 0; i < n; i++) {
      cplx_t acc, prod;
      cxSetZero(&acc);
      for(j = k + 1; j < n; j++) {
        mulComplexComplex(CX(a, n, i, j), v + j, &prod, ctx);
        addComplex(&acc, &prod, &acc, ctx);
      }
      mulComplexReal(&acc, &two, &acc, ctx);
      cxCopy(&acc, w + i);
    }
    for(i = 0; i < n; i++) {
      for(j = k + 1; j < n; j++) {
        cplx_t cv, prod;
        cxConj(v + j, &cv);
        mulComplexComplex(w + i, &cv, &prod, ctx);
        subComplex(CX(a, n, i, j), &prod, CX(a, n, i, j), ctx);
      }
    }

    /* the annihilated entries are structurally zero: set them so exactly */
    for(i = k + 2; i < n; i++) {
      cxSetZero(CX(a, n, i, k));
    }
  }
}

/* ------------------------------------------------------------------------ *
 * Complex Givens rotation: c real, s complex, with
 *   [ c        s ] [ f ]   [ r ]
 *   [ -conj(s) c ] [ g ] = [ 0 ]
 * ------------------------------------------------------------------------ */
static void cxGivens(const cplx_t *f, const cplx_t *g, real_t *c, cplx_t *s, cplx_t *r, realContext_t *ctx) {
  real_t af, ag, t;

  complexMagnitude(f, &af, ctx);
  complexMagnitude(g, &ag, ctx);

  if(realIsZero(&ag)) {
    realSetOne(c);
    cxSetZero(s);
    cxCopy(f, r);
    return;
  }
  if(realIsZero(&af)) {
    realSetZero(c);
    cxSetZero(s);
    realSetOne(&s->Real);                /* s = 1 */
    cxCopy(g, r);
    return;
  }

  realMultiply(&af, &af, &t, ctx);
  realFMA(&ag, &ag, &t, &t, ctx);
  realSquareRoot(&t, &t, ctx);           /* t = sqrt(|f|^2 + |g|^2) */

  realDivide(&af, &t, c, ctx);           /* c = |f| / t */

  {
    cplx_t phase, cg;
    divComplexReal(f, &af, &phase, ctx);  /* phase = f / |f| */
    cxConj(g, &cg);
    mulComplexComplex(&phase, &cg, s, ctx);
    divComplexReal(s, &t, s, ctx);       /* s = phase * conj(g) / t */
    mulComplexReal(&phase, &t, r, ctx);  /* r = phase * t */
  }
}

/* ------------------------------------------------------------------------ *
 * One explicit shifted QR sweep on the active Hessenberg window [lo..hi].
 * Rotations are kept in cs (n reals) and sn (n complex).
 * ------------------------------------------------------------------------ */
static void hessenbergQrSweep(cplx_t *a, uint16_t n, uint16_t lo, uint16_t hi,
                              const cplx_t *shift, real_t *cs, cplx_t *sn, realContext_t *ctx) {
  uint16_t k, i, j;

  for(i = lo; i <= hi; i++) {
    subComplex(CX(a, n, i, i), shift, CX(a, n, i, i), ctx);
  }

  /* QR: annihilate the subdiagonal left to right */
  for(k = lo; k < hi; k++) {
    real_t c;
    cplx_t s, r;

    cxGivens(CX(a, n, k, k), CX(a, n, k + 1, k), &c, &s, &r, ctx);
    realCopy(&c, cs + k);
    cxCopy(&s, sn + k);

    cxCopy(&r, CX(a, n, k, k));
    cxSetZero(CX(a, n, k + 1, k));

    for(j = k + 1; j <= hi; j++) {
      cplx_t t1, t2, u, w, cs2;

      cxCopy(CX(a, n, k,     j), &u);
      cxCopy(CX(a, n, k + 1, j), &w);

      mulComplexReal(&u, &c, &t1, ctx);
      mulComplexComplex(&s, &w, &t2, ctx);
      addComplex(&t1, &t2, CX(a, n, k, j), ctx);       /* c*u + s*w */

      cxConj(&s, &cs2);
      mulComplexComplex(&cs2, &u, &t1, ctx);
      mulComplexReal(&w, &c, &t2, ctx);
      subComplex(&t2, &t1, CX(a, n, k + 1, j), ctx);   /* c*w - conj(s)*u */
    }
  }

  /* RQ: apply the rotations from the right */
  for(k = lo; k < hi; k++) {
    real_t c = *(cs + k);
    cplx_t s, cs2;
    uint16_t last;

    cxCopy(sn + k, &s);
    cxConj(&s, &cs2);
    last = (k + 2 <= hi) ? (uint16_t)(k + 2) : hi;

    for(i = lo; i <= last; i++) {
      cplx_t t1, t2, u, w;

      cxCopy(CX(a, n, i, k    ), &u);
      cxCopy(CX(a, n, i, k + 1), &w);

      mulComplexReal(&u, &c, &t1, ctx);
      mulComplexComplex(&cs2, &w, &t2, ctx);
      addComplex(&t1, &t2, CX(a, n, i, k), ctx);       /* c*u + conj(s)*w */

      mulComplexComplex(&s, &u, &t1, ctx);
      mulComplexReal(&w, &c, &t2, ctx);
      subComplex(&t2, &t1, CX(a, n, i, k + 1), ctx);   /* c*w - s*u */
    }
  }

  for(i = lo; i <= hi; i++) {
    addComplex(CX(a, n, i, i), shift, CX(a, n, i, i), ctx);
  }
}

/* Both eigenvalues of the trailing 2x2 of the active window, [[p,q],[rr,s]] at rows hi-1 and hi, from its trace and determinant. */
static void cxEig2x2Old(const cplx_t *a, uint16_t n, uint16_t hi, cplx_t *l1, cplx_t *l2, realContext_t *ctx) {
  const cplx_t *p = CX(a, n, hi - 1, hi - 1);
  const cplx_t *q = CX(a, n, hi - 1, hi    );
  const cplx_t *rr= CX(a, n, hi,     hi - 1);
  const cplx_t *s = CX(a, n, hi,     hi    );
  cplx_t tr, det, disc, t1, t2;

  addComplex(p, s, &tr, ctx);
  mulComplexComplex(p, s, &t1, ctx);
  mulComplexComplex(q, rr, &t2, ctx);
  subComplex(&t1, &t2, &det, ctx);

  mulComplexComplex(&tr, &tr, &disc, ctx);
  mulComplexReal(&det, const_4, &t1, ctx);
  subComplex(&disc, &t1, &disc, ctx);
  cxSqrt(&disc, &disc, ctx);

  addComplex(&tr, &disc, l1, ctx);
  mulComplexReal(l1, const_1on2, l1, ctx);
  subComplex(&tr, &disc, l2, ctx);
  mulComplexReal(l2, const_1on2, l2, ctx);
}

static void cxEig2x2(const cplx_t *a, uint16_t n, uint16_t hi, cplx_t *l1, cplx_t *l2, realContext_t *ctx) {
  const cplx_t *p = CX(a, n, hi - 1, hi - 1);
  const cplx_t *q = CX(a, n, hi - 1, hi    );
  const cplx_t *rr= CX(a, n, hi,     hi - 1);
  const cplx_t *s = CX(a, n, hi,     hi    );
  cplx_t x, u2, y, d, t;
  real_t r1, r2;
  bool_t flipped;

  /* LAPACK zlahqr's Wilkinson shift: with x = (p - s) / 2 and u2 = q r, y = sqrt(x^2 + u2) takes the sign that makes x + y the
     larger of x -+ y, and the roots are p + u2 / (x + y) and s - u2 / (x + y). Neither subtracts two nearly equal numbers, where
     (tr - sqrt(tr^2 - 4 det)) / 2 cancels to 0 for a root many orders below the other. l1 keeps (tr + sqrt) / 2, l2 (tr - sqrt) / 2. */
  subComplex(p, s, &x, ctx);
  mulComplexReal(&x, const_1on2, &x, ctx);
  mulComplexComplex(q, rr, &u2, ctx);
  mulComplexComplex(&x, &x, &y, ctx);
  addComplex(&y, &u2, &y, ctx);
  cxSqrt(&y, &y, ctx);
  realMultiply(&x.Real, &y.Real, &r1, ctx);
  realFMA(&x.Imag, &y.Imag, &r1, &r2, ctx);
  flipped = realIsNegative(&r2) && !realIsZero(&r2);
  if(flipped) {
    realChangeSign(&y.Real);
    realChangeSign(&y.Imag);
  }
  addComplex(&x, &y, &d, ctx);
  if(cxIsZero(&d)) {                                                      // x = y = 0: p = s and q r = 0, a double root
    cxCopy(p, l1);
    cxCopy(s, l2);
    return;
  }
  divComplexComplex(&u2, &d, &t, ctx);
  if(flipped) {
    subComplex(s, &t, l1, ctx);
    addComplex(p, &t, l2, ctx);
  }
  else {
    addComplex(p, &t, l1, ctx);
    subComplex(s, &t, l2, ctx);
  }
}

/* Wilkinson shift: the eigenvalue of that 2x2 nearer its trailing entry. */
static void wilkinsonShift(const cplx_t *a, uint16_t n, uint16_t hi, cplx_t *shift, realContext_t *ctx) {
  const cplx_t *s = CX(a, n, hi, hi);
  cplx_t t1, t2, l1, l2;
  real_t d1, d2;

  cxEig2x2Old(a, n, hi, &l1, &l2, ctx);

  subComplex(&l1, s, &t1, ctx);
  complexMagnitude(&t1, &d1, ctx);
  subComplex(&l2, s, &t2, ctx);
  complexMagnitude(&t2, &d2, ctx);

  cxCopy(realCompareLessThan(&d1, &d2) ? &l1 : &l2, shift);
}

/* ------------------------------------------------------------------------ *
 * Eigenvalues of a general complex matrix, n > 3.
 * a is destroyed. On success the eigenvalues are written to the diagonal of
 * eig, which is zeroed first. Scratch comes from the caller's q and r.
 * Returns false when the iteration does not converge, so the caller can fall
 * back to the legacy path.
 * ------------------------------------------------------------------------ */
static bool_t eigenHessenbergQr(cplx_t *a, cplx_t *eig, cplx_t *scratch1, real_t *scratch2,
                                uint16_t n, realContext_t *ctx) {
  bool_t inputWasReal = true;
  real_t scale;
  cplx_t *v  = scratch1;                     /* n complex */
  cplx_t *w  = scratch1 + n;                 /* n complex */
  real_t *cs = scratch2;                     /* n real    */
  cplx_t *sn = (cplx_t *)(scratch2 + n);     /* n complex */
  real_t eps, tmp, t1, t2, w1, w2;
  cplx_t shift;
  uint16_t hi, i, j;
  uint32_t sweeps = 0, sinceDeflation = 0;
  const uint32_t itmax = 30u * ((uint32_t)n > 10u ? (uint32_t)n : 10u);   // LAPACK dlaqr0: MAX(30, 2*KEXSH) * MAX(10, nh)

  realAdd(const_1on2, const_1on4, &w1, ctx);                                // LAPACK WILK1 = 0.75 and WILK2 = -7/16, built once per call from the
  realMultiply(const_1on4, const_1on4, &w2, ctx);                           // generated constants; both are exact in decimal arithmetic
  realMultiply(&w2, const_7, &w2, ctx);
  realChangeSign(&w2);
  realSetOne(&eps);
  eps.exponent -= (eigenTolerance > 3 ? eigenTolerance - 3 : eigenTolerance);

  realSetZero(&scale);
  for(i = 0; i < n; i++) {
    for(j = 0; j < n; j++) {
      if(!realIsZero(&CX(a, n, i, j)->Imag)) {
        inputWasReal = false;
      }
      complexMagnitude(CX(a, n, i, j), &tmp, ctx);
      if(realCompareGreaterThan(&tmp, &scale)) {
        realCopy(&tmp, &scale);
      }
    }
  }

  hessenbergReduce(a, n, v, w, ctx);

  hi = (uint16_t)(n - 1);
  while(hi > 0) {
    if(sweeps++ >= itmax) {
      return false;                                                       // the caller reports it: no silent wrong answer
    }

    /* deflate every negligible subdiagonal in the active window */
    for(i = hi; i >= 1; i--) {
      cxAbs1(CX(a, n, i, i - 1), &tmp, ctx);
      cxAbs1(CX(a, n, i - 1, i - 1), &t1, ctx);
      cxAbs1(CX(a, n, i, i), &t2, ctx);
      realAdd(&t1, &t2, &t1, ctx);
      realMultiply(&t1, &eps, &t1, ctx);
      if(realIsZero(&tmp)) {
        cxSetZero(CX(a, n, i, i - 1));
      }
      else if(realCompareLessThan(&tmp, &t1)) {
        /* Ahues and Tisseur, the criterion dlahqr adopted: a subdiagonal small beside its own two
           diagonal entries is not on its own grounds to deflate. Where the two differ by orders of
           magnitude the naive test drops the coupling and reports the diagonal entry, so
           [[1E3000,1],[1,1E-3000]] yields 1E-3000 in place of the 0 the entries express. Compare the
           two off-diagonals against the separation instead. */
        real_t ab, ba, aa, bb, ss, lhs, rhs;
        cplx_t dif;

        cxAbs1(CX(a, n, i - 1, i), &ba, ctx);
        realCopy(&tmp, &ab);
        if(realCompareLessThan(&ab, &ba)) {
          realCopy(&tmp, &t2);
          realCopy(&ba, &ab);
          realCopy(&t2, &ba);
        }
        subComplex(CX(a, n, i - 1, i - 1), CX(a, n, i, i), &dif, ctx);
        cxAbs1(&dif, &bb, ctx);
        cxAbs1(CX(a, n, i, i), &aa, ctx);
        if(realCompareLessThan(&aa, &bb)) {
          realCopy(&aa, &t2);
          realCopy(&bb, &aa);
          realCopy(&t2, &bb);
        }
        realAdd(&aa, &ab, &ss, ctx);
        if(!realIsZero(&ss)) {
          realDivide(&ab, &ss, &lhs, ctx);
          realMultiply(&ba, &lhs, &lhs, ctx);
          realDivide(&aa, &ss, &rhs, ctx);
          realMultiply(&bb, &rhs, &rhs, ctx);
          realMultiply(&rhs, &eps, &rhs, ctx);
          if(realCompareLessThan(&lhs, &rhs) || realCompareEqual(&lhs, &rhs)) {
            cxSetZero(CX(a, n, i, i - 1));
          }
        }
        else {
          cxSetZero(CX(a, n, i, i - 1));
        }
      }
      if(i == 1) {
        break;
      }
    }

    if(cxIsZero(CX(a, n, hi, hi - 1))) {
      hi--;                              /* a[hi][hi] is an eigenvalue */
      sinceDeflation = 0;
      continue;
    }

    /* A trailing 2x2 is solved from its own trace and determinant rather than iterated, as
       dlahqr hands one to dlanv2. Iterating it converges to a rounding of the pair instead of
       the pair: on [[1E3000,1],[1,1E-3000]] the quadratic gives the exact 0 that the entries
       express, where a sweep leaves 1E-3000 behind. */
    if(hi == 1 || cxIsZero(CX(a, n, hi - 1, hi - 2))) {
      cplx_t l1, l2;

      cxEig2x2Old(a, n, hi, &l1, &l2, ctx);
      if(cxIsZero(&l1) || cxIsZero(&l2)) {                                 // (tr - sqrt(tr^2 - 4 det)) / 2 cancelled to 0: take zlahqr's form, which keeps a root
        cxEig2x2(a, n, hi, &l1, &l2, ctx);                                 // many orders below the other
      }

      cxCopy(&l1, CX(a, n, hi - 1, hi - 1));
      cxCopy(&l2, CX(a, n, hi, hi));
      cxSetZero(CX(a, n, hi, hi - 1));
      hi = (hi >= 2) ? (uint16_t)(hi - 2) : 0;            // hi == 1 means the block is the leading one: stop rather than wrap
      sinceDeflation = 0;
      continue;
    }

    /* start of the active block: the first non-negligible subdiagonal above hi */
    {
      uint16_t lo = hi;
      while(lo > 0 && !cxIsZero(CX(a, n, lo, lo - 1))) {
        lo--;
      }

      /* Break a cycle the trailing 2x2 cannot see: every KEXSH sweeps without a deflation, take the shift from a 2x2 built on the last two subdiagonals,
         as LAPACK's dlaqr0 forms it. WILK2 is negative, so that 2x2 has a complex pair and the shift carries the iteration off a symmetric stall. */
      if(++sinceDeflation % 10 == 0) {
        real_t ss;
        cplx_t bb, cc, disc, aa;

        complexMagnitude(CX(a, n, hi, hi - 1), &ss, ctx);
        if(hi >= lo + 2) {
          complexMagnitude(CX(a, n, hi - 1, hi - 2), &tmp, ctx);
          realAdd(&ss, &tmp, &ss, ctx);
        }
        realMultiply(&ss, &w1, &aa.Real, ctx);
        realSetZero(&aa.Imag);
        addComplex(&aa, CX(a, n, hi, hi), &aa, ctx);                      // AA = WILK1*SS + H(hi,hi)
        realCopy(&ss, &bb.Real);
        realSetZero(&bb.Imag);                                            // BB = SS
        realMultiply(&ss, &w2, &cc.Real, ctx);
        realSetZero(&cc.Imag);                                            // CC = WILK2*SS
        mulComplexComplex(&bb, &cc, &disc, ctx);
        cxSqrt(&disc, &disc, ctx);
        addComplex(&aa, &disc, &shift, ctx);                              // an eigenvalue of [[AA,BB],[CC,AA]]
      }
      else {
        wilkinsonShift(a, n, hi, &shift, ctx);
      }

      hessenbergQrSweep(a, n, lo, hi, &shift, cs, sn, ctx);
    }
  }

  /* A real matrix has a real characteristic polynomial, so its eigenvalues are real
     or exact conjugate pairs. An imaginary part far below the eigenvalue's own scale
     is therefore convergence residue and not a feature the input could encode: the
     same reasoning slvp.c already applies to its roots. Left in place it would type
     a real spectrum as complex. */
  if(inputWasReal) {
    for(i = 0; i < n; i++) {
      cplx_t *d = CX(a, n, i, i);
      if(!realIsZero(&d->Imag)) {
        int32_t top = realIsZero(&d->Real) ? realGetExponent(&scale) : realGetExponent(&d->Real);
        if(realGetExponent(&d->Imag) < top - (int32_t)(toleranceDigits)) {
          realSetZero(&d->Imag);
        }
      }
    }
  }

  for(i = 0; i < n; i++) {
    for(j = 0; j < n; j++) {
      cxSetZero(CX(eig, n, i, j));
    }
  }
  for(i = 0; i < n; i++) {
    cxCopy(CX(a, n, i, i), CX(eig, n, i, i));
  }
  return true;
}


EIGENVALUES_LINKAGE void calculateEigenvalues(cplx_t *a, cplx_t *scratch, cplx_t *eig, uint16_t size, bool_t shifted, bool_t reducedSignificantDigits, realContext_t *realContext) {


  uint16_t i, j;
  bool_t converged = false;



  #if defined(EIGENDEBUG1)
  printf("Before start: size=%d\n", size);
  #endif

 // shifted = false;       //Can disable shifts here - they cause instability in minimal cases.

                                                          #if defined(EIGENDEBUG)
                                                          printf("Input matrix verification:\n");
                                                          for(int row = 0; row < size; row++) {
                                                            for(int col = 0; col < size; col++) {
                                                              real_t val_re, val_im;
                                                              char reStr[80], imStr[80];
                                                              realPlus(&a[row * size + col].Real, &val_re, &ctxtReal4);
                                                              realPlus(&a[row * size + col].Imag, &val_im, &ctxtReal4);
                                                              realToString(&val_re, reStr);
                                                              realToString(&val_im, imStr);
                                                              printf("[%s+%si] ", reStr, imStr);
                                                            }
                                                            printf("\n");
                                                          }
                                                          #endif // EIGENDEBUG

  bool_t is_real_symmetric = isRealSymmetric(a, size, realContext);
  #if defined(EIGENDEBUG) || defined(EIGENDEBUGMINIMAL)
    if(is_real_symmetric) {
      printf("DETECTED: Real Symmetric Matrix\n");
    }
  #endif

// initialize



  // Initialize shift values


    // Initialize eig (size*size*2 reals) to zero.
    for(int i = 0; i < size * size; i++) {
      realSetZero(&eig[i].Real);
      realSetZero(&eig[i].Imag);
    }

    // Then copy input matrix to eig
    for(int i = 0; i < size; i++) {
      for(int j = 0; j < size; j++) {
        realCopy(&a[i * size + j].Real, &eig[i * size + j].Real);
        realCopy(&a[i * size + j].Imag, &eig[i * size + j].Imag);
      }
    }

#if defined(EIGENDEBUG)
  // Copy input matrix to original_matrix
  cplx_t original_matrix[size * size];
  for(int i = 0; i < size * size; i++) {
    realCopy(&a[i].Real, &original_matrix[i].Real);
    realCopy(&a[i].Imag, &original_matrix[i].Imag);
  }
#endif

  // Init done



  #if defined(EIGENDEBUGMINIMAL) || defined(EIGENDEBUG)
    printf("START, looking for easy analytic solves\n");
  #endif //EIGENDEBUGMINIMAL) || defined(EIGENDEBUG)

  currentKeyCode = 255;
  ++currentSolverNestingDepth;                                                                // the epilogue decrement runs on every path: increment here too, not in the QR branch only,
  setSystemFlag(FLAG_SOLVING);                                                                // or each analytic 2x2/3x3 solve underflows the depth and FLAG_SOLVING never clears

  if(size == 2) {
    calculateEigenvalues22(a, size, &eig[0], &eig[3], is_real_symmetric, realContext);
    sortEigenvalues(eig, size, 0, (size + 1) / 2, size - 1, realContext);
    dropNoise(eig, size, 39);
  }
  else if(size == 3) {
    calculateEigenvalues33(a, size, &eig[0], &eig[4], &eig[8], is_real_symmetric, realContext);
    sortEigenvalues(eig, size, 0, (size + 1) / 2, size - 1, realContext);
    dropNoise(eig, size, 39);
  }
  else { //size > 3
    if(is_real_symmetric && calculateSymmetricEigenvalues(eig, size, realContext)) {
      sortEigenvalues(eig, size, 0, (size + 1) / 2, size - 1, realContext);
      dropNoise(eig, size, 39);
      goto eigenvaluesDone;
    }

    #if defined(EIGENDEBUGMINIMAL) || defined(EIGENDEBUG)
      printf("None found, continue setting up QR solve\n");
    #endif //EIGENDEBUGMINIMAL) || defined(EIGENDEBUG)

    real_t tol;
    if(reducedSignificantDigits) {
      if(toleranceDigits >= 34 || toleranceDigits == 0) {        // typ 37
        realSetOne(&tol);
        tol.exponent -= eigenTolerance;  // typ 70
      }
      else {
        realSetOne(&tol);
        tol.exponent -= toleranceDigits; // typ 37
      }
    }
    else {
      realSetOne(&tol);
      tol.exponent -= eigenTolerance;    // typ 70
    }

    #if defined(EIGENDEBUG) || defined(EIGENDEBUGMINIMAL)
      printRealToConsole(&tol, "calculateEigenvalues deep tolerance: ", " ");
      printf("called with realContext = %d digits, ", (int)(realContext->digits));
      printRealToConsole(&SumTolerance, "user tolerance + extra: ", "\n");
      printf("significantDigits=%d\n", significantDigits);
    #endif



    if(isMatrixDiagonal(a, size, &tol, realContext)) {
      for(i = 0; i < size * size; i++) {
        realCopy(&a[i].Real, &eig[i].Real);
        realCopy(&a[i].Imag, &eig[i].Imag);
      }
      converged = true;
    }





    // Destroys a. On non-convergence the caller raises ERROR_NO_ROOT_FOUND and writes no result.
    if(!converged && lastErrorCode == ERROR_NONE) {
      if(eigenHessenbergQr(a, eig, scratch, &scratch[(size_t)size * 2].Real, size, realContext)) {
        converged = true;
      }
      else {
        displayCalcErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE);               // LAPACK reports which eigenvalues
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)                                            // converged and returns; it does not
          sprintf(errorMessage, "QR did not converge for a %d x %d matrix", size, size); // retry with a weaker algorithm
          moreInfoOnError("In function calculateEigenvalues:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }


#if defined(EIGENDEBUG) || defined(EIGENDEBUG2) || defined(EIGENDEBUGMINIMAL)
  printf("==== End QR loop and start post processing after QR loop exit \n");
#endif



                                                                    #if defined(EIGENDEBUG) || defined(EIGENDEBUGMINIMAL)
                                                                    debugf("Exiting Main QR Loop");
                                                                    printf("\n=== EXITED MAIN LOOP ===\n");
                                                                    printf("converged = %d\n", converged);
                                                                    printf("lastErrorCode = %d\n", lastErrorCode);
                                                                    printf("\nFinal diagonal elements:\n");
                                                                    for(int k = 0; k < size; k++) {
                                                                      printf("eig[%d][%d] = ", k, k);
                                                                      printRealToConsole(&eig[k * size + k].Real, "", " + ");
                                                                      printRealToConsole(&eig[k * size + k].Imag, "", "i\n");
                                                                    }
                                                                    printf("\nFinal subdiagonal elements:\n");
                                                                    for(int k = 1; k < size; k++) {
                                                                      printf("eig[%2d][%2d] = ", k, k-1);
                                                                      printRealToConsole(&eig[k * size + (k-1)].Real, "", " + ");
                                                                      printRealToConsole(&eig[k * size + (k-1)].Imag, "", "i\n");
                                                                      printf("eig[%2d][%2d] = ", k-1, k);
                                                                      printRealToConsole(&eig[(k-1) * size + (k)].Real, "", " + ");
                                                                      printRealToConsole(&eig[(k-1) * size + (k)].Imag, "", "i\n");
                                                                    }
                                                                    printEigenvaluesComparison("EIGENVAL after main QR loop", a, eig, size, converged);
                                                                    #endif //EIGENDEBUG



    #if defined(POST_QR_RELATIVE_BLOCK_CHECK)
      // Compute relative threshold based on average diagonal magnitude
      real_t diag_sum, avg_diag, rel_threshold, matrixSize;
      sumOfSubSupDiagonalAll("", eig, NULL, size, size, DIAG, &diag_sum, true, realContext);
      // avg_diag = diag_sum / size
      int32ToReal(size, &matrixSize);
      realDivide(&diag_sum, &matrixSize, &avg_diag, realContext);
      realCopy(&avg_diag, &rel_threshold);
      rel_threshold.exponent -= finalBlockTolerance;
    #endif //POST_QR_RELATIVE_BLOCK_CHECK



#if defined(EIGENDEBUG) || defined(EIGENDEBUG2) || defined(EIGENDEBUGMINIMAL)
  printf("==== 2x2 block scan and solve \n");
#endif


    #if defined(EIGENDEBUG)
    printf("DEBUG: Starting 2x2 block scan, size=%d\n", size);
    fflush(stdout);
    #endif

    // Solve each block of the quasi-triangular eig on its own: solveEigenBlock writes the eigenvalues of a 2x2 or 3x3 block on the diagonal of a. A subdiagonal
    // element under the threshold ends a block, a 1x1 block keeps the diagonal of a, and a longer block, or one with an element below it at the threshold or
    // above, is reported as no root found unless an error is already set. eigenHessenbergQr returns a diagonal eig, so its blocks are all 1x1; the diagonal
    // shortcut above copies a whole matrix whose off-diagonals are merely under the deep tolerance, and it is that path this scan solves rather than checks.
    {
      real_t blockThreshold, mag;
      bool_t solveWhole = false;
      #if defined(POST_QR_RELATIVE_BLOCK_CHECK)
        realCopy(&rel_threshold, &blockThreshold);
      #else // !POST_QR_RELATIVE_BLOCK_CHECK
        realSetOne(&blockThreshold);
        blockThreshold.exponent -= blockDetectionTolerance;
      #endif // POST_QR_RELATIVE_BLOCK_CHECK

      for(i = 0; i < size; i = j + 1) {
        bool_t coupled = false;
        for(j = i; j + 1 < size; j++) {                                                     // j stops on the last row of the block that starts on row i
          real_t offdiag_mag;
          complexMagnitude(&eig[(j + 1) * size + j], &offdiag_mag, realContext);
          if(realIsZero(&offdiag_mag) || realCompareLessThan(&offdiag_mag, &blockThreshold)) {
            break;
          }
        }
        #if defined(EIGENDEBUG)
          printf("Block from %d to %d\n", i, j);
        #endif // EIGENDEBUG

        for(uint16_t row = j + 1; row < size && !coupled; row++) {
          for(uint16_t col = i; col <= j && !coupled; col++) {
            complexMagnitude(&eig[row * size + col], &mag, realContext);
            coupled = !realIsZero(&mag) && !realCompareLessThan(&mag, &blockThreshold);
          }
        }
        if(!coupled && (j == i + 1 || j == i + 2)) {
          solveEigenBlock(a, eig, size, i, j, is_real_symmetric, realContext);
        }
        else if((coupled || j > i + 2) && lastErrorCode == ERROR_NONE) {
          solveWhole = true;
        }
      }
      if(solveWhole) {                                                                       // the diagonal shortcut took couplings no 2x2 or 3x3 block solver takes:
        for(i = 0; i < size * size; i++) {                                                   // eig still holds the input, so solve the whole matrix
          realCopy(&eig[i].Real, &a[i].Real);
          realCopy(&eig[i].Imag, &a[i].Imag);
        }
        if(!eigenHessenbergQr(a, eig, scratch, &scratch[(size_t)size * 2].Real, size, realContext)) {
          displayCalcErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "QR did not converge for a %d x %d matrix", size, size);
            moreInfoOnError("In function calculateEigenvalues:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
      }
    }

                                                                    #if defined(EIGENDEBUG)
                                                                    printEigenvaluesComparison("before sorting and conditioning", a, eig, size, converged);
                                                                    #endif //EIGENDEBUG


    // Copy from a to eig before sorting (in case a was updated by block solvers)
    for(i = 0; i < size; i++) {
      realCopy(&a[i * size + i].Real, &eig[i * size + i].Real);
      realCopy(&a[i * size + i].Imag, &eig[i * size + i].Imag);
    }

    sortEigenvalues(eig, size, 0, (size + 1) / 2, size - 1, realContext);

    for(i = 0; i + 1 < size; i++) {                                                  // Give a conjugate pair one real part and the positive imaginary part first. The two
      cplx_t *u = &eig[i * size + i], *v = &eig[(i + 1) * size + (i + 1)];               // members are computed at different deflation steps, so the sort above ranks them by
      real_t du, dv, t, mean;                                                        // magnitudes that agree only to within a few ulp, and their real parts carry the
                                                                                     // residue of each. Identify the pair by a real difference and an imaginary sum both
                                                                                     // under the tolerance, then order it by sign
      if(realIsZero(&u->Imag) || realIsZero(&v->Imag) || realIsNegative(&u->Imag) == realIsNegative(&v->Imag)) {
        continue;
      }
      realSubtract(&u->Real, &v->Real, &du, realContext);
      realAdd(&u->Imag, &v->Imag, &dv, realContext);
      realCopyAbs(&du, &du);
      realCopyAbs(&dv, &dv);
      complexMagnitude(u, &t, realContext);                                          // Scale by the pair's own magnitude: a pair on the imaginary axis has a zero
      t.exponent -= toleranceDigits;                                                 // real part, and nothing is below a tolerance scaled by that
      if(realCompareLessThan(&du, &t) && realCompareLessThan(&dv, &t)) {
        realAdd(&u->Real, &v->Real, &mean, realContext);                             // A real matrix has a real characteristic polynomial, so the two members
        realMultiply(&mean, const_1on2, &mean, realContext);                         // share a real part exactly; the mean is that value where they agree and
        realCopy(&mean, &u->Real);                                                   // drops the residue where they do not
        realCopy(&mean, &v->Real);
        if(realIsNegative(&u->Imag)) {
          cplx_t swap;
          realCopy(&u->Real, &swap.Real);
          realCopy(&u->Imag, &swap.Imag);
          realCopy(&v->Real, &u->Real);
          realCopy(&v->Imag, &u->Imag);
          realCopy(&swap.Real, &v->Real);
          realCopy(&swap.Imag, &v->Imag);
        }
      }
    }
                                                                    #if defined(EIGENDEBUG)
                                                                    printEigenvaluesComparison("after sorting", a, eig, size, converged);
                                                                    #endif //EIGENDEBUG

    dropNoise(eig, size, 39);


                                                                #if defined(EIGENDEBUG) || defined(EIGENDEBUGMINIMAL)
                                                                  // Copy sorted and conditioned eigenvalues from eig back to a
                                                                  for(i = 0; i < size; i++) {
                                                                    realCopy(&eig[i * size + i].Real, &a[i * size + i].Real    );
                                                                    realCopy(&eig[i * size + i].Imag, &a[i * size + i].Imag);
                                                                  }

                                                                    printEigenvaluesComparison("EIGENVAL after conditioning...", a, eig, size, converged);
                                                                #endif //EIGENDEBUG

  } // size > 3

                                                                #if defined(EIGENDEBUG) || defined(EIGENDEBUGMINIMAL)
                                                                  printEigenvaluesComparison("eig end of 2, 3, >3:", a, eig, size, converged);
                                                            //      printComplexMatrix("eig end of 2, 3, >3:",eig, size, size, &ctxtReal4);
                                                                #endif

                                                                #if defined(EIGENDEBUG)
                                                                  printf("\n=== EIGENVALUE VERIFICATION ===\n");
                                                                  // Calculate trace of original matrix
                                                                  real_t original_trace_re, original_trace_im;
                                                                  realSetZero(&original_trace_re);
                                                                  realSetZero(&original_trace_im);
                                                                  for(int i = 0; i < size; i++) {
                                                                    realAdd(&original_trace_re, &original_matrix[i * size + i].Real, &original_trace_re, realContext);
                                                                    realAdd(&original_trace_im, &original_matrix[i * size + i].Imag, &original_trace_im, realContext);
                                                                  }
                                                                  // Calculate sum of computed eigenvalues
                                                                  real_t eigenval_sum_re, eigenval_sum_im;
                                                                  realSetZero(&eigenval_sum_re);
                                                                  realSetZero(&eigenval_sum_im);
                                                                  for(int i = 0; i < size; i++) {
                                                                    realAdd(&eigenval_sum_re, &a[i * size + i].Real, &eigenval_sum_re, realContext);
                                                                    realAdd(&eigenval_sum_im, &a[i * size + i].Imag, &eigenval_sum_im, realContext);
                                                                  }
                                                                  // Calculate trace difference
                                                                  real_t trace_error;
                                                                  cplx_t trace_diff;
                                                                  realSubtract(&original_trace_re, &eigenval_sum_re, &trace_diff.Real, realContext);
                                                                  realSubtract(&original_trace_im, &eigenval_sum_im, &trace_diff.Imag, realContext);
                                                                  complexMagnitude(&trace_diff, &trace_error, realContext);
                                                                  //
                                                                  printf("Original trace: ");
                                                                  printRealToConsole(&original_trace_re, "", "");
                                                                  printRealToConsole(&original_trace_im, "+", "i\n");
                                                                  printf("Eigenvalue sum: ");
                                                                  printRealToConsole(&eigenval_sum_re, "", "");
                                                                  printRealToConsole(&eigenval_sum_im, "+", "i\n");
                                                                  printf("Trace error: ");
                                                                  printRealToConsole(&trace_error, "", "\n");
                                                                  printf("=== END VERIFICATION ===\n");
                                                                #endif // EIGENDEBUG

eigenvaluesDone:
  if((--currentSolverNestingDepth) == 0) {
    clearSystemFlag(FLAG_SOLVING);
  }

}


/* Hold (A - lambda I) v to eps3 times the largest entry of v. The augmented system fixes unknowns to break a singular solve, and a choice that fixes the wrong
   ones meets the augmented rows while leaving the original ones unmet, so the vector it returns solves a different problem. LAPACK zlaein sets INFO instead,
   when inverse iteration does not reach its growth test. */
static bool_t solvesTheEigenProblem(const any34Matrix_t *matrix, bool_t isComplex, const cplx_t *v, const cplx_t *lambda, uint16_t size,
                                    const real_t *eps3, realContext_t *realContext) {
  uint16_t i, j;
  real_t mag, resid, vmax, bound;
  cplx_t acc, elem, prod;

  realSetZero(&resid);
  realSetZero(&vmax);
  for(i = 0; i < size; i++) {
    complexMagnitude(&v[i], &mag, realContext);
    if(realCompareGreaterThan(&mag, &vmax)) {
      realCopy(&mag, &vmax);
    }
  }
  if(realIsZero(&vmax)) {
    return false;
  }

  for(i = 0; i < size; i++) {
    realSetZero(&acc.Real);
    realSetZero(&acc.Imag);
    for(j = 0; j < size; j++) {
      if(isComplex) {
        real34ToReal(VARIABLE_REAL34_DATA(matrix->complexMatrix.matrixElements + i * size + j), &elem.Real);
        real34ToReal(VARIABLE_IMAG34_DATA(matrix->complexMatrix.matrixElements + i * size + j), &elem.Imag);
      }
      else {
        real34ToReal(matrix->realMatrix.matrixElements + i * size + j, &elem.Real);
        realSetZero(&elem.Imag);
      }
      if(i == j) {
        realSubtract(&elem.Real, &lambda->Real, &elem.Real, realContext);
        realSubtract(&elem.Imag, &lambda->Imag, &elem.Imag, realContext);
      }
      mulComplexComplex(&elem, &v[j], &prod, realContext);
      realAdd(&acc.Real, &prod.Real, &acc.Real, realContext);
      realAdd(&acc.Imag, &prod.Imag, &acc.Imag, realContext);
    }
    complexMagnitude(&acc, &mag, realContext);
    if(realCompareGreaterThan(&mag, &resid)) {
      realCopy(&mag, &resid);
    }
  }
  realMultiply(eps3, &vmax, &bound, realContext);
  return realCompareLessThan(&resid, &bound) || realCompareEqual(&resid, &bound);
}


/* Report whether v is in the span of the m orthonormal columns of basis. When it is not, its remainder, normalised, is written as column m of basis for the caller
   to keep. Eigenvectors of distinct eigenvalues are independent, so a vector in the span of the earlier columns, a multiple of one of them or a combination of
   several, leaves a rank deficient set. Projecting the columns out twice keeps them orthogonal to working precision (Giraud, Langou and Rozloznik, 2005), and the
   remainder is compared against v at 10^-(toleranceDigits/2): a defective multiple eigenvalue splits by about a root of the working precision, so the vectors of
   its copies agree to about half the digits. */
static bool_t isInSpanOfColumns(const cplx_t *v, cplx_t *basis, uint16_t size, uint16_t m, realContext_t *realContext) {
  cplx_t *w = basis + m * size;
  const cplx_t *b;
  uint16_t i, j, round;
  real_t t, vNorm, wNorm;
  cplx_t dot, bConj, prod;

  for(i = 0; i < size; i++) {
    realCopy(&v[i].Real, &w[i].Real);
    realCopy(&v[i].Imag, &w[i].Imag);
  }
  for(round = 0; round < 2; round++) {
    for(j = 0; j < m; j++) {
      b = basis + j * size;
      realSetZero(&dot.Real);
      realSetZero(&dot.Imag);
      for(i = 0; i < size; i++) {                                                                  // b^H w
        realCopy(&b[i].Real, &bConj.Real);
        realCopy(&b[i].Imag, &bConj.Imag);
        realChangeSign(&bConj.Imag);
        mulComplexComplex(&bConj, &w[i], &prod, realContext);
        realAdd(&dot.Real, &prod.Real, &dot.Real, realContext);
        realAdd(&dot.Imag, &prod.Imag, &dot.Imag, realContext);
      }
      for(i = 0; i < size; i++) {
        mulComplexComplex(&dot, &b[i], &prod, realContext);
        realSubtract(&w[i].Real, &prod.Real, &w[i].Real, realContext);
        realSubtract(&w[i].Imag, &prod.Imag, &w[i].Imag, realContext);
      }
    }
  }

  realSetZero(&vNorm);
  realSetZero(&wNorm);
  for(i = 0; i < size; i++) {
    realMultiply(&v[i].Real, &v[i].Real, &t, realContext);
    realAdd(&vNorm, &t, &vNorm, realContext);
    realMultiply(&w[i].Real, &w[i].Real, &t, realContext);
    realAdd(&wNorm, &t, &wNorm, realContext);
    realMultiply(&v[i].Imag, &v[i].Imag, &t, realContext);
    realAdd(&vNorm, &t, &vNorm, realContext);
    realMultiply(&w[i].Imag, &w[i].Imag, &t, realContext);
    realAdd(&wNorm, &t, &wNorm, realContext);
  }
  realSquareRoot(&vNorm, &vNorm, realContext);
  realSquareRoot(&wNorm, &wNorm, realContext);
  realCopy(&vNorm, &t);
  t.exponent -= (int32_t)(toleranceDigits) / 2;
  if(!realCompareGreaterThan(&wNorm, &t)) {
    return true;
  }
  for(i = 0; i < size; i++) {
    divComplexReal(&w[i], &wNorm, &w[i], realContext);
  }
  return false;
}


static bool_t isRepeatedEigenvalue(const cplx_t *u, const cplx_t *v, const real_t *eps3, realContext_t *realContext) {
  real_t re, im;                                                                   // LAPACK zhsein compares CABS1(w(i) - wk), the sum of the
  realSubtract(&u->Real, &v->Real, &re, realContext);                              // absolute parts, against EPS3 rather than testing equality
  realSubtract(&u->Imag, &v->Imag, &im, realContext);
  realCopyAbs(&re, &re);
  realCopyAbs(&im, &im);
  realAdd(&re, &im, &re, realContext);
  return realCompareLessThan(&re, eps3);
}


/* Reduce A - lambda I in b by Gaussian elimination with partial pivoting, the columns taken from the last to the first, and return its nullity. A column whose
   pivot candidates are all below 10^-(toleranceDigits/2) of the norm of A has no pivot: its index goes to freeCol, in ascending order, and each row left without a
   pivot goes to zeroRow. The pivots taken from the right leave the leading columns free, so fixing the unknowns of freeCol to the d-th unit vector, with a slack
   on each row of zeroRow, gives the null vector whose free coordinates are the d-th unit vector: sympy nullspace of A - lambda I with its columns reversed, read
   back in the original order. The Schur complement of that augmented system is -I, so it is not singular. Golub and Wilkinson (1976) take the SVD for the rank:
   a pivot misjudged here gives a vector that fails solvesTheEigenProblem, or a copy with no vector, and both are reported as a defective matrix. */
static uint16_t nullSpaceOfShiftedMatrix(const any34Matrix_t *matrix, bool_t isComplex, const cplx_t *lambda, uint16_t size, const real_t *eps3, cplx_t *b,
                                         uint16_t *freeCol, uint16_t *zeroRow, realContext_t *realContext) {
  uint16_t i, j, c, col, p, rank = 0, nullity = 0;
  real_t tol, best, mag;
  cplx_t fac, prod;

  for(i = 0; i < size; i++) {
    for(j = 0; j < size; j++) {
      if(isComplex) {
        real34ToReal(VARIABLE_REAL34_DATA(matrix->complexMatrix.matrixElements + i * size + j), &b[i * size + j].Real);
        real34ToReal(VARIABLE_IMAG34_DATA(matrix->complexMatrix.matrixElements + i * size + j), &b[i * size + j].Imag);
      }
      else {
        real34ToReal(matrix->realMatrix.matrixElements + i * size + j, &b[i * size + j].Real);
        realSetZero(&b[i * size + j].Imag);
      }
    }
    realSubtract(&b[i * size + i].Real, &lambda->Real, &b[i * size + i].Real, realContext);
    realSubtract(&b[i * size + i].Imag, &lambda->Imag, &b[i * size + i].Imag, realContext);
    zeroRow[i] = i;
  }
  realCopy(eps3, &tol);
  tol.exponent += 33 - (int32_t)(toleranceDigits) / 2;                                             // EPS3 is 1E-33 of the norm of A

  for(c = 0; c < size; c++) {
    col = size - 1 - c;
    realSetZero(&best);
    p = rank;
    for(i = rank; i < size; i++) {
      complexMagnitude(&b[i * size + col], &mag, realContext);
      if(realCompareGreaterThan(&mag, &best)) {
        realCopy(&mag, &best);
        p = i;
      }
    }
    if(!realCompareGreaterThan(&best, &tol)) {
      freeCol[nullity++] = col;
      continue;
    }
    if(p != rank) {
      for(j = 0; j <= col; j++) {
        realCopy(&b[p    * size + j].Real, &mag                    );
        realCopy(&b[rank * size + j].Real, &b[p    * size + j].Real);
        realCopy(&mag,                     &b[rank * size + j].Real);
        realCopy(&b[p    * size + j].Imag, &mag                    );
        realCopy(&b[rank * size + j].Imag, &b[p    * size + j].Imag);
        realCopy(&mag,                     &b[rank * size + j].Imag);
      }
      j = zeroRow[p];
      zeroRow[p] = zeroRow[rank];
      zeroRow[rank] = j;
    }
    for(i = rank + 1; i < size; i++) {
      divComplexComplex(&b[i * size + col], &b[rank * size + col], &fac, realContext);
      for(j = 0; j <= col; j++) {
        mulComplexComplex(&fac, &b[rank * size + j], &prod, realContext);
        realSubtract(&b[i * size + j].Real, &prod.Real, &b[i * size + j].Real, realContext);
        realSubtract(&b[i * size + j].Imag, &prod.Imag, &b[i * size + j].Imag, realContext);
      }
    }
    rank++;
  }

  for(i = 0; i < nullity; i++) {
    zeroRow[i] = zeroRow[rank + i];
  }
  for(i = 0; i < nullity / 2; i++) {
    j = freeCol[i];
    freeCol[i] = freeCol[nullity - 1 - i];
    freeCol[nullity - 1 - i] = j;
  }
  return nullity;
}


static void calculateEigenvectors(const any34Matrix_t *matrix, bool_t isComplex, cplx_t *a, cplx_t *q, cplx_t *r, cplx_t *eig, realContext_t *realContext) {
  // Call after the eigenvalues are calculated!
  const uint16_t size = matrix->header.matrixRows;
  uint16_t       i, j, k = 0;
  cplx_t         *v = NULL, *basis = NULL;
  uint16_t       basisSize = 0;
  uint16_t       freeUnknowns = 1;
  uint16_t       duplicateEigenvalueCount = 0;
  uint16_t       rep = 0, systemBuiltFor = 0;
  bool_t         rebuildSystem = true;
  bool_t         acceptedVector = false;
  real_t         eps3, rowSum;
  cplx_t         mag;
  uint16_t       *unknownsToFill = NULL, *slackRow = NULL;
  bool_t         repeated = false;
  bool_t         pairedSlack = false;

  if(matrix->header.matrixRows == matrix->header.matrixColumns) {
    for(i = 0; i < size * size; i++) {
      realSetZero(&r[i].Real);
      realSetZero(&r[i].Imag);
    }
    // The digits past the 34th are residue of the iteration. Rounding them away makes an exact eigenvalue exact, so dependent rows of A - lambda I cancel in the solve.
    dropNoise(eig, size, 34);

    // Fast path: diagonal matrix has trivial unit eigenvectors permuted to match the eigenvalue ordering. The augmented-system technique below
    // fails on diagonal A because (A - lambda*I) has at least one entirely zero row for any eigenvalue, making the augmented matrix singular.
    bool_t isDiagonal = true;
    for(i = 0; i < size && isDiagonal; ++i) {
      for(j = 0; j < size && isDiagonal; ++j) {
        if(i != j) {
          if(isComplex) {
            if(!real34IsZero(VARIABLE_REAL34_DATA(matrix->complexMatrix.matrixElements + i * size + j)) ||
               !real34IsZero(VARIABLE_IMAG34_DATA(matrix->complexMatrix.matrixElements + i * size + j))) {
              isDiagonal = false;
            }
          }
          else {
            if(!real34IsZero(matrix->realMatrix.matrixElements + i * size + j)) {
              isDiagonal = false;
            }
          }
        }
      }
    }
    if(isDiagonal) {
      // For each eigenvalue eig[k], find the first unclaimed diagonal position j where A[j][j] == eig[k], then set r[j][k] = 1. Repeated
      // eigenvalues land in distinct columns; the "unclaimed" check uses r itself (zero-initialized above, becomes nonzero where claimed).
      for(k = 0; k < size; ++k) {
        for(j = 0; j < size; ++j) {
          bool_t alreadyUsed = false;
          for(uint16_t kPrev = 0; kPrev < k; ++kPrev) {
            if(!realIsZero(&r[j * size + kPrev].Real)) {
              alreadyUsed = true;
              break;
            }
          }
          if(alreadyUsed) {
            continue;
          }
          // Tolerance-based match: |eig[k] - A[j][j]| <= max(|eig|, |A[j][j]|, 1) * 1e-30.
          // Exact equality fails when QR produces tiny numerical residuals (e.g. eigenvalue 0 emerges as ~1e-37) while diagonal entries are exact zeros from the input.
          real_t diff_r, diff_i;
          cplx_t diag;
          real_t mag_eig, mag_diag, scale, tol;
          if(isComplex) {
            real34ToReal(VARIABLE_REAL34_DATA(matrix->complexMatrix.matrixElements + j * size + j), &diag.Real);
            real34ToReal(VARIABLE_IMAG34_DATA(matrix->complexMatrix.matrixElements + j * size + j), &diag.Imag);
          }
          else {
            real34ToReal(matrix->realMatrix.matrixElements + j * size + j, &diag.Real);
            realSetZero(&diag.Imag);
          }
          realSubtract(&eig[k * size + k].Real, &diag.Real, &diff_r, realContext);
          realSubtract(&eig[k * size + k].Imag, &diag.Imag, &diff_i, realContext);
          complexMagnitude(&eig[k * size + k], &mag_eig, realContext);
          complexMagnitude(&diag, &mag_diag, realContext);
          realCopy(&mag_eig, &scale);
          if(realCompareLessThan(&scale, &mag_diag)) {
            realCopy(&mag_diag, &scale);
          }
          if(realCompareLessThan(&scale, const_1)) {
            realCopy(const_1, &scale);
          }
          realMultiply(&scale, const_1e_30, &tol, realContext);
          bool_t matches = isElementWithinTolerance(&diff_r, &diff_i, &tol, realContext);
          if(matches) {
            realCopy(const_1, &r[j * size + k].Real);
            break;
          }
        }
      }
      return;
    }

    // An eigenvalue within EPS3 = ULP * norm(A) of an earlier one is the same eigenvalue, as in LAPACK zhsein, which perturbs
    // such a root by EPS3 before inverse iteration and compares every earlier eigenvalue rather than the previous one alone.
    // The eigenvalues are rounded to 34 digits above, so ULP is 1E-33. A defective multiple eigenvalue leaves the QR iteration
    // split by about the square root of the working precision, 1E-38 at 75 digits: equality misses the repeat, and each copy
    // then solves a system of its own and returns the same eigenvector again.
    realSetZero(&eps3);
    for(i = 0; i < size; i++) {
      realSetZero(&rowSum);
      for(j = 0; j < size; j++) {
        if(isComplex) {
          real34ToReal(VARIABLE_REAL34_DATA(matrix->complexMatrix.matrixElements + i * size + j), &mag.Real);
          real34ToReal(VARIABLE_IMAG34_DATA(matrix->complexMatrix.matrixElements + i * size + j), &mag.Imag);
          complexMagnitude(&mag, &mag.Real, realContext);
        }
        else {
          real34ToReal(matrix->realMatrix.matrixElements + i * size + j, &mag.Real);
          realCopyAbs(&mag.Real, &mag.Real);
        }
        realAdd(&rowSum, &mag.Real, &rowSum, realContext);
      }
      if(realCompareGreaterThan(&rowSum, &eps3)) {
        realCopy(&rowSum, &eps3);
      }
    }
    if(realIsZero(&eps3)) {
      realSetOne(&eps3);                                                                          // zhsein takes SMLNUM when the norm is zero
    }
    eps3.exponent -= 33;

    if((unknownsToFill = allocC47Blocks(size * 2 * REAL_SIZE_IN_BLOCKS(75) * 2))) {
      slackRow = unknownsToFill + size;
      basis = q + size * size * 2;                                                               // the right-hand side and the elimination use the first half of q
      for(k = 0; k < size && lastErrorCode != ERROR_RAM_FULL; k++) {                              // a full RAM stops the remaining columns
        rep = k;
        duplicateEigenvalueCount = 0;
        repeated = false;
        for(j = 0; j < size; j++) {
          if(j != k && isRepeatedEigenvalue(&eig[j * size + j], &eig[k * size + k], &eps3, realContext)) {
            repeated = true;
            if(j < k) {
              if(duplicateEigenvalueCount == 0) {
                rep = j;                                                                          // the first copy of this eigenvalue: its system is the one to solve again
              }
              ++duplicateEigenvalueCount;
            }
          }
        }
        rebuildSystem = (duplicateEigenvalueCount == 0) || (rep != systemBuiltFor);
        if(rebuildSystem) {
          systemBuiltFor = rep;
          pairedSlack = false;
          if(repeated) {                                                                          // copy d of a repeated eigenvalue takes the d-th null vector
            freeUnknowns = nullSpaceOfShiftedMatrix(matrix, isComplex, &eig[rep * size + rep], size, &eps3, q, unknownsToFill, slackRow, realContext);
          }
          else {
            freeUnknowns = 1;
            unknownsToFill[0] = 0;
          }
        }
        acceptedVector = false;
        if((v = allocC47Blocks(size * 2 * REAL_SIZE_IN_BLOCKS(75) * 2))) {
          while(duplicateEigenvalueCount < freeUnknowns) {                                        // a copy beyond the nullity has no eigenvector of its own
            for(j = 0; j < size * 2; j++) {
              realSetNaN(&v[j].Real);
              realSetNaN(&v[j].Imag);
            }
            if(!repeated) {
              slackRow[0] = pairedSlack ? unknownsToFill[0] : 0;
            }

            if(rebuildSystem) {
              // Restore the original matrix
              for(i = 0; i < size; i++) {
                if(isComplex) {
                  for(j = 0; j < size; j++) {
                    real34ToReal(VARIABLE_REAL34_DATA(matrix->complexMatrix.matrixElements + i * size + j), &a[i * (size + freeUnknowns) + j].Real    );
                    real34ToReal(VARIABLE_IMAG34_DATA(matrix->complexMatrix.matrixElements + i * size + j), &a[i * (size + freeUnknowns) + j].Imag);
                  }
                }
                else {
                  for(j = 0; j < size; j++) {
                    real34ToReal(matrix->realMatrix.matrixElements + i * size + j, &a[i * (size + freeUnknowns) + j].Real);
                    realSetZero(&a[i * (size + freeUnknowns) + j].Imag);
                  }
                }
                for(j = 0; j < freeUnknowns; j++) {
                  realCopy(slackRow[j] == i ? const_1 : const_0, &a[i * (size + freeUnknowns) + j + size].Real);
                  realSetZero(&a[i * (size + freeUnknowns) + j + size].Imag);
                }
              }
              for(i = 0; i < freeUnknowns; i++) {
                for(j = 0; j < size; j++) {
                  realCopy(j == unknownsToFill[i] ? const_1 : const_0, &a[(i + size) * (size + freeUnknowns) + j].Real);
                  realSetZero(&a[(i + size) * (size + freeUnknowns) + j].Imag);
                }
                for(j = 0; j < freeUnknowns; j++) {
                  realCopy(j == i ? const_1 : const_0, &a[(i + size) * (size + freeUnknowns) + j + size].Real);
                  realSetZero(&a[(i + size) * (size + freeUnknowns) + j + size].Imag);
                }
              }

              // Subtract an eigenvalue
              for(j = 0; j < size; j++) {
                realSubtract(&a[j * (size + freeUnknowns) + j].Real, &eig[rep * size + rep].Real, &a[j * (size + freeUnknowns) + j].Real, realContext);
                realSubtract(&a[j * (size + freeUnknowns) + j].Imag, &eig[rep * size + rep].Imag, &a[j * (size + freeUnknowns) + j].Imag, realContext);
              }
            }
            if(duplicateEigenvalueCount != 0) {
              rebuildSystem = false;                                                              // a further copy solves the system the first copy built, for the next null-space vector
            }

            // Make the equation matrices
            for(j = 0; j < size + freeUnknowns; j++) {
              realCopy(j == duplicateEigenvalueCount + size ? const_1 : const_0, &q[j].Real    );
              realSetZero(                                                &q[j].Imag);
            }

            // Solve linear equations from the submatrix
            lastErrorCode = ERROR_NONE;
            cpxLinearEqn(a, q, v, size + freeUnknowns, 2 - 34, realContext);                    // a 34-digit eigenvalue is off by up to half a unit in its 34th digit
            if(lastErrorCode != ERROR_SINGULAR_MATRIX) {
              bool_t usable = solvesTheEigenProblem(matrix, isComplex, v, &eig[k * size + k], size, &eps3, realContext);
              if(usable) {
                usable = !isInSpanOfColumns(v, basis, size, basisSize, realContext);
              }
              if(usable) {
                ++basisSize;                                                                      // isInSpanOfColumns wrote the remainder of v as this column of basis
                acceptedVector = true;
                break;
              }
            }

            // Build the matrix again on every retry: unknownsToFill moves the fixed unknowns, pairedSlack moves the slack and freeUnknowns is the row stride,
            // so a retry that keeps the old matrix reads elements never written at the new stride.
            rebuildSystem = true;
            if(repeated) {
              break;                                                                              // the null space gives the only choice of unknowns and slacks
            }

            // Next choice for a simple eigenvalue: the fixed unknown moves down the rows with its slack on the first row, then again with the slack on its own
            // row. One of the latter is not singular: adj(A - lambda I) is c x y^T with y^T x not 0, so some row u has x_u y_u not 0. Two fixed unknowns would
            // only reach the same null vector again.
            if(++unknownsToFill[0] == size) {
              if(pairedSlack) {
                break;
              }
              pairedSlack = true;
              unknownsToFill[0] = 0;
            }
          }
          if(!acceptedVector) {
            // Zero-fill on failure. The caller (realEigenvectors / complexEigenvectors) detects
            // zero columns and reports the defective-matrix error to the user.
            for(i = 0; i < size; i++) {
              realSetZero(&v[i].Real    );
              realSetZero(&v[i].Imag);
            }
            if(lastErrorCode == ERROR_SINGULAR_MATRIX) {                                          // Clear only the singular solve the retries expect; any other
              lastErrorCode = ERROR_NONE;                                                         // error reaches the caller, which reports it
            }
          }
          for(i = 0; i < size; i++) {
            realCopy(&v[i].Real, &r[i * size + k].Real    );
            realCopy(&v[i].Imag, &r[i * size + k].Imag);
          }

          freeC47Blocks(v, size * 2 * REAL_SIZE_IN_BLOCKS(75) * 2);
          v = NULL;
        }
        else {
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "Ram full, 1av");
            moreInfoOnError("In function calculateEigenvectors:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          for(i = 0; i < size; i++) {
            realSetNaN(&r[i * size + k].Real    );
            realSetNaN(&r[i * size + k].Imag);
          }
        }
      }
      freeC47Blocks(unknownsToFill, size * 2 * REAL_SIZE_IN_BLOCKS(75) * 2);
      unknownsToFill = NULL;
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 2aw");
        moreInfoOnError("In function calculateEigenvectors:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      for(i = 0; i < size; i++) {
        realSetNaN(&r[i * size + k].Real    );
        realSetNaN(&r[i * size + k].Imag);
      }
    }
  }
}


static void realEigenvalues(const real34Matrix_t *matrix, real34Matrix_t *res, real34Matrix_t *ires) {
  const uint16_t size = matrix->header.matrixRows;
  cplx_t *bulk, *a, *scratch, *eig;
  uint16_t i;
  bool_t isComplex;
  bool_t shifted = true;
  size_t bulkSize = (size_t) REAL_SIZE_IN_BLOCKS(75) * (size * size * 2 * 2 + size * 7);

  if(matrix->header.matrixRows == matrix->header.matrixColumns) {
    if((bulk = allocC47Blocks(bulkSize))) {
      a       = bulk;
      eig     = bulk + size * size;
      scratch = bulk + size * size * 2;

      // Convert real34 to real
      for(i = 0; i < size * size; i++) {
        real34ToReal(matrix->matrixElements + i, &a[i].Real);
        realSetZero(&a[i].Imag);
      }

      // Calculate
      calculateEigenvalues(a, scratch, eig, size, shifted, true, eigenContext);
      shifted = false;

      // Check imaginary part (mutually conjugate complex roots are possible in real quadratic equations)
      isComplex = false;
      for(i = 0; i < size; i++) {
        if(!realIsZero(&eig[i * size + i].Imag)) {
          isComplex = true;
          break;
        }
      }

      // Write back
      if(matrix == res || realMatrixInit(res, size, size)) {
        for(i = 0; i < size; i++) {
          realToReal34(&eig[i * size + i].Real, res->matrixElements + (i * size + i));
        }
        if(isComplex && (ires != NULL)) {
          if(matrix == ires || res == ires || realMatrixInit(ires, size, size)) {
            for(i = 0; i < size; i++) {
              realToReal34(&eig[i * size + i].Imag, ires->matrixElements + (i * size + i));
            }
          }
          else {
            displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Ram full, 1ax");
              moreInfoOnError("In function realEigenvalues:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
        }
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 2ay");
          moreInfoOnError("In function realEigenvalues:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }

      freeC47Blocks(bulk, bulkSize);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 3az");
        moreInfoOnError("In function realEigenvalues:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
}


static void complexEigenvalues(const complex34Matrix_t *matrix, complex34Matrix_t *res) {
  const uint16_t size = matrix->header.matrixRows;
  cplx_t *bulk, *a, *scratch, *eig;
  uint16_t i;
  bool_t shifted = true;
  size_t bulkSize = (size_t)REAL_SIZE_IN_BLOCKS(75) * (size * size * 2 * 2 + size * 7);

  if(matrix->header.matrixRows == matrix->header.matrixColumns) {
    if((bulk = allocC47Blocks(bulkSize))) {
      a       = bulk;
      eig     = bulk + size * size;
      scratch = bulk + size * size * 2;

      // Convert real34 to real
      for(i = 0; i < size * size; i++) {
        real34ToReal(VARIABLE_REAL34_DATA(matrix->matrixElements + i), &a[i].Real    );
        real34ToReal(VARIABLE_IMAG34_DATA(matrix->matrixElements + i), &a[i].Imag);
      }

      // Calculate
      calculateEigenvalues(a, scratch, eig, size, shifted, true, eigenContext);
      shifted = false;

      // Write back
      if(matrix == res || complexMatrixInit(res, size, size)) {
        for(i = 0; i < size; i++) {
          realToReal34(&eig[i * size + i].Real, VARIABLE_REAL34_DATA(res->matrixElements + (i * size + i)));
          realToReal34(&eig[i * size + i].Imag, VARIABLE_IMAG34_DATA(res->matrixElements + (i * size + i)));
        }
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 1ba");
          moreInfoOnError("In function complexEigenvalues:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }

      freeC47Blocks(bulk, bulkSize);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 2bb");
        moreInfoOnError("In function complexEigenvalues:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
}


/**
 * normaliseEigenvectorColumns - scale every real or complex column to Euclidean unit norm.
 */
static void normaliseEigenvectorColumns(cplx_t *vectors, uint16_t size, realContext_t *realContext) {
  for(uint16_t column = 0; column < size; column++) {
    real_t sum;
    realSetZero(&sum);
    for(uint16_t row = 0; row < size; row++) {
      real_t term;
      realFMA(&vectors[row * size + column].Real, &vectors[row * size + column].Real, &sum, &term, realContext);
      realCopy(&term, &sum);
      realFMA(&vectors[row * size + column].Imag, &vectors[row * size + column].Imag, &sum, &term, realContext);
      realCopy(&term, &sum);
    }
    realSquareRoot(&sum, &sum, realContext);
    if(!realIsZero(&sum) && !realIsSpecial(&sum)) {
      for(uint16_t row = 0; row < size; row++) {
        realDivide(&vectors[row * size + column].Real, &sum, &vectors[row * size + column].Real, realContext);
        realDivide(&vectors[row * size + column].Imag, &sum, &vectors[row * size + column].Imag, realContext);
      }
    }
  }
}


static void realEigenvectors(const real34Matrix_t *matrix, real34Matrix_t *res, real34Matrix_t *ires) {
  const uint16_t size = matrix->header.matrixRows;
  cplx_t *bulk, *a, *q, *r, *scratch, *eig;
  uint16_t i, j;
  bool_t isComplex;
  bool_t shifted = true;
  size_t bulkSize = (size_t)REAL_SIZE_IN_BLOCKS(75) * (size * size * 4 * 2 * 4 + size * 7);

  if(matrix->header.matrixRows == matrix->header.matrixColumns) {
    if((bulk = allocC47Blocks(bulkSize))) {
      a   = bulk;
      q   = bulk + size * size * 4;
      r   = bulk + size * size * 4 * 2;
      eig = bulk + size * size * 4 * 3;
      scratch = bulk + size * size * 4 * 4;

      // Convert real34 to real
      for(i = 0; i < size * size; i++) {
        real34ToReal(matrix->matrixElements + i, &a[i].Real);
        realSetZero(&a[i].Imag);
      }

      // Calculate eigenvalues
      calculateEigenvalues(a, scratch, eig, size, shifted, false, eigenContext);
      shifted = false;
      if(lastErrorCode == ERROR_NO_ROOT_FOUND) {                                                  // the eigenvalues did not converge, so no eigenvectors are returned
        goto fail;
      }
      calculateEigenvectors((any34Matrix_t *)matrix, false, a, q, r, eig, eigenContext);
      if(lastErrorCode == ERROR_RAM_FULL) {                                                       // RAM is full, so no eigenvectors are returned
        goto fail;
      }

      // Detect failure: calculateEigenvectors zero-fills any column where it could not find a linearly independent eigenvector (defective matrix).
      // A genuine eigenvector is never zero, so a zero column unambiguously signals failure.
      for(j = 0; j < size; j++) {
        bool_t allZero = true;
        for(i = 0; i < size; i++) {
          if(!realIsZero(&r[i * size + j].Real) || !realIsZero(&r[i * size + j].Imag)) {
            allZero = false;
            break;
          }
        }
        if(allZero) {
          // No linearly independent eigenvector for this column. Caller checks
          // res.matrixElements; fnEigenvectors raises ERROR_SINGULAR_MATRIX,
          // fnMatrixSquareRoot's eigen path falls back silently.
          goto fail;
        }
      }

      // Check imaginary part (mutually conjugate complex roots are possible in real quadratic equations)
      isComplex = false;
      for(i = 0; i < size * size; i++) {
        if(!realIsZero(&r[i].Imag)) {
          isComplex = true;
          break;
        }
      }

      normaliseEigenvectorColumns(r, size, eigenContext);

      // Write back
      if(matrix == res || realMatrixInit(res, size, size)) {
        for(i = 0; i < size * size; i++) {
          realToReal34(&r[i].Real, res->matrixElements + i);
        }
        if(isComplex && (ires != NULL)) {
          if(matrix == ires || res == ires || realMatrixInit(ires, size, size)) {
            for(i = 0; i < size * size; i++) {
              realToReal34(&r[i].Imag, ires->matrixElements + i);
            }
          }
          else {
            displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Ram full, 1bc");
              moreInfoOnError("In function realEigenvectors:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            if(matrix != res) {
              realMatrixFree(res);
            }
          }
        }
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 2be");
          moreInfoOnError("In function realEigenvectors:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }

      freeC47Blocks(bulk, bulkSize);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 3bf");
        moreInfoOnError("In function realEigenvectors:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto fail;
    }
  }
  return;

fail:
  res->matrixElements = NULL;
  res->header.matrixRows = 0;
  res->header.matrixColumns = 0;
  freeC47Blocks(bulk, bulkSize);
}


static void complexEigenvectors(const complex34Matrix_t *matrix, complex34Matrix_t *res) {
  const uint16_t size = matrix->header.matrixRows;
  cplx_t *bulk, *a, *q, *r, *scratch, *eig;
  uint16_t i, j;
  bool_t shifted = true;
  size_t bulkSize = (size_t)REAL_SIZE_IN_BLOCKS(75) * (size * size * 4 * 2 * 4 + size * 7);

  if(matrix->header.matrixRows == matrix->header.matrixColumns) {
    if((bulk = allocC47Blocks(bulkSize))) {
      a   = bulk;
      q   = bulk + size * size * 4;
      r   = bulk + size * size * 4 * 2;
      eig = bulk + size * size * 4 * 3;
      scratch = bulk + size * size * 4 * 4;

      // Convert real34 to real
      for(i = 0; i < size * size; i++) {
        real34ToReal(VARIABLE_REAL34_DATA(matrix->matrixElements + i), &a[i].Real    );
        real34ToReal(VARIABLE_IMAG34_DATA(matrix->matrixElements + i), &a[i].Imag);
      }

      // Calculate eigenvalues
      calculateEigenvalues(a, scratch, eig, size, shifted, false, eigenContext);
      shifted = false;
      if(lastErrorCode == ERROR_NO_ROOT_FOUND) {                                                  // the eigenvalues did not converge, so no eigenvectors are returned
        goto fail;
      }
      calculateEigenvectors((any34Matrix_t *)matrix, true, a, q, r, eig, eigenContext);
      if(lastErrorCode == ERROR_RAM_FULL) {                                                       // RAM is full, so no eigenvectors are returned
        goto fail;
      }

      // Detect failure: calculateEigenvectors zero-fills any column where it could not find a linearly independent eigenvector (defective matrix).
      // A genuine eigenvector is never zero, so a zero column unambiguously signals failure.
      for(j = 0; j < size; j++) {
        bool_t allZero = true;
        for(i = 0; i < size; i++) {
          if(!realIsZero(&r[i * size + j].Real) || !realIsZero(&r[i * size + j].Imag)) {
            allZero = false;
            break;
          }
        }
        if(allZero) {
          goto fail;
        }
      }

      normaliseEigenvectorColumns(r, size, eigenContext);

      // Write back
      if(matrix == res || complexMatrixInit(res, size, size)) {
        for(i = 0; i < size * size; i++) {
          realToReal34(&r[i].Real, VARIABLE_REAL34_DATA(res->matrixElements + i));
          realToReal34(&r[i].Imag, VARIABLE_IMAG34_DATA(res->matrixElements + i));
        }
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Ram full, 1");
          moreInfoOnError("In function complexEigenvectors:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }

      freeC47Blocks(bulk, bulkSize);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ram full, 2bg");
        moreInfoOnError("In function complexEigenvectors:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto fail;
    }
  }
  return;

fail:
  res->matrixElements = NULL;
  res->header.matrixRows = 0;
  res->header.matrixColumns = 0;
  freeC47Blocks(bulk, bulkSize);
}


#endif // OPTION_EIGEN


static void elementwiseRemaGetResult(bool_t *complex, real34Matrix_t *x, complex34Matrix_t *xc, int i) {
  cplx_t value;
  bool_t isComplex = false;

  getRegisterAsComplexOrReal(REGISTER_X, &value, &isComplex);
  if(isComplex || *complex) {
    if(!*complex) {
      convertReal34MatrixToComplex34Matrix(x, xc);
      if(lastErrorCode != ERROR_RAM_FULL) {
        realMatrixFree(x);
        *complex = true;
      }
    }
    if(*complex) { // promotion may have failed with RAM_FULL; only write xc when it actually exists
      realToReal34(&value.Real, VARIABLE_REAL34_DATA(xc->matrixElements + i));
      realToReal34(&value.Imag, VARIABLE_IMAG34_DATA(xc->matrixElements + i));
    }
  }
  else {
    realToReal34(&value.Real, x->matrixElements + i);
  }
}


/* Elementwise function call */
void elementwiseRema(void (*f)(void)) {
  real34Matrix_t x;
  complex34Matrix_t xc;
  bool_t complex = false;

  convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &x);
  const unsigned int numOfElements = x.header.matrixRows * x.header.matrixColumns;

  for(unsigned int i = 0; i < numOfElements; ++i) {
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    if(complex) {
      real34Copy(VARIABLE_REAL34_DATA(xc.matrixElements + i), REGISTER_REAL34_DATA(REGISTER_X));
    }
    else {
      real34Copy(x.matrixElements + i, REGISTER_REAL34_DATA(REGISTER_X));
    }
    f();
    elementwiseRemaGetResult(&complex, &x, &xc, i);
  }

  if(complex) {
    convertComplex34MatrixToComplex34MatrixRegister(&xc, REGISTER_X);
    complexMatrixFree(&xc);
  }
  else {
    convertReal34MatrixToReal34MatrixRegister(&x, REGISTER_X);
    realMatrixFree(&x);
  }
}


void elementwiseRema_UInt16(void (*f)(uint16_t), uint16_t param) {
  real34Matrix_t x;
  complex34Matrix_t xc;
  bool_t complex = false;

  convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &x);
  const unsigned int numOfElements = x.header.matrixRows * x.header.matrixColumns;

  for(unsigned int i = 0; i < numOfElements; ++i) {
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    if(complex) {
      real34Copy(VARIABLE_REAL34_DATA(xc.matrixElements + i), REGISTER_REAL34_DATA(REGISTER_X));
    }
    else {
      real34Copy(x.matrixElements + i, REGISTER_REAL34_DATA(REGISTER_X));
    }
    f(param);
    elementwiseRemaGetResult(&complex, &x, &xc, i);
  }

  if(complex) {
    convertComplex34MatrixToComplex34MatrixRegister(&xc, REGISTER_X);
    complexMatrixFree(&xc);
  }
  else {
    convertReal34MatrixToReal34MatrixRegister(&x, REGISTER_X);
    realMatrixFree(&x);
  }
}


// Every elementwise routine with a real matrix on one side and a scalar on the other walks the matrix, putting one element in the matrix register and the scalar
// back in the other, running f and collecting the result. They differed only in how each one took the scalar aside and put it back, and one register snapshot
// serves every scalar type, so there is one body. matrixReg names the register holding the matrix.
static void elementwiseRemaScalar(void (*f)(void), calcRegister_t matrixReg) {
  const calcRegister_t scalarReg = (matrixReg == REGISTER_X) ? REGISTER_Y : REGISTER_X;
  real34Matrix_t m;
  complex34Matrix_t mc;
  snap_t scalar;
  bool_t complex = false;

  saveRegisterSnapshot(scalarReg, &scalar);
  if(scalar.t == dtReal34 || scalar.t == dtComplex34) {
    scalar.tag = amNone;                                                    // a real or complex scalar goes back untagged, so an angular value is taken by its value alone
  }
  convertReal34MatrixRegisterToReal34Matrix(matrixReg, &m);

  const unsigned int numOfElements = m.header.matrixRows * m.header.matrixColumns;

  for(unsigned int i = 0; i < numOfElements; ++i) {
    reallocateRegister(matrixReg, dtReal34, 0, amNone);
    if(complex) {
      real34Copy(VARIABLE_REAL34_DATA(mc.matrixElements + i), REGISTER_REAL34_DATA(matrixReg));
    }
    else {
      real34Copy(m.matrixElements + i, REGISTER_REAL34_DATA(matrixReg));
    }
    applyRegisterSnapshot(scalarReg, &scalar);
    f();
    elementwiseRemaGetResult(&complex, &m, &mc, i);
  }

  freeRegisterSnapshot(&scalar);

  if(complex) {
    convertComplex34MatrixToComplex34MatrixRegister(&mc, REGISTER_X);
    complexMatrixFree(&mc);
  }
  else {
    convertReal34MatrixToReal34MatrixRegister(&m, REGISTER_X);
    realMatrixFree(&m);
  }
}


void elementwiseRemaLonI(void (*f)(void)) {
  elementwiseRemaScalar(f, REGISTER_Y);
}


void elementwiseRemaReal(void (*f)(void)) {
  elementwiseRemaScalar(f, REGISTER_Y);
}


void elementwiseRemaShoI(void (*f)(void)) {
  elementwiseRemaScalar(f, REGISTER_Y);
}


void elementwiseRealRema(void (*f)(void)) {
  elementwiseRemaScalar(f, REGISTER_X);
}

void elementwiseRemaRema(void (*f)(void)) {
  real34Matrix_t x, y;
  complex34Matrix_t xc;
  bool_t complex = false;

  convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &x);
  convertReal34MatrixRegisterToReal34Matrix(REGISTER_Y, &y);
  if(x.header.matrixRows != y.header.matrixRows || x.header.matrixColumns != y.header.matrixColumns) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    return;
  }

  const unsigned int numOfElements = x.header.matrixRows * x.header.matrixColumns;
  for(unsigned int i = 0; i < numOfElements; ++i) {
    reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    real34Copy(y.matrixElements + i, REGISTER_REAL34_DATA(REGISTER_Y));
    if(complex) {
      real34Copy(VARIABLE_REAL34_DATA(xc.matrixElements + i), REGISTER_REAL34_DATA(REGISTER_X));
    }
    else {
      real34Copy(x.matrixElements + i, REGISTER_REAL34_DATA(REGISTER_X));
    }
    f();
    elementwiseRemaGetResult(&complex, &x, &xc, i);
  }

  realMatrixFree(&y);
  if(complex) {
    convertComplex34MatrixToComplex34MatrixRegister(&xc, REGISTER_X);
    complexMatrixFree(&xc);
  }
  else {
    convertReal34MatrixToReal34MatrixRegister(&x, REGISTER_X);
    realMatrixFree(&x);
  }
}


static void elementwiseCxmaGetResult(complex34Matrix_t *x, int i) {
  cplx_t value;

  getRegisterAsComplex(REGISTER_X, &value);
  realToReal34(&value.Real, VARIABLE_REAL34_DATA(x->matrixElements + i));
  realToReal34(&value.Imag, VARIABLE_IMAG34_DATA(x->matrixElements + i));
}


void elementwiseCxma(void (*f)(void)) {
  complex34Matrix_t x;

  convertComplex34MatrixRegisterToComplex34Matrix(REGISTER_X, &x);

  for(int i = 0; i < x.header.matrixRows * x.header.matrixColumns; ++i) {
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    complex34Copy(x.matrixElements + i, REGISTER_COMPLEX34_DATA(REGISTER_X));
    f();
    elementwiseCxmaGetResult(&x, i);
  }

  convertComplex34MatrixToComplex34MatrixRegister(&x, REGISTER_X);

  complexMatrixFree(&x);
}


void elementwiseCxma_UInt16(void (*f)(uint16_t), uint16_t param) {
  complex34Matrix_t x;

  convertComplex34MatrixRegisterToComplex34Matrix(REGISTER_X, &x);

  const unsigned int numOfElements = x.header.matrixRows * x.header.matrixColumns;
  for(unsigned int i = 0; i < numOfElements; ++i) {
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    complex34Copy(x.matrixElements + i, REGISTER_COMPLEX34_DATA(REGISTER_X));
    f(param);
    elementwiseCxmaGetResult(&x, i);
  }

  convertComplex34MatrixToComplex34MatrixRegister(&x, REGISTER_X);

  complexMatrixFree(&x);
}


// The complex matrix counterpart. fromReal says the matrix register holds a real matrix taken up as a complex one, which is how a real matrix meets a complex
// scalar.
static void elementwiseCxmaScalar(void (*f)(void), calcRegister_t matrixReg, bool_t fromReal) {
  const calcRegister_t scalarReg = (matrixReg == REGISTER_X) ? REGISTER_Y : REGISTER_X;
  complex34Matrix_t m;
  snap_t scalar;

  saveRegisterSnapshot(scalarReg, &scalar);
  if(scalar.t == dtReal34 || scalar.t == dtComplex34) {
    scalar.tag = amNone;                                                    // a real or complex scalar goes back untagged, so an angular value is taken by its value alone
  }
  if(fromReal) {
    convertReal34MatrixRegisterToComplex34Matrix(matrixReg, &m);
  }
  else {
    convertComplex34MatrixRegisterToComplex34Matrix(matrixReg, &m);
  }

  const unsigned int numOfElements = m.header.matrixRows * m.header.matrixColumns;
  for(unsigned int i = 0; i < numOfElements; ++i) {
    reallocateRegister(matrixReg, dtComplex34, 0, amNone);
    complex34Copy(m.matrixElements + i, REGISTER_COMPLEX34_DATA(matrixReg));
    applyRegisterSnapshot(scalarReg, &scalar);
    f();
    elementwiseCxmaGetResult(&m, i);
  }

  freeRegisterSnapshot(&scalar);

  convertComplex34MatrixToComplex34MatrixRegister(&m, REGISTER_X);
  complexMatrixFree(&m);
}


void elementwiseCxmaLonI(void (*f)(void)) {
  elementwiseCxmaScalar(f, REGISTER_Y, false);
}


void elementwiseCxmaReal(void (*f)(void)) {
  elementwiseCxmaScalar(f, REGISTER_Y, false);
}


void elementwiseCxmaShoI(void (*f)(void)) {
  elementwiseCxmaScalar(f, REGISTER_Y, false);
}


void elementwiseCxmaCplx(void (*f)(void)) {
  elementwiseCxmaScalar(f, REGISTER_Y, false);
}


void elementwiseRealCxma(void (*f)(void)) {
  elementwiseCxmaScalar(f, REGISTER_X, false);
}


void elementwiseCplxCxma(void (*f)(void)) {
  elementwiseCxmaScalar(f, REGISTER_X, false);
}


void elementwiseCplxRema(void (*f)(void)) {
  elementwiseCxmaScalar(f, REGISTER_X, true);
}


void elementwiseRemaCplx(void (*f)(void)) {
  elementwiseCxmaScalar(f, REGISTER_Y, true);
}


void elementwiseCxmaRema(void (*f)(void)) {
  real34Matrix_t x;
  complex34Matrix_t y;

  convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &x);
  convertComplex34MatrixRegisterToComplex34Matrix(REGISTER_Y, &y);
  if(x.header.matrixRows != y.header.matrixRows || x.header.matrixColumns != y.header.matrixColumns) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    return;
  }

  const unsigned int numOfElements = x.header.matrixRows * x.header.matrixColumns;
  for(unsigned int i = 0; i < numOfElements; ++i) {
    reallocateRegister(REGISTER_Y, dtComplex34, 0, amNone);
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    complex34Copy(y.matrixElements + i, REGISTER_COMPLEX34_DATA(REGISTER_Y));
    real34Copy(x.matrixElements + i, REGISTER_REAL34_DATA(REGISTER_X));
    f();
    elementwiseCxmaGetResult(&y, i);
  }

  realMatrixFree(&x);
  convertComplex34MatrixToComplex34MatrixRegister(&y, REGISTER_X);
  complexMatrixFree(&y);
}


void elementwiseRemaCxma(void (*f)(void)) {
  real34Matrix_t y;
  complex34Matrix_t x;

  convertReal34MatrixRegisterToReal34Matrix(REGISTER_Y, &y);
  convertComplex34MatrixRegisterToComplex34Matrix(REGISTER_X, &x);
  if(x.header.matrixRows != y.header.matrixRows || x.header.matrixColumns != y.header.matrixColumns) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    return;
  }

  const unsigned int numOfElements = x.header.matrixRows * x.header.matrixColumns;
  for(unsigned int i = 0; i < numOfElements; ++i) {
    reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    real34Copy(y.matrixElements + i, REGISTER_REAL34_DATA(REGISTER_Y));
    complex34Copy(x.matrixElements + i, REGISTER_COMPLEX34_DATA(REGISTER_X));
    f();
    elementwiseCxmaGetResult(&x, i);
  }

  realMatrixFree(&y);
  convertComplex34MatrixToComplex34MatrixRegister(&x, REGISTER_X);
  complexMatrixFree(&x);
}


void elementwiseCxmaCxma(void (*f)(void)) {
  complex34Matrix_t x, y;

  convertComplex34MatrixRegisterToComplex34Matrix(REGISTER_Y, &y);
  convertComplex34MatrixRegisterToComplex34Matrix(REGISTER_X, &x);
  if(x.header.matrixRows != y.header.matrixRows || x.header.matrixColumns != y.header.matrixColumns) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    return;
  }

  const unsigned int numOfElements = x.header.matrixRows * x.header.matrixColumns;
  for(unsigned int i = 0; i < numOfElements; ++i) {
    reallocateRegister(REGISTER_Y, dtComplex34, 0, amNone);
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    complex34Copy(y.matrixElements + i, REGISTER_COMPLEX34_DATA(REGISTER_Y));
    complex34Copy(x.matrixElements + i, REGISTER_COMPLEX34_DATA(REGISTER_X));
    f();
    elementwiseCxmaGetResult(&x, i);
  }

  complexMatrixFree(&y);
  convertComplex34MatrixToComplex34MatrixRegister(&x, REGISTER_X);
  complexMatrixFree(&x);
}


void callByVectorElement(bool_t (*real_f)(real34Matrix_t *), bool_t (*complex_f)(complex34Matrix_t *)) {

  const int16_t i = getIRegisterAsInt(true);
  const int16_t j = getJRegisterAsInt(true);

  if(matrixIndex == INVALID_VARIABLE || !regInRange(matrixIndex)) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot execute: destination register is out of range: %d", matrixIndex);
      moreInfoOnError("In function callByVectorElement:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
    real34Matrix_t mat;
    convertReal34MatrixRegisterToReal34Matrix(matrixIndex, &mat);
    if(i < 0 || i >= mat.header.matrixRows || j < 0 || j >= mat.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Cannot execute: element (%" PRId16 ", %" PRId16 ") out of range", (int16_t)(i + 1), (int16_t)(j + 1));
        moreInfoOnError("In function callByVectorElement:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
        if(real_f(&mat)) {
          convertReal34MatrixToReal34MatrixRegister(&mat, matrixIndex);
        }
    }
    realMatrixFree(&mat);
  }
  else if(getRegisterDataType(matrixIndex) == dtComplex34Matrix) {
    complex34Matrix_t mat;
    convertComplex34MatrixRegisterToComplex34Matrix(matrixIndex, &mat);
    if(i < 0 || i >= mat.header.matrixRows || j < 0 || j >= mat.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Cannot execute: element (%" PRId16 ", %" PRId16 ") out of range", (int16_t)(i + 1), (int16_t)(j + 1));
        moreInfoOnError("In function callByVectorElement:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
        if(complex_f(&mat)) {
          convertComplex34MatrixToComplex34MatrixRegister(&mat, matrixIndex);
        }
    }
    complexMatrixFree(&mat);
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot execute: something other than a matrix is indexed %s", getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function callByVectorElement:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

void callByIndexedMatrix(bool_t (*real_f)(real34Matrix_t *), bool_t (*complex_f)(complex34Matrix_t *)) {
  const int16_t i = getIRegisterAsInt(true);
  const int16_t j = getJRegisterAsInt(true);

  if(matrixIndex == INVALID_VARIABLE || !regInRange(matrixIndex)) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot execute: destination register is out of range: %d", matrixIndex);
      moreInfoOnError("In function callByIndexedMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
    real34Matrix_t mat;
    convertReal34MatrixRegisterToReal34Matrix(matrixIndex, &mat);
    if(i < 0 || i >= mat.header.matrixRows || j < 0 || j >= mat.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Cannot execute: element (%" PRId16 ", %" PRId16 ") out of range", (int16_t)(i + 1), (int16_t)(j + 1));
        moreInfoOnError("In function callByIndexedMatrix:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
        if(real_f(&mat)) {
          convertReal34MatrixToReal34MatrixRegister(&mat, matrixIndex);
        }
    }
    realMatrixFree(&mat);
  }
  else if(getRegisterDataType(matrixIndex) == dtComplex34Matrix) {
    complex34Matrix_t mat;
    convertComplex34MatrixRegisterToComplex34Matrix(matrixIndex, &mat);
    if(i < 0 || i >= mat.header.matrixRows || j < 0 || j >= mat.header.matrixColumns) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Cannot execute: element (%" PRId16 ", %" PRId16 ") out of range", (int16_t)(i + 1), (int16_t)(j + 1));
        moreInfoOnError("In function callByIndexedMatrix:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
        if(complex_f(&mat)) {
          convertComplex34MatrixToComplex34MatrixRegister(&mat, matrixIndex);
        }
    }
    complexMatrixFree(&mat);
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot execute: something other than a matrix is indexed %s", getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function callByIndexedMatrix:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



//   Conversion from [x, y, z] to Spherical [r, θ, φ]:
//     Radius          : r = √(x² + y² + z²)
//     Azimuthal angle : θ = tan⁻¹(y/x) (measured from the positive x-axis in the xy-plane)
//     Polar angle     : φ = cos⁻¹(z/r) (measured from the positive z-axis)
//     r: Distance from origin
//     θ: Angle from positive x-axis toward positive y-axis (counterclockwise in xy-plane)
//     φ: Angle from positive z-axis toward xy-plane
//
//   Conversion from [x, y, z] to Cylindrical [r, θ, z]:
//     Radius          : r = √(x² + y²)
//     Azimuthal angle : θ = tan⁻¹(y/x) (measured from the positive x-axis in the xy-plane)
//     Height          : z = z (remains the same)
//     r: Distance from z-axis
//     θ: Angle from positive x-axis toward positive y-axis (counterclockwise in xy-plane)
//     z: Height along z-axis
//

//   Special Cases in Coordinate Conversions Rectangular to Cylindrical [r, θ, z]
//     Origin: [0, 0, 0] → [0, undefined°, 0]
//     Z-axis points: [0, 0, z] → [0, undefined°, z]
//     Positive x-axis: [x, 0, z] → [x, 0°, z]
//     Negative x-axis: [-x, 0, z] → [x, 180°, z]
//     Positive y-axis: [0, y, z] → [y, 90°, z]
//     Negative y-axis: [0, -y, z] → [y, 270°, z]
//
//   Special Cases in Coordinate Conversions Rectangular to Spherical [r, θ, φ]
//     Origin: [0, 0, 0] → [0, undefined°, undefined°]
//     Positive z-axis: [0, 0, z] → [z, 0°, undefined°]
//     Negative z-axis: [0, 0, -z] → [z, 180°, undefined°]
//     Positive x-axis: [x, 0, 0] → [x, 90°, 0°]
//     Negative x-axis: [-x, 0, 0] → [x, 90°, 180°]
//     Positive y-axis: [0, y, 0] → [y, 90°, 90°]
//     Negative y-axis: [0, -y, 0] → [y, 90°, 270°]
//     XY-plane points: [x, y, 0] → [√(x²+y²), 90°, arctan(y/x)°]
//
//   Note: "undefined°" indicates angles that are mathematically indeterminate but can be set to any value by convention (chosen to be 0°)


#if defined(OPTION_VECTOR)
  void convert3DtoSPH(const real34Matrix_t *matrix, real_t *r, real_t *th1, real_t *th2, uint8_t am, decContext *ctxtRealDisplay) {
    real_t z, xyRadius;
    cplx_t xy;
    _euclideanNormRealMatrix(matrix, 2, r, ctxtRealDisplay);

    real34ToReal(matrix->matrixElements, &xy.Real);
    real34ToReal(matrix->matrixElements + 1, &xy.Imag);
    real34ToReal(matrix->matrixElements + 2, &z);

    if(realIsZero(&xy.Real) && realIsZero(&xy.Imag) && realIsZero(&z)) { //by convention [0 0 0] ==> Both angles (θ and φ) are undefined since there's no direction from the origin. By convention, both angles can be set to 0, but any values could be used
      realSetZero(r);
      realSetZero(th1);
      realSetZero(th2);
      goto SPH_ret1;
    }

    realRectangularToPolar(&xy, &xyRadius, th1, ctxtRealDisplay);
    C47_WP34S_Atan2(&xyRadius, &z, th2, ctxtRealDisplay);

SPH_ret1:
    convertAngleFromTo(th1, amRadian, am, ctxtRealDisplay);
    if(realIsZero(th1)) {
      realSetZero(th1);
    }
    convertAngleFromTo(th2, amRadian, am, ctxtRealDisplay);
    if(realIsZero(th2)) {
      realSetZero(th2);
    }
  }


  void convertSPHto3D(real_t *r, real_t *th1, real_t *th2, uint8_t am1, uint8_t am2, real34Matrix_t *matrix, decContext *ctxtRealDisplay) {
    cplx_t xy, zRadius;

    realPolarToRectangularInUnit(r, th2, am2, &zRadius, ctxtRealDisplay);
    if(realIsZero(&zRadius.Imag)) {
      realSetZero(&xy.Real);
      realSetZero(&xy.Imag);
    }
    else {
      realPolarToRectangularInUnit(&zRadius.Imag, th1, am1, &xy, ctxtRealDisplay);
    }

    realToReal34(&xy.Real, matrix->matrixElements);
    realToReal34(&xy.Imag, matrix->matrixElements + 1);
    realToReal34(&zRadius.Real, matrix->matrixElements + 2);
  }


  void convert3DtoCYL(const real34Matrix_t *matrix, real_t *r, real_t *th1, real_t *z, uint8_t am, decContext *ctxtRealDisplay) {
    cplx_t xy;
    real34ToReal(matrix->matrixElements, &xy.Real);
    real34ToReal(matrix->matrixElements + 1, &xy.Imag);
    real34ToReal(matrix->matrixElements + 2, z);

    realRectangularToPolar(&xy, r, th1, ctxtRealDisplay);

    convertAngleFromTo(th1, amRadian, am, ctxtRealDisplay);
    if(realIsZero(th1)) {
      realSetZero(th1);
    }
  }


  void convertCYLto3D(real_t *r, real_t *th1, real_t *z, uint8_t am, real34Matrix_t *matrix, decContext *ctxtRealDisplay) {
    cplx_t xy;

    realPolarToRectangularInUnit(r, th1, am, &xy, ctxtRealDisplay);

    realToReal34(&xy.Real, matrix->matrixElements);
    realToReal34(&xy.Imag, matrix->matrixElements + 1);
    realToReal34(z, matrix->matrixElements + 2);
  }


  void convert2DtoPOL(const real34Matrix_t *matrix, real_t *r, real_t *th1, uint8_t am, decContext *ctxtRealDisplay) {
      real_t x, y;
      _euclideanNormRealMatrix(matrix, 2, r, ctxtRealDisplay);

      real34ToReal(matrix->matrixElements, &x);
      real34ToReal(matrix->matrixElements + 1, &y);

      C47_WP34S_Atan2(&y, &x, th1, ctxtRealDisplay);
      convertAngleFromTo(th1, amRadian, am, ctxtRealDisplay);
      if(realIsZero(th1)) {
        realSetZero(th1);
      }
  }

  void convertPOLto2D(real_t *r, real_t *th1, uint8_t am, real34Matrix_t *matrix, decContext *ctxtRealDisplay) {
    cplx_t xy;
    realPolarToRectangularInUnit(r, th1, am, &xy, ctxtRealDisplay);
    realToReal34(&xy.Real, matrix->matrixElements);
    realToReal34(&xy.Imag, matrix->matrixElements + 1);
  }


  void V3err(int err) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_POLAR_RECT, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Err %d: 2D or 3D Vector required, not %s, %ix%i", err, getRegisterDataTypeName(REGISTER_X, true, false), REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows, REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns);
      moreInfoOnError("In function V3RectoToSph/V3RectoToCyl:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }


  bool_t VtoAngleMode(angularMode_t angleMode) {
    if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
      if(isRegisterMatrixVector(REGISTER_X)) {
        setVectorRegisterAngularMode(REGISTER_X, (angularMode_t)angleMode);
        temporaryInformation = TI_VECTOR;
      }
      else {
        return false;
      }
    }
    else {
      return false;
    }
    return true;
  }


#endif //OPTION_VECTOR

  bool_t is_2D3D_Register_Ready(uint32_t *ang2Dx, uint32_t *ang2Dy, uint32_t *ang3Dx, uint32_t *ang3Dy, uint32_t *ang3Dz, bool_t *validPolarInput, bool_t *valid2DRInput, bool_t *validSPHInput, bool_t *validCYLInput, bool_t *valid3DRInput, uint16_t constVector) {
    *ang2Dx = (registerIsNoAngle(REGISTER_X) || constVector != VECT_CR_yx) ? amNone : getRegisterAngularMode(REGISTER_X);
    *ang2Dy = (registerIsNoAngle(REGISTER_Y) || constVector != VECT_CR_yx) ? amNone : getRegisterAngularMode(REGISTER_Y);
    *ang3Dx = (registerIsNoAngle(REGISTER_X) || (constVector != VECT_CR_zyx && constVector != VECT_CR_zxy)) ? amNone : getRegisterAngularMode(REGISTER_X);
    *ang3Dy = (registerIsNoAngle(REGISTER_Y) || (constVector != VECT_CR_zyx && constVector != VECT_CR_zxy)) ? amNone : getRegisterAngularMode(REGISTER_Y);
    *ang3Dz = (registerIsNoAngle(REGISTER_Z) || (constVector != VECT_CR_zyx && constVector != VECT_CR_zxy)) ? amNone : getRegisterAngularMode(REGISTER_Z);
    *validPolarInput = (*ang2Dx != amNone && *ang2Dy == amNone && constVector == VECT_CR_yx);
    *valid2DRInput   = (*ang2Dx == amNone && *ang2Dy == amNone && constVector == VECT_CR_yx);
    *validSPHInput   = (*ang3Dx != amNone && *ang3Dy != amNone && *ang3Dz == amNone && (constVector == VECT_CR_zyx || constVector == VECT_CR_zxy));
    *validCYLInput   = (*ang3Dx == amNone && *ang3Dy != amNone && *ang3Dz == amNone && (constVector == VECT_CR_zyx || constVector == VECT_CR_zxy));
    *valid3DRInput   = (*ang3Dx == amNone && *ang3Dy == amNone && *ang3Dz == amNone);
    // printf("\nconstVector = %d\n",constVector);
    // printf("ang2Dx=    %2u ang2Dy=  %2u ang3Dx=  %2u ang3Dy=  %2u ang3Dz=  %2u\n", *ang2Dx, *ang2Dy, *ang3Dx, *ang3Dy, *ang3Dz);
    // printf("validPolar=%2d valid2DR=%2d validSPH=%2d validCYL=%2d valid3DR=%2d\n", *validPolarInput, *valid2DRInput, *validSPHInput, *validCYLInput, *valid3DRInput);
    return (*validPolarInput || *valid2DRInput || *validSPHInput || *validCYLInput || *valid3DRInput);
  }


static bool columnMinMaxReal(real34Matrix_t *matrix, bool calcMax) {
    uint16_t r, res_r = 0;
    real34_t res_val;
    longInteger_t longIntVar;

    const int16_t i = getIRegisterAsInt(true);
    const int16_t j = getJRegisterAsInt(true);

    realToReal34(calcMax ? const_minusInfinity : const_plusInfinity, &res_val);
    if(i >= 0 && j >= 0 && i < matrix->header.matrixRows && j < matrix->header.matrixColumns) {
      for(r = 0; r < matrix->header.matrixRows; ++r) {
        if((!calcMax) && real34CompareLessEqual(&matrix->matrixElements[r * matrix->header.matrixColumns + j], &res_val)) {
          real34Copy(&matrix->matrixElements[r * matrix->header.matrixColumns + j], &res_val);
          res_r = r;
        }
        else if(calcMax && real34CompareGreaterEqual(&matrix->matrixElements[r * matrix->header.matrixColumns + j], &res_val)) {
          real34Copy(&matrix->matrixElements[r * matrix->header.matrixColumns + j], &res_val);
          res_r = r;
        }
      }

      liftStack();
      setSystemFlag(FLAG_ASLIFT);
      liftStack();

      longIntegerInit(longIntVar);
      uInt32ToLongInteger(res_r + 1, longIntVar);
      convertLongIntegerToLongIntegerRegister(longIntVar, REGISTER_Y);
      longIntegerFree(longIntVar);

      reallocateRegister(REGISTER_X, dtReal34, REAL34_SIZE_IN_BYTES, amNone);
      real34Copy(&res_val, REGISTER_REAL34_DATA(REGISTER_X));
    }
    else {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "rows %" PRIu16 " and/or %" PRIu16 " out of range.", i, j);
        moreInfoOnError("In function columnMinMaxReal:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }

    return true;
  }

  static bool columnMinMatrix(real34Matrix_t *matrix) {
    return columnMinMaxReal(matrix, false);
  }

  static bool columnMaxMatrix(real34Matrix_t *matrix) {
    return columnMinMaxReal(matrix, true);
  }

  static bool columnMinMaxComplex(complex34Matrix_t *matrix) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot apply this function to %s", getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function columnMinMaxComplex:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }


  static bool matrixFindAny(any34Matrix_t *matrix, const matrixKind_t *kind) {
    complex34_t searchVal;                                                    // large enough for an element of either kind
    const size_t size = kind->elementSize;
    uint8_t *e;
    uint16_t r, c;

    switch(getRegisterDataType(REGISTER_X)) {
      case dtShortInteger: {
        real_t x;
        convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
        realToReal34(&x, VARIABLE_REAL34_DATA(&searchVal));
        real34SetZero(VARIABLE_IMAG34_DATA(&searchVal));
        break;
      }

      case dtLongInteger: {
        convertLongIntegerRegisterToReal34(REGISTER_X, VARIABLE_REAL34_DATA(&searchVal));
        real34SetZero(VARIABLE_IMAG34_DATA(&searchVal));
        break;
      }

      case dtReal34: {
        if(getRegisterAngularMode(REGISTER_X) == amNone) {
          real34Copy(REGISTER_REAL34_DATA(REGISTER_X), VARIABLE_REAL34_DATA(&searchVal));
          real34SetZero(VARIABLE_IMAG34_DATA(&searchVal));
        }
        else {
          temporaryInformation = TI_FALSE;
          return true;
        }
        break;
      }

      case dtComplex34: {
        if(kind->isComplex || real34IsZero(REGISTER_IMAG34_DATA(REGISTER_X))) {
          real34Copy(REGISTER_REAL34_DATA(REGISTER_X), VARIABLE_REAL34_DATA(&searchVal));
          real34Copy(REGISTER_IMAG34_DATA(REGISTER_X), VARIABLE_IMAG34_DATA(&searchVal));
        }
        else {
          temporaryInformation = TI_FALSE;
          return true;
        }
        break;
      }

      default: {
        temporaryInformation = TI_FALSE;
        return true;
      }
    }

    e = MATRIX_BYTES(matrix);
    for(r = 0; r < matrix->header.matrixRows; ++r) {
      for(c = 0; c < matrix->header.matrixColumns; ++c) {
        if(kind->equalElement(e + (r * matrix->header.matrixColumns + c)*size, &searchVal)) {
          setIRegisterAsInt(true, r);
          setJRegisterAsInt(true, c);
          temporaryInformation = TI_TRUE;
          return true;
        }
      }
    }

    temporaryInformation = TI_FALSE;
    return true;
  }

  static bool matrixFindReal(real34Matrix_t *matrix) {
    return matrixFindAny((any34Matrix_t*)matrix, &matrixKindReal);
  }

  static bool matrixFindComplex(complex34Matrix_t *matrix) {
    return matrixFindAny((any34Matrix_t*)matrix, &matrixKindComplex);
  }


  void fnColumnMin(uint16_t unusedParamButMandatory) {
    callByIndexedMatrix(columnMinMatrix, columnMinMaxComplex);
  }

  void fnColumnMax(uint16_t unusedParamButMandatory) {
    callByIndexedMatrix(columnMaxMatrix, columnMinMaxComplex);
  }

  void fnMatrixFind(uint16_t unusedParamButMandatory) {
    callByIndexedMatrix(matrixFindReal, matrixFindComplex);
  }

#if defined(OPTION_VECTOR)


  #define _3DCYL 1
  #define _3DSPH 2
  int isStack3DReadyConvertIfNot(int16_t mode, uint16_t constVector) {
    uint32_t ang2Dx, ang2Dy, ang3Dx, ang3Dy, ang3Dz;
    bool_t validPolarInput, valid2DRInput, validSPHInput, validCYLInput, valid3DRInput;
    is_2D3D_Register_Ready(&ang2Dx, &ang2Dy, &ang3Dx, &ang3Dy, &ang3Dz, &validPolarInput, &valid2DRInput, &validSPHInput, &validCYLInput, &valid3DRInput, constVector);
    bool_t is_3D_Register_Ready = validSPHInput || validCYLInput || valid3DRInput;

    if(!is_3D_Register_Ready) {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_POLAR_RECT, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "No valid coordinates for 3D Spherical/Cylindrical");
        moreInfoOnError("In function isStack3DReadyConvertIfNot:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }

    real_t xx;
    if(!(getRegisterAsReal(REGISTER_X, &xx) && getRegisterAsReal(REGISTER_Y, &xx) && getRegisterAsReal(REGISTER_Z, &xx))) {
      return false;
    }

    if((validCYLInput && mode == _3DCYL) || (validSPHInput && mode == _3DSPH)) {
      return true;
    }

    if(valid3DRInput && (mode == _3DCYL || mode == _3DSPH)) { // change the middle register Y to angle using ASM
      if(getRegisterDataType(REGISTER_Y) == dtShortInteger) {
        convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
      }
      if(getRegisterDataType(REGISTER_Y) == dtLongInteger) {
        convertLongIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
      }
      setRegisterAngularMode(REGISTER_Y, currentAngularMode);

      if(mode == _3DSPH) { // change the last register X to angle using ASM
        if(getRegisterDataType(REGISTER_X) == dtShortInteger) {
          convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
        }
        if(getRegisterDataType(REGISTER_X) == dtLongInteger) {
          convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
        }
        setRegisterAngularMode(REGISTER_X, currentAngularMode);
      }
      return true; // converted now ready for SPH or CYL
    }

    //at this point the registers are not 3D polar capable
    return false;
  }

  void V3RectoToSph(uint16_t am) {
    if(getRegisterDataType(REGISTER_X) != dtReal34Matrix && isStack3DReadyConvertIfNot(_3DSPH, VECT_CR_zyx)) {
      fnConvertStkToMx(VECT_CR_zyx);
    }
    else {
      angularMode_t angleMode = (am == 255 ? currentAngularMode : am);
      if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
        if(isRegisterMatrix3dVector(REGISTER_X)) {
          setVectorRegisterPolarMode(REGISTER_X, amPolarSPH);
          setVectorRegisterAngularMode(REGISTER_X, angleMode);
          temporaryInformation = TI_VECTOR;
        }
        else {
          V3err(1);
        }
      }
      else {
        V3err(2);
      }
    }
  }

  void V3RectoToCyl(uint16_t am) {
    if(getRegisterDataType(REGISTER_X) != dtReal34Matrix && isStack3DReadyConvertIfNot(_3DCYL, VECT_CR_zyx)) {
      fnConvertStkToMx(VECT_CR_zyx);
    }
    else {
      angularMode_t angleMode = (am == 255 ? currentAngularMode : am);
      if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
        if(isRegisterMatrix3dVector(REGISTER_X)) {
          setVectorRegisterPolarMode(REGISTER_X, amPolarCYL);
          setVectorRegisterAngularMode(REGISTER_X, angleMode);
          temporaryInformation = TI_VECTOR;
        }
        else {
          V3err(3);
        }
      }
      else {
        V3err(4);
      }
    }
  }



  void fnComplexToVector (uint16_t opType) {
    real34Matrix_t matrix;
    if(isRegisterMatrix2dVector(REGISTER_X) && (opType == ITM_CPXexV || opType == ITM_VtoCPX)) {
      //MatrixVector2D ==> Complex
      copySourceRegisterToDestRegister(REGISTER_X, TEMP_REGISTER_1);
      reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
      linkToRealMatrixRegister(TEMP_REGISTER_1,  &matrix);
      real34Copy(matrix.matrixElements, REGISTER_REAL34_DATA(REGISTER_X));
      real34Copy(matrix.matrixElements + 1, REGISTER_IMAG34_DATA(REGISTER_X));
      adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
      setComplexRegisterAngularMode(REGISTER_X, getVectorRegisterAngularMode(TEMP_REGISTER_1));
      setComplexRegisterPolarMode(REGISTER_X, (getVectorRegisterPolarMode(TEMP_REGISTER_1) == amPolar) ? amPolar : amNone);
      clearRegister(TEMP_REGISTER_1);
      return;
    }
    else if(getRegisterDataType(REGISTER_X) == dtComplex34  && (opType == ITM_CPXexV || opType == ITM_CPXtoV)) {
      //Complex ==> MatrixVector2D
      copySourceRegisterToDestRegister(REGISTER_X, TEMP_REGISTER_1);
      //Initialize Memory for Matrix
      if(initMatrixRegister(REGISTER_X, 1, 2, false)) {
      }
      else {
        // ignore error condition, because no extra bytes are needed converting from complex34 to Real matrix 2D vector
        return;
      }
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
      linkToRealMatrixRegister(REGISTER_X,  &matrix);
      real34Copy(REGISTER_REAL34_DATA(TEMP_REGISTER_1), matrix.matrixElements);
      real34Copy(REGISTER_IMAG34_DATA(TEMP_REGISTER_1), matrix.matrixElements + 1);
      adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
      setVectorRegisterAngularMode(REGISTER_X, getComplexRegisterAngularMode(TEMP_REGISTER_1));
      setVectorRegisterPolarMode(REGISTER_X,   getComplexRegisterPolarMode(TEMP_REGISTER_1));
      clearRegister(TEMP_REGISTER_1);
      return;
    }

      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "invalid data type %s and %s", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnComplexToVector:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

#else //OPTION_VECTOR
  void convert3DtoSPH(const real34Matrix_t *matrix, real_t *r, real_t *th1, real_t *th2, uint8_t am, decContext *ctxtRealDisplay) {;}
  void convertSPHto3D(real_t *r, real_t *th1, real_t *th2, uint8_t am1, uint8_t am2, real34Matrix_t *matrix, decContext *ctxtRealDisplay) {;}
  void convert3DtoCYL(const real34Matrix_t *matrix, real_t *r, real_t *th1, real_t *z, uint8_t am, decContext *ctxtRealDisplay) {;}
  void convertCYLto3D(real_t *r, real_t *th1, real_t *z, uint8_t am, real34Matrix_t *matrix, decContext *ctxtRealDisplay) {;}
  void convert2DtoPOL(const real34Matrix_t *matrix, real_t *r, real_t *th1, uint8_t am, decContext *ctxtRealDisplay) {;}
  void convertPOLto2D(real_t *r, real_t *th1, uint8_t am, real34Matrix_t *matrix, decContext *ctxtRealDisplay) {;}
  bool_t VtoAngleMode(angularMode_t angleMode) {return false;}
  void V3RectoToSph(uint16_t am) {;}
  void V3RectoToCyl(uint16_t am) {;}
  void fnComplexToVector (uint16_t opType) {;}
#endif //OPTION_VECTOR

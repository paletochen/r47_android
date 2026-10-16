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

/********************************************//**
 * \brief Convert tagged real storage to the common unit used for comparison.
 *
 * \param[in] regist calcRegister_t
 * \param[in,out] value real_t
 * \return void
 ***********************************************/
static void normaliseRegisterRealForComparison(calcRegister_t regist, real_t *value) {
  const uint32_t type = getRegisterDataType(regist);

  if(type == dtTime) {
    realDivide(value, const_3600, value, &ctxtReal39);
  }
  else if(type == dtReal34) {
    const angularMode_t mode = getRegisterAngularMode(regist);

    if(mode != amNone && mode != currentAngularMode) {
      realContext_t *const context = mode == amRadian || currentAngularMode == amRadian ? &ctxtReal75 : &ctxtReal39;
      convertAngleFromTo(value, mode, currentAngularMode, context);
    }
  }
}

/********************************************//**
 * \brief Read a real-compatible register and normalise its comparison unit.
 *
 * \param[in] regist calcRegister_t
 * \param[out] value real_t
 * \return bool_t
 ***********************************************/
static bool_t getRegisterAsComparisonReal(calcRegister_t regist, real_t *value) {
  if(!getRegisterAsAnyRealQuiet(regist, value)) {
    return false;
  }
  normaliseRegisterRealForComparison(regist, value);
  return true;
}

/********************************************//**
 * \brief Compares a Real34 and long-integer pair without changing compatible units
 *
 * \param[in] regist1 first register
 * \param[in] regist2 second register
 * \param[out] comparison signed comparison result
 * \return true when the exact mixed comparison applies, false otherwise
 ***********************************************/
static bool_t compareMixedRealLong(calcRegister_t regist1, calcRegister_t regist2, int8_t *comparison) {
  const uint32_t type1 = getRegisterDataType(regist1);
  const uint32_t type2 = getRegisterDataType(regist2);
  calcRegister_t realRegister;
  int32_t value;

  if(type1 == dtReal34 && type2 == dtLongInteger) {
    realRegister = regist1;
  }
  else if(type1 == dtLongInteger && type2 == dtReal34) {
    realRegister = regist2;
  }
  else {
    return false;
  }

  const angularMode_t mode = getRegisterAngularMode(realRegister);
  if(mode != amNone && mode != currentAngularMode) {
    return false;
  }
  if(!compareFiniteReal34LongIntegerRegisters(regist1, regist2, &value)) {
    return false;
  }

  *comparison = (int8_t)value;
  return true;
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

    if(compareMixedRealLong(regist1, regist2, res)) {
      return true;
    }
    if(!getRegisterAsComparisonReal(regist1, &real1) || !getRegisterAsComparisonReal(regist2, &real2)) {
      return false;
    }
    if(realIsNaN(&real1) || realIsNaN(&real2)) {
      return false;
    }
    realCompare(&real1, &real2, &rcmp, &ctxtReal39);
    *res = realIsZero(&rcmp) ? 0 : realIsPositive(&rcmp) ? 1 : -1;
  }
  return true;
}

static void registerMinMax(calcRegister_t regist1, calcRegister_t regist2, calcRegister_t dest, int maximum) {
  int8_t cmp;
  calcRegister_t rMin = regist1;
  calcRegister_t rMax = regist2;
  calcRegister_t where;
  real_t real1, real2;

  if(getRegisterAsComparisonReal(regist1, &real1) && getRegisterAsComparisonReal(regist2, &real2)) {
    const bool_t nan1 = realIsNaN(&real1);
    const bool_t nan2 = realIsNaN(&real2);

    if(nan1 || nan2) {
      where = nan1 == nan2 ? regist1 : (nan1 ? regist2 : regist1);
      if(where != dest) {
        copySourceRegisterToDestRegister(where, dest);
      }
      return;
    }

    if(realIsZero(&real1) && realIsZero(&real2) && realIsNegative(&real1) != realIsNegative(&real2)) {
      const bool_t chooseFirst = maximum ? realIsPositive(&real1) : realIsNegative(&real1);
      where = chooseFirst ? regist1 : regist2;
      if(where != dest) {
        copySourceRegisterToDestRegister(where, dest);
      }
      return;
    }
  }

  if(registerCmp(rMin, rMax, &cmp)) {
    if(cmp == 0 && (dest == regist1 || dest == regist2)) {
      return;
    }
    if(cmp > 0) {
      rMin = regist2;
      rMax = regist1;
    }
    where = maximum ? rMax : rMin;
    if(where != dest) {
      copySourceRegisterToDestRegister(where, dest);
    }
  }
  else {
    badTypeError(regist1);
  }
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
  if(typeX == dtReal34Matrix) {
    linkToRealMatrixRegister(REGISTER_X, &mReal);
    for(i=k=0; i<mReal.header.matrixRows; i++) {
      for(j=0; j<mReal.header.matrixColumns; j++, k++) {
        if(!real34CompareEqual(&mReal.matrixElements[k], i==j ? r : const34_0)) {
          goto different;
        }
      }
    }
  }
  else {
    linkToComplexMatrixRegister(REGISTER_X, &mCplx);
    for(i=k=0; i<mCplx.header.matrixRows; i++) {
      for(j=0; j<mCplx.header.matrixColumns; j++, k++) {
        if(!real34CompareEqual(&mCplx.matrixElements[k].real, i==j ? r : const34_0) || !real34IsZero((&mCplx.matrixElements[k].imag))) {
          goto different;
        }
      }
    }
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
  real34Matrix_t xReal, rReal;
  complex34Matrix_t xComplex, rComplex;
  int xRows, xColumns, rRows, rColumns;

  if(typeX == dtComplex34Matrix) {
    linkToComplexMatrixRegister(REGISTER_X, &xComplex);
    xRows = xComplex.header.matrixRows;
    xColumns = xComplex.header.matrixColumns;
  }
  else {
    linkToRealMatrixRegister(REGISTER_X, &xReal);
    xRows = xReal.header.matrixRows;
    xColumns = xReal.header.matrixColumns;
  }
  if(typeR == dtComplex34Matrix) {
    linkToComplexMatrixRegister(regist, &rComplex);
    rRows = rComplex.header.matrixRows;
    rColumns = rComplex.header.matrixColumns;
  }
  else {
    linkToRealMatrixRegister(regist, &rReal);
    rRows = rReal.header.matrixRows;
    rColumns = rReal.header.matrixColumns;
  }

  if(xRows == rRows && xColumns == rColumns) {
    temporaryInformation = TI_TRUE;
    for(int i = 0; i < xRows * xColumns; ++i) {
      const real34_t *xRe = typeX == dtComplex34Matrix ? VARIABLE_REAL34_DATA(&xComplex.matrixElements[i]) : &xReal.matrixElements[i];
      const real34_t *xIm = typeX == dtComplex34Matrix ? VARIABLE_IMAG34_DATA(&xComplex.matrixElements[i]) : const34_0;
      const real34_t *rRe = typeR == dtComplex34Matrix ? VARIABLE_REAL34_DATA(&rComplex.matrixElements[i]) : &rReal.matrixElements[i];
      const real34_t *rIm = typeR == dtComplex34Matrix ? VARIABLE_IMAG34_DATA(&rComplex.matrixElements[i]) : const34_0;

      if(!matrixCompareRealEqual(xRe, rRe) || !matrixCompareRealEqual(xIm, rIm)) {
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

/* ============================================================================
 *  Type-pair helpers (replaces GLUE2 / GLUE2X / GLUE2Y)
 *  - Encodes two 8-bit type IDs into one 16-bit key:
 *      low byte  = left operand type (REGISTER_X)
 *      high byte = right operand type (regist)
 *  IMPORTANT: This assumes dtXXX values fit into 0..255. If not, use 16+16.
 * ========================================================================== */
#define type_pair_u8(lo, hi) ((uint16_t)lo | ((uint16_t)hi << 8))
#define type_lo_u8(p) ((uint8_t)(p & 0xFFu))
#define type_hi_u8(p) ((uint8_t)(p >> 8))

/* Normalize any comparator output into -1 / 0 / +1 */
static inline int cmp_sign(int v) { return (v > 0) - (v < 0); }

static inline bool mode_is_equality(uint8_t mode) {
  return (mode == COMPARE_MODE_EQUAL) || (mode == COMPARE_MODE_NOT_EQUAL);
}

/* ============================================================================
 *  Register eligibility helpers (precondition)
 *  Goal: replace the hard-to-read compound boolean with readable helpers.
 * ========================================================================== */

static inline bool in_range_inclusive(uint16_t v, uint16_t lo, uint16_t hi) {
  return (v >= lo) && (v <= hi);
}

static inline bool is_local_register(uint16_t r) {
  /* Assumption: currentNumberOfLocalRegisters is a COUNT -> upper bound exclusive */
  const uint16_t first = FIRST_LOCAL_REGISTER;
  const uint16_t end_exclusive =
      (uint16_t)(FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters);
  return (r >= first) && (r < end_exclusive);
}

static inline bool is_named_variable(uint16_t r) {
  /* Assumption: numberOfNamedVariables is a COUNT.
     If it is already an inclusive max index, adjust accordingly. */
  if(numberOfNamedVariables == 0) {
    return false;
  }
  const uint16_t lo = FIRST_NAMED_VARIABLE;
  const uint16_t hi = (uint16_t)(FIRST_NAMED_VARIABLE + numberOfNamedVariables - 1);
  return in_range_inclusive(r, lo, hi);
}

static inline bool is_reserved_variable(uint16_t r) {
  return in_range_inclusive(r, FIRST_RESERVED_VARIABLE, LAST_RESERVED_VARIABLE);
}

static inline bool is_global_register(uint16_t r) {
  return in_range_inclusive(r, FIRST_GLOBAL_REGISTER, LAST_SPARE_REGISTER);
}

static inline bool is_comparable_register(uint16_t r) {
  return is_local_register(r) ||
         is_global_register(r) ||
         is_named_variable(r) ||
         is_reserved_variable(r) ||
         (r == TEMP_REGISTER_1);
}

/* ============================================================================
 *  Numeric comparison helpers
 * ========================================================================== */

static inline void compare_reals_to_temporaryInformation(real_t *a, real_t *b, uint8_t mode) {
  /* Ordered comparison and equality are false for NaN; inequality is true. */
  if(realIsNaN(a) || realIsNaN(b)) {
    SET_TI_TRUE_FALSE(mode == COMPARE_MODE_NOT_EQUAL);
    return;
  }

  /* realCompare stores the sign of (a ? b) into diff */
  real_t diff;
  realCompare(a, b, &diff, &ctxtReal39);

  const int cmp = realIsZero(&diff) ? 0 : realIsNegative(&diff) ? -1 : 1;
  cmpToResult(cmp, mode);
}

static inline void compare_complex_to_temporaryInformation(cplx_t *a, cplx_t *b,
                                                       uint8_t mode,
                                                       uint16_t regist_for_error) {
  // Complex numbers do not have a total ordering; only == and != are supported
  if(!mode_is_equality(mode)) {
    compareTypeError(regist_for_error);
    return;
  }
  const bool_t equal = realCompareEqual(&a->Real, &b->Real) && realCompareEqual(&a->Imag, &b->Imag);
  SET_TI_TRUE_FALSE(mode == COMPARE_MODE_EQUAL ? equal : !equal);
}

/* ============================================================================
 *  Main compare function
 * ========================================================================== */
static void compareRegisters(uint16_t regist, uint8_t mode) {
  /* Precondition: only proceed for valid/comparable registers regardless types */
  if(!is_comparable_register(regist)) {
    // TODO: raiser a proper error
    return;
  }
  /* Encode type pair */
  const uint8_t xType = (uint8_t)getRegisterDataType(REGISTER_X);
  const uint8_t rType = (uint8_t)getRegisterDataType(regist);
  const uint16_t test = type_pair_u8(xType, rType);

  cplx_t x, r;

  switch(test) {

    /* ------------------------------------------------------------------------
     * String vs String
     * ---------------------------------------------------------------------- */
    case type_pair_u8(dtString, dtString): {
      const int c = compareString(REGISTER_STRING_DATA(REGISTER_X),
                                  REGISTER_STRING_DATA(regist),
                                  CMP_EXTENSIVE);
      cmpToResult(c, mode);
    } break;

    /* ------------------------------------------------------------------------
     * Config vs Config (equality only)
     * NOTE: memcmp is safe only if configs are raw byte blobs without padding.
     * ---------------------------------------------------------------------- */
    case type_pair_u8(dtConfig, dtConfig): {
      if(!mode_is_equality(mode)) {
        compareTypeError(regist);
        break;
      }
      int c = memcmp(REGISTER_CONFIG_DATA(REGISTER_X),
                     REGISTER_CONFIG_DATA(regist),
                     TO_BYTES(CONFIG_SIZE_IN_BLOCKS));
      cmpToResult(cmp_sign(c), mode);
    } break;

    /* ------------------------------------------------------------------------
     * Matrix vs Matrix (equality only)
     * Supports same-type matrices and (optionally) real/complex matrix mix.
     * We forward both types to compareMatrices so it can decide how to handle it.
     * ---------------------------------------------------------------------- */
    case type_pair_u8(dtReal34Matrix,    dtReal34Matrix):
    case type_pair_u8(dtComplex34Matrix, dtComplex34Matrix):
    /* Optional mixed matrix equality support (enable if compareMatrices supports it) */
    case type_pair_u8(dtReal34Matrix,    dtComplex34Matrix):
    case type_pair_u8(dtComplex34Matrix, dtReal34Matrix): {
      if(!mode_is_equality(mode)) {
        compareTypeError(regist);
        break;
      }

      compareMatrices(regist, mode, xType, rType);
    } break;

    case type_pair_u8(dtReal34Matrix, dtReal34):
    case type_pair_u8(dtComplex34Matrix, dtReal34): {
      if(regist != TEMP_REGISTER_1 || !mode_is_equality(mode)) {
        compareTypeError(regist);
        break;
      }
      compareMatrix01(regist, mode, xType);
    } break;

    /* ------------------------------------------------------------------------
     * Integer pairs (short/long mix)
     * Use big-int comparison to avoid precision loss.
     * IMPORTANT: initialize to NULL to avoid freeing uninitialized values.
     * ---------------------------------------------------------------------- */
    case type_pair_u8(dtShortInteger, dtShortInteger):
    case type_pair_u8(dtLongInteger,  dtLongInteger):
    case type_pair_u8(dtShortInteger, dtLongInteger):
    case type_pair_u8(dtLongInteger,  dtShortInteger): {
      longInteger_t xInt;
      longInteger_t rInt;

      if(!getRegisterAsLongInt(REGISTER_X, xInt, NULL)) {
        goto end;
      }

      if(!getRegisterAsLongInt(regist, rInt, NULL)) {
        compareTypeError(regist);
        longIntegerFree(rInt);
end:
        longIntegerFree(xInt);
        compareTypeError(REGISTER_X);
        break;
      }

      cmpToResult(longIntegerCompare(xInt, rInt), mode);
      longIntegerFree(rInt);
      longIntegerFree(xInt);
    }
    break;

    /* ------------------------------------------------------------------------
     * (complex, real) x (complex, real, shortI, longI)
     * short & long integers will be casted as real
     * Rule:
     *  - If any operand is truly complex (imag != 0), only == and != are supported.
     *  - If both are effectively real (imag == 0), allow full real comparison.
     * ---------------------------------------------------------------------- */
    case type_pair_u8(dtComplex34, dtComplex34):
    case type_pair_u8(dtComplex34, dtReal34):
    case type_pair_u8(dtComplex34, dtShortInteger):
    case type_pair_u8(dtComplex34, dtLongInteger):

    case type_pair_u8(dtReal34, dtComplex34):
    case type_pair_u8(dtReal34, dtReal34):
    case type_pair_u8(dtReal34, dtShortInteger):
    case type_pair_u8(dtReal34, dtLongInteger):

    case type_pair_u8(dtShortInteger, dtComplex34):
    case type_pair_u8(dtShortInteger, dtReal34):
    case type_pair_u8(dtLongInteger, dtComplex34):
    case type_pair_u8(dtLongInteger, dtReal34): {
      bool isComplex = false;
      int8_t comparison;

      if(compareMixedRealLong(REGISTER_X, regist, &comparison)) {
        cmpToResult(comparison, mode);
        break;
      }

      // Note: getRegisterAsComplexOrAnyReal sets isComplex to true or leaves it unchanged,
      // effectively implementing a logical OR.
      if(!getRegisterAsComplexOrAnyReal(REGISTER_X, &x, &isComplex)) {
        compareTypeError(REGISTER_X);
        break;
      }
      if(!getRegisterAsComplexOrAnyReal(regist, &r, &isComplex)) {
        compareTypeError(regist);
        break;
      }

      normaliseRegisterRealForComparison(REGISTER_X, &x.Real);
      normaliseRegisterRealForComparison(regist, &r.Real);

      if(isComplex) {
        compare_complex_to_temporaryInformation(&x, &r, mode, regist);
      }
      else {
        compare_reals_to_temporaryInformation(&x.Real, &r.Real, mode);
      }
    } break;


    /* ------------------------------------------------------------------------
     * (time, real, shoI, longI) x (time, real, shoI, longI)
     * short & long integers will be casted as real, see above
     * ---------------------------------------------------------------------- */
    case type_pair_u8(dtReal34, dtTime):
    case type_pair_u8(dtTime, dtReal34):
    case type_pair_u8(dtLongInteger, dtTime):
    case type_pair_u8(dtShortInteger, dtTime):
    case type_pair_u8(dtTime, dtLongInteger):
    case type_pair_u8(dtTime, dtShortInteger):
    case type_pair_u8(dtTime, dtTime): {
      bool_t cannotBeComplex;
      if(!getRegisterAsComplexOrAnyReal(REGISTER_X, &x, &cannotBeComplex)) {
        compareTypeError(REGISTER_X);
        break;
      }
      if(!getRegisterAsComplexOrAnyReal(regist, &r, &cannotBeComplex)) {
        compareTypeError(regist);
        break;
      }

      normaliseRegisterRealForComparison(REGISTER_X, &x.Real);
      normaliseRegisterRealForComparison(regist, &r.Real);

      compare_reals_to_temporaryInformation(&x.Real, &r.Real, mode);
    } break;


    /* ------------------------------------------------------------------------
     * Unsupported combinations
     * ---------------------------------------------------------------------- */
    default:
      compareTypeError(regist);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "local register .%02d", regist - FIRST_LOCAL_REGISTER);
        moreInfoOnError("In function compareRegisters:", errorMessage, "is not defined!", NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      break;
  }
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
    if(getRegisterDataType(REGISTER_X) == dtReal34Matrix && getRegisterDataType(regist) == dtReal34Matrix) {
      real34Matrix_t x, r;
      convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &x);
      convertReal34MatrixRegisterToReal34Matrix(regist, &r);
      if(lastErrorCode != 0) { // a convert ran out of RAM; free locals and return before rounding
        realMatrixFree(&x);
        realMatrixFree(&r);
        return;
      }
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

      if(lastErrorCode != 0) { // a convert ran out of RAM; free locals and return before rounding
        complexMatrixFree(&x);
        complexMatrixFree(&r);
        if(!xIsCxma || !rIsCxma) {
          realMatrixFree(&m);
        }
        return;
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
}

static void almostEqualScalar(uint16_t regist, const uint16_t test) {
  snap_t snap1, snap2;   // saveRegisterSnapshot/restoreRegisterSnapshot are in registerValueConversions.h

  // Snapshot real values before rounding
  saveRegisterSnapshot(REGISTER_X, &snap1);
  saveRegisterSnapshot(regist, &snap2);

  switch(test) {
    case type_pair_u8(dtComplex34, dtComplex34):
    case type_pair_u8(dtComplex34, dtReal34):
    case type_pair_u8(dtComplex34, dtShortInteger):
    case type_pair_u8(dtComplex34, dtLongInteger):
      roundCplx();
      break;

    case type_pair_u8(dtReal34, dtComplex34):
    case type_pair_u8(dtReal34, dtReal34):
    case type_pair_u8(dtReal34, dtShortInteger):
    case type_pair_u8(dtReal34, dtLongInteger):
      roundReal();
      break;

    //ret20260309: when SI vs Real or Complex, cast to real
    case type_pair_u8(dtShortInteger, dtComplex34):
    case type_pair_u8(dtShortInteger, dtReal34):
      convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      roundReal();
      break;

    //ret20260309: when LI vs Real or Complex, cast to real
    case type_pair_u8(dtLongInteger, dtComplex34):
    case type_pair_u8(dtLongInteger, dtReal34):
      convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      roundReal();
      break;

    //jm20260422: when time vs Real or longinteger
    case type_pair_u8(dtTime, dtReal34):
      roundTime();
      convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
      break;

    case type_pair_u8(dtShortInteger, dtTime):
      convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      convertReal34RegisterToTimeRegister(REGISTER_X, REGISTER_X);
      roundTime();
      convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
      break;

    case type_pair_u8(dtLongInteger, dtTime):
      convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      // Fall through
    case type_pair_u8(dtReal34, dtTime):
      convertReal34RegisterToTimeRegister(REGISTER_X, REGISTER_X);
      roundTime();
      convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
      break;

    case type_pair_u8(dtTime, dtShortInteger):
    case type_pair_u8(dtTime, dtLongInteger):
      roundTime();
      convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
      break;

    case type_pair_u8(dtTime, dtTime):
      roundTime();
      break;
    default:
        fnSwapX(regist);
        compareTypeError(regist);
        restoreRegisterSnapshot(REGISTER_X, &snap1);
        restoreRegisterSnapshot(regist, &snap2);
        return;
  }
  if(regist != TEMP_REGISTER_1) {
    fnSwapX(regist);
    switch(test) {
      case type_pair_u8(dtComplex34, dtComplex34):
      case type_pair_u8(dtReal34, dtComplex34):
      case type_pair_u8(dtShortInteger, dtComplex34):
      case type_pair_u8(dtLongInteger, dtComplex34):
        roundCplx();
        break;

      case type_pair_u8(dtComplex34, dtReal34):
      case type_pair_u8(dtReal34, dtReal34):
      case type_pair_u8(dtShortInteger, dtReal34):
      case type_pair_u8(dtLongInteger, dtReal34):
        roundReal();
        break;

      //ret20260309: when SI vs Real or Complex, cast to real
      case type_pair_u8(dtComplex34, dtShortInteger):
      case type_pair_u8(dtReal34, dtShortInteger):
        convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
        roundReal();
        break;

      //ret20260309: when LI vs Real or Complex, cast to real
      case type_pair_u8(dtComplex34, dtLongInteger):
      case type_pair_u8(dtReal34, dtLongInteger):
        convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
        roundReal();
        break;

      case type_pair_u8(dtReal34, dtTime):
        roundTime();
        convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
        break;

      case type_pair_u8(dtTime, dtShortInteger):
        convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
        convertReal34RegisterToTimeRegister(REGISTER_X, REGISTER_X);
        roundTime();
        convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
        break;

      case type_pair_u8(dtTime, dtLongInteger):
        convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
        // Fall through
      case type_pair_u8(dtTime, dtReal34):
        convertReal34RegisterToTimeRegister(REGISTER_X, REGISTER_X);
        roundTime();
        convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
        break;

      case type_pair_u8(dtShortInteger, dtTime):
      case type_pair_u8(dtLongInteger, dtTime):
        roundTime();
        convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
        break;

      case type_pair_u8(dtTime, dtTime):
        roundTime();
        break;

      default:
          fnSwapX(regist);
          compareTypeError(regist);
          restoreRegisterSnapshot(REGISTER_X, &snap1);
          restoreRegisterSnapshot(regist, &snap2);
          return;
    }
    fnSwapX(regist);
  }

  compareRegisters(regist, COMPARE_MODE_EQUAL);

  restoreRegisterSnapshot(REGISTER_X, &snap1);
  restoreRegisterSnapshot(regist, &snap2);
}


void fnXAlmostEqual(uint16_t regist) {
  /* Encode type pair */
  const uint8_t xType = (uint8_t)getRegisterDataType(REGISTER_X);
  const uint8_t rType = (uint8_t)getRegisterDataType(regist);
  const uint16_t test = type_pair_u8(xType, rType);
  switch(test) {
    case type_pair_u8(dtReal34Matrix, dtReal34Matrix):
    case type_pair_u8(dtReal34Matrix, dtComplex34Matrix):
    case type_pair_u8(dtComplex34Matrix, dtReal34Matrix):
    case type_pair_u8(dtComplex34Matrix, dtComplex34Matrix):
      almostEqualMatrix(regist);
      break;

    case type_pair_u8(dtComplex34, dtComplex34):
    case type_pair_u8(dtComplex34, dtReal34):
    case type_pair_u8(dtComplex34, dtShortInteger):
    case type_pair_u8(dtComplex34, dtLongInteger):

    case type_pair_u8(dtReal34, dtComplex34):
    case type_pair_u8(dtReal34, dtReal34):
    case type_pair_u8(dtReal34, dtShortInteger):
    case type_pair_u8(dtReal34, dtLongInteger):

    case type_pair_u8(dtShortInteger, dtComplex34):
    case type_pair_u8(dtShortInteger, dtReal34):
    case type_pair_u8(dtLongInteger, dtComplex34):
    case type_pair_u8(dtLongInteger, dtReal34):

    case type_pair_u8(dtReal34, dtTime):
    case type_pair_u8(dtTime, dtReal34):
    case type_pair_u8(dtLongInteger, dtTime):
    case type_pair_u8(dtTime, dtLongInteger):
    case type_pair_u8(dtShortInteger, dtTime):
    case type_pair_u8(dtTime, dtShortInteger):
    case type_pair_u8(dtTime, dtTime):
      almostEqualScalar(regist, test);
      break;

    case type_pair_u8(dtShortInteger, dtShortInteger):
    case type_pair_u8(dtShortInteger, dtLongInteger):
    case type_pair_u8(dtLongInteger, dtShortInteger):
    case type_pair_u8(dtLongInteger, dtLongInteger):
      // No need to round
      compareRegisters(regist, COMPARE_MODE_EQUAL);
      break;

    default:
      compareTypeError(regist);
      break;
  }
}


#undef COMPARE_MODE_LESS_THAN
#undef COMPARE_MODE_EQUAL
#undef COMPARE_MODE_LESS_EQUAL
#undef COMPARE_MODE_GREATER_THAN
#undef COMPARE_MODE_NOT_EQUAL
#undef COMPARE_MODE_GREATER_EQUAL

void fnIsConverged(uint16_t mode) {
  real_t tol;
  cplx_t x, y;
  bool_t isComplex = false;

  convergenceTolerence(&tol);
  if(!getRegisterAsComplexOrReal(REGISTER_X, &x, &isComplex) || !getRegisterAsComplexOrReal(REGISTER_Y, &y, &isComplex)) {
    compareTypeError(REGISTER_Y);
    return;
  }

  if(realIsNaN(&x.Real) || realIsNaN(&y.Real) || realIsNaN(&x.Imag) || realIsNaN(&y.Imag)) {
    SET_TI_TRUE_FALSE((mode & 0x4) != 0);
  }
  else if(realIsInfinite(&x.Real) || realIsInfinite(&y.Real) || realIsInfinite(&x.Imag) || realIsInfinite(&y.Imag)) {
    SET_TI_TRUE_FALSE((mode & 0x2) != 0);
  }
  else if(mode & 0x01) {
    SET_TI_TRUE_FALSE(isComplex ? WP34S_ComplexAbsError(&x, &y, &tol, &ctxtReal39) : WP34S_AbsoluteError(&x.Real, &y.Real, &tol, &ctxtReal39));
  }
  else {
    SET_TI_TRUE_FALSE(isComplex ? WP34S_ComplexRelativeError(&x, &y, &tol, &ctxtReal39) : WP34S_RelativeError(&x.Real, &y.Real, &tol, &ctxtReal39));
  }
}

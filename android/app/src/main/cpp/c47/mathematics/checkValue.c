// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/****************************************************//**
 * \file checkValue.c
 *******************************************************/

#include "c47.h"

static bool_t checkXisType(uint16_t type) {
  return getRegisterDataType(REGISTER_X) == type;
}

void fnCheckType(uint16_t type) {
  SET_TI_TRUE_FALSE(checkXisType(type));
}

void fnCheckReal(uint16_t unusedButMandatoryParameter) {
  const uint32_t t = getRegisterDataType(REGISTER_X);

  SET_TI_TRUE_FALSE(t <= dtDate || t == dtShortInteger);
}

void fnCheckNumber(uint16_t unusedButMandatoryParameter) {
  int res = 1;

  switch(getRegisterDataType(REGISTER_X)) {
    default:
      res = 0;
      break;

    case dtLongInteger:
    case dtShortInteger:
      break;

    case dtComplex34:
      res = !real34IsSpecial(REGISTER_IMAG34_DATA(REGISTER_X));
      /* FALL THROUGH */
    case dtTime:
    case dtDate:
    case dtReal34:
      res &= !real34IsSpecial(REGISTER_REAL34_DATA(REGISTER_X));
      break;
  }
  SET_TI_TRUE_FALSE(res);
}

void fnCheckAngle(uint16_t unusedButMandatoryParameter) {
  SET_TI_TRUE_FALSE(checkXisType(dtReal34) && getRegisterAngularMode(REGISTER_X) != amNone);
}

void fnCheckMatrix(uint16_t unusedButMandatoryParameter) {
  const uint32_t t = getRegisterDataType(REGISTER_X);

  SET_TI_TRUE_FALSE(t == dtReal34Matrix || t == dtComplex34Matrix);
}

void fnCheckMatrixSquare(uint16_t unusedButMandatoryParameter) {
  const uint32_t t = getRegisterDataType(REGISTER_X);

  if(t == dtReal34Matrix || t == dtComplex34Matrix) {
    SET_TI_TRUE_FALSE(REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows == REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns);
  }
  else {
    compareTypeErrorX();
  }
}

void fnCheckForZero(uint16_t mode) {
  real_t xReal, xImag;
  bool_t cmplx;
  if(!getRegisterAsComplexOrAnyRealQuiet(REGISTER_X, &xReal, &xImag, &cmplx)) {
    compareTypeErrorX();
    return;
  }
  switch(mode) {
    case ITM_ISREZQ  : SET_TI_TRUE_FALSE(realIsZero(&xReal)); break;
    case ITM_ISIMZQ  : SET_TI_TRUE_FALSE(realIsZero(&xImag)); break;
    case ITM_ISRENZQ : SET_TI_TRUE_FALSE(!realIsZero(&xReal));break;
    case ITM_ISIMNZQ : SET_TI_TRUE_FALSE(!realIsZero(&xImag));break;
    default:;
  }
}

void fnCheckIsVect2d (uint16_t unusedButMandatoryParameter) {
  if(checkXisType(dtReal34Matrix)) {
    const matrixHeader_t *h = REGISTER_MATRIX_HEADER(REGISTER_X);

    SET_TI_TRUE_FALSE(isMatrix2dVector(h->matrixRows, h->matrixColumns));
  }
  else {
    compareTypeErrorX();
  }
}

void fnCheckIsVect3d (uint16_t unusedButMandatoryParameter) {
  if(checkXisType(dtReal34Matrix)) {
    const matrixHeader_t *h = REGISTER_MATRIX_HEADER(REGISTER_X);

    SET_TI_TRUE_FALSE(isMatrix3dVector(h->matrixRows, h->matrixColumns));
  }
  else {
    compareTypeErrorX();
  }
}


static uint32_t matrixXNumElem(void) {
  return REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows * (uint32_t)REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns;
}

static void specialRealCheck(int (*checkFn)(const real34_t *)) {

  int check = 0;
  uint32_t elements, i;

  switch(getRegisterDataType(REGISTER_X)) {
    default:
      compareTypeErrorX();
      return;
    case dtComplex34:
      check = checkFn(REGISTER_IMAG34_DATA(REGISTER_X));
      /* FALL THROUGH */
    case dtTime:
    case dtDate:
    case dtReal34:
      check |= checkFn(REGISTER_REAL34_DATA(REGISTER_X));
      break;

    case dtReal34Matrix: {
      const real34_t *r = REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_X);

      elements = matrixXNumElem();
      for(i = 0; i < elements; ++i) {
        if(checkFn(r + i)) {
          check = 1;
          break;
        }
      }
      break;
    }

    case dtComplex34Matrix: {
      const complex34_t *cpx = REGISTER_COMPLEX34_MATRIX_ELEMENTS(REGISTER_X);

      elements = matrixXNumElem();
      for(i = 0; i < elements; ++i) {
        if(checkFn(&cpx[i].real) || checkFn(&cpx[i].imag)) {
          check = 1;
          break;
        }
      }
      break;
    }
  }
  SET_TI_TRUE_FALSE(check);
}

static int wrapperDecQuadIsNaN(const real34_t *r) {
    return (int)decQuadIsNaN(r);
}

static int wrapperDecQuadIsInfinite(const real34_t *r) {
    return (int)decQuadIsInfinite(r);
}

static int wrapperCheckRealSpecial(const real34_t *r) {
    return wrapperDecQuadIsNaN(r) || wrapperDecQuadIsInfinite(r);
}

void fnCheckNaN(uint16_t unusedButMandatoryParameter) {
    specialRealCheck(wrapperDecQuadIsNaN);
}

void fnCheckInfinite(uint16_t unusedButMandatoryParameter) {
    specialRealCheck(wrapperDecQuadIsInfinite);
}

void fnCheckSpecial(uint16_t unusedButMandatoryParameter) {
    specialRealCheck(wrapperCheckRealSpecial);
}

static void zeroCheck(int neg) {
  int check = 0;

  switch(getRegisterDataType(REGISTER_X)) {
    default:
      compareTypeErrorX();
      break;

    case dtLongInteger:
      if(!neg) {
        longInteger_t val;

        convertLongIntegerRegisterToLongInteger(REGISTER_X, val);
        check = longIntegerIsZero(val);
        longIntegerFree(val);
      }
      break;

    case dtShortInteger: {
      uint64_t u64;
      int16_t sign16;

      convertShortIntegerRegisterToUInt64(REGISTER_X, &sign16, &u64);
      check = u64 == 0 && sign16 == neg;
      break;
    }

    case dtComplex34: {
      const complex34_t *cpx = REGISTER_COMPLEX34_MATRIX_ELEMENTS(REGISTER_X);

      check = real34IsZero(&cpx->real) && real34IsZero(&cpx->imag) &&
              (real34IsNegative(&cpx->real) == neg || real34IsNegative(&cpx->imag) == neg);
      break;
    }

    case dtTime:
    case dtDate:
    case dtReal34:
      check = real34IsNegative(REGISTER_REAL34_DATA(REGISTER_X)) == neg && real34IsZero(REGISTER_REAL34_DATA(REGISTER_X));
      break;
  }
  SET_TI_TRUE_FALSE(check);
}

void fnCheckPlusZero(uint16_t unusedButMandatoryParameter) {
  zeroCheck(0);
}

void fnCheckMinusZero(uint16_t unusedButMandatoryParameter) {
  zeroCheck(1);
}




// GETTYPE return values
//
// Type              Value      Notes
// ───────────────────────────────────────────────────────────────────────
// Long Integer      0          (integer)
// Real              1.0–1.5    RECT=1.0; +angle suffix/10 (see below)
// Complex           2.0–2.5    RECT=2.0; +angle suffix/10 (see below)
// Time              3          (integer)
// Date              4          (integer)
// String            5          (integer)
// Config            9          (integer)
// ───────────────────────────────────────────────────────────────────────
// 6 Real Matrix    .  0rect    0rect     0nonVector/1row/2col : +0 is Rectangular suffix/10 (see below); +0 is Rectangular +PolRec/100; +T/1000 (see below)
// 6 Real Matrix    .  0rect    2rect2D   0nonVector/1row/2col : +0 is Rectangular suffix/10 (see below); +2 is 2D Vector   +PolRec/100; +T/1000 (see below)
// 6 Real Matrix    .  0rect    3rect3D   0nonVector/1row/2col : +0 is Rectangular suffix/10 (see below); +3 is 2D Vector   +PolRec/100; +T/1000 (see below)
// 6 Real Matrix    .  +angle   2polar2D  0nonVector/1row/2col : +angle suffix/10 (see below); +2 is 2D vector Polar        +PolRec/100; +T/1000 (see below)
// 6 Real Matrix    .  +angle   3SPH3D    0nonVector/1row/2col : +angle suffix/10 (see below); +3 is 3D vector Spherical    +PolRec/100; +T/1000 (see below)
// 6 Real Matrix    .  +angle   4CYL3D    0nonVector/1row/2col : +angle suffix/10 (see below); +4 is 3D vector Cylindrical  +PolRec/100; +T/1000 (see below)
// ───────────────────────────────────────────────────────────────────────
// 7 Complex Matrix .  0rect    0rect     0nonVector/1row/2col : +0 is Rectangular suffix/10 (see below); +0 is Rectangular +PolRec/100; +T/1000 (see below)
// 7 Complex Matrix .  +angle   1polar    0nonVector/1row/2col : +angle suffix/10 (see below); +1 is Polar                  +PolRec/100; +T/1000 (see below)
// ───────────────────────────────────────────────────────────────────────
// Short Integer     8.bb       bb=base (e.g. 02=binary, 16=hex)
// ───────────────────────────────────────────────────────────────────────

// Angle suffix (tenths for Real/Complex AND for ComplexMatrix/vectors):
//   RECT / no angle   +0
//   Multi-Pi          +1
//   DMS               +2
//   Degrees           +3
//   Grad              +4
//   Radians           +5

// PolRec suffix (hundredths for polar / rect options:
// +0 is Rect
// +1 is Polar
// +2 is 2D vector Polar
// +3 is 3D vector Spherical
// +4 is 3D vector Cylindrical

// +T/1000 means
// T=0=Non-Vector
// T=1=RowVector
// T=2=ColumnVector


static void _pushIntOut(int dtp) {
  longInteger_t lgInt;
  longIntegerInit(lgInt);
  uInt32ToLongInteger(dtp, lgInt);
  setSystemFlag(FLAG_ASLIFT); // 5
  liftStack();
  convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
  longIntegerFree(lgInt);
  setSystemFlag(FLAG_ASLIFT);
}

static void _pushRealOut(real34_t* rr) {
  setSystemFlag(FLAG_ASLIFT); // 5
  liftStack();
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  real34Copy(rr, REGISTER_REAL34_DATA(REGISTER_X));
  setSystemFlag(FLAG_ASLIFT);
}


void fnGetType(uint16_t unusedButMandatoryParameter) {
  real34_t realOut;
  const int dtp = getRegisterDataType(REGISTER_X);

  int dam = getRegisterAngularMode(REGISTER_X);          // rad=0 grad=1 deg=2 dms=3 mulpi=4 rect=5
  switch(dtp) {
    case dtLongInteger:
    case dtTime:
    case dtDate:
    case dtString:
    case dtReal34Matrix:
    case dtConfig: {
      if(isRegisterMatrixVector(REGISTER_X)) {
        int angle  = 5 - (dam & 0x07);                  // 0=RECT 1=MulPi 2=DMS 3=Deg 4=Grad 5=Rad
        int isCol  = REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows > 1 && REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns == 1;
        int isRow  = REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows == 1 && REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns > 1;
        int T      = isCol ? 2 : isRow ? 1 : 0;         // 0=non-vec 1=row 2=col
        int polRec;
        if(isRegisterMatrix2dVector(REGISTER_X)) {
          polRec = 2;                                    // 2D vector POLAR or RECT
        }
        else if(isRegisterMatrix3dVector(REGISTER_X)) {
          int polarMode = getVectorRegisterPolarMode(REGISTER_X);
          polRec = (polarMode == amPolarCYL) ? 4 : 3;    // CYL=4, SPH/RECT=3
        }
        else {
          polRec = 0;                                    // 1D vector
        }
        uInt32ToReal34(dtp*1000 + 100*angle + 10*polRec + T, &realOut);
      }
      else if(dtp == dtReal34Matrix) {
        int isCol = REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows > 1 && REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns == 1;
        int isRow = REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows == 1 && REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns > 1;
        if(isCol || isRow) {
          uInt32ToReal34(dtp*1000 + (isCol ? 2 : 1), &realOut);  // 1D row/col: angle=0 polRec=0
        }
        else {
          _pushIntOut(dtp);
          break;
        }
      }
      else {
        _pushIntOut(dtp);
        break;
      }
      real34Divide(&realOut, const34_1000, &realOut);
      _pushRealOut(&realOut);
      break;
    }
    case dtComplex34Matrix: {
      int isPolar = getComplexRegisterPolarMode(REGISTER_X); //  (dam & 0x07) != 5;                  // RECT=5; any other angle mode = polar
      int angle   = isPolar ? (5 - (dam & 0x07)) : 0;   // 0=RECT 1=MulPi 2=DMS 3=Deg 4=Grad 5=Rad
      int isCol   = REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows > 1 && REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns == 1;
      int isRow   = REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows == 1 && REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns > 1;
      int T       = isCol ? 2 : isRow ? 1 : 0;          // 0=non-vec 1=row 2=col
      int polRec  = isPolar ? 1 : 0;                    // type 7: 0=RECT 1=POLAR only
      uInt32ToReal34(dtp*1000 + 100*angle + 10*polRec + T, &realOut);
      real34Divide(&realOut, const34_1000, &realOut);
      _pushRealOut(&realOut);
      break;
    }
    case dtShortInteger:
    case dtReal34:
    case dtComplex34: {
      uint32_t v = (dtp == dtShortInteger) ? 10*(dtp*100 + (dam & 0x1F))  // 8.bb: bb=base, e.g. 8.16=hex
                                           : 100*(dtp*10 + 5 - (dam & 0x07)); // 1.3=Degrees, 2.0=RECT etc.
      uInt32ToReal34(v, &realOut);
      real34Divide(&realOut, const34_1000, &realOut);
      _pushRealOut(&realOut);
      break;
    }
    default:;
  }
  temporaryInformation = TI_REGTYPE;
}

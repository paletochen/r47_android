// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file reToCx.c
 ***********************************************/

#include "c47.h"

/********************************************//**
 * \brief regX ==> regL and regY+i*regX ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnReToCx(uint16_t unusedButMandatoryParameter) {
  angularMode_t tempAngle = currentAngularMode;
  uint32_t dataTypeX = getRegisterDataType(REGISTER_X);
  uint32_t dataTypeY = getRegisterDataType(REGISTER_Y);
  bool_t xIsAReal;

  if(    (dataTypeX == dtReal34 || dataTypeX == dtLongInteger)
      && (dataTypeY == dtReal34 || dataTypeY == dtLongInteger)) {

    if(!saveLastX()) {
    return;
  }
    fnSetFlag(FLAG_CPXRES);
    fnRefreshState();                                 //drJM

    xIsAReal = true;
    if(getSystemFlag(FLAG_POLAR)) { // polar mode
      if(dataTypeX == dtReal34 && getRegisterAngularMode(REGISTER_X) != amNone) {
        tempAngle = getRegisterAngularMode(REGISTER_X);
        convertAngle34FromTo(REGISTER_REAL34_DATA(REGISTER_X), getRegisterAngularMode(REGISTER_X), amRadian);
        setRegisterAngularMode(REGISTER_X, amNone);
        xIsAReal = false;
      }
    }

    if(dataTypeX == dtLongInteger) {
      convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
    }

    if(dataTypeY == dtLongInteger) {
      convertLongIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
    }

    complex34_t temp;

    real34Copy(REGISTER_REAL34_DATA(REGISTER_Y), &temp);
    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), VARIABLE_IMAG34_DATA(&temp));
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);

    if(getSystemFlag(FLAG_POLAR)) { // polar mode
      if(real34CompareEqual(VARIABLE_REAL34_DATA(&temp), const34_0)) {
        real34Zero(REGISTER_REAL34_DATA(REGISTER_X));
        real34Zero(REGISTER_IMAG34_DATA(REGISTER_X));
      }
      else {
        real_t magnitude, theta;

        real34ToReal(VARIABLE_REAL34_DATA(&temp), &magnitude);
        real34ToReal(VARIABLE_IMAG34_DATA(&temp), &theta);
        if(xIsAReal) {
          convertAngleFromTo(&theta, currentAngularMode, amRadian, &ctxtReal39);
        }
        if(realCompareLessThan(&magnitude, const_0)) {
          realSetPositiveSign(&magnitude);
          realAdd(&theta, const_pi, &theta, &ctxtReal39);
          mod2Pi(&theta, &theta, &ctxtReal39);
        }
        realPolarToRectangular(&magnitude, &theta, &magnitude, &theta, &ctxtReal39); // theta in radian
        convertRealToReal34ResultRegister(&magnitude, REGISTER_X);
        realToReal34(&theta,     REGISTER_IMAG34_DATA(REGISTER_X));
        setComplexRegisterAngularMode(REGISTER_X, tempAngle);
      }
    }
    else { // rectangular mode
      complex34Copy(&temp, REGISTER_COMPLEX34_DATA(REGISTER_X));
    }

    fnDropY(NOPARAM);
    if(lastErrorCode == ERROR_NONE) {
      setSystemFlag(FLAG_ASLIFT);
    }
  }
  else if(dataTypeX == dtReal34Matrix && dataTypeY == dtReal34Matrix) {
    real34Matrix_t rMat, iMat;
    complex34Matrix_t cMat;

    linkToRealMatrixRegister(REGISTER_Y, &rMat);
    convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &iMat);

    if(rMat.header.matrixRows == iMat.header.matrixRows && rMat.header.matrixColumns == iMat.header.matrixColumns) {
      if(complexMatrixInit(&cMat, rMat.header.matrixRows, rMat.header.matrixColumns)) {
        if(!saveLastX()) {
          return;
        }
        fnSetFlag(FLAG_CPXRES);

        for(uint16_t i = 0; i < rMat.header.matrixRows * rMat.header.matrixColumns; ++i) {
          real34Copy(&rMat.matrixElements[i], VARIABLE_REAL34_DATA(&cMat.matrixElements[i]));
          real34Copy(&iMat.matrixElements[i], VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]));

          if(getSystemFlag(FLAG_POLAR)) { // polar mode
            if(real34CompareEqual(VARIABLE_REAL34_DATA(&cMat.matrixElements[i]), const34_0)) {
              real34Zero(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]));
            }
            else {
              real_t magnitude, theta;

              real34ToReal(VARIABLE_REAL34_DATA(&cMat.matrixElements[i]), &magnitude);
              real34ToReal(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]), &theta);
              convertAngleFromTo(&theta, currentAngularMode, amRadian, &ctxtReal39);
              if(realCompareLessThan(&magnitude, const_0)) {
                realSetPositiveSign(&magnitude);
                realAdd(&theta, const_pi, &theta, &ctxtReal39);
                mod2Pi(&theta, &theta, &ctxtReal39);
              }
              realPolarToRectangular(&magnitude, &theta, &magnitude, &theta, &ctxtReal39); // theta in radian
              realToReal34(&magnitude, VARIABLE_REAL34_DATA(&cMat.matrixElements[i]));
              realToReal34(&theta,     VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]));
            }
          }
        }
        convertComplex34MatrixToComplex34MatrixRegister(&cMat, REGISTER_X);
        complexMatrixFree(&cMat);
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      }
    }
    else {
      displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot Re->Cx %" PRIu16 STD_CROSS "%" PRIu16 "-matrix and %" PRIu16 STD_CROSS "%" PRIu16" -matrix",
                rMat.header.matrixRows, rMat.header.matrixColumns,
                iMat.header.matrixRows, iMat.header.matrixColumns);
        moreInfoOnError("In function fnReToCx:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }

    realMatrixFree(&iMat);
    if(lastErrorCode == ERROR_NONE) {
      fnDropY(NOPARAM);
      setSystemFlag(FLAG_ASLIFT);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X); // Invalid input data type for this operation
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "You cannot use Re->Cx with %s in X and %s in Y!", getDataTypeName(getRegisterDataType(REGISTER_X), true, false), getDataTypeName(getRegisterDataType(REGISTER_Y), true, false));
      moreInfoOnError("In function fnReToCx:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

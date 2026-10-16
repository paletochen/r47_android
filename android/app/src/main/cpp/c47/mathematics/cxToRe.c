// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cxToRe.c
 ***********************************************/

#include "c47.h"


/**
 * \brief Converts rectangular Real34 components to polar Real34 values in a
 * selected angular unit.
 *
 * \param[in] real34 Real component
 * \param[in] imag34 Imaginary component
 * \param[out] magnitude34 Magnitude
 * \param[out] theta34 Angle in angularMode
 * \param[in] angularMode Unit for theta34
 */
static void rectangularToPolar34InUnit(const real34_t *real34, const real34_t *imag34,
                                       real34_t *magnitude34, real34_t *theta34,
                                       angularMode_t angularMode) {
  cplx_t z;
  REAL_T_PTR(magnitude, 39);
  REAL_T_PTR(theta, 39);

  real34ToReal(real34, &z.Real);
  real34ToReal(imag34, &z.Imag);
  realRectangularToPolar(&z, magnitude, theta, &ctxtReal39);
  convertAngleFromTo(theta, amRadian, angularMode, &ctxtReal39);
  realToReal34(magnitude, magnitude34);
  realToReal34(theta, theta34);
}

/********************************************//**
 * \brief regX ==> regL and re(regX) ==> regY, im(regX) ==> regX or magnitude(regX) ==> regY, angle(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCxToRe(uint16_t unusedButMandatoryParameter) {
  uint32_t dataTypeX = getRegisterDataType(REGISTER_X);
  angularMode_t tempAngle = currentAngularMode;

  if(dataTypeX == dtComplex34) {
    if(!saveLastX()) {
    return;
  }
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);

    setSystemFlag(FLAG_ASLIFT);
    if(getComplexRegisterPolarMode(REGISTER_L)) { // polar mode
      if(getComplexRegisterAngularMode(REGISTER_L) != amNone) {
        tempAngle = getComplexRegisterAngularMode(REGISTER_L);
      }
      liftStack();
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      rectangularToPolar34InUnit(REGISTER_REAL34_DATA(REGISTER_L), REGISTER_IMAG34_DATA(REGISTER_L),
                                 REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), tempAngle);
      setRegisterAngularMode(REGISTER_X, tempAngle);
      temporaryInformation = TI_THETA_RADIUS;
    }
    else { // rectangular mode
      real34Copy(REGISTER_REAL34_DATA(REGISTER_L), REGISTER_REAL34_DATA(REGISTER_X));
      liftStack();
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      real34Copy(REGISTER_IMAG34_DATA(REGISTER_L), REGISTER_REAL34_DATA(REGISTER_X));
      temporaryInformation = TI_RE_IM;
    }
  }

  else if(dataTypeX == dtComplex34Matrix) {
    complex34Matrix_t cMat;
    real34Matrix_t rMat, iMat;

    if(!saveLastX()) {
    return;
  }

    linkToComplexMatrixRegister(REGISTER_X, &cMat);
    if(realMatrixInit(&rMat, cMat.header.matrixRows, cMat.header.matrixColumns)) {
      if(realMatrixInit(&iMat, cMat.header.matrixRows, cMat.header.matrixColumns)) {
        for(uint16_t i = 0; i < cMat.header.matrixRows * cMat.header.matrixColumns; ++i) {
          if(getSystemFlag(FLAG_POLAR)) { // polar mode
            rectangularToPolar34InUnit(VARIABLE_REAL34_DATA(&cMat.matrixElements[i]), VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]),
                                       &rMat.matrixElements[i], &iMat.matrixElements[i], currentAngularMode);
          }
          else { // rectangular mode
            real34Copy(VARIABLE_REAL34_DATA(&cMat.matrixElements[i]), &rMat.matrixElements[i]);
            real34Copy(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]), &iMat.matrixElements[i]);
          }
        }

        setSystemFlag(FLAG_ASLIFT);
        liftStack();
        convertReal34MatrixToReal34MatrixRegister(&rMat, REGISTER_Y);
        convertReal34MatrixToReal34MatrixRegister(&iMat, REGISTER_X);
        realMatrixFree(&iMat);
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      }
      realMatrixFree(&rMat);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    }
  }

  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE); // Invalid input data type for this operation
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "You cannot use Cx->Re with %s in X!", getDataTypeName(getRegisterDataType(REGISTER_X), true, false));
      moreInfoOnError("In function fnCxToRe:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file matrix.h
 ***********************************************/
#if !defined(MATRIX_H)
  #define MATRIX_H

  /**
   * Creates new Matrix of size y->m x x ->n.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnNewMatrix                    (uint16_t unusedParamButMandatory);

  /**
   * (Re-)dimension matrix X.
   *
   * \param[in] regist
   */
  bool_t     getDimensionArg                (uint32_t *rows, uint32_t *cols);
  void       fnSetMatrixDimensions          (uint16_t regist);
  void       fnSetMatrixDimensionsGr        (uint16_t regist);

  /**
   * Get dimensions of matrix X.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnGetMatrixDimensions          (uint16_t unusedParamButMandatory);

  /**
   * Transpose matrix.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnTranspose                    (uint16_t unusedParamButMandatory);

  /**
   * LU decomposition.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnLuDecomposition              (uint16_t unusedParamButMandatory);

  /**
   * Determinant.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnDeterminant                  (uint16_t unusedParamButMandatory);

  /**
   * Invert a square matrix.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnInvertMatrix                 (uint16_t unusedParamButMandatory);

  /**
   * Euclidean norm of matrix X.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnEuclideanNorm                (uint16_t unusedParamButMandatory);
  void       fnVectorDist                   (uint16_t unusedParamButMandatory);

  /**
   * Row sum of matrix X.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnRowSum                       (uint16_t unusedParamButMandatory);

  /**
   * Row norm of matrix X.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnRowNorm                      (uint16_t unusedParamButMandatory);


  /**
   * Angle between vectors X and Y.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnVectorAngle                  (uint16_t unusedParamButMandatory);


  /**
   * Index a named matrix.
   *
   * \param[in] regist
   */
  bool_t     isMatrixIndexed                (void);
  void       fnIndexMatrix                  (uint16_t regist);

  /**
   * Get submatrix of the indexed matrix.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnGetMatrix                    (uint16_t unusedParamButMandatory);

  /**
   * Put submatrix to the indexed matrix.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnPutMatrix                    (uint16_t unusedParamButMandatory);

  /**
   * Swap rows of the indexed matrix.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnSwapRows                     (uint16_t unusedParamButMandatory);

  /**
   * Initialize simultaneous linear equation solver.
   *
   * \param[in] numberOfUnknowns
   */
  void       fnSimultaneousLinearEquation   (uint16_t numberOfUnknowns);
  void       fnEditLinearEquationMatrixA    (uint16_t unusedParamButMandatory);
  void       fnEditLinearEquationMatrixB    (uint16_t unusedParamButMandatory);
  void       fnEditLinearEquationMatrixX    (uint16_t unusedParamButMandatory);

  /**
   * QR decomposition. Square matrices only.
   * Returns unitary matrix Q in Y and upper triangular matrix R in X.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnQrDecomposition              (uint16_t unusedParamButMandatory);

  /**
   * Calculate eigenvalues.
   * Generally returns in a diagonal matrix.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnEigenvalues                  (uint16_t unusedParamButMandatory);

  void       fnEigenvectors                 (uint16_t unusedParamButMandatory);

  /**
   * Saves the STATS matrix if STATS is available.
   *
   * \return true if succeeded or not needed to backup, false if unsuccessful allocation
   */
  bool_t   saveStatsMatrix                (void);

  /**
   * Restores the STATS matrix if backed up STATS is available.
   *
   * \return true if succeeded, otherwise false
   */
  bool_t   recallStatsMatrix              (void);

  /**
   * Initialize a real matrix.
   *
   * \param[out] matrix
   * \param[in] rows
   * \param[in] cols
   * \return true if succeeded, false otherwise
   */
  bool_t   realMatrixInit                 (real34Matrix_t *matrix, uint16_t rows, uint16_t cols);

  /**
   * Free a real matrix.
   *
   * \param[in] matrix
   */
  void     realMatrixFree                 (real34Matrix_t *matrix);

  /**
   * Returns identity matrix of given size.
   *
   * \param[in] matrix
   */
  void     realMatrixIdentity             (real34Matrix_t *matrix, uint16_t size);

  /**
   * Redemention a real matrix.
   *
   * \param[in] matrix
   */
  void     realMatrixRedim                (real34Matrix_t *matrix, uint16_t rows, uint16_t cols);

  /**
   * Initialize a complex matrix.
   *
   * \param[out] matrix
   * \param[in] rows
   * \param[in] cols
   * \return true if succeeded, false otherwise
   */
  bool_t   complexMatrixInit              (complex34Matrix_t *matrix, uint16_t rows, uint16_t cols);

  /**
   * Free a complex matrix.
   *
   * \param[in] matrix
   */
  void     complexMatrixFree              (complex34Matrix_t *matrix);

  /**
   * Returns identity matrix of given size in complex34Matrix_t.
   *
   * \param[in] matrix
   */
  void     complexMatrixIdentity          (complex34Matrix_t *matrix, uint16_t size);

  /**
   * Redemention a complex matrix.
   *
   * \param[in] matrix
   */
  void     complexMatrixRedim             (complex34Matrix_t *matrix, uint16_t rows, uint16_t cols);

  /**
   * Displays the matrix editor.
   */
  void     showMatrixEditor               (void);
  void     mimEnter                       (bool_t commit);
  void     mimAddNumber                   (int16_t item);
  void     mimRunFunction                 (int16_t func, uint16_t param);
  void     mimFinalize                    (void);
  void     mimRestore                     (void);

  /**
   * Displays a real matrix.
   *
   * \param[in] matrix
   * \param[in] prefixWidth
   */
  #define toDisplayVectorMatrix true
  void     showRealMatrix                 (const real34Matrix_t *matrix, int16_t prefixWidth, bool_t toDisplay);

  /**
   * Calculates width of columns of a real matrix.
   *
   * \param[in] matrix
   * \param[in] prefixWidth
   * \param[in] font
   * \param[out] colWidth Column width. This must be initialized as int16_t[MATRIX_MAX_COLUMNS].
   * \param[out] rPadWidth Right padding of each elements. This is for alignment of decimal points. This must be initialized as int16_t[MATRIX_MAX_ROWS * MATRIX_MAX_COLUMNS].
   * \param[out] digits Number of digits which will be shown. This is for adjustment in ALL mode.
   * \param[in] maxCols Maximum number of columns to allow
   * \return Width of the matrix excluding brackets
   */
  int16_t  getRealMatrixColumnWidths      (const real34Matrix_t *matrix, int16_t prefixWidth, const font_t *font, int16_t *colWidth, int16_t *rPadWidth, int16_t *digits, uint16_t maxCols, bool_t *allElementsInColAreIntegers);

  /**
   * Displays a complex matrix.
   *
   * \param[in] matrix
   * \param[in] prefixWidth
   */
  void     showComplexMatrix              (const complex34Matrix_t *matrix, int16_t prefixWidth, angularMode_t angleMode, bool_t polarMode);

  /**
   * Calculates width of columns of a complex matrix.
   *
   * \param[in] matrix
   * \param[in] prefixWidth
   * \param[in] font
   * \param[out] colWidth Column width. This must be initialized as int16_t[MATRIX_MAX_COLUMNS].
   * \param[out] colWidth_r Column width of real part. This must be initialized as int16_t[MATRIX_MAX_COLUMNS].
   * \param[out] colWidth_i Column width of imaginary part. This must be initialized as int16_t[MATRIX_MAX_COLUMNS].
   * \param[out] rPadWidth_r Right padding of real part of each elements. This is for alignment of decimal points. This must be initialized as int16_t[MATRIX_MAX_ROWS * MATRIX_MAX_COLUMNS].
   * \param[out] rPadWidth_i Right padding of imaginary part of each elements. This is for alignment of decimal points. This must be initialized as int16_t[MATRIX_MAX_ROWS * MATRIX_MAX_COLUMNS].
   * \param[out] digits Number of digits which will be shown. This is for adjustment in ALL mode.
   * \param[in] maxCols Maximum number of columns to allow
   * \return Width of the matrix excluding brackets
   */
  int16_t  getComplexMatrixColumnWidths   (const complex34Matrix_t *matrix, int16_t prefixWidth, const font_t *font, int16_t *colWidth, int16_t *colWidth_r, int16_t *colWidth_i,
                                            int16_t *rPadWidth_r, int16_t *rPadWidth_i, int16_t *digits, uint16_t maxCols, angularMode_t angleMode, bool_t polarMode);

  void     getMatrixFromRegister          (calcRegister_t regist);

  /**
   * Creates a zero matrix at given register.
   *
   * \param[in] regist
   * \param[in] rows
   * \param[in] cols
   * \param[in] complex  true for complex matrix, false for real matrix.
   * \return true if succeeded, false otherwise
   */
  bool_t   initMatrixRegister             (calcRegister_t regist, uint16_t rows, uint16_t cols, bool_t complex);

  /**
   * Redimentions the matrix at given register.
   * Shrinking the matrix is in situ while enlarging the matrix is not.
   * The elements are preserved as many as possible.
   *
   * \warning This function invalidates variables assosiated by \link linkToRealMatrixRegister() \endlink/\link linkToComplexMatrixRegister() \endlink
   *          if you are making the matrix larger.
   * \warning Redo \link linkToRealMatrixRegister() \endlink/\link linkToComplexMatrixRegister() \endlink to use the variables again.
   * \param[in] regist
   * \param[in] rows
   * \param[in] cols
   * \return true if succeeded, false otherwise
   */
  bool_t   redimMatrixRegister            (calcRegister_t regist, uint16_t rows, uint16_t cols, uint16_t dimMode);

  /**
   * Allocates a named matrix. Redimentions if the matrix already existed.
   *
   * \param[in] name
   * \param[in] rows
   * \param[in] cols
   * \return register ID where the allocated matrix lies. \c INVALID_VARIABLE if allocation failed.
   */
  calcRegister_t allocateNamedMatrix      (const char *name, uint16_t rows, uint16_t cols);

  /**
   * Appends a row to the matrix at given register.
   * For named matrix, use together with findNamedVariable().
   *
   * \warning This function invalidates variables assosiated by \link linkToRealMatrixRegister() \endlink/\link linkToComplexMatrixRegister() \endlink.
   * \warning Redo \link linkToRealMatrixRegister() \endlink/\link linkToComplexMatrixRegister() \endlink to use the variables again.
   * \param[in] regist
   * \return true if succeeded, false otherwise
   */
  bool_t   appendRowAtMatrixRegister      (calcRegister_t regist);

  void     copyRealMatrix                 (const real34Matrix_t *matrix, real34Matrix_t *res);

  void     insRowRealMatrix               (real34Matrix_t *matrix, uint16_t beforeRowNo, bool_t add);
  void     delRowRealMatrix               (real34Matrix_t *matrix, uint16_t beforeRowNo);
  void     insColRealMatrix               (real34Matrix_t *matrix, uint16_t beforeRowNo, bool_t add);
  void     delColRealMatrix               (real34Matrix_t *matrix, uint16_t beforeRowNo);
  void     transposeRealMatrix            (const real34Matrix_t *matrix, real34Matrix_t *res);

  void     copyComplexMatrix              (const complex34Matrix_t *matrix, complex34Matrix_t *res);

  void     insRowComplexMatrix            (complex34Matrix_t *matrix, uint16_t beforeRowNo, bool_t add);
  void     delRowComplexMatrix            (complex34Matrix_t *matrix, uint16_t beforeRowNo);
  void     insColComplexMatrix            (complex34Matrix_t *matrix, uint16_t beforeRowNo, bool_t add);
  void     delColComplexMatrix            (complex34Matrix_t *matrix, uint16_t beforeRowNo);
  void     transposeComplexMatrix         (const complex34Matrix_t *matrix, complex34Matrix_t *res);

  void     addRealMatrices                (const real34Matrix_t *y, const real34Matrix_t *x, real34Matrix_t *res);
  void     subtractRealMatrices           (const real34Matrix_t *y, const real34Matrix_t *x, real34Matrix_t *res);

  void     addComplexMatrices             (const complex34Matrix_t *y, const complex34Matrix_t *x, complex34Matrix_t *res);
  void     subtractComplexMatrices        (const complex34Matrix_t *y, const complex34Matrix_t *x, complex34Matrix_t *res);

  void     multiplyRealMatrix             (const real34Matrix_t *matrix, const real34_t *x, real34Matrix_t *res);
  void     _multiplyRealMatrix            (const real34Matrix_t *matrix, const real_t *x, real34Matrix_t *res, realContext_t *realContext);
  void     multiplyRealMatrices           (const real34Matrix_t *y, const real34Matrix_t *x, real34Matrix_t *res);

  void     multiplyComplexMatrix          (const complex34Matrix_t *matrix, const real34_t *xr, const real34_t *xi, complex34Matrix_t *res);
  void     _multiplyComplexMatrix         (const complex34Matrix_t *matrix, const real_t *xr, const real_t *xi, complex34Matrix_t *res, realContext_t *realContext);
  void     multiplyComplexMatrices        (const complex34Matrix_t *y, const complex34Matrix_t *x, complex34Matrix_t *res);

  void     euclideanNormRealMatrix        (const real34Matrix_t *matrix, real34_t *res);
  void     euclideanNormComplexMatrix     (const complex34Matrix_t *matrix, real34_t *res);

  uint16_t realVectorSize                 (const real34Matrix_t *matrix);
  void     dotRealVectors                 (const real34Matrix_t *y, const real34Matrix_t *x, real34_t *res);
  void     crossRealVectors               (const real34Matrix_t *y, const real34Matrix_t *x, real34Matrix_t *res);

  uint16_t complexVectorSize              (const complex34Matrix_t *matrix);
  void     dotComplexVectors              (const complex34Matrix_t *y, const complex34Matrix_t *x, real34_t *res_r, real34_t *res_i);
  void     crossComplexVectors            (const complex34Matrix_t *y, const complex34Matrix_t *x, complex34Matrix_t *res);

  void     vectorAngle                    (const real34Matrix_t *y, const real34Matrix_t *x, real34_t *radians);

  void     WP34S_LU_decomposition         (const real34Matrix_t *matrix, real34Matrix_t *lu, uint16_t *p);
  void     realMatrixSwapRows             (const real34Matrix_t *matrix, real34Matrix_t *res, uint16_t a, uint16_t b);
  void     detRealMatrix                  (const real34Matrix_t *matrix, real34_t *res);
  void     invertRealMatrix               (const real34Matrix_t *matrix, real34Matrix_t *res);
  void     divideRealMatrix               (const real34Matrix_t *matrix, const real34_t *x, real34Matrix_t *res);
  void     _divideRealMatrix              (const real34Matrix_t *matrix, const real_t *x, real34Matrix_t *res, realContext_t *realContext);
  void     divideByRealMatrix             (const real34_t *y, const real34Matrix_t *matrix, real34Matrix_t *res);
  void     _divideByRealMatrix            (const real_t *y, const real34Matrix_t *matrix, real34Matrix_t *res, realContext_t *realContext);
  void     divideRealMatrices             (const real34Matrix_t *y, const real34Matrix_t *x, real34Matrix_t *res);

  void     complex_LU_decomposition       (const complex34Matrix_t *matrix, complex34Matrix_t *lu, uint16_t *p);
  void     complexMatrixSwapRows          (const complex34Matrix_t *matrix, complex34Matrix_t *res, uint16_t a, uint16_t b);
  void     detComplexMatrix               (const complex34Matrix_t *matrix, real34_t *res_r, real34_t *res_i);
  void     invertComplexMatrix            (const complex34Matrix_t *matrix, complex34Matrix_t *res);
  void     divideComplexMatrix            (const complex34Matrix_t *matrix, const real34_t *xr, const real34_t *xi, complex34Matrix_t *res);
  void     _divideComplexMatrix           (const complex34Matrix_t *matrix, const real_t *xr, const real_t *xi, complex34Matrix_t *res, realContext_t *realContext);
  void     divideByComplexMatrix          (const real34_t *yr, const real34_t *yi, const complex34Matrix_t *matrix, complex34Matrix_t *res);
  void     _divideByComplexMatrix         (const real_t *yr, const real_t *yi, const complex34Matrix_t *matrix, complex34Matrix_t *res, realContext_t *realContext);
  void     divideComplexMatrices          (const complex34Matrix_t *y, const complex34Matrix_t *x, complex34Matrix_t *res);

  void     real_matrix_linear_eqn         (const real34Matrix_t *a, const real34Matrix_t *b, real34Matrix_t *r);
  void     complex_matrix_linear_eqn      (const complex34Matrix_t *a, const complex34Matrix_t *b, complex34Matrix_t *r);

  void     real_QR_decomposition          (const real34Matrix_t *matrix, real34Matrix_t *q, real34Matrix_t *r);
  void     complex_QR_decomposition       (const complex34Matrix_t *matrix, complex34Matrix_t *q, complex34Matrix_t *r);

  void     realEigenvalues                (const real34Matrix_t *matrix, real34Matrix_t *res, real34Matrix_t *ires);
  void     complexEigenvalues             (const complex34Matrix_t *matrix, complex34Matrix_t *res);
  void     realEigenvectors               (const real34Matrix_t *matrix, real34Matrix_t *res, real34Matrix_t *ires);
  void     complexEigenvectors            (const complex34Matrix_t *matrix, complex34Matrix_t *res);
  void     callByIndexedMatrix            (bool_t (*real_f)(real34Matrix_t *), bool_t (*complex_f)(complex34Matrix_t *));

  void       linkToRealMatrixRegister       (calcRegister_t regist, real34Matrix_t *linkedMatrix);
  void       linkToComplexMatrixRegister    (calcRegister_t regist, complex34Matrix_t *linkedMatrix);

  void       elementwiseRema                (void (*f)(void));
  void       elementwiseRema_UInt16         (void (*f)(uint16_t), uint16_t param);
  void       elementwiseRemaLonI            (void (*f)(void));
  void       elementwiseRemaReal            (void (*f)(void));
  void       elementwiseRemaShoI            (void (*f)(void));
  void       elementwiseRealRema            (void (*f)(void));
  void       elementwiseRemaRema            (void (*f)(void));

  void       elementwiseCxma                (void (*f)(void));
  void       elementwiseCxma_UInt16         (void (*f)(uint16_t), uint16_t param);
  void       elementwiseCxmaLonI            (void (*f)(void));
  void       elementwiseCxmaReal            (void (*f)(void));
  void       elementwiseCxmaShoI            (void (*f)(void));
  void       elementwiseCxmaCplx            (void (*f)(void));
  void       elementwiseRealCxma            (void (*f)(void));
  void       elementwiseCplxCxma            (void (*f)(void));
  void       elementwiseCxmaCxma            (void (*f)(void));
  void       elementwiseRemaCxma            (void (*f)(void));
  void       elementwiseCxmaRema            (void (*f)(void));
  void       elementwiseCplxRema            (void (*f)(void));
  void       elementwiseRemaCplx            (void (*f)(void));

#endif // !MATRIX_H

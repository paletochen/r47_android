// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file registerValueConversions.h
 */
#if !defined(REGISTERVALUECONVERSIONS_H)
  #define REGISTERVALUECONVERSIONS_H

  void convertLongIntegerRegisterToReal34Register            (calcRegister_t source, calcRegister_t destination);
  void convertLongIntegerRegisterToShortIntegerRegister      (calcRegister_t source, calcRegister_t destination);
  void convertLongIntegerRegisterToReal34                    (calcRegister_t source, real34_t *destination);
  void convertLongIntegerRegisterToReal                      (calcRegister_t source, real_t *destination, realContext_t *ctxt);
  /********************************************//**
   * \brief Reads long integer value from a register and sets given long integer variable.
   *        The destination variable will be initialized in this function.
   * \warning Do not forget to call longIntegerFree() in order not to leak memory.
   * \param[in] regist the source register
   * \param[out] longInteger the destination value
   ***********************************************/
  void convertLongIntegerRegisterToLongInteger               (calcRegister_t regist, longInteger_t longInteger);

  void convertLongIntegerToLongIntegerRegister               (const longInteger_t longInteger, calcRegister_t regist);
  void convertLongIntegerToShortIntegerRegister              (longInteger_t lgInt, uint32_t base, calcRegister_t destination);
  void convertLongIntegerToReal                              (longInteger_t source, real_t *destination, realContext_t *ctxt);
  void convertLongIntegerToReal34                            (longInteger_t source, real34_t *destination);

  void convertShortIntegerRegisterToReal34Register           (calcRegister_t source, calcRegister_t destination);
  void convertShortIntegerRegisterToLongIntegerRegister      (calcRegister_t source, calcRegister_t destination);
  void convertShortIntegerRegisterToLongInteger              (calcRegister_t source, longInteger_t lgInt);
  void convertShortIntegerRegisterToReal                     (calcRegister_t source, real_t *destination, realContext_t *ctxt);

  void convertShortIntegerRegisterToUInt64                   (calcRegister_t regist, int16_t *sign, uint64_t *value);
  void convertUInt64ToShortIntegerRegister                   (int16_t sign, uint64_t value, uint32_t base, calcRegister_t regist);

  void convertReal34ToLongInteger                            (const real34_t *real34, longInteger_t lgInt, enum rounding mode);
  void convertReal34ToLongIntegerRegister                    (const real34_t *real34, calcRegister_t dest, enum rounding mode);
  void convertRealToLongInteger                              (const real_t *real, longInteger_t lgInt, enum rounding mode);
  void convertRealToLongIntegerRegister                      (const real_t *real, calcRegister_t dest, enum rounding mode);
  void realToIntegralValue                                   (const real_t *source, real_t *destination, enum rounding mode, realContext_t *realContext);

  /********************************************//**
   * \brief Sets function result in real type to a real34 register.
   *        This follows preferences of number of significant digits.
   *        For complex34 register, sets the real part of the register value.
   *        This function is intended to avoid error with rounding twice.
   * \warning The destination register must be initialized in advance with real34 or complex34 data type,
   *          or else breaks the data in the destination register.
   * \param[in] real the resulting value
   * \param[in] dest the destination register. Usually REGISTER_X or REGISTER_Y.
   ***********************************************/
  void convertRealToReal34ResultRegister                     (const real_t *real, calcRegister_t dest);

  /********************************************//**
   * \brief Sets function result in real type to the imaginary part of a complex34 register.
   *        This follows preferences of number of significant digits.
   *        This function is intended to avoid error with rounding twice.
   * \warning The destination register must be initialized in advance with complex34 data type,
   *          or else breaks the data in the destination register.
   * \param[in] real the resulting value
   * \param[in] dest the destination register. Usually REGISTER_X or REGISTER_Y.
   ***********************************************/
  void convertRealToImag34ResultRegister                     (const real_t *real, calcRegister_t dest);

  void convertRealToResultRegister                           (const real_t *x, calcRegister_t dest, angularMode_t angle);
  void convertComplexToResultRegister                        (const real_t *real, const real_t *imag, calcRegister_t dest);
  void convertComplexToResultRegisterRPangle                 (const real_t *real, const real_t *imag, calcRegister_t dest, angularMode_t angl, uint8_t polarTag);

  void convertTimeRegisterToReal34Register                   (calcRegister_t source, calcRegister_t destination);
  void convertReal34RegisterToTimeRegister                   (calcRegister_t source, calcRegister_t destination);
  void convertLongIntegerRegisterToTimeRegister              (calcRegister_t source, calcRegister_t destination);

  void convertDateRegisterToReal34Register                   (calcRegister_t source, calcRegister_t destination);
  #define YYSystem true
  void convertReal34RegisterToDateRegister                   (calcRegister_t source, calcRegister_t destination, bool_t handleYY);

  void convertReal34MatrixRegisterToReal34Matrix             (calcRegister_t regist, real34Matrix_t *matrix);
  void convertReal34MatrixToReal34MatrixRegister             (const real34Matrix_t *matrix, calcRegister_t regist);

  void convertComplex34MatrixRegisterToComplex34Matrix       (calcRegister_t regist, complex34Matrix_t *matrix);
  void convertComplex34MatrixToComplex34MatrixRegister       (const complex34Matrix_t *matrix, calcRegister_t regist);

  void convertReal34MatrixToComplex34Matrix                  (const real34Matrix_t *source, complex34Matrix_t *destination);
  void convertReal34MatrixRegisterToComplex34Matrix          (calcRegister_t source, complex34Matrix_t *destination);
  void convertReal34MatrixRegisterToComplex34MatrixRegister  (calcRegister_t source, calcRegister_t destination);


  //Section to convert doubles and floats
  void    convertDoubleToString                              (double x, int16_t n, char *buff);  //Reformatting double/float strings that are formatted according to different locale settings
  void    convertDoubleToReal                                (double x, real_t *destination, realContext_t *ctxt);
  void    convertDoubleToReal34Register                      (double x, calcRegister_t destination);
  void    convertDoubleToReal34RegisterPush                  (double x, calcRegister_t destination);

  void    realToFloat                                        (const real_t *vv, float *v);
  void    realToDouble  /*float used (not double)*/          (const real_t *vv, double *v);
  double  convertRegisterToDouble                            (calcRegister_t regist);
  #define DOUBLE_NOT_INIT 3.402823466e+38f //maximum float value

  void badTypeError(calcRegister_t reg);
  void badDomainError(calcRegister_t reg);

  void badTypeErrorX(void);
  void badDomainErrorX(void);

  bool_t getRegisterAsComplex(calcRegister_t reg, real_t *r, real_t *c);
  bool_t getRegisterAsComplexOrReal(calcRegister_t reg, real_t *r, real_t *c, bool_t *cmplx);
  bool_t getRegisterAsComplexOrAnyReal(calcRegister_t reg, real_t *r, real_t *i, bool_t *cmplx);
  bool_t getRegisterAsReal(calcRegister_t reg, real_t *val);
  bool_t getRegisterAsAnyReal(calcRegister_t reg, real_t *val);
  bool_t getRegisterAsReal34Quiet(calcRegister_t reg, real34_t *val);
  bool_t getRegisterAsRealAngle(calcRegister_t reg, real_t *val, angularMode_t *xAngularMode);
  bool_t getRegisterAsLongInt(calcRegister_t reg, longInteger_t val, bool_t *fractional);
  bool_t getRegisterAsShortInt(calcRegister_t reg, bool_t *sign, uint64_t *val, bool_t *overflow, bool_t *fractional);
  bool_t getRegisterAsRawShortInt(calcRegister_t reg, uint64_t *val, uint32_t *base);

  bool_t getRegisterAsRealQuiet(calcRegister_t reg, real_t *val);
  bool_t getRegisterAsAnyRealQuiet(calcRegister_t reg, real_t *val);
  bool_t getRegisterAsComplexOrRealQuiet(calcRegister_t reg, real_t *r, real_t *c, bool_t *cmplx);
  bool_t getRegisterAsComplexOrAnyRealQuiet(calcRegister_t reg, real_t *r, real_t *i, bool_t *cmplx);
  /* returns error code or ERROR_NONE if okay */
  int getRegisterAsLongIntQuiet(calcRegister_t reg, longInteger_t val, bool_t *fractional);

  void processRealComplexMonadicFunction(void (*realf)(void), void (*complexf)(void));
  void processIntRealComplexMonadicFunction(void (*realf)(void), void (*complexf)(void), void (*shortintf)(void), void (*longintf)(void));
  void processRealComplexDyadicFunction(void (*realf)(void), void (*complexf)(void));
  void processIntRealComplexDyadicFunction(void (*realf)(void), void (*complexf)(void), void (*shortintf)(void), void (*longintf)(void));
#endif // !REGISTERVALUECONVERSIONS_H

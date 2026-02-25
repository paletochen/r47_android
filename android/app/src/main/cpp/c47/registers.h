// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file registers.h
 */
#if !defined(REGISTERS_H)
  #define REGISTERS_H

  /**
   * Returns the data type of a register.
   *
   * \param[in] regist Register number
   * \return Data type
   */
  uint32_t       getRegisterDataType             (calcRegister_t regist);

  /**
   * Returns the data pointer of a register.
   *
   * \param[in] regist Register number
   * \return Data pointer
   */
  void          *getRegisterDataPointer          (calcRegister_t regist);

  // Helper macros to get register data in the appropriate format
  #define REGISTER_REAL34_DATA(a)                                ((real34_t             *)(getRegisterDataPointer(a)))
  #define REGISTER_IMAG34_DATA(a)                                ((real34_t             *)(getRegisterDataPointer(a) + REAL34_SIZE_IN_BYTES))
  #define REGISTER_COMPLEX34_DATA(a)                             ((complex34_t          *)(getRegisterDataPointer(a)))

  #define REGISTER_STRING_HEADER(a)                              ((strLgIntHeader_t     *)(getRegisterDataPointer(a)))
  #define REGISTER_STRING_DATA(a)                                ((char                 *)(getRegisterDataPointer(a) + sizeof(strLgIntHeader_t)))

  #define REGISTER_CONFIG_DATA(a)                                ((dtConfigDescriptor_t *)(getRegisterDataPointer(a)))

  #define REGISTER_MATRIX_HEADER(a)                              ((matrixHeader_t       *)(getRegisterDataPointer(a)))
  #define REGISTER_REAL34_MATRIX_ELEMENTS(a)                     ((real34_t             *)(getRegisterDataPointer(a) + sizeof(matrixHeader_t)))
  #define REGISTER_REAL34_MATRIX(a)                              ((real34Matrix_t       *)(getRegisterDataPointer(a)))
  #define REGISTER_COMPLEX34_MATRIX_ELEMENTS(a)                  ((complex34_t          *)(getRegisterDataPointer(a) + sizeof(matrixHeader_t)))
  #define REGISTER_COMPLEX34_MATRIX(a)                           ((complex34Matrix_t    *)(getRegisterDataPointer(a)))

  #define REGISTER_SHORT_INTEGER_DATA(a)                         ((uint64_t             *)(getRegisterDataPointer(a)))

  #define REGISTER_LONG_INTEGER_HEADER(a)                        ((strLgIntHeader_t     *)(getRegisterDataPointer(a)))
  #define REGISTER_LONG_INTEGER_DATA(a)                          ((uint8_t              *)(getRegisterDataPointer(a) + sizeof(strLgIntHeader_t)))

  /**
   * Returns the data information of a register.
   * This is the angular mode or base.
   *
   * \param[in] regist Register number
   * \return Angular mode
   */
  uint32_t       getRegisterTag                  (calcRegister_t regist);

  /**
   * Returns the max length of a string.
   * This is including the trailing 0, or a long integer in blocks.
   *
   * \param[in] regist Register number
   * \return Number of blocks
   */
  uint16_t       getRegisterMaxDataLengthInBlocks(calcRegister_t regist);

  /**
   * Sets the data type of a register.
   *
   * \param[in] regist   Register number
   * \param[in] dataType Data type
   * \param[in] tag      Tag
   */
  void           setRegisterDataType             (calcRegister_t regist, uint16_t dataType, uint32_t tag);

  /**
   * Sets the data pointer of a register.
   *
   * \param[in] regist Register number
   * \param[in] memPtr Data pointer
   */
  void           setRegisterDataPointer          (calcRegister_t regist, const void *memPtr);

  /**
   * Sets the data information of a register.
   * This is the angular mode or base.
   *
   * \param[in] regist Register number
   * \param[in] tag    Angular mode
   */
  void           setRegisterTag                  (calcRegister_t regist, uint32_t tag);

  /**
   * Sets the max length of string in blocks.
   *
   * \param[in] regist     Register number
   * \param[in] maxDataLen Max length of the string
   */
  void           setRegisterMaxDataLengthInBlocks(calcRegister_t regist, uint16_t maxDataLen);

  /**
   * Allocates local registers.
   * Works when increasing and when decreasing the number of local registers.
   *
   * \param[in] numberOfRegistersToAllocate Number of registers to allocate
   */
  void           allocateLocalRegisters          (uint16_t numberOfRegistersToAllocate);

  /**
   * Check if the given name follows the naming convention
   *
   * \param[in] name                 Name of variable/label/menu
   * \return `true` if given name is valid, `false` otherwise
   */
  bool_t         validateName                    (const char *name);

  /**
   * Check if the given name is not yet in use as a user menu
   *
   * \param[in] name                 Name of menu
   * \return `true` if given name is unique, `false` if duplicate
   */
  bool_t         isUniqueMenuName                (const char *name);

  /**
   * Allocates one named variable.
   *
   * \param[in] variableName         Variable name
   * \param[in] dataType             The content type of the variable
   * \param[in] fullDataSizeInBlocks How many blocks the named variable will require for storage
   */
  void           allocateNamedVariable           (const char *variableName, dataType_t dataType, uint16_t fullDataSizeInBlocks);

  /**
   * Retrieves the register number for the named variable.
   *
   * \param[in] variableName Register name
   * \return register number to be used by other functions, or INVALID_VARIABLE
   *         if not found
   */
  calcRegister_t findNamedVariable               (const char *variableName);

  /**
   * Retrieves the register number for the named variable, allocating it if it doesn't exist.
   *
   * \param[in] variableName Register name
   * \return Register number to be used by other functions, or INVALID_VARIABLE
   *         if not possible to allocate (all named variables defined)
   */
  calcRegister_t findOrAllocateNamedVariable     (const char *variableName);

  /**
   * Returns the full data size of a register in blocks.
   *
   * \param[in] regist Register number
   * \return Number of blocks. For a string this
   *         is the number of bytes reserved for
   *         the string (including the ending 0)
   *         plus 1 block holding the max size
   *         of the string.
   */
  uint16_t       getRegisterFullSizeInBlocks             (calcRegister_t regist);

  /**
   * Clears a register.
   * That is, set it to 0,0 real34.
   *
   * \param[in] regist Register number
   */
  void           clearRegister                   (calcRegister_t regist);

  /**
   * Clears all the regs.
   * This includes all globals and locals, which are set to 0,0 real34s
   *
   * \param[in] confirmation Current status of the confirmation of clearing registers
   */
  void           fnClearRegisters                (uint16_t confirmation);

  /**
   * Deletes one named variable.
   * After deleting a variable, register numbers of remaining named variables
   * will be shifted so that there is no unallocated gap.
   *
   * \param[in] regist Register
   */
  void           fnDeleteVariable                (uint16_t regist);

  /**
   * Deletes all named variable.
   * After deleting all named variables, Mat_A, Mat-B and Mat_X will be created for SIM EQ.
   *
   * \param[in] confirmation Current status of the confirmation of dleting user variables
   */
  void           fnDeleteAllVariables           (uint16_t confirmation);

  /**
   * Clear all named variable.
   * Reserved variables STATS and HISTO are cleared by a call ClSigma, Mat_A, Mat-B and Mat_X are redimmed to single elements matrices for SIM EQ.
   *
   * \param[in] confirmation Current status of the confirmation of dleting user variables
   */
  void           fnClearAllVariables            (uint16_t confirmation);

  /**
   * Sets X to the number of local registers.
   *
   * \param[in] unusedButMandatoryParameter
   */
  void           fnGetLocR                       (uint16_t unusedButMandatoryParameter);

  void           adjustResult                    (calcRegister_t result, bool_t dropY, bool_t setCpxRes, calcRegister_t op1, calcRegister_t op2, calcRegister_t op3);

  /**
   * Duplicates register source to register destination.
   *
   * \param[in] sourceRegister Source register
   * \param[in] destRegister   Destination register
   */
  void           copySourceRegisterToDestRegister(calcRegister_t rSource, calcRegister_t rDest);

  /**
   * Returns the integer part of the value of a register.
   *
   * \param regist Register
   */
  int16_t        indirectAddressing              (calcRegister_t regist, uint16_t parameterType, int16_t minValue, int16_t maxValue, bool_t tryAllocate);

  void           reallocateRegister              (calcRegister_t regist, uint32_t dataType, uint16_t dataSizeWithoutDataLenBlocks, uint32_t tag);
  void           fnToReal                        (uint16_t unusedButMandatoryParameter);

  void           printStringToConsole            (const char *str, const char *before, const char *after);
  void           printReal34ToConsole            (const real34_t *value, const char *before, const char *after);
  void           printRealToConsole              (const real_t *value, const char *before, const char *after);
  void           printComplex34ToConsole         (const complex34_t *value, const char *before, const char *after);

  /**
   * Prints the content of a long integer to the console.
   *
   * \param[in] value  long integer value to print
   * \param[in] before text to display before the value
   * \param[in] after  text to display after the value
   */
  void           printLongIntegerToConsole       (const longInteger_t value, const char *before, const char *after);

  /**
   * Prints the content of a register to the console.
   *
   * \param[in] regist register number
   * \param[in] before text to display before the register value
   * \param[in] after  text to display after the register value
   */
  void           printRegisterToConsole          (calcRegister_t regist, const char *before, const char *after);

  void           printRegisterDescriptorToConsole(calcRegister_t regist);


  #define getRegisterAngularMode(reg)            getRegisterTag(reg)
  #define setRegisterAngularMode(reg, am)        setRegisterTag(reg, am)
  #define getRegisterShortIntegerBase(reg)       getRegisterTag(reg)
  #define setRegisterShortIntegerBase(reg, base) setRegisterTag(reg, base)
  #define getRegisterLongIntegerSign(reg)        getRegisterTag(reg)
  #define setRegisterLongIntegerSign(reg, sign)  setRegisterTag(reg, sign)

  #define getComplexRegisterAngularMode(reg)     (getRegisterTag(reg) & amAngleMask)
  #define setComplexRegisterAngularMode(reg, am) setRegisterTag(reg, (am & amAngleMask) | (getRegisterTag(reg) & amPolar))    // ok. amAngleMask = 15; amPolar = 16
  #define getComplexRegisterPolarMode(reg)       (getRegisterTag(reg) & amPolar)
  #define setComplexRegisterPolarMode(reg, pm)   setRegisterTag(reg, (getRegisterTag(reg) & amAngleMask) | (pm & amPolar))    // Intended to maintain bits 0-3 for amAngle (amAngleMask), clear the polar bit 4, and then OR only the polar bit.

  #define isXYRegisterMatrix                      ((getRegisterDataType(REGISTER_X) == dtReal34Matrix) || (getRegisterDataType(REGISTER_X) == dtComplex34Matrix) || (getRegisterDataType(REGISTER_Y) == dtReal34Matrix) || (getRegisterDataType(REGISTER_X) == dtComplex34Matrix) )


  #define amPolarCYL 64  //  virtual bit, working in addition to the tag bit 4 = amPolar = 16; bit 5 usid by 32-bit pointer changes; real bits 6 & 7 spare. Real bits not used, in favour of these virtual logic bits,  as the register header also only has bits 0-4.
  #define amPolarSPH 128 //  virtual bit, see typeDefinitions.h, amPolar
  #define isRegisterMatrix3dVector(reg)          ((getRegisterDataType(reg) == dtReal34Matrix) && isMatrix3dVector(REGISTER_MATRIX_HEADER(reg)->matrixRows,REGISTER_MATRIX_HEADER(reg)->matrixColumns))
  #define isRegisterMatrix2dVector(reg)          ((getRegisterDataType(reg) == dtReal34Matrix) && isMatrix2dVector(REGISTER_MATRIX_HEADER(reg)->matrixRows,REGISTER_MATRIX_HEADER(reg)->matrixColumns))
  #define isRegisterMatrixVector(reg)            (isRegisterMatrix3dVector(reg) || isRegisterMatrix2dVector(reg))
  #define getVectorRegisterAngularMode(reg)      ((getRegisterDataType(reg) == dtReal34Matrix) ? (getTagAngularMode(getRegisterTag(reg)) & amAngleMask) : amNone) //maybe conditional on 2D 3D????
  #define setVectorRegisterAngularMode(reg, am)  (setRegisterTag(reg, (am & amAngleMask) | (getRegisterTag(reg) & amPolar)))                                      //note maybe conditional on Mx???
  #define getVectorRegisterPolarMode(reg)        ( ((getRegisterDataType(reg) == dtReal34Matrix) && ((getRegisterTag(reg) & amAngleMask) != amNone)) \
                                                     ? (isRegisterMatrix3dVector(reg)) \
                                                        ? /*3D*/ ((((getRegisterTag(reg) & amPolar) == amPolar)) ? amPolarSPH : amPolarCYL)\
                                                        : (isRegisterMatrix2dVector(reg)) \
                                                           ? /*2D*/ (getRegisterTag(reg) & amPolar) \
                                                           : 0 \
                                                     : 0 )
  #define setVectorRegisterPolarMode(reg, pm)   setRegisterTag(reg, ((pm == 0) ? ((getRegisterTag(reg) & ~(amAngleMask | amPolar)) + amNone) : \
                                                                      ((getRegisterTag(reg) & (amAngleMask | amPolar)) | ((pm == amPolarSPH || pm == amPolar) ? amPolar : 0)) \
                                                                      & ((pm == amPolarCYL) ? ((~amPolar) & (amAngleMask | amPolar)) : 255) \
                                                                        ))           /*if Polar is cleared, also clear angle */
                                                                                     /*existing status, with polar bit ORred for 2DPolar or SPH*/
                                                                                     /*   ANDed to force Polar 0 for CYL */

  /********************************************//**
   * \brief Save register X to register X
   *
   * \return true if succeeded
   ***********************************************/
  bool_t         saveLastX                       (void);

  void           fnRegClr                        (uint16_t unusedButMandatoryParameter);
  void           fnRegCopy                       (uint16_t unusedButMandatoryParameter);
  void           fnRegSort                       (uint16_t unusedButMandatoryParameter);
  void           fnRegSwap                       (uint16_t unusedButMandatoryParameter);
  bool_t         isFunctionAllowingNewVariable   (uint16_t op);
#endif // !REGISTERS_H

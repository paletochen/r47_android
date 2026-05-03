// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file realType.h
 */

#if !defined(REALTYPE_H)
  #define REALTYPE_H

  typedef decContext realContext_t;
  typedef decNumber  real_t;   // 75 digits and 60 bytes
  typedef decQuad    real34_t; // 34 digits and 128 bits = 16 bytes

  #define REAL_MAX_DIGITS(digits)      (((digits + 2) / 6) * 6 + 3) // Assuming DECDPUN=3 and memory alignment is 4 bytes
  #define REAL_SIZE_IN_BYTES(digits)   (10 + sizeof(decNumberUnit) * (REAL_MAX_DIGITS(digits) / DECDPUN)) // This is always a multiple of 4
  #define REAL_SIZE_IN_BLOCKS(digits)  TO_BLOCKS(REAL_SIZE_IN_BYTES(digits))
  #define REAL_T_PTR(name, digits)     uint32_t _ ## name ## _data[REAL_SIZE_IN_BYTES(digits) / 4]; real_t *const name=(real_t *)_ ## name ## _data

  typedef struct {
    real34_t real, imag;
  } complex34_t;

  #define REAL34_SIZE_IN_BLOCKS    TO_BLOCKS(sizeof(real34_t))
  #define COMPLEX34_SIZE_IN_BLOCKS TO_BLOCKS(sizeof(complex34_t))

  #define REAL34_SIZE_IN_BYTES     TO_BYTES(REAL34_SIZE_IN_BLOCKS)
  #define COMPLEX34_SIZE_IN_BYTES  TO_BYTES(COMPLEX34_SIZE_IN_BLOCKS)


  // The 2 macros below are for checking the pointer type passed to the
  // decNumber library functions. Both macros are only used in this file.
  // A wrong type generates a compilation error.
  #define TO_REAL34_T(ptr) (                        \
    _Generic((ptr),                                 \
      real34_t *         : (ptr),                   \
      complex34_t *      : (real34_t *)(ptr),       \
      const complex34_t *: (const real34_t *)(ptr), \
      const real34_t *   : (const real34_t *)(ptr)  \
    )                                               \
  )

  #define TO_REAL_T(ptr) (   \
    _Generic((ptr),          \
      real_t *      : (ptr), \
      const real_t *: (ptr)  \
    )                        \
  )

  #define VARIABLE_REAL34_DATA(a)  ((real34_t *)(a))
  #define VARIABLE_IMAG34_DATA(a)  ((real34_t *)(a) + 1)

  #if !defined(bool_t)
    typedef bool bool_t;
  #endif // bool_t

  int32_t  realToInt32C47      (const real_t *r, bool_t *error);
  uint32_t realToUint32C47     (const real_t *r, bool_t *error);
  //int64_t  realToInt64C47      (const real_t *r);
  //uint64_t realToUint64C47     (const real_t *r);
  void     realSetZero         (real_t *r);
  void     realSetOne          (real_t *r);
  void     realSetNaN          (real_t *r);
  void     realSetPlusInfinity (real_t *r);
  void     realSetMinusInfinity(real_t *r);

  #define complex34ChangeSign(operand)                           do { real34ChangeSign(TO_REAL34_T(operand)); real34ChangeSign(TO_REAL34_T(operand) + 1); } while(0)
  #define complex34Copy(source, destination)                     do {  *(uint64_t *)(destination)     =   *(uint64_t *)(source);     \
                                                                     *(((uint64_t *)(destination))+1) = *(((uint64_t *)(source))+1); \
                                                                     *(((uint64_t *)(destination))+2) = *(((uint64_t *)(source))+2); \
                                                                     *(((uint64_t *)(destination))+3) = *(((uint64_t *)(source))+3); \
                                                                 } while(0)
  #define int32ToReal34(source, destination)                     decQuadFromInt32         (TO_REAL34_T(destination), source)
  #define real34Add(operand1, operand2, res)                     decQuadAdd               (TO_REAL34_T(res), TO_REAL34_T(operand1), TO_REAL34_T(operand2), &ctxtReal34)
  #define real34ChangeSign(operand)                              TO_REAL34_T(operand)->bytes[15] ^= 0x80
  #define real34Compare(operand1, operand2, res)                 decQuadCompare           (TO_REAL34_T(res), TO_REAL34_T(operand1), TO_REAL34_T(operand2), &ctxtReal34)
  //#define real34Copy(source, destination)                        decQuadCopy            (TO_REAL34_T(destination), TO_REAL34_T(source))
  //#define real34Copy(source, destination)                        xcopy(destination, source, REAL34_SIZE_IN_BYTES)
  #define real34Copy(source, destination)                        do { *(uint64_t *)(destination)     =   *(uint64_t *)(source);     \
                                                                    *(((uint64_t *)(destination))+1) = *(((uint64_t *)(source))+1); \
                                                                 } while(0)
  #define real34CopyAbs(source, destination)                     decQuadCopyAbs           (TO_REAL34_T(destination), TO_REAL34_T(source))
  #define real34Digits(source)                                   decQuadDigits            (TO_REAL34_T(source))
  #define real34Divide(operand1, operand2, res)                  decQuadDivide            (TO_REAL34_T(res), TO_REAL34_T(operand1), TO_REAL34_T(operand2), &ctxtReal34)
  #define real34DivideRemainder(operand1, operand2, res)         decQuadRemainder         (TO_REAL34_T(res), TO_REAL34_T(operand1), TO_REAL34_T(operand2), &ctxtReal34)
  #define real34FMA(factor1, factor2, term, res)                 decQuadFMA               (TO_REAL34_T(res), TO_REAL34_T(factor1),  TO_REAL34_T(factor2),  TO_REAL34_T(term), &ctxtReal34)
  #define real34GetCoefficient(source, destination)              decQuadGetCoefficient    (TO_REAL34_T(source), (uint8_t *)(destination))
  #define real34GetExponent(source)                              decQuadGetExponent       (TO_REAL34_T(source))
  #define real34IsInfinite(source)                               decQuadIsInfinite        (TO_REAL34_T(source))
  #define real34IsNaN(source)                                    decQuadIsNaN             (TO_REAL34_T(source))
  #define real34IsNegative(source)                               (((TO_REAL34_T(source)->bytes[15]) & 0x80) == 0x80)
  #define real34IsPositive(source)                               (((TO_REAL34_T(source)->bytes[15]) & 0x80) == 0x00)
  #define real34IsSpecial(source)                                (decQuadIsNaN(TO_REAL34_T(source)) || decQuadIsSignaling(TO_REAL34_T(source)) || decQuadIsInfinite(TO_REAL34_T(source)))
  #define real34IsZero(source)                                   decQuadIsZero            (TO_REAL34_T(source))
  #define real34Minus(operand, res)                              decQuadMinus             (TO_REAL34_T(res), TO_REAL34_T(operand), &ctxtReal34)
  #define real34Multiply(operand1, operand2, res)                decQuadMultiply          (TO_REAL34_T(res), TO_REAL34_T(operand1), TO_REAL34_T(operand2), &ctxtReal34)
  #define real34NextMinus(operand, res)                          decQuadNextMinus         (TO_REAL34_T(res), TO_REAL34_T(operand), &ctxtReal34)
  #define real34NextPlus(operand, res)                           decQuadNextPlus          (TO_REAL34_T(res), TO_REAL34_T(operand), &ctxtReal34)
  #define real34Plus(operand, res)                               decQuadPlus              (TO_REAL34_T(res), TO_REAL34_T(operand), &ctxtReal34)
  #define real34Reduce(operand, res)                             decQuadReduce            (TO_REAL34_T(res), TO_REAL34_T(operand), &ctxtReal34)
  #define real34SetNegativeSign(operand)                         TO_REAL34_T(operand)->bytes[15] |= 0x80
  #define real34SetPositiveSign(operand)                         TO_REAL34_T(operand)->bytes[15] &= 0x7F
  #define real34Subtract(operand1, operand2, res)                decQuadSubtract          (TO_REAL34_T(res), TO_REAL34_T(operand1), TO_REAL34_T(operand2), &ctxtReal34)
  #define real34ToInt32(source)                                  decQuadToInt32           (TO_REAL34_T(source), &ctxtReal34, DEC_ROUND_DOWN)
  #define real34ToIntegralValue(source, destination, mode)       decQuadToIntegralValue   (TO_REAL34_T(destination), TO_REAL34_T(source), &ctxtReal34, mode)
  #define real34ToReal(source, destination)                      decQuadToNumber          (TO_REAL34_T(source), destination)
  #define real34ToString(source, destination)                    decQuadToString          (TO_REAL34_T(source), destination)
  #define real34ToUInt32(source)                                 decQuadToUInt32          (TO_REAL34_T(source), &ctxtReal34, DEC_ROUND_DOWN)
  #define real34SetZero(destination)                             decQuadZero              (destination)
  #define real34SetOne(destination)                              decQuadFromInt32         (destination, 1)
  #define real34ScaleB(source, shift, destination)               decQuadScaleB            ((real34_t *)(destination), (real34_t *)(source), (real34_t *)(shift), &ctxtReal34)
  //#define real34SetZero(destination)                              xcopy                    (destination, const34_0, REAL34_SIZE_IN_BYTES)
  /*#define real34SetZero(destination)                              do {   *(uint64_t *)(destination)     =   *(uint64_t *)const34_0;     \
                                                                         *(((uint64_t *)(destination))+1) = *(((uint64_t *)const34_0)+1); \
                                                                    } while(0)*/
  #define stringToReal34(source, destination)                    decQuadFromString        (TO_REAL34_T(destination), source, &ctxtReal34)
  #define uInt32ToReal34(source, destination)                    decQuadFromUInt32        (TO_REAL34_T(destination), source)





  #define int32ToReal(source, destination)                       decNumberFromInt32       (TO_REAL_T(destination), source)
  #define realAdd(operand1, operand2, res, ctxt)                 decNumberAdd             (TO_REAL_T(res), TO_REAL_T(operand1), TO_REAL_T(operand2), ctxt)
  #define realChangeSign(operand)                                TO_REAL_T(operand)->bits ^= 0x80
  #define realCompare(operand1, operand2, res, ctxt)             decNumberCompare         (TO_REAL_T(res), TO_REAL_T(operand1), TO_REAL_T(operand2), ctxt)
  #define realCopy(source, destination)                          decNumberCopy            (TO_REAL_T(destination), TO_REAL_T(source))
  #define realCopyAbs(source, destination)                       decNumberCopyAbs         (TO_REAL_T(destination), TO_REAL_T(source))
  #define realDivide(operand1, operand2, res, ctxt)              decNumberDivide          (TO_REAL_T(res), TO_REAL_T(operand1), TO_REAL_T(operand2), ctxt)
  #define realDivideBy2(operand, ctxt)                           decNumberMultiply        (TO_REAL_T(operand), TO_REAL_T(operand), const_1on2, ctxt)
  #define realDivideRemainder(operand1, operand2, res, ctxt)     decNumberRemainder       (TO_REAL_T(res), TO_REAL_T(operand1), TO_REAL_T(operand2), ctxt)
  #define realFMA(factor1, factor2, term, res, ctxt)             decNumberFMA             (TO_REAL_T(res), TO_REAL_T(factor1),  TO_REAL_T(factor2),  TO_REAL_T(term), ctxt)
  #define realGetCoefficient(source, destination)                decNumberGetBCD          (TO_REAL_T(source), (uint8_t *)(destination))
  //#define realGetExponent(source)                                (TO_REAL_T(source)->digits)
  #define realGetExponent(source)                                (TO_REAL_T(source)->digits + (source)->exponent - 1)
  #define realGetSign(source)                                    ((TO_REAL_T(source)->bits) & 0x80) // 0x80=negative and 0x00=positive
  #define realIsInfinite(source)                                 decNumberIsInfinite      (TO_REAL_T(source))
  #define realIsNaN(source)                                      decNumberIsNaN           (TO_REAL_T(source))
  #define realIsNegative(source)                                 (((TO_REAL_T(source)->bits) & 0x80) == 0x80)
  #define realIsPositive(source)                                 (((TO_REAL_T(source)->bits) & 0x80) == 0x00)
  #define realIsSpecial(source)                                  decNumberIsSpecial       (TO_REAL_T(source))
  #define realIsZero(source)                                     decNumberIsZero          (TO_REAL_T(source))
  #define realMinus(operand, res, ctxt)                          decNumberMinus           (TO_REAL_T(res), TO_REAL_T(operand), ctxt)
  #define realMultiply(operand1, operand2, res, ctxt)            decNumberMultiply        (TO_REAL_T(res), TO_REAL_T(operand1), TO_REAL_T(operand2), ctxt)
  #define realNextToward(from, toward, res, ctxt)                decNumberNextToward      (TO_REAL_T(res), TO_REAL_T(from),     TO_REAL_T(toward),   ctxt)
  #define realPlus(operand, res, ctxt)                           decNumberPlus            (TO_REAL_T(res), TO_REAL_T(operand), ctxt)
  #define realPower(operand1, operand2, res, ctxt)               decNumberPower           (TO_REAL_T(res), TO_REAL_T(operand1), TO_REAL_T(operand2), ctxt)
  #define realRescale(operand, res, acc, ctxt)                   decNumberRescale         (TO_REAL_T(res), TO_REAL_T(operand), TO_REAL_T(acc), ctxt)
  #define realReduce(operand, res, ctxt)                         decNumberReduce          (TO_REAL_T(res), TO_REAL_T(operand), ctxt)
  #define realSetNegativeSign(operand)                           (TO_REAL_T(operand)->bits) |= 0x80
  #define realSetPositiveSign(operand)                           (TO_REAL_T(operand)->bits) &= 0x7F
  #define realSquareRoot(operand, res, ctxt)                     decNumberSquareRoot      (TO_REAL_T(res), TO_REAL_T(operand), ctxt)
  #define realSubtract(operand1, operand2, res, ctxt)            decNumberSubtract        (TO_REAL_T(res), TO_REAL_T(operand1), TO_REAL_T(operand2), ctxt)
  #define realToReal34(source, destination)                      decQuadFromNumber        (TO_REAL34_T(destination), TO_REAL_T(source), &ctxtReal34)
  #define realToString(source, destination)                      decNumberToString        (TO_REAL_T(source), destination)
  #define stringToReal(source, destination, ctxt)                decNumberFromString      (TO_REAL_T(destination), source, ctxt)
  #define uInt32ToReal(source, destination)                      decNumberFromUInt32      (TO_REAL_T(destination), source)
#endif // !REALTYPE_H

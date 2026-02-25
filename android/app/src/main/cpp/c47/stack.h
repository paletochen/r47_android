// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file stack.c
 * Stack management.
 */
#if !defined(STACK_H)
#define STACK_H

/**
 * Clears X and refreshes the stack.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnClX         (uint16_t unusedButMandatoryParameter);

/**
 * Drops X from the stack and refreshes the stack.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnDrop        (uint16_t unusedButMandatoryParameter);

/**
 * Lifts the stack if allowed and reallocates the X register.
 */
void liftStack     (void);

/**
 * Drops Y from the stack and refreshes the stack.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnDropY       (uint16_t unusedButMandatoryParameter);
void fnDropZ       (uint16_t unusedButMandatoryParameter);
void fnDropT       (uint16_t unusedButMandatoryParameter);
void fnDropN       (uint16_t number);
void fnDupN        (uint16_t number);
void fnSwapN       (uint16_t number);

/**
 * Rolls the stack up and refreshes the stack.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnRollUp      (uint16_t unusedButMandatoryParameter);

/**
 * Rolls the stack down and refreshes the stack.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnRollDown    (uint16_t unusedButMandatoryParameter);

/**
 * Sets the number of stack registers displayed.
 *
 * \param[in] numberOfStackLines
 */
void fnDisplayStack(uint16_t numberOfStackLines);

/**
 * Swaps X with the target register.
 *
 * \param[in] regist
 */
void fnSwapX       (uint16_t regist);

/**
 * Swaps Y with the target register.
 *
 * \param[in] regist
 */
void fnSwapY       (uint16_t regist);

/**
 * Swaps Z with the target register.
 *
 * \param[in] regist
 */
void fnSwapZ       (uint16_t regist);

/**
 * Swaps T with the target register.
 *
 * \param[in] regist
 */
void fnSwapT       (uint16_t regist);

/**
 * Swaps X and Y and refreshes the stack.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnSwapXY      (uint16_t unusedButMandatoryParameter);

/**
 * Shuffles the registers and and refreshes the stack.
 * the Shuffle order determined from the parameter with each
 * consecutive two bits indicating the number above the X
 * register
 *
 * For example
 * - 11100100 indicates X, Y, Z, T
 * - 10110001 indicates Z, T, X, Y
 *
 * \param[in] regist_order
 */
void fnShuffle     (uint16_t regist_order);

  /*
 * Fills the stack with the value of X and refreshes the stack.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnFillStack   (uint16_t unusedButMandatoryParameter);

/**
 * Sets X to the stack size and refreshes the stack.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnGetStackSize(uint16_t unusedButMandatoryParameter);

/**
 * Clears the stack and refreshes the stack.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnClearStack  (uint16_t unusedButMandatoryParameter);

void fnUndo        (uint16_t unusedButMandatoryParameter);
void saveForUndo   (void);
void undo          (void);
void fillStackWithReal0(void);

#endif // !STACK_H

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file eulersFormula.c
 ***********************************************/

#if !defined(EULERSFORMULA_H)
#define EULERSFORMULA_H

void fnEulersFormula(uint16_t unusedButMandatoryParameter);
void eulersFormula(const real_t *inReal, const real_t *inImag, real_t *outReal, real_t *outImag, realContext_t *ctxt);

#endif // !EULERSFORMULA_H

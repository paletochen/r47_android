// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file eulersFormula.h
 ***********************************************/

#if !defined(EULERSFORMULA_H)
#define EULERSFORMULA_H

void fnEulersFormula(uint16_t unusedButMandatoryParameter);
void eulersFormula(const cplx_t *in, cplx_t *out, realContext_t *ctxt);

#endif // !EULERSFORMULA_H

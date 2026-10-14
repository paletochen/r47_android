// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//** //JM
 * \file jmgraph.c Graphing module
 ***********************************************/


#if !defined(GRAPH_H)
#define GRAPH_H

extern char plotStatMx[8];

#define EQ_CPXSOLVE       0   //fnEqSolvGraph
#define EQ_CPXSOLVE_LU    1   //fnEqSolvGraph
#define EQ_REALSOLVE      2   //fnEqSolvGraph
#define EQ_REALSOLVE_LU   3   //fnEqSolvGraph
#define EQ_PLOT           4   //fnEqSolvGraph
#define EQ_PLOT_LU        5   //fnEqSolvGraph
#define EQ__PLOT          6   //fnEqSolvGraph


#define noInitDrwMx 0
#define initDrwMx   1

void    fnEqSolvGraph (uint16_t func);
void    graph_stat(uint16_t unusedButMandatoryParameter);
int32_t drawMxN(void);
void    fnClDrawMx(uint8_t origin);
#endif // !GRAPH_H

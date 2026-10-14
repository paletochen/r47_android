// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//** //JM
 * \file graphs.h Graphing module
 ***********************************************/

/* ADDITIONAL C47 functions and routines */

//graphs.h

#if !defined(GRAPHS_H)
#define GRAPHS_H

#define statZoomRangeHi  0
#define statZoomRangeLo -3
#define zoomRangeHi +16
#define zoomRangeLo -16
#define zoomOverride 18


//Graph functions
void    graph_reset        (void);
void    fnClGrf            (uint16_t unusedButMandatoryParameter);
void    fnPline            (uint16_t unusedButMandatoryParameter);
void    fnP2line           (uint16_t unusedButMandatoryParameter);
void    fnPcros            (uint16_t unusedButMandatoryParameter);
void    fnPplus            (uint16_t unusedButMandatoryParameter);
void    fnPbox             (uint16_t unusedButMandatoryParameter);
void    fnPcurve           (uint16_t unusedButMandatoryParameter);
void    fnPintg            (uint16_t unusedButMandatoryParameter);
void    fnPdiff            (uint16_t unusedButMandatoryParameter);
void    fnPrms             (uint16_t unusedButMandatoryParameter);
void    fnPvect            (uint16_t unusedButMandatoryParameter);
void    fnPNvect           (uint16_t unusedButMandatoryParameter);
void    fnScale            (uint16_t unusedButMandatoryParameter);
void    fnPshade           (uint16_t unusedButMandatoryParameter);
void    fnComplexPlot      (uint16_t unusedButMandatoryParameter);
void    fnPMzoom           (uint16_t unusedButMandatoryParameter);
void    fnPlotZoom         (uint16_t unusedButMandatoryParameter);
void    fnPx               (uint16_t unusedButMandatoryParameter);
void    fnPy               (uint16_t unusedButMandatoryParameter);
void    fnListXY           (uint16_t unusedButMandatoryParameter);
void    fnStatList         (void);
void    graph_plotmem      (void);
void    fnPlotf            (uint16_t unusedButMandatoryParameter);
void    fnPlotSQ           (uint16_t unusedButMandatoryParameter);
void    fnPlotReset        (uint16_t unusedButMandatoryParameter);
void    fnPlotStatAdv      (uint16_t unusedButMandatoryParameter);

#define PLOTSTAT true
void graphResetCommon      (void);
void graph_Include0        (bool_t mode,  uint16_t statnum); //using global: FLAG_SHOWX, x_min, x_max, FLAG_SHOWY, y_min, y_max
extern  real_t *const x_min;
extern  real_t *const x_max;
extern  real_t *const y_min;
extern  real_t *const y_max;
extern  int8_t   PLOT_ZMY;
extern  bool_t   plotXYroute;

#endif // !GRAPHS_H

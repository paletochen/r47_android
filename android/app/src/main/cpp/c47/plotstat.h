// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file plot.h
 ***********************************************/
#if !defined(PLOTSTAT_H)
#define PLOTSTAT_H

#define   useFLOAT           0
#define   useREAL4           4
#define   useREAL39         39
#define   zoomfactor     0.05f     // default is 0.05, which is 5% space around the data points. Use 0.05 * 40 for wide view
#define   numberIntervals   50     // default 50, auto increase if jumps found
#define   fittedcurveboxes   0     // default 0 = smooth line
#define   USEFLOATING useFLOAT     // USEFLOATING can be used to change the STATS graphing to work using different number types.
                                   //   Note that limited precision is required as only the pixels on screen need to be considered
                                   //   useFLOAT is much faster plotting STATS graphs on the real hardware than useREAL4 and useREAL39
#define   FLoatingMax    1e38f     // convenient round figures used for maxima and minima determination
#define   FLoatingMin    -1e38f


//Graph options
extern  float    graph_dx;
extern  float    graph_dy;
extern  bool_t   roundedTicks;
extern  bool_t   PLOT_INTG;
extern  bool_t   PLOT_DIFF;
extern  bool_t   PLOT_RMS;
extern  bool_t   PLOT_SHADE;
extern  bool_t   PLOT_AXIS;
extern  int8_t   PLOT_ZOOM;
extern  uint8_t  drawHistogram;

#define _VECT 0
#define _SCAT 1

extern  int8_t   plotmode;    //VECTOR or SCATTER
extern  float    tick_int_x;
extern  float    tick_int_y;
extern  uint32_t xzero;
extern  uint32_t yzero;



//Screen limits
#define SCREEN_MIN_GRAPH    20
#define SCREEN_HEIGHT_GRAPH SCREEN_HEIGHT
#define SCREEN_WIDTH_GRAPH  SCREEN_WIDTH
#define SCREEN_NONSQ_HMIN   0



//Utility functions
void    placePixel         (uint32_t x, uint32_t y);
void    removePixel        (uint32_t x, uint32_t y);
void    clearScreenPixels  ();
void    plotcross          (int16_t xn, int16_t yn);                      // Plots line from xo,yo to xn,yn; uses temporary x1,y1
void    plotplus           (int16_t xn, int16_t yn);                      // Plots line from xo,yo to xn,yn; uses temporary x1,y1
void    plotbox            (int16_t xn, int16_t yn);                      // Plots line from xo,yo to xn,yn; uses temporary x1,y1
void    plotrect           (int16_t a,  int16_t b,  int16_t c, int16_t d); // Plots rectangle from xo,yo to xn,yn; uses temporary x1,y1
void    pixelline          (int16_t xo, int16_t yo, int16_t xn, int16_t yn, bool_t vmNormal);              // Plots line from xo,yo to xn,yn; uses temporary x1,y1
void    plotline1          (int16_t xo, int16_t yo, int16_t xn, int16_t yn);
void    plotline2          (int16_t xo, int16_t yo, int16_t xn, int16_t yn);
void    plotline3          (int16_t xo, int16_t yo, int16_t xn, int16_t yn, bool_t first_time, bool_t final_segment);
void    graphAxisDraw      (void);
void    graph_axis         (void);
float   auto_tick          (float tick_int_f);
void    plotPointGeneric   (int16_t xn, int16_t yn, int16_t xo, int16_t yo, bool_t PLOT_CROSS, bool_t PLOT_BOXFAT, bool_t PLOT_BOX, bool_t PLOT_PLUS, bool_t PLOT_LINE);
void    showGraphTickText  (float tick_int_x, float tick_int_y, int32_t xoff, int32_t yoff1, int32_t yoff2, uint16_t acc);

//graph functions
float   grf_x(int i);
float   grf_y(int i);

#define PLOT_TMP_BUF_SIZE   32
char    *radixProcess(char *output, const char * ss);
void    grphNumFormatter(char* s02, const char* s01, double inreal, int8_t digits, const char* s05);
char    *padEquals(char *output, const char * ss);
char    *smallE(char *output, const char * ss);
char    *formatCore(double value, int digits, bool handle_zero, char* buf, int widthLimit);
char    *formatDoubleWidth(real34_t *real34, int digits, char* itemName, bool_t* success, int actual_max_width, char* buf, int digitswidthLimit);

int16_t screen_window_x(float x_min, float x, float x_max);
int16_t screen_window_y(float y_min, float y, float y_max);
int16_t screen_window_y_nolimit(float y_min, float y, float y_max);
int32_t statMxN(void);

void    statGraphReset     (void);
void    graphPlotstat      (uint16_t selection);
void    graphDrawLRline    (uint16_t selection);
//void    fnPlotClose        (uint16_t unusedButMandatoryParameter);
void    fnPlotCloseSmi     (uint16_t unusedButMandatoryParameter);
void    fnPlotStat         (uint16_t unusedButMandatoryParameter);
#endif // !PLOTSTAT_H

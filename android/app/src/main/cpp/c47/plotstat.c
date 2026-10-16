// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"


void fnPlotRegressionLine(uint16_t plotMode);


static real_t RR, SMI, aa0, aa1, aa2, sa0, sa1; //L.R. variables
static void drawline(uint16_t selection, real_t *RR, real_t *SMI, real_t *aa0, real_t *aa1, real_t *aa2, real_t *sa0, real_t *sa1);


bool_t    roundedTicks;
bool_t    PLOT_AXIS;
int8_t    PLOT_ZOOM;
uint8_t   drawHistogram;
uint8_t   plotStatScale;    // equal-scale axes for the fnPlotStat plots (SCATR/CENTRL) only

int8_t    plotmode;
double    tick_int_x;
double    tick_int_y;
uint32_t  xzero;
uint32_t  yzero;
uint16_t  plotXYn = 0;      // plotXYn: Counter, also boolean enabler;




void statGraphReset(void){
  graphResetCommon();
  currentKeyCode = 255;
  roundedTicks  = true;
  clearSystemFlag(FLAG_SHOWX);
  clearSystemFlag(FLAG_PLINE);
  realSetZero(y_min);
  realSetOne(y_max);
}



  float grf_x(int i) {
                                  #if defined(STATDEBUG)
                                    char prefix[100];
                                    memcpy(prefix, allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName + 1, allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName[0]);
                                    strcpy(prefix + allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName[0], " :");
                                    printf("X: Reading from Matrix %s\n", prefix);
                                  #endif // STATDEBUG
//    if(keyWaiting()) {
//       return 0;
//    }
    float xf=0;
    real_t xr;

    if(plotXYn) {
      return (float)(i % plotXYn);                     // count-vs-value: x is the sample index
    }
    calcRegister_t regStats = regStatsXY;
    if(regStats != INVALID_VARIABLE) {
      real34Matrix_t stats;
      linkToRealMatrixRegister(regStats, &stats);
      const uint16_t cols = stats.header.matrixColumns;
      real34ToReal(&stats.matrixElements[i * cols    ], &xr);
      realToFloat(&xr, &xf);
    }
    else {
      xf = 0;
    }
    return xf;
  }


  void grf_x_r(int i, real_t *v) {                   // real_t reader of the stat/draw matrix; the float grf_x stays for float-precision consumers
    if(plotXYn) {
      int32ToReal(i % plotXYn, v);                   // count-vs-value: x is the sample index
      return;
    }
    calcRegister_t regStats = regStatsXY;
    if(regStats != INVALID_VARIABLE) {
      real34Matrix_t stats;
      linkToRealMatrixRegister(regStats, &stats);
      const uint16_t cols = stats.header.matrixColumns;
      real34ToReal(&stats.matrixElements[i * cols    ], v);
    }
    else {
      realSetZero(v);
    }
  }


  void grf_y_r(int i, real_t *v) {
    calcRegister_t regStats = regStatsXY;
    if(regStats != INVALID_VARIABLE) {
      real34Matrix_t stats;
      linkToRealMatrixRegister(regStats, &stats);
      const uint16_t cols = stats.header.matrixColumns;
      real34ToReal(&stats.matrixElements[plotXYn ? (i % plotXYn) * cols + i / plotXYn : i * cols + 1], v);   // count-vs-value: the x column for i < N, the y column after
    }
    else {
      realSetZero(v);
    }
  }


  float grf_y(int i) {
                                  #if defined(STATDEBUG)
                                    char prefix[100];
                                    memcpy(prefix, allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName + 1, allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName[0]);
                                    strcpy(prefix + allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName[0], " :");
                                    printf("Y: Reading from Matrix %s\n", prefix);
                                  #endif // STATDEBUG
//    if(keyWaiting()) {
//       return 0;
//    }
    float yf=0;
    real_t yr;

    calcRegister_t regStats = regStatsXY;
    if(regStats != INVALID_VARIABLE) {
      real34Matrix_t stats;
      linkToRealMatrixRegister(regStats, &stats);
      const uint16_t cols = stats.header.matrixColumns;
      real34ToReal(&stats.matrixElements[plotXYn ? (i % plotXYn) * cols + i / plotXYn : i * cols + 1], &yr);   // count-vs-value: the x column for i < N, the y column after
      realToFloat(&yr, &yf);
    }
    else {
      yf = 0;
    }
    return yf;
  }


#define ROUND_F2I(f) ((int)((f) >= 0 ? (f) + 0.5f : (f) - 0.5f))

// (x - x_min) / (x_max - x_min) * scale in real_t, rounded half away from zero like ROUND_F2I, clamped to +-32767 like the float original.
static int16_t screenWindowRatio(const real_t *v_min, const real_t *v, const real_t *v_max, int32_t scale) {
  int32_t temp;
  real_t tempr, den;
  bool_t err = false;

  realSubtract(v, v_min, &tempr, &ctxtReal39);       // tempr = x - x_min
  realSubtract(v_max, v_min, &den, &ctxtReal39);     // den = x_max - x_min
  realDivide(&tempr, &den, &tempr, &ctxtReal39);     // tempr = (x - x_min) / (x_max - x_min)
  int32ToReal(scale, &den);                          // den = scale
  realMultiply(&tempr, &den, &tempr, &ctxtReal39);   // tempr = (x - x_min) / (x_max - x_min) * scale
  if(realIsNegative(&tempr)) {                       // ROUND_F2I equivalent: +-0.5, then truncate towards zero in realToInt32C47
    realSubtract(&tempr, const_1on2, &tempr, &ctxtReal39);
  }
  else {
    realAdd(&tempr, const_1on2, &tempr, &ctxtReal39);
  }
  temp = realToInt32C47(&tempr, &err);               // err is set when tempr is NaN, infinite or beyond int32 range, e.g. after division by zero when v_max == v_min

  if(err) {
    temp = realIsNegative(&tempr) ? -32767 : 32767;
  }
  else if(temp > 32766) {
    temp = 32767;
  }
  else if(temp < -32766) {
    temp = -32767;
  }

  return (int16_t)temp;
}

int16_t screen_window_x_r(const real_t *x_min, const real_t *x, const real_t *x_max) {
  int16_t temp = screenWindowRatio(x_min, x, x_max, SCREEN_HEIGHT_GRAPH - 1);

  if(temp > SCREEN_HEIGHT_GRAPH - 1) {
    temp = SCREEN_HEIGHT_GRAPH - 1;
  }
  else if(temp < 0) {
    temp = 0;
  }

  return temp + SCREEN_WIDTH - SCREEN_HEIGHT_GRAPH;
}

int16_t screen_window_x(float x_min, float x, float x_max) {
  real_t x_min_r, x_r, x_max_r;
  convertDoubleToReal(x_min, &x_min_r, &ctxtReal39);
  convertDoubleToReal(x,     &x_r,     &ctxtReal39);
  convertDoubleToReal(x_max, &x_max_r, &ctxtReal39);
  return screen_window_x_r(&x_min_r, &x_r, &x_max_r);
}


#define minn 0
static int16_t _screen_window_y_r(const real_t *y_min, const real_t *y, const real_t *y_max, bool_t nolimit) {
  int16_t temp = screenWindowRatio(y_min, y, y_max, SCREEN_HEIGHT_GRAPH - 1 - minn);

  if(!nolimit) {
    if(temp > SCREEN_HEIGHT_GRAPH - 1 - minn) {
      temp = SCREEN_HEIGHT_GRAPH - 1 - minn;
    }
    else if(temp < 0) {
      temp = 0;
    }
  }

  #if defined(PC_BUILD)
  if(SCREEN_HEIGHT_GRAPH - 1 - temp < 0) {
    printf("In function screen_window_y Y NEGATIVE %6d; ", SCREEN_HEIGHT_GRAPH - 1 - temp);
  }
  if(SCREEN_HEIGHT_GRAPH - 1 - temp > 239) {
    printf("In function screen_window_y Y EXCEEDED %6d; ", SCREEN_HEIGHT_GRAPH - 1 - temp);
  }
  #endif

  return (int16_t)(SCREEN_HEIGHT_GRAPH - 1 - temp);
}

int16_t _screen_window_y(float y_min, float y, float y_max, bool_t nolimit) {
  real_t y_min_r, y_r, y_max_r;
  convertDoubleToReal(y_min, &y_min_r, &ctxtReal39);
  convertDoubleToReal(y,     &y_r,     &ctxtReal39);
  convertDoubleToReal(y_max, &y_max_r, &ctxtReal39);
  return _screen_window_y_r(&y_min_r, &y_r, &y_max_r, nolimit);
}

  #define nolimit true
  #define limit false

  int16_t screen_window_y_nolimit_r(const real_t *y_min, const real_t *y, const real_t *y_max) {
    return _screen_window_y_r(y_min, y, y_max, nolimit);
  }

  int16_t screen_window_y_r(const real_t *y_min, const real_t *y, const real_t *y_max) {
    return _screen_window_y_r(y_min, y, y_max, limit);
  }

  int16_t screen_window_y_nolimit(float y_min, float y, float y_max) {
    return _screen_window_y(y_min, y, y_max, nolimit);
  }

  int16_t screen_window_y(float y_min, float y, float y_max) {
    return _screen_window_y(y_min, y, y_max, limit);
  }

// Map float data values against the real_t range globals. Stat data stays float precision; the range survives any exponent.
int16_t screenX(double x) {
  real_t t;
  convertDoubleToReal(x, &t, &ctxtReal39);
  return screen_window_x_r(x_min, &t, x_max);
}
int16_t screenY(double y) {
  real_t t;
  convertDoubleToReal(y, &t, &ctxtReal39);
  return screen_window_y_r(y_min, &t, y_max);
}



void placePixel(uint32_t x, uint32_t y) {
  if(x < SCREEN_WIDTH_GRAPH && y < SCREEN_HEIGHT_GRAPH && y >= 1 + minn) {
    setBlackPixel(x, y);
  }
}


void removePixel(uint32_t x, uint32_t y) {
  if(x < SCREEN_WIDTH_GRAPH && y < SCREEN_HEIGHT_GRAPH && y >= 1 + minn) {
    setWhitePixel(x, y);
  }
}


void clearScreenPixels(void) {
  lcd_fill_rect(SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH, 0, SCREEN_HEIGHT_GRAPH, SCREEN_HEIGHT_GRAPH, LCD_SET_VALUE);
  lcd_fill_rect(0, Y_POSITION_OF_REGISTER_T_LINE, SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH, 171-5-Y_POSITION_OF_REGISTER_T_LINE+1, LCD_SET_VALUE);
  lcd_fill_rect(19, 171-5, SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH-19+1, 5, LCD_SET_VALUE);
}


void plotcross(int16_t xn, int16_t yn) {              // Plots cross at xn, yn
  plotline1(max((int16_t)xn-2, 0), max((int16_t)yn-2, 0), xn+2, yn+2);                       //   PLOT a cross
  plotline1(max((int16_t)xn-2, 0), yn+2, xn+2, max((int16_t)yn-2, 0));
}


void plotplus(int16_t xn, int16_t yn) {              // Plots plus xn, yn
  plotline1(max((int16_t)xn-3, 0), yn, xn+3, yn);          // PLOT a plus
  plotline1(xn, yn+3, xn, max((int16_t)yn-3, 0));
}


void plotbox(int16_t xn, int16_t yn) {                // Plots line from xo, yo to xn, yn; uses temporary x1, y1
  plotline1   (max((int16_t)xn-2, 0), max((int16_t)yn-2, 0), max((int16_t)xn-2, 0), max((int16_t)yn-1, 0));                       //   PLOT a box
  placePixel  (max((int16_t)xn-1, 0), max((int16_t)yn-2, 0));
  plotline1   (max((int16_t)xn-2, 0), yn+2, max((int16_t)xn-2, 0), yn+1);
  placePixel  (max((int16_t)xn-1, 0), yn+2);
  plotline1   (xn+2, max((int16_t)yn-2, 0), xn+1, max((int16_t)yn-2, 0));
  placePixel  (xn+2, max((int16_t)yn-1, 0));
  plotline1   (xn+2, yn+2, xn+2, yn+1);
  placePixel  (xn+1, yn+2);
}


void plotrect(int16_t a, int16_t b, int16_t c, int16_t d) {                // Plots rectangle from xo, yo to xn, yn; uses temporary x1, y1
  plotline1(a, b, c, b);
  plotline1(a, b, a, d);
  plotline1(c, d, c, b);
  plotline1(c, d, a, d);
}


#if defined(OPTION_GRAPHICS)
  static void plotHisto_coln(int16_t x, int16_t y, int16_t y_min, int16_t y_wid, int16_t colw) {  //x is 0..(n-1)
    plotrect(max((int16_t)x - colw, 0), y_min + y_wid,  x + colw, y);
    }
#endif //OPTION_GRAPHICS



void plotbox_fat(int16_t xn, int16_t yn) {                                         // Plots line from xo, yo to xn, yn; uses temporary x1, y1
  plotrect(max((int16_t)xn-3, 0), max((int16_t)yn-3, 0), xn+3, yn+3);
  plotrect(max((int16_t)xn-2, 0), max((int16_t)yn-2, 0), xn+2, yn+2);
}


void plotline1(int16_t xo, int16_t yo, int16_t xn, int16_t yn) {                   // Plots line from xo, yo to xn, yn; uses temporary x1, y1
   pixelline(xo, yo, xn, yn, 1);
}


void plotline2(int16_t xo, int16_t yo, int16_t xn, int16_t yn) {                   // Plots line from xo, yo to xn, yn; uses temporary x1, y1
   //printf("From (%8d, %8d) to (%8d, %8d)\n", xo, yo, xn, yn);
   pixelline(xo, yo, xn, yn, 1);
   pixelline(max((int16_t)xo-1, 0), yo, max((int16_t)xn-1, 0), yn, 1);
   pixelline(xo, max((int16_t)yo-1, 0), xn, max((int16_t)yn-1, 0), 1);
   //   pixelline(xo+1, yo, xn+1, yn, 1);   //Do not use the full doubling, without it give as nice profile if the slope changes
   //   pixelline(xo, yo+1, xn, yn+1, 1);
}



// plotline3 does curve fitting every 2, 3, 4, and 5 points plotted.
//           it needs to be started prior to data using plotline3(0, 0, 0, 0, true, false)
//           it needs to be stopped after the last data plotline3(0, 0, 0, 0, false, true) which will plot the last segment provided there were more than 4 points

#if defined(USECURVES)
  static void evalHermite(float t, float x1, float x2, float tx1, float tx2, float y1, float y2, float ty1, float ty2, float *ox, float *oy) {
    float h1 =  2*t*t*t - 3*t*t + 1;
    float h2 = -2*t*t*t + 3*t*t;
    float h3 =    t*t*t - 2*t*t + t;
    float h4 =    t*t*t -   t*t;

    *ox = h1*x1 + h2*x2 + h3*tx1 + h4*tx2;
    *oy = h1*y1 + h2*y2 + h3*ty1 + h4*ty2;
  }


  static bool_t ifAnyMax(uint16_t *px, uint16_t *py, int cnt) {
    for(int rr = 0; rr < cnt; rr++) {
      //printf("%d %d\n",px[rr],py[rr]);
      if(px[rr] < 1 || px[rr] > 398 || py[rr] < 1 || py[rr] > 238) {
        return true;
      }
    }
    return false;
  }
#endif //USECURVES


void plotline3(int16_t xo, int16_t yo, int16_t xn, int16_t yn, bool_t first_time, bool_t final_segment) {
  #if !defined(USECURVES)
    plotline2(xo, yo, xn, yn);
  #else // USECURVES
    //printf("plotline3: %10d %10d %10d %10d ", xo, yo, xn, yn);
    #define maxSteps 6
    static uint16_t px[5];
    static uint16_t  py[5];
    static int count = 0;
    static int z[5] = {0};

    if(first_time) {
      count = 0;
      memset(z, 0, sizeof(z));
      return;
    }

    if(!final_segment) {
      if(count < 5) {
        px[count] = xn;
        py[count] = yn;

        if(count > 0) {
          int dx = abs((int)px[count] - (int)px[count - 1]);
          int dy = abs((int)py[count] - (int)py[count - 1]);
          z[count] = (int)(sqrtf((float)(dx*dx + dy*dy)) + 0.5f);
        }

        count++;
      }
      else {
        for(int i = 0; i < 4; i++) {
          px[i] = px[i + 1];
          py[i] = py[i + 1];
          z[i] = z[i + 1];
        }
        px[4] = xn;
        py[4] = yn;

        int dx = abs((int)px[4] - (int)px[3]);
        int dy = abs((int)py[4] - (int)py[3]);
        z[4] = (int)(sqrtf((float)(dx*dx + dy*dy)) + 0.5f);
      }

      if(count < 2) {
        return;
      }

      if(count == 2) {
        float prev_x = px[0], prev_y = py[0];
        if(ifAnyMax(px, py, count)) {
          int steps = min(1, max(maxSteps, z[1]));
          for(int i = 1; i <= steps; i++) {
            float t = (float)i / steps;
            float sx = (1 - t) * px[0] + t * px[1];
            float sy = (1 - t) * py[0] + t * py[1];
            plotline2(ROUND_F2I(prev_x), ROUND_F2I(prev_y), ROUND_F2I(sx), ROUND_F2I(sy));
            prev_x = sx;
            prev_y = sy;
          }
        }
        else {
          plotline2(ROUND_F2I(px[0]), ROUND_F2I(py[0]), ROUND_F2I(px[1]), ROUND_F2I(py[1]));
        }



      }
      else if(count == 3) {
        float prev_x = px[0], prev_y = py[0];
        if(!ifAnyMax(px, py, count)) {
          int steps = min(1, max(maxSteps, z[1] + z[2]));
          for(int i = 1; i <= steps; i++) {
            float t = (float)i / steps;
            float u = 1 - t;
            float sx = u*u*px[0] + 2*u*t*px[1] + t*t*px[2];
            float sy = u*u*py[0] + 2*u*t*py[1] + t*t*py[2];
            plotline2(ROUND_F2I(prev_x), ROUND_F2I(prev_y), ROUND_F2I(sx), ROUND_F2I(sy));
            prev_x = sx;
            prev_y = sy;
          }
        }
        else {
          plotline2(ROUND_F2I(px[1]), ROUND_F2I(py[1]), ROUND_F2I(px[2]), ROUND_F2I(py[2]));
        }



      }
      else if(count == 4) {
        float t1x = (px[2] - px[0]) / 2.0f;
        float t1y = (py[2] - py[0]) / 2.0f;
        float t2x = (px[3] - px[1]) / 2.0f;
        float t2y = (py[3] - py[1]) / 2.0f;

        float prev_x = px[1], prev_y = py[1];
        if(!ifAnyMax(px, py, count)) {
          int steps = min(1, max(maxSteps, z[1] + z[2] + z[3]));
          for(int i = 1; i <= steps; i++) {
            float sx, sy;
            evalHermite((float)i / steps,    px[1], px[2], t1x, t2x,    py[1], py[2], t1y, t2y,    &sx, &sy);
            plotline2(ROUND_F2I(prev_x), ROUND_F2I(prev_y), ROUND_F2I(sx), ROUND_F2I(sy));
            prev_x = sx;
            prev_y = sy;
          }
        }
        else {
          plotline2(ROUND_F2I(px[2]), ROUND_F2I(py[2]), ROUND_F2I(px[3]), ROUND_F2I(py[3]));
        }

      }
      else { // count == 5
        float t1x = (px[2] - px[0]) / 2.0f;
        float t1y = (py[2] - py[0]) / 2.0f;
        float t2x = (px[3] - px[1]) / 2.0f;
        float t2y = (py[3] - py[1]) / 2.0f;

        float prev_x = px[1], prev_y = py[1];
        if(!ifAnyMax(px, py, 5)) {
          int steps = min(1, max(maxSteps, z[1] + z[2]));
          for(int i = 1; i <= steps; i++) {
            float sx, sy;
            evalHermite((float)i / steps, px[1], px[2], t1x, t2x, py[1], py[2], t1y, t2y, &sx, &sy);
            plotline2(ROUND_F2I(prev_x), ROUND_F2I(prev_y), ROUND_F2I(sx), ROUND_F2I(sy));
            prev_x = sx;
            prev_y = sy;
          }
        }
        else {
          plotline2(ROUND_F2I(px[2]), ROUND_F2I(py[2]), ROUND_F2I(px[3]), ROUND_F2I(py[3]));
        }

        t1x = (px[3] - px[1]) / 2.0f;
        t1y = (py[3] - py[1]) / 2.0f;
        t2x = (px[4] - px[2]) / 2.0f;
        t2y = (py[4] - py[2]) / 2.0f;

        prev_x = px[2], prev_y = py[2];
        if(!ifAnyMax(px, py, 5)) {
          int steps = min(1, max(maxSteps, z[2] + z[3] + z[4]));
          for(int i = 1; i <= steps; i++) {
            float sx, sy;
            evalHermite((float)i / steps, px[2], px[3], t1x, t2x, py[2], py[3], t1y, t2y, &sx, &sy);
            plotline2(ROUND_F2I(prev_x), ROUND_F2I(prev_y), ROUND_F2I(sx), ROUND_F2I(sy));
            prev_x = sx;
            prev_y = sy;
          }
        }
      }
    }
  }
  else if(final_segment && count == 5) {
    float t1x = (px[4] - px[2]) / 2.0f;
    float t1y = (py[4] - py[2]) / 2.0f;
    float t2x = (px[4] - px[3]) / 2.0f;
    float t2y = (py[4] - py[3]) / 2.0f;

    if(!ifAnyMax(px, py, count)) {
      float prev_x = px[3], prev_y = py[3];
      int steps = min(1, max(maxSteps, z[3] + z[4]));
      for(int i = 1; i <= steps; i++) {
        float sx, sy;
        evalHermite((float)i / steps,    px[3], px[4], t1x, t2x,    py[3], py[4], t1y, t2y,    &sx, &sy);
        plotline2(ROUND_F2I(prev_x), ROUND_F2I(prev_y), ROUND_F2I(sx), ROUND_F2I(sy));

        prev_x = sx;
        prev_y = sy;
      }
    }
    else {
      plotline2(ROUND_F2I(px[2]), ROUND_F2I(py[2]), ROUND_F2I(px[3]), ROUND_F2I(py[3]));
    }
  }
  #endif // USECURVES
}




//Exhange the name of this routine with pixelline() above to try Bresenham
void pixelline(int16_t xo, int16_t yo, int16_t xn, int16_t yn, bool_t vmNormal) { // Plots line from xo,yo to xn,yn; uses temporary x1,y1
  #if defined(STATDEBUG_VERBOSE) && defined(PC_BUILD)
    printf("pixelline 1: xo,yo,  xn,yn: %d,%d    %d,%d \n", xo, yo, xn, yn);
  #endif // STATDEBUG_VERBOSE && PC_BUILD

  //Bresenham line drawing: Pauli's link. Also here: http://forum.6502.org/viewtopic.php?f=10&t=2247&start=555
  int dx =  abs((int)(xn) - (int)(xo)), sx = (xo < xn ? 1 : -1);
  int dy = -abs((int)(yn) - (int)(yo)), sy = (yo < yn ? 1 : -1);
  int err = dx + dy, e2; /* error value e_xy */

  for(;;) {
    if(vmNormal) {
      placePixel(xo, yo);
    }
    else {
      removePixel(xo, yo);
    }
    if(xo==xn && yo==yn) {
      break;
    }
    e2 = 2*err;
    if(e2 >= dy) { /* e_xy+e_x > 0 */
      err += dy;
      xo += sx;
    }
    if(e2 <= dx) { /* e_xy+e_y < 0 */
      err += dx;
      yo += sy;
    }
  }
}



void graphAxisDraw (void){
#if defined(OPTION_GRAPHICS)
  if(realIsSpecial(x_min) || realIsSpecial(x_max) || realIsSpecial(y_min) || realIsSpecial(y_max)
     || realCompareGreaterEqual(x_min, x_max) || realCompareGreaterEqual(y_min, y_max)) {   //rejects NaN/infinite, reversed, empty and never-autoscaled ranges (the +-1E38 seeds); any magnitude is drawable
    return;
  }
  uint32_t cnt;

  clearScreenPixels();
  //GRAPH ZERO AXIS
  yzero = screen_window_y_r(y_min, const_0, y_max);
  xzero = screen_window_x_r(x_min, const_0, x_max);

  uint32_t minnx, minny;
  minny = 0;
  minnx = SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH;


  //SEPARATING LINE
  cnt = minny;
  while(cnt!=SCREEN_HEIGHT_GRAPH) {
    setBlackPixel(minnx-1, cnt);
    setBlackPixel(minnx-2, cnt);
    cnt++;
  }

    #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("xzero=%d yzero=%d   \n", (int)xzero, (int)yzero);
    #endif // STATDEBUG && PC_BUILD

  double x;
  double y;
  double fx_min, fx_max, fy_min, fy_max;               //tick loops step in double; the interval underflows to 0 only beyond 1e+-308 and the tick_int_* > 0 guards then skip the ticks
  realToDouble(x_min, &fx_min);
  realToDouble(x_max, &fx_max);
  realToDouble(y_min, &fy_min);
  realToDouble(y_max, &fy_max);

  if( PLOT_AXIS && !(yzero == SCREEN_HEIGHT_GRAPH-1 || yzero == minny)) {
    //DRAW XAXIS
    cnt = minnx;

    while(cnt != SCREEN_WIDTH_GRAPH - 1) {
      setBlackPixel(cnt, yzero);
        #if defined(STATDEBUG) && defined(PC_BUILD)
        printf("cnt=%d   \n", (int)cnt);
        #endif // STATDEBUG && PC_BUILD
      cnt++;
    }

   force_refresh(timed);

   if(tick_int_x > 0 && 0<fx_max && 0>fx_min) {
     for(x=0; x<=fx_max; x+=tick_int_x) {                           //draw x ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
         printf(">> x=%d   \n", (int)x);
          #endif // STATDEBUG && PC_BUILD
       cnt = screenX(x);
       //printf(">>>>>A %f %d ", x, cnt);
       setBlackPixel(cnt, min(yzero+1, SCREEN_HEIGHT_GRAPH-1));    //tick
       setBlackPixel(cnt, max(yzero-1, minny));                    //tick
     }
     for(x=0; x>=fx_min; x+=-tick_int_x) {                         //draw x ticks
       cnt = screenX(x);
       //printf(">>>>>B %f %d ", x, cnt);
       setBlackPixel(cnt, min(yzero+1, SCREEN_HEIGHT_GRAPH-1));    //tick
       setBlackPixel(cnt, max(yzero-1, minny));                    //tick
     }
     for(x=0; x<=fx_max; x+=tick_int_x*5) {                        //draw x ticks
       cnt = screenX(x);
       setBlackPixel(cnt, min(yzero+2, SCREEN_HEIGHT_GRAPH-1));    //tick
       setBlackPixel(cnt, max(yzero-2, minny));                    //tick
       setBlackPixel(cnt, min(yzero+3, SCREEN_HEIGHT_GRAPH-1));    //tick
       setBlackPixel(cnt, max(yzero-3, minny));                    //tick
     }
     for(x=0; x>=fx_min; x+=-tick_int_x*5) {                       //draw x ticks
       cnt = screenX(x);
       setBlackPixel(cnt, min(yzero+2, SCREEN_HEIGHT_GRAPH-1));    //tick
       setBlackPixel(cnt, max(yzero-2, minny));                    //tick
       setBlackPixel(cnt, min(yzero+3, SCREEN_HEIGHT_GRAPH-1));    //tick
       setBlackPixel(cnt, max(yzero-3, minny));                    //tick
     }
   }
   else if(tick_int_x > 0) {
     for(x=fx_min; x<=fx_max; x+=tick_int_x) {                     //draw x ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>x=%d   \n", (int)x);
          #endif // STATDEBUG && PC_BUILD
        cnt = screenX(x);
        //printf(">>>>>A %f %d ", x, cnt);
          setBlackPixel(cnt, min(yzero+1, SCREEN_HEIGHT_GRAPH-1)); //tick
          setBlackPixel(cnt, max(yzero-1, minny));                 //tick
       }
      for(x=fx_min; x<=fx_max; x+=tick_int_x*5) {                  //draw x ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>>x=%d   \n", (int)x);
          #endif // STATDEBUG && PC_BUILD
        cnt = screenX(x);
          setBlackPixel(cnt, min(yzero+2, SCREEN_HEIGHT_GRAPH-1)); //tick
          setBlackPixel(cnt, max(yzero-2, minny));                 //tick
          setBlackPixel(cnt, min(yzero+3, SCREEN_HEIGHT_GRAPH-1)); //tick
          setBlackPixel(cnt, max(yzero-3, minny));                 //tick
       }
     }
   }

  if( PLOT_AXIS && !(xzero == SCREEN_WIDTH-1 || xzero == minnx)) {
    //Write North arrow
    if(getSystemFlag(FLAG_NVECT)) {
      char tmpString2[100];
      showString("N", &standardFont, xzero-4, minny+14, vmNormal, true, true);
      showString("x", &standardFont, xzero-4, minny+28, vmNormal, true, true);
      tmpString2[0]=(char)((uint8_t)0x80 | (uint8_t)0x22);
      tmpString2[1]=0x06;
      tmpString2[2]=0;
      showString(tmpString2, &standardFont, xzero-4, minny+0, vmNormal, true, true);
    }

    //DRAW YAXIS
    lcd_fill_rect(xzero, minny, 1, SCREEN_HEIGHT_GRAPH-minny, LCD_EMPTY_VALUE);

    //printf("PLOT_ZMY=%i tick_int_x=%f, tick_int_y=%f\n",PLOT_ZMY, tick_int_x, tick_int_y);


    force_refresh(timed);
    if(tick_int_y > 0 && 0<fy_max && 0>fy_min) {
      for(y=0; y<=fy_max; y+=tick_int_y) {                     //draw y ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>y=%d   \n", (int)y);
          #endif // STATDEBUG && PC_BUILD
        cnt = screenY(y);
        setBlackPixel(max(xzero-1, 0), cnt);                    //tick
        setBlackPixel(min(xzero+1, SCREEN_WIDTH_GRAPH-1), cnt); //tick
      }
      for(y=0; y>=fy_min; y+=-tick_int_y) {                     //draw y ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>y=%d   \n", (int)y);
          #endif // STATDEBUG && PC_BUILD
        cnt = screenY(y);
        setBlackPixel(max(xzero-1, 0), cnt);                    //tick
        setBlackPixel(min(xzero+1, SCREEN_WIDTH_GRAPH-1), cnt); //tick
      }
      for(y=0; y<=fy_max; y+=tick_int_y*5) {                    //draw y ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>>y=%d   \n", (int)y);
          #endif // STATDEBUG && PC_BUILD
        cnt = screenY(y);
        setBlackPixel(max(xzero-2, 0), cnt);                    //tick
        setBlackPixel(min(xzero+2, SCREEN_WIDTH_GRAPH-1), cnt); //tick
        setBlackPixel(max(xzero-3, 0), cnt);                    //tick
        setBlackPixel(min(xzero+3, SCREEN_WIDTH_GRAPH-1), cnt); //tick
      }
      for(y=0; y>=fy_min; y+=-tick_int_y*5) {                   //draw y ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>>>y=%d   \n", (int)y);
          #endif // STATDEBUG && PC_BUILD
        cnt = screenY(y);
        setBlackPixel(max(xzero-2, 0), cnt);                    //tick
        setBlackPixel(min(xzero+2, SCREEN_WIDTH_GRAPH-1), cnt); //tick
        setBlackPixel(max(xzero-3, 0), cnt);                    //tick
        setBlackPixel(min(xzero+3, SCREEN_WIDTH_GRAPH-1), cnt); //tick
      }
    }
    else if(tick_int_y > 0) {
      for(y=fy_min; y<=fy_max; y+=tick_int_y) {                 //draw y ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>>>>>y=%d   \n", (int)y);
          #endif // STATDEBUG && PC_BUILD
        cnt = screenY(y);
        setBlackPixel(max(xzero-1, 0), cnt);                    //tick
        setBlackPixel(min(xzero+1, SCREEN_WIDTH_GRAPH-1), cnt); //tick
      }
      for(y=fy_min; y<=fy_max; y+=tick_int_y*5) {               //draw y ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>>>>>>y=%d   \n", (int)y);
          #endif // STATDEBUG && PC_BUILD
        cnt = screenY(y);
        setBlackPixel(max(xzero-2, 0), cnt);                    //tick
        setBlackPixel(min(xzero+2, SCREEN_WIDTH_GRAPH-1), cnt); //tick
        setBlackPixel(max(xzero-3, 0), cnt);                    //tick
        setBlackPixel(min(xzero+3, SCREEN_WIDTH_GRAPH-1), cnt); //tick
      }
    }
  }
  //printf("PLOT_ZMY=%i tick_int_x=%f, tick_int_y=%f\n",PLOT_ZMY, tick_int_x, tick_int_y);
  force_refresh(timed);
//  #endif
#endif //OPTION_GRAPHICS
}


double auto_tick(double tick_int_f) {
  #if defined(OPTION_GRAPHICS)
    if(!roundedTicks) {
      return tick_int_f;
    }
    //Obtain scaling of ticks, to about 20 intervals left to right.
    //Numeric mantissa/exponent split; the previous "%.1e" string surgery needs _printf_float which is no longer linked on DMCP
    double tick_m = fabs((double)tick_int_f);
    double tick_mult = 1.0;
    if(tick_m > 0) {
      //One iteration per decade of the value; this is the loop the log10/pow version replaced, restored
      //so the last double log10/pow leaves the link
      while(tick_m < 1.0) {
        tick_m *= 10.0;
        tick_mult /= 10.0;
      }
      while(tick_m >= 10.0) {
        tick_m /= 10.0;
        tick_mult *= 10.0;
      }
      tick_m = (double)(int32_t)(tick_m * 10.0 + 0.5) / 10.0; //round mantissa to 1 decimal as "%.1e" did; 10 <= argument < 101 so the cast is the floor
      if(tick_m >= 10.0) {
        tick_m /= 10.0;
        tick_mult *= 10.0;
      }
    }
    tick_int_f = tick_m;
    double tick_int_f_mult = tick_mult;

    if(tick_int_f > 0) {
      //if(tick_int_f <= 0.3) {
      //  tick_int_f = 0.2f;
      //}
      //else if(tick_int_f <= 0.6) {
      //  tick_int_f = 0.5f;
      //}
      //else
      if(tick_int_f <= 1.3) {
        tick_int_f = 1.0;
      }
      else if(tick_int_f <= 1.7) {
        tick_int_f = 1.5;
      }
      else if(tick_int_f <= 3.0) {
        tick_int_f = 2.0;
      }
      else if(tick_int_f <= 6.5) {
        tick_int_f = 5.0;
      }
      else if(tick_int_f <= 9.9) {
        tick_int_f = 7.5;
      }
      //no higher values than 9.9 possible
    }
    else { //tick_int_f == 0
      tick_int_f = 1;
    }
    tick_int_f *= tick_int_f_mult;

    //printf("tick2 %f\n",tick_int_f);
  #endif // OPTION_GRAPHICS

return tick_int_f;
}


void graph_axis (void){
#if defined(OPTION_GRAPHICS)
    real_t w;
    double wd;
    realSubtract(x_max, x_min, &w, &ctxtReal39);   // w = x_max - x_min
    realToDouble(&w, &wd);
    tick_int_x = auto_tick(wd/20.0);               // tick_int_x = auto_tick((x_max - x_min) / 20)

    realSubtract(y_max, y_min, &w, &ctxtReal39);   // w = y_max - y_min
    realToDouble(&w, &wd);
    tick_int_y = auto_tick(wd/20.0);               // tick_int_y = auto_tick((y_max - y_min) / 20)

    #if defined(STATDEBUG)
     printf("TICKS X=%f Y=%f\n", tick_int_x, tick_int_y);
    #endif // STATDEBUG


  graphAxisDraw();
#endif //OPTION_GRAPHICS
}


char * radixProcess(char *output, const char * ss) {  //  .  HIERDIE WERK GLAD NIE
  int8_t ix = 0, iy = 0;

  while( ss[ix] != 0 ){
    if(ss[ix]==',' || ss[ix]=='.') {
      output[iy++] = RADIX34_MARK_CHAR;
    }
    else if(ss[ix]=='#') {
      output[iy++] = ';';
    }
    else {
      output[iy++] = ss[ix];
    }
    ix++;
  }
  output[iy] = '\0';
  return output;
}

void nanCheck(char* s02) { //eg. change (nanE-3 or ;nanE-3) to NaN
  if(stringByteLength(s02) > 2) {
    for(int ix = 2; s02[ix]!=0; ix++) {
      if(s02[ix]=='n' && s02[ix-1]=='a' && s02[ix-2]=='n') { //check for nan
        if(s02[0] == '(' && s02[ix+1] != 0) {
          strcpy(s02, "(NaN");
        }
        else if(s02[0] == ';' && s02[stringByteLength(s02)-1] == ')' && s02[ix+1] != 0 && s02[ix+2] != 0) {
          strcpy(s02, ";NaN)");
        }
      }
    }
  }
}

char * padEquals(char *output, const char * ss) {
  int8_t ix = 0, iy = 0;

  while( ss[ix] != 0 ){
    if(!(ss[ix] & 0x80)) {
      if(ss[ix]=='=') {
        output[iy++] = STD_SPACE_PUNCTUATION[0];
        output[iy++] = STD_SPACE_PUNCTUATION[1];
        output[iy++] = '=';
        output[iy++] = STD_SPACE_PUNCTUATION[0];
        output[iy++] = STD_SPACE_PUNCTUATION[1];
      }
      else {
        output[iy++] = ss[ix];
      }
    }
    else {
      output[iy++] = ss[ix];
      if(ss[ix+1] != 0) {
        output[iy++] = ss[++ix];
      }
    }
    ix++;
  }
  output[iy] = '\0';
  return output;
}

char * smallE(char *output, const char * ss) {
  int8_t ix = 0, iy = 0;

  while( ss[ix] != 0 ){
    if(!(ss[ix] & 0x80)) {
      if(ss[ix]=='E') {
        output[iy++] = STD_SUB_E[0];
        output[iy++] = STD_SUB_E[1];
      }
      else {
        output[iy++] = ss[ix];
      }
    }
    else {
      output[iy++] = ss[ix];
      if(ss[ix+1] != 0) {
        output[iy++] = ss[++ix];
      }
    }
    ix++;
  }
  output[iy] = '\0';
  return output;
}







//**************************************************************************************************************

  static int checkWidthWithPrefix(const char* itemName, const char* numStr, uint32_t max_width) {
    char test_buffer[128];
    snprintf(test_buffer, sizeof(test_buffer), "%s%s", itemName, numStr);
      //printf("ssss=%d\n",stringWidthC47(test_buffer, stdNoEnlarge, !nocompress, false, false));
      return stringWidthC47(test_buffer, stdNoEnlarge, !nocompress, false, false) < max_width;
  }


  static void cleanupTrailingZeros(char* str) {
    char* e_pos = strchr(str, 'E');
    if(!e_pos) {
      e_pos = strchr(str, 'e');
    }
    if(e_pos && *e_pos == 'e') {
      *e_pos = 'E';
    }
    if(e_pos) {
      char* decimal_pos = strchr(str, '.');
      if(decimal_pos && decimal_pos < e_pos) {
        char* p = e_pos - 1;
        while(p > decimal_pos && *p == '0') {
          p--;
        }
        if(p == decimal_pos) {
          memmove(decimal_pos, e_pos, strlen(e_pos) + 1);
        }
        else {
          memmove(p + 1, e_pos, strlen(e_pos) + 1);
        }
      }
      e_pos = strchr(str, 'E');
      if(e_pos) {
        char* exp_start = e_pos + 1;
        if(*exp_start == '+') {
          exp_start++;
        }
        if(*exp_start == '-') {
          exp_start++;
        }
        while(*exp_start == '0' && *(exp_start + 1) != '\0') {
          memmove(exp_start, exp_start + 1, strlen(exp_start + 1) + 1);
        }
      }
    }
    else {
      char* decimal_pos = strchr(str, '.');
      if(decimal_pos) {
        int len = strlen(str);
        int i = len - 1;
        while(i > decimal_pos - str && str[i] == '0') {
          str[i] = '\0';
          i--;
        }
        if(i == decimal_pos - str) {
          str[i] = '\0';
        }
      }
    }
  }


  // On a narrow key the radix mark of 1.2E6 is the one glyph that will not fit. The same number with the mantissa digits shifted left and the exponent taken down by the
  // same count needs no mark: 12E5. Second choice only, tried after the marked form has been refused, so a key with room for 1.2E6 still draws it.
  static void dropRadixMark(char* str) {
    char* decimal_pos = strchr(str, '.');
    char* e_pos       = decimal_pos ? strchr(decimal_pos, 'E') : NULL;
    if(e_pos == NULL || e_pos == decimal_pos + 1) {
      return;
    }
    const int shift    = (int)(e_pos - decimal_pos) - 1;                                    // the digits that stood after the mark, and how far the exponent comes down
    const int exponent = atoi(e_pos + 1) - shift;
    const int left     = (int)(decimal_pos - str);
    char      mantissa[40];
    if(exponent == 0 || left + shift >= (int)sizeof(mantissa)) {
      return;                                                                               // 12E0 says nothing a plain 12 does not
    }
    memcpy(mantissa, str, left);
    memcpy(mantissa + left, decimal_pos + 1, shift);
    mantissa[left + shift] = 0;
    sprintf(str, "%sE%d", mantissa, exponent);
  }


//success flag set if convertion was done and maxwidth is NOT overreached. Additionally ?? is output when no conversion is done.
char* formatDoubleWidth(real34_t *real34, int digits, char* itemName, bool_t* success, int actual_max_width, char* buf, int digitswidthLimit) {
    uint8_t savedDisplayFormatDigits = displayFormatDigits;
    uint8_t saveddisplayFormat = displayFormat;
    bool_t  ovrENG = getSystemFlag(FLAG_ENGOVR);
    clearSystemFlag(FLAG_ENGOVR);
    if(real34IsZero(real34)) {
      strcpy(buf, "0");
      *success = 1;
      return buf;
    }

    real_t reall10, real;
    real34ToReal(real34, &real);
    bool isNegative = realIsNegative(&real);
    real_t threshold9E99, threshold1E_99;
    stringToReal("9E99", &threshold9E99, &ctxtReal39);
    stringToReal("1E-99", &threshold1E_99, &ctxtReal39);
    if(realCompareAbsGreaterThan(&real, &threshold9E99)) {
      strcpy(buf, isNegative ? STD_GAUSS_WHITE_L STD_GAUSS_WHITE_L STD_GAUSS_WHITE_L /* <<< */ : STD_GAUSS_WHITE_R STD_GAUSS_WHITE_R STD_GAUSS_WHITE_R /* >>> */);
      *success = 1;
      goto done;
    }
    if(realCompareAbsLessThan(&real, &threshold1E_99)) {
      strcpy(buf, isNegative ? STD_GAUSS_WHITE_L STD_SUB_0 : STD_GAUSS_WHITE_R STD_SUB_0);   //  "<0" : ">0");
      *success = 1;
      goto done;
    }
    realLog10(&real, &reall10, &ctxtReal39);
    const int32_t exp10 = realToInt32C47(&reall10, NULL);
    for(int ddd = 8; ddd >= 2; ddd--) {
      if(exp10 < digits) {                                                       // the integer part fits the significant count, so SIG runs the whole ladder here
        displayFormat = DF_SF;                                                   // and ends on one digit at ddd 2, the same floor the SCI arm below has
        displayFormatDigits = min(digits, ddd - 2);
      }
      else if(exp10 < ddd) {
        displayFormat = DF_FIX;                                                  // the whole integer fits in ddd digits, so it is written out with no ten exponent
        displayFormatDigits = 0;
      }
      else {                                                                     // ddd digits cannot hold the number, so real34ToDisplayString2 leaves its FIX block and
        displayFormat = DF_SF;                                                   // prints the SCI form, whose mantissa length is displayFormatDigits. FIX has no
        displayFormatDigits = min(digits, ddd - 2);                              // such count, and its 0 gave that form one digit: 1234567 as 1E6. SF is the mode that
      }                                                                          // holds one. The ladder ends on that same single digit at ddd 2, so nothing that used
                                                                                 // to fit stops fitting
      updateDisplayValueX = true;
      displayValueX[0] = 0;
      real34ToDisplayString(real34, amNone, buf, &standardFont, digitswidthLimit == 0 ? 60 : digitswidthLimit, ddd, LIMITEXP, !FRONTSPACE, NOIRFRAC);
      updateDisplayValueX = false;
      strcpy(buf, displayValueX);
      cleanupTrailingZeros(buf);

      if(checkWidthWithPrefix(itemName, buf, actual_max_width)) {
         *success = false;
         goto done;
      }
      dropRadixMark(buf);                                                        // the marked form was refused, so offer the same digits without the mark
      if(checkWidthWithPrefix(itemName, buf, actual_max_width)) {
         *success = false;
         goto done;
      }
    }
    strcpy(buf, "??");
    *success = 0;
done:
    displayFormatDigits = savedDisplayFormatDigits;
    displayFormat = saveddisplayFormat;
    if(ovrENG) {
      setSystemFlag(FLAG_ENGOVR);
    }
    else {
      clearSystemFlag(FLAG_ENGOVR);
    }
  return buf;
}

char* formatCore(double value, int digits, bool handle_zero, char* buf, int widthLimit) {
  const char* sign = (value < 0.0) ? "-" : "";
  if(value < 0.0) {
    value = -value;
  }

  if(handle_zero && value == 0.0) {
    sprintf(buf, "%s0.0", sign);
  }
  else {
    real34_t value34;
    real_t valueR;
    bool_t success;
    char tmpBuf[150];
    convertDoubleToReal(value, &valueR, &ctxtReal39);
    realToReal34(&valueR, &value34);
    strcpy(buf, sign);
    strcat(buf, formatDoubleWidth(&value34, digits, "", &success, widthLimit == 0 ? 50 : widthLimit, tmpBuf, widthLimit == 0 ? 50 : widthLimit));
  }
  radixProcess(buf, buf);
  return  buf;//radixProcess(buf2, buf);
}

void grphNumFormatter(char* s02, const char* s01, double inreal, int8_t digits, const char* s05) {
  char format_buf[64];
  formatCore(inreal, digits, false, format_buf, 70);
  sprintf(s02, "%s%s%s", s01, format_buf, s05);
  nanCheck(s02);
}

//  void test_format_functions() {
//      printf("Testing formatCore and grphNumFormatter with digits 2-8 horizontally\n");
//      printf("Input Value\t\t");
//      for(int d = 2; d <= 8; d++) {
//          printf("formatCore(%d)\tgrphNumFormatter(%d)\t", d, d);
//      }
//      printf("\n");
//
//      printf("===========\t\t");
//      for(int d = 2; d <= 8; d++) {
//          printf("============\t================\t");
//      }
//      printf("\n");
//
//      // Test values spanning the full range
//      double test_values[] = {
//          1.123e-130, 5.67e-120, 2.34e-100, 8.91e-80, 4.56e-60,
//          7.89e-40, 1.23e-30, 6.78e-20, 3.45e-15, 9.12e-10,
//          2.34e-8, 5.67e-6, 1.23e-4, 4.56e-3, 7.89e-2,
//          0.123, 0.456, 0.789, 1.23, 4.56, 7.89,
//          12.3, 45.6, 78.9, 123.0, 456.0, 789.0,
//          1.23e3, 4.56e3, 7.89e3, 1.23e4, 4.56e4, 7.89e4,
//          1.23e6, 4.56e8, 7.89e10, 1.23e15, 4.56e20, 7.89e25,
//          1.23e30, 4.56e40, 7.89e50, 1.23e60, 4.56e80, 7.89e100,
//          1.23e120, 1.0e130,
//          // Negative values
//          -1.123e-130, -5.67e-100, -2.34e-50, -8.91e-20, -4.56e-10,
//          -7.89e-5, -1.23e-2, -0.456, -7.89, -123.0, -4.56e3,
//          -7.89e10, -1.23e30, -4.56e60, -1.0e100,
//          // Special cases
//          0.0, -0.0, 1.0, -1.0, 10.0, -10.0, 100.0, -100.0
//      };
//
//      int num_values = sizeof(test_values) / sizeof(test_values[0]);
//      char result_buf[256];
//
//      for(int i = 0; i < num_values; i++) {
//          printf("%.3e\t\t", test_values[i]);
//
//          for(int digits = 2; digits <= 8; digits++) {
//              formatCore(test_values[i], digits, false, result_buf, 256);
//              printf("%s\t", result_buf);
//              grphNumFormatter(result_buf, "[", test_values[i], digits, "]");
//              printf("%s\t", result_buf);
//          }
//          printf("\n");
//
//          if(i % 15 == 14) {
//            printf("\n"); // Add spacing every 15 values
//          }
//      }
//
//      printf("\nTotal test cases: %d values × 7 digit settings × 2 functions = %d tests\n", num_values, num_values * 7 * 2);
//  }



#define horOffsetR 109+5 //digit righ side aliognment
#define autoinc 19 //text line spacing
#define autoshift -4 //text line offset
#define horOffset 1 //labels from the left


  int32_t statMxN(void) {
    uint16_t rows = 0;

    if(plotStatMx[0]=='D') {
      return 0;                //Only allow S and H
    }
    else {
      calcRegister_t regStats = findNamedVariable(plotStatMx);
      if(regStats == INVALID_VARIABLE) {
        return 0;
      }
      else {
        if(isStatsMatrix(&rows, plotStatMx)) {
          real34Matrix_t stats;
          linkToRealMatrixRegister(regStats, &stats);
          return stats.header.matrixRows;
        }
        else {
          return 0;
        }
      }
    }
  }

void plotPointGeneric(int16_t xn, int16_t yn, int16_t xo, int16_t yo, bool_t PLOT_CROSS, bool_t PLOT_BOXFAT, bool_t PLOT_BOX, bool_t PLOT_PLUS, bool_t PLOT_LINE) {
  if(PLOT_CROSS) {
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("Plotting cross to x=%d y=%d\n", xn, yn);
    #endif // STATDEBUG && PC_BUILD
    plotcross(xn, yn);
  }
  else if(PLOT_BOXFAT) {
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("Plotting box to x=%d y=%d\n", xn, yn);
    #endif // STATDEBUG && PC_BUILD
    plotbox_fat(xn, yn);
  }
  else if(PLOT_BOX) {
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("Plotting box to x=%d y=%d\n", xn, yn);
    #endif // STATDEBUG && PC_BUILD
    plotbox(xn, yn);
  }
  else if(PLOT_PLUS) {
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("Plotting plus to x=%d y=%d\n", xn, yn);
    #endif // STATDEBUG && PC_BUILD
    plotplus(xn, yn);
  }

  if(PLOT_LINE) {
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("Plotting line to x=%d y=%d\n", xn, yn);
    #endif // STATDEBUG && PC_BUILD
    plotline1(xo, yo, xn, yn);
  }
}



//ASSESS; HISTO; SCATR

void graphPlotstat(uint16_t selection){
currentKeyCode = 255;
#if defined(OPTION_GRAPHICS)
  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("#####>>> graphPlotstat: selection:%u:%s  lastplotmode:%u  lrSelection:%u lrChosen:%u\n", selection, getCurveFitModeName(selection), lastPlotMode, lrSelection, lrChosen);
  #endif // STATDEBUG && PC_BUILD
  uint16_t  cnt, ix, numberOfPlotPoints;
  int16_t  xo, xn, xN;
  int16_t  yo, yn, yN;
  real_t xr, yr;

  numberOfPlotPoints = 0;
  roundedTicks = false;

  if((plotStatMx[0]=='S' && checkMinimumDataPoints(const_2)) ||
     (plotStatMx[0]=='D' && drawMxN() >= 2) ||
     (plotStatMx[0]=='H' && statMxN() >= 3)) {
    switch(plotStatMx[0]) {
      case 'S': numberOfPlotPoints = realToInt32C47(SIGMA_N, NULL); break;
      case 'D': numberOfPlotPoints = drawMxN();                     break;
      case 'H': numberOfPlotPoints = statMxN();                     break;
      default: ;
    }
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("graphPlotstat: numberOfPlotPoints n=%d\n", numberOfPlotPoints);
    #endif // STATDEBUG && PC_BUILD


    if(reDraw) {
      regStatsXY = findNamedVariable(plotStatMx);
      //  graphAxisDraw();                        //Draw the axis on any uncontrolled scale to start. Maybe optimize by remembering if there is an image on screen Otherwise double axis draw.
      graph_axis();
      plotmode = _SCAT;

      #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
        printf("       graphPlotstat: Drawing, text only\n");
      #endif // PC_BUILD &&MONITOR_CLRSCR
      reDraw = false;
      clearScreenGraphs(3, !clrTextArea, clrGraphArea);

      //AUTOSCALE
      convertDoubleToReal(FLoatingMax, x_min, &ctxtReal39);                // seed the range impossibly wide open: x_min = +1e38, x_max = -1e38,
      convertDoubleToReal(FLoatingMin, x_max, &ctxtReal39);                //   so the first data point replaces both
      convertDoubleToReal(FLoatingMax, y_min, &ctxtReal39);
      convertDoubleToReal(FLoatingMin, y_max, &ctxtReal39);
      #if defined(STATDEBUG) && defined(PC_BUILD)
        printf("Axis0: x: %f -> %f y: %f -> %f   \n", dbl(x_min), dbl(x_max), dbl(y_min), dbl(y_max));
      #endif // STATDEBUG && PC_BUILD


    //#################################################### vvv SCALING LOOP  vvv
    for(cnt=0; (cnt < numberOfPlotPoints); cnt++) {
      #if defined(STATDEBUG) && defined(PC_BUILD)
        printf("Axis0a: x: %f y: %f   \n", grf_x(cnt), grf_y(cnt));
      #endif // STATDEBUG && PC_BUILD
      grf_x_r(cnt, &xr);                             // xr = grf_x(cnt)
      grf_y_r(cnt, &yr);                             // yr = grf_y(cnt)
      if(realCompareLessThan(&xr, x_min)) {          // if(grf_x(cnt) < x_min) x_min = grf_x(cnt)
        realCopy(&xr, x_min);
      }
      if(realCompareGreaterThan(&xr, x_max)) {       // if(grf_x(cnt) > x_max) x_max = grf_x(cnt)
        realCopy(&xr, x_max);
      }
      if(realCompareLessThan(&yr, y_min)) {          // if(grf_y(cnt) < y_min) y_min = grf_y(cnt)
        realCopy(&yr, y_min);
      }
      if(realCompareGreaterThan(&yr, y_max)) {       // if(grf_y(cnt) > y_max) y_max = grf_y(cnt)
        realCopy(&yr, y_max);
      }
      #if defined(STATDEBUG) && defined(PC_BUILD)
        printf("Axis0b: x: %f -> %f y: %f -> %f   \n", dbl(x_min), dbl(x_max), dbl(y_min), dbl(y_max));
      #endif // STATDEBUG && PC_BUILD
      if(exitKeyWaiting()) {
        return;
      }
    }
    //##  ################################################## ^^^ SCALING LOOP ^^^
      #if defined(STATDEBUG) && defined(PC_BUILD)
        printf("Axis1a: x: %f -> %f y: %f -> %f   \n", dbl(x_min), dbl(x_max), dbl(y_min), dbl(y_max));
      #endif // STATDEBUG && PC_BUILD

      if((realIsInfinite(x_min) && realIsNegative(x_min)) || (realIsInfinite(x_max) && realIsNegative(x_max)) || (realIsInfinite(y_min) && realIsNegative(y_min)) || (realIsInfinite(y_max) && realIsNegative(y_max))) {
        goto scaleMinusInfinity;   //-infinite data; previously was x <= -1E38
      }
      if(realIsSpecial(x_min) || realIsSpecial(x_max) || realIsSpecial(y_min) || realIsSpecial(y_max) || realCompareGreaterThan(x_min, x_max) || realCompareGreaterThan(y_min, y_max)) {
        goto scalePlusInfinity;    //+infinite or NaN data, or the +-1E38 seeds untouched (no plottable point); any finite magnitude passes
      }


    graph_Include0(PLOTSTAT, numberOfPlotPoints);

    if(drawHistogram == 1 && selection == CF_GAUSS_FITTING) {
      //HNORM window covers the fitted bell: x = a1 +- 3 sigma (1.1% of peak) unioned with the bar range, y_max clears the peak by 10%;
      //a flipped fit (a2 >= 0) has no finite tails and keeps the bar range
      real_t rr, smi, ga0, ga1, ga2, t, sig3;
      processCurvefitSelection(selection, &rr, &smi, &ga0, &ga1, &ga2);
      if(!realIsNaN(&ga0) && !realIsNaN(&ga1) && !realIsNaN(&ga2) && realIsNegative(&ga2)) {
        realMultiply(&ga2, const_1on2, &sig3, &ctxtReal39);          // sigma^2 = -a2/2
        realSetPositiveSign(&sig3);
        realSquareRoot(&sig3, &sig3, &ctxtReal39);                   // sigma
        int32ToReal(3, &t);
        realMultiply(&sig3, &t, &sig3, &ctxtReal39);                 // 3 sigma
        realSubtract(&ga1, &sig3, &t, &ctxtReal39);                  // a1 - 3 sigma
        if(realCompareLessThan(&t, x_min)) {
          realCopy(&t, x_min);
        }
        realAdd(&ga1, &sig3, &t, &ctxtReal39);                       // a1 + 3 sigma
        if(realCompareGreaterThan(&t, x_max)) {
          realCopy(&t, x_max);
        }
        convertDoubleToReal(1.1, &t, &ctxtReal39);
        realMultiply(&ga0, &t, &t, &ctxtReal39);                     // 1.1 * a0
        if(realCompareGreaterThan(&t, y_max)) {
          realCopy(&t, y_max);
        }
      }
    }



    //graphAxisDraw();

//Removed - old portion ex WP43, replaced by graphs.c, where PLSTAT and EQN draw lives - remove when sure
//   Also, the 'D' reference in pltStatMx need to be surgically removed, bearing in mind that the PLSTAT and EQN draw in plotstat may need it.
//printf("XXXX2 plotStatMx=%s\n",plotStatMx);
//    if(calcMode == CM_GRAPH) {
//      roundedTicks = true;
//    }
//    else {
      roundedTicks = false;
//    }

    if((realIsInfinite(x_min) && realIsNegative(x_min)) || (realIsInfinite(x_max) && realIsNegative(x_max)) || (realIsInfinite(y_min) && realIsNegative(y_min)) || (realIsInfinite(y_max) && realIsNegative(y_max))) {
      goto scaleMinusInfinity;     //-infinite range after graph_Include0
    }
    if(realIsSpecial(x_min) || realIsSpecial(x_max) || realIsSpecial(y_min) || realIsSpecial(y_max) || realCompareGreaterThan(x_min, x_max) || realCompareGreaterThan(y_min, y_max)) {
      goto scalePlusInfinity;      //+infinite, NaN or reversed range after graph_Include0
    }



    graph_axis();
    grf_y_r(0, &yr);
    grf_x_r(0, &xr);
    yn = screen_window_y_r(y_min, &yr, y_max);
    xn = screen_window_x_r(x_min, &xr, x_max);
    xN = xn;
    yN = yn;
      #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("Axis3c: x: %f -> %f y: %f -> %f   \n", dbl(x_min), dbl(x_max), dbl(y_min), dbl(y_max));
      #endif // STATDEBUG && PC_BUILD

      grf_x_r(numberOfPlotPoints - 1, &yr);            //yr and xr hold the last and first x values for the bar pitch
      int16_t xLast = screen_window_x_r(x_min, &yr, x_max);
      int16_t barPitch = (int16_t)(  (float)(xLast - xn)
                                   / (float)(numberOfPlotPoints > 1 ? numberOfPlotPoints - 1 : 1) + 0.5f);   //centre-to-centre bar spacing, one integer for all bars
      int16_t barX0 = (int16_t)((xn + xLast - (numberOfPlotPoints - 1) * barPitch + 1) / 2);   //bars go on the integer grid barX0 + ix*barPitch, anchored mid-span so drift splits between both ends
      int16_t colw = (int16_t)(barPitch / 2.0f) - 1;   //half bar width; bar width 2*colw+1 and gap barPitch-2*colw-1 are constant across the plot
        //#################################################### vvv MAIN GRAPH LOOP vvv #########################
      for(ix = 0; (ix < numberOfPlotPoints); ++ix) {
        grf_x_r(ix, &xr);
        grf_y_r(ix, &yr);
        xo = xN;
        yo = yN;
        xN = screen_window_x_r(x_min, &xr, x_max);
        yN = screen_window_y_r(y_min, &yr, y_max);
        if(drawHistogram != 0) {
          xN = (int16_t)(barX0 + ix * barPitch);       //rounding each bin centre separately varied the gaps between bars
        }

        #if defined(STATDEBUG) && defined(PC_BUILD)
          printf("plotting graph table[%d] = x:%f y:%f xN:%d yN:%d drawHistogram:%d ", ix, grf_x(ix), grf_y(ix), xN, yN, drawHistogram);
        #endif // STATDEBUG && PC_BUILD

        int16_t minN_y, minN_x;
        minN_y = 0;
        minN_x = SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH;

        if(xN<SCREEN_WIDTH_GRAPH && xN>=minN_x && yN<SCREEN_HEIGHT_GRAPH && yN>=minN_y) {
          yn = yN;
          xn = xN;

          if(drawHistogram != 0) {
            plotHisto_coln(xN, yN, minN_y, SCREEN_HEIGHT_GRAPH - minN_y, colw);
          }

          plotPointGeneric(xn, yn, xo, yo,
                             getSystemFlag(FLAG_PCROS)  /*cross*/ ,
                             getSystemFlag(FLAG_PBOX)   /*fatbox*/,
                             false                      /*box*/   ,
                             getSystemFlag(FLAG_PPLUS)  /*plus*/  ,
                             getSystemFlag(FLAG_PLINE)  /*line*/   );
        }
        else {
            #if defined(PC_BUILD)
            printf("Not plotted point: (%u %u) ", xN, yN);
              if(xN >= SCREEN_WIDTH_GRAPH) {
                printf("x>>%u ", SCREEN_WIDTH_GRAPH);
              }
              else if(xN < minN_x) {
                printf("x<<%u ", minN_x);
              }

              if(yN >= SCREEN_HEIGHT_GRAPH) {
                printf("y>>%u ", SCREEN_HEIGHT_GRAPH);
              }
              else if(yN < 1+minN_y) {
                printf("y<<%u ", 1+minN_y);
              }
            printf("\n");
            //printf("Not plotted: xN=%d<SCREEN_WIDTH_GRAPH=%d && xN=%d>minN_x=%d && yN=%d<SCREEN_HEIGHT_GRAPH=%d && yN=%d>1+minN_y=%d\n", xN, SCREEN_WIDTH_GRAPH, xN, minN_x, yN, SCREEN_HEIGHT_GRAPH, yN, 1+minN_y);
            #endif // PC_BUILD
        }
        if(exitKeyWaiting()) {
           return;
        }
      }
      //#################################################### ^^^ MAIN GRAPH LOOP ^^^
    }
    else {
      #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
        printf("       graphPlotstat: Not Drawing, text only\n");
      #endif // PC_BUILD &&MONITOR_CLRSCR
      clearScreenGraphs(4, clrTextArea, !clrGraphArea);
    } //continue with text only

    if(drawHistogram == 1 && selection == 0) { // HISTO
      int32_t n;
      float lB, hB, nB;
      real_t lBr, hBr, nBr;
      char ss[100], tt[100];
      char tmpbuf[PLOT_TMP_BUF_SIZE];
      int16_t index = -1;
      real34ToReal(&loBinR, &lBr);
      real34ToReal(&hiBinR, &hBr);
      real34ToReal(&nBins,  &nBr);
      realToFloat(&lBr, &lB);
      realToFloat(&hBr, &hB);
      realToFloat(&nBr, &nB);

      strcpy(ss, "Histogram(");
      strcat(ss, histElementXorY == 1 ? "y)" : "x)");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffset + 17, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -7 +autoshift, vmNormal, false, false);

      double xmaxd, ymaxd, xmind, ymind;
      realToDouble(x_max, &xmaxd);
      realToDouble(y_max, &ymaxd);
      realToDouble(x_min, &xmind);
      realToDouble(y_min, &ymind);
      grphNumFormatter(ss, "(", xmaxd, 2, "");
      grphNumFormatter(tt, radixProcess(tmpbuf, "#"), ymaxd, 2, ")");
      strcat(tt, padEquals(tmpbuf, ss));
      n = showString(padEquals(tmpbuf, ss), &standardFont, 160-2-3-2 - stringWidth(tt, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index + 2   -3  +autoshift, vmNormal, false, false);
      grphNumFormatter(ss, radixProcess(tmpbuf, "#"), ymaxd, 2, ")");
      showString(padEquals(tmpbuf, ss), &standardFont, n+3,           Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++  -3  + autoshift + 2, vmNormal, false, false);

      grphNumFormatter(ss, "(", xmind, 2, "");
      n = showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index    -6  + autoshift + 2, vmNormal, false, false);
      grphNumFormatter(ss, radixProcess(tmpbuf, "#"), ymind, 2, ")");
      showString(padEquals(tmpbuf, ss), &standardFont, n+3,           Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++  -6  + autoshift + 2, vmNormal, false, false);

      strcpy(ss, "Bin centres:");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -4 +autoshift, vmNormal, false, false);
      grphNumFormatter(ss, "", lB, 3, "");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -4 +autoshift, vmNormal, false, false);
      strcpy(ss, STD_DOWN_ARROW "BIN" "=");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -4 +autoshift, vmNormal, false, false);

      grphNumFormatter(ss, "", hB, 3, "");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -1 +autoshift, vmNormal, false, false);
      strcpy(ss, STD_UP_ARROW "BIN" "=");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -1 +autoshift, vmNormal, false, false);

      grphNumFormatter(ss, "", nB, 3, "");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -1 +autoshift, vmNormal, false, false);
      strcpy(ss, "nBINS" "=");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -1 +autoshift, vmNormal, false, false);
      grphNumFormatter(ss, "", (hB-lB)/nB, 3, "");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -1 +autoshift, vmNormal, false, false);
      strcpy(ss, "Width" "=");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -1 +autoshift, vmNormal, false, false);

    }




  }
  else {
    calcMode = CM_NORMAL;
    displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "There is no statistical data available!");
      moreInfoOnError("In function graphPlotstat:", errorMessage, NULL, NULL);
    #endif
  }
 return;


  scalePlusInfinity:
  calcMode = CM_NORMAL;
  displayCalcErrorMessage(ERROR_OVERFLOW_PLUS_INF, ERR_REGISTER_LINE);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    sprintf(errorMessage, "Plus Infinity encountered!");
    moreInfoOnError("In function graphPlotstat:", errorMessage, NULL, NULL);
  #endif
  return;

  scaleMinusInfinity:
  calcMode = CM_NORMAL;
  displayCalcErrorMessage(ERROR_OVERFLOW_MINUS_INF, ERR_REGISTER_LINE);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    sprintf(errorMessage, "Minus Infinity encountered!");
    moreInfoOnError("In function graphPlotstat:", errorMessage, NULL, NULL);
  #endif


#endif //OPTION_GRAPHICS
}


  void demo_plot(void) {
    int8_t ix;
    time_t t;

    srand((unsigned) time(&t));
    runFunction(ITM_CLSIGMA);
    plotSelection = 0;
    srand((unsigned int)time(NULL));
    for(ix=0; ix!=40; ix++) {
      int mv = 11000 + rand() % 110 - 55;
      //instrument measuring RMS voltage of an 11 kV installation, with +- 0.1% variance, offset to the + for convenience

      setSystemFlag(FLAG_ASLIFT);
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      int32ToReal34(mv + rand()%4 - 2, REGISTER_REAL34_DATA(REGISTER_X));
      // reading 1 has additional +0 to +3 variance to the said random number

      setSystemFlag(FLAG_ASLIFT);
      liftStack();
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      int32ToReal34(mv + rand()%4 - 2, REGISTER_REAL34_DATA(REGISTER_X));
      // reading 2 has additional +0 to +3 variance to the said random number

      runFunction(ITM_SIGMAPLUS);
    }
  }


void graphDrawLRline(uint16_t selection) {
  //demo_plot();
  if(selection != 0) {
    processCurvefitSelection(selection, &RR, &SMI, &aa0, &aa1, &aa2);
    realMultiply(&RR, &RR, &RR, &ctxtReal39);
    if(orOrtho(selection) == CF_ORTHOGONAL_FITTING) {
      if(realCompareLessThan(SIGMA_N, const_3)) {  // the standard errors divide by n - 2: two points draw the line without them
        realSetNaN(&sa0);
        realSetNaN(&sa1);
      }
      else {
        processCurvefitSA(&sa0, &sa1);
      }
    }
    drawline(selection, &RR, &SMI, &aa0, &aa1, &aa2, &sa0, &sa1);
  }
}


 static void drawline(uint16_t selection, real_t *RR, real_t *SMI, real_t *aa0, real_t *aa1, real_t *aa2, real_t *sa0, real_t *sa1) {
#if defined(OPTION_GRAPHICS)
    int32_t n = 0;
    uint16_t NN;
    char tmpbuf[PLOT_TMP_BUF_SIZE];

    switch(plotStatMx[0]) {
      case 'S': n = realToInt32C47(SIGMA_N, NULL); break;
      case 'D': n = drawMxN();                     break;
      case 'H': n = statMxN();                     break;
      default: ;
    }
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("drawline: n=%d\n", n);
    #endif // STATDEBUG && PC_BUILD

    NN = (uint16_t) n;
    bool_t isValidDraw =
         selection != 0
      && n >= (int32_t)minLRDataPoints(selection)
      && !realCompareGreaterThan(RR, const_1)
      && !realIsNaN(RR)
      && !realIsNaN(aa0)
      && !realIsNaN(aa1)
      && (!realIsNaN(aa2) || minLRDataPoints(selection)==2)
      && (!realIsNaN(SMI) || !(orOrtho(selection) & CF_ORTHOGONAL_FITTING));

    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("#####>>> drawline: selection:%u:%s  lastplotmode:%u  lrSelection:%u lrChosen:%u\n", selection, getCurveFitModeName(selection), lastPlotMode, lrSelection, lrChosen);
    #endif //  STATDEBUG && PC_BUILD
    float rr, smi, a0, a1, a2, ssa0, ssa1;
    char ss[100], tt[100];

    real_t XX, YY;
    realSetZero(&XX);                                //XX is written only by the USEFLOATING real branches below
    if(!selection) {
      return;
    }

    realToFloat(RR,  &rr );
    realToFloat(SMI, &smi);
    realToFloat(aa0, &a0 );
    realToFloat(aa1, &a1 );
    realToFloat(aa2, &a2 );
    realToFloat(sa0, &ssa0 );
    realToFloat(sa1, &ssa1 );

    if(isValidDraw) {
      #if defined(STATDEBUG) && defined(PC_BUILD)
        printf("plotting line: a2 %f a1 %f a0 %f\n", a2, a1, a0);
      #endif // STATDEBUG && PC_BUILD
      if((selection==0 && a2 == 0 && a1 == 0 && a0 == 0)) {
        #if defined(STATDEBUG) && defined(PC_BUILD)
          printf("return, nothing selected, zero parameters, nothing to draw\n");
        #endif // STATDEBUG && PC_BUILD
        return;
      }
      double  ix;
      int16_t   xo = 0, xn, xN = 0;
      int16_t   yo = 0, yn, yN = 0;
      double fx_min, fx_max;                           //curve fit overlay evaluates in double; the real_t range converts once here
      realToDouble(x_min, &fx_min);
      realToDouble(x_max, &fx_max);
      double    x = fx_min;
      double    y = 0.0;
      int16_t   Intervals = numberIntervals; //starting point to calculate dx
      uint16_t  iterations = 0;
      double    intervalW = (fx_max-fx_min)/(double)(Intervals);

      int16_t minN_y, minN_x;
      minN_y = 0;
      minN_x = SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH;
      for(ix = fx_min-intervalW; iterations < 2000 && x < fx_max+(fx_max-fx_min)*0.5 && xN < SCREEN_WIDTH-1; iterations++) {       //Variable accuracy line plot
        xo = xN;
        yo = yN;
        uint16_t xx;
        for( xx=0; xx<14; xx++) {      //the starting point is ix + dx where dx=2^-0*interval and reduce it to dx=2^-31*interval until dy<=2
          x = ix + intervalW / ((double)((uint16_t) 1 << xx));
          if(USEFLOATING == useREAL4) {
            //TODO create REAL from x (double) if REALS will be used
            //snprintf(ss, 100, "%f", x);
            //stringToReal(ss, &XX, &ctxtReal4);
            convertDoubleToReal(x, &XX, &ctxtReal4);
          }
          else {
            if(USEFLOATING == useREAL39) {
              //TODO create REAL from x (double) if REALS will be used
              //snprintf(ss, 100, "%f", x);
              //stringToReal(ss, &XX, &ctxtReal39);
              convertDoubleToReal(x, &XX, &ctxtReal39);
            }
          }
          yIsFnx( USEFLOATING, selection, x, &y, a0, a1, a2, &XX, &YY, RR, SMI, aa0, aa1, aa2);
          xN = screenX((float)x);
          yN = screenY((float)y);
          if((abs((int)yN-(int)yo)<=2 /*&& abs((int)xN-(int)xo)<=2*/) || iterations == 0 || xN <= minN_x) {
            break;
          }
        }
        ix = x;
        //printf("### iter:%u ix=%20lf xx=%i x=%lf y=%lf xN=%u yN=%u\n", iterations, ix, xx, x, y, xN, yN);
        if(iterations > 0) {  //Allow for starting values to accumulate in the registers at ix = 0
          #if defined(STATDEBUG) && defined(PC_BUILD)
            printf("plotting graph: iter:%u ix:%f I.vals:%u ==>xmin:%f (x:%f) xmax:%f ymin:%f (y:%f) ymax:%f xN:%d yN:%d \n", iterations, ix, Intervals, fx_min, x, fx_max, dbl(y_min), y, dbl(y_max),  xN, yN);
          #endif // STATDEBUG && PC_BUILD
          #define tol 1                                    // Gauss tails asymptote along the bottom and must draw to the frame edges
          if(xN<SCREEN_WIDTH_GRAPH && xN>minN_x && yN<SCREEN_HEIGHT_GRAPH-tol && yN>minN_y) {
            yn = yN;
            xn = xN;
            #if defined(STATDEBUG_VERBOSE) && defined(PC_BUILD)
              printf("Plotting box to x=%d y=%d\n", xn, yn);
            #endif // STATDEBUG_VERBOSE && PC_BUILD
            if(fittedcurveboxes) {
              plotbox(xn, yn);
            }
            if(xo < SCREEN_WIDTH_GRAPH && xo > minN_x && yo < SCREEN_HEIGHT_GRAPH-tol && yo > minN_y) {
              #if defined(STATDEBUG_VERBOSE) && defined(PC_BUILD)
                printf("Plotting line to x=%d y=%d\n", xn, yn);
              #endif // STATDEBUG_VERBOSE && PC_BUILD
              plotline2(xo, yo, xn, yn);
            }
          }
          else {
            #if defined(STATDEBUG) && defined(PC_BUILD)
              printf("Not plotted line: (%u %u) ", xN, yN);
              if(xN >= SCREEN_WIDTH_GRAPH) {
                printf("x>>%u ", SCREEN_WIDTH_GRAPH);
              }
              else if(xN <= minN_x) {
                printf("x<<%u ", minN_x);
              }
              if(yN >= SCREEN_HEIGHT_GRAPH) {
                printf("y>>%u ", SCREEN_HEIGHT_GRAPH);
              }
              else if(yN <= 1+minN_y) {
                printf("y<<%u ", 1+minN_y);
              }
              printf("\n");
            #endif // STATDEBUG && PC_BUILD
          }
        }
      }
      #if defined(PC_BUILD)
        printf("Drawline: %u / 2000 iterations\n", iterations);
      #endif // PC_BUILD
    }


    int16_t index = -1;
    if(selection!=0) {
      strcpy(ss, eatSpacesEnd(getCurveFitModeName(selection)));
      if(lrCountOnes(lrSelection)>1 && selection == lrChosen) {
        strcat(ss, lrChosen == 0 ? "" : STD_SUP_ASTERISK);
      }
      showString(ss, &standardFont, horOffset + 17, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -10 +autoshift, vmNormal, false, false);
      if(selection != CF_GAUSS_FITTING && selection != CF_CAUCHY_FITTING) {
        strcpy(ss, "y=");
        strcat(ss, getCurveFitModeFormula(selection));
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7 +autoshift, vmNormal, false, false);
      }
      else {
        strcpy(ss, "y=");
        strcat(ss, getCurveFitModeFormula(selection));
        compressString = 1;
        showString(          ss,  &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7 +autoshift, vmNormal, false, false);
      }
    }

    if(isValidDraw) {
      if(softmenu[softmenuStack[0].softmenuId].menuItem != -MNU_PLOT_SCATR) {
        sprintf(ss, "%u", NN);
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -2 +autoshift, vmNormal, false, false);
        sprintf(ss, STD_SPACE_PUNCTUATION STD_SPACE_PUNCTUATION "n=");
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++ -2 +autoshift, vmNormal, false, false);
      }

      if(orOrtho(selection) != CF_ORTHOGONAL_FITTING) {
        grphNumFormatter(ss, "", a0, 3, "");
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -4 +autoshift, vmNormal, false, false);
        strcpy(ss, "a" STD_SUB_0 "=");
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -4 +autoshift, vmNormal, false, false);

        grphNumFormatter(ss, "", a1, 3, "");
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -1 +autoshift, vmNormal, false, false);
        strcpy(ss, "a" STD_SUB_1 "=");
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -1 +autoshift, vmNormal, false, false);

        if(selection == CF_PARABOLIC_FITTING || selection == CF_GAUSS_FITTING || selection == CF_CAUCHY_FITTING) {
          grphNumFormatter(ss, "", a2, 3, "");
          showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -1 +autoshift, vmNormal, false, false);
          strcpy(ss, "a" STD_SUB_2 "=");
          showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -1 +autoshift, vmNormal, false, false);
        }

        char tmpBuf[100];
        strcpy(ss, formatCore(rr, 5, false, tmpBuf, 50));
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  +2  +autoshift, vmNormal, false, false);
        strcpy(ss, "r" STD_SUP_2 "=");
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   +2 +autoshift, vmNormal, false, false);

        double xmaxd, ymaxd, xmind, ymind;
        realToDouble(x_max, &xmaxd);
        realToDouble(y_max, &ymaxd);
        realToDouble(x_min, &xmind);
        realToDouble(y_min, &ymind);
        grphNumFormatter(ss, "(", xmaxd, 2, "");
        uint16_t ssw = showStringEnhanced(padEquals(tmpbuf, ss), &standardFont, 0, 0, vmNormal, false, false, NO_compress, NO_raise, NO_Show, NO_Bold, NO_LF);
        grphNumFormatter(tt, radixProcess(tmpbuf, "#"), ymaxd, 2, ")");
        uint16_t ttw = showStringEnhanced(padEquals(tmpbuf, tt), &standardFont, 0, 0, vmNormal, false, false, NO_compress, NO_raise, NO_Show, NO_Bold, NO_LF);
        n = showString(padEquals(tmpbuf, ss), &standardFont, 160-3-2-ssw-ttw, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index + 2       +autoshift, vmNormal, false, false);
        showString(padEquals(tmpbuf, tt), &standardFont, n+3, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++ +autoshift + 2, vmNormal, false, false);
        grphNumFormatter(ss, "(", xmind, 2, "");
        n = showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index    -2  +autoshift + 2, vmNormal, false, false);
        grphNumFormatter(ss, radixProcess(tmpbuf, "#"), ymind, 2, ")");
        showString(padEquals(tmpbuf, ss), &standardFont, n+3,       Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++  -2  +autoshift + 2, vmNormal, false, false);

      }
      else {                          //ORTHOF
        char tmpBuf[100];
        strcpy(ss, formatCore(a0, 3, false, tmpBuf, 50));
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -4 +autoshift, vmNormal, false, false);
        strcpy(ss, "a" STD_SUB_0 "=");
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -4 +autoshift, vmNormal, false, false);

        strcpy(ss, formatCore(ssa0, 3, false, tmpBuf, 50));
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -4 +autoshift, vmNormal, false, false);
        strcpy(ss, "    " STD_PLUS_MINUS);
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -4 +autoshift, vmNormal, false, false);

        strcpy(ss, formatCore(a1, 3, false, tmpBuf, 50));
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -1 +autoshift, vmNormal, false, false);
        strcpy(ss, "a" STD_SUB_1 "=");
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -1 +autoshift, vmNormal, false, false);

        strcpy(ss, formatCore(ssa1, 3, false, tmpBuf, 50));
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -1 +autoshift, vmNormal, false, false);
        strcpy(ss, "    " STD_PLUS_MINUS);
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -1 +autoshift, vmNormal, false, false);

        if(softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_PLOT_SCATR) {
          if(n >= 30) {
            grphNumFormatter(ss, "", smi, 3, "");
          }
          else {
            strcpy(ss, "  | n < 30");
          }

          showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  +1 +autoshift, vmNormal, false, false);
          strcpy(ss, "s" STD_SUB_m STD_SUB_i "=");
          showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   +1 +autoshift, vmNormal, false, false);
        }
        else {
          strcpy(ss, formatCore(rr, 3, false, tmpBuf, 50));
          showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  +2  +autoshift, vmNormal, false, false);
          strcpy(ss, "r" STD_SUP_2 "=");
          showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   +2 +autoshift, vmNormal, false, false);
        }
      }
    }
    else {
      if(n < 0) {
        showString("invalid n", &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7+2 +autoshift, vmNormal, false, false);
      }
      else if(isnan(a0) || isnan(a1) || (isnan(a2) && minLRDataPoints(selection)!=2) ) {
        if(selection & 448) {
          showString("invalid a0,a1,a2", &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7+2 +autoshift, vmNormal, false, false);
        }
        else {
          showString("invalid a0,a1", &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7+2 +autoshift, vmNormal, false, false);
      }
      }
      else if((orOrtho(selection) & CF_ORTHOGONAL_FITTING) && isnan(smi)) {
        showString("invalid smi", &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7+2 +autoshift, vmNormal, false, false);
      }
      else if(rr>1 || isnan(rr)) {
        showString("invalid r", &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7+2 +autoshift, vmNormal, false, false);
      }
      else if(NN < minLRDataPoints(selection) ) {
        showString("insufficient data", &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7+2 +autoshift, vmNormal, false, false);
        sprintf(ss, " %u < %u", NN, minLRDataPoints(selection));
        showString(ss, &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7+2 +autoshift, vmNormal, false, false);
        }
      else if(selection == 0) {
        showString("No Valid L.R.", &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7+2 +autoshift, vmNormal, false, false);
      }
      else {
        showString("L.R. error", &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7+2 +autoshift, vmNormal, false, false);
    }
  }
#endif // OPTION_GRAPHICS
  }


void fnPlotCloseSmi(uint16_t unusedButMandatoryParameter){
  fnKeyExit(0);
  #if defined(DEBUGUNDO)
    printf(">>> Undo from fnPlotCloseSmi\n");
  #endif // DEBUGUNDO
  fnMinExpStdDev(0);
  temporaryInformation = TI_SCATTER_SMI;
}


//** Called from keyboard
//** plotSelection = 0 means that no curve fit is plotted
//
void fnPlotStat(uint16_t plotMode){
#if defined(OPTION_GRAPHICS)
  if(plotMode != PLOT_NXT && plotMode != PLOT_REV && statMx[0] != 'S') {
    restoreStats();                   // a new plot starts from restored stats after an HNORM takeover, as EXIT does
  }
  switch(plotMode) {
    case PLOT_ORTHOF:
    case PLOT_START:
    case PLOT_LR: {
      drawHistogram = 0;
      strcpy(plotStatMx, "STATS");
      lastPlotMode = PLOT_NOTHING;    // an explicit plot request; lastPlotMode must not override it below
      break;
    }
    case PLOT_REV:
    case PLOT_NXT: {                  // modify the plot on screen, so lastPlotMode stays
      drawHistogram = 0;
      strcpy(plotStatMx, "STATS");
      break;
    }
    case H_PLOT: {
      drawHistogram = 1;
      strcpy(plotStatMx, "HISTO");
      lastPlotMode = PLOT_NOTHING;
      plotSelection = 0;              // no fit overlay on a plain histogram
      break;
    }
    case H_NORM: {
      drawHistogram = 1;
      strcpy(plotStatMx, "HISTO");    // HNORM plots the HISTO matrix
      lrSelectionHistobackup = lrSelection;
      lrChosenHistobackup = lrChosen;
      fnCurveFitting(CF_GAUSS_FITTING); // must precede the statMx takeover below: fnCurveFitting restores the stats when statMx is "HISTO"
      strcpy(statMx, "HISTO");
      calcSigma(0);
      plotMode = PLOT_LR;
      lastPlotMode = PLOT_START;
      break;
    }
    default: {
      break;
    }
  }

                                  #if defined(STATDEBUG) && defined(PC_BUILD)
                                  printf("fnPlotStat1: plotSelection = %u; Plotmode=%u\n", plotSelection, plotMode);
                                  printf("#####>>> fnPlotStat1: plotSelection:%u:%s  Plotmode:%u lastplotmode:%u  lrSelection:%u lrChosen:%u plotStatMx:%s\n", plotSelection, getCurveFitModeName(plotSelection), plotMode, lastPlotMode, lrSelection, lrChosen, plotStatMx);
                                    if( (plotStatMx[0]=='S' && checkMinimumDataPoints(const_2)) ||
                                        (plotStatMx[0]=='D' && drawMxN() >= 2) ||
                                        (plotStatMx[0]=='H' && statMxN() >= 3) ) {
                                      int16_t cnt = 0;
                                      switch(plotStatMx[0]) {
                                        case 'S': cnt = realToInt32C47(SIGMA_N, NULL); break;
                                        case 'D': cnt = drawMxN();                     break;
                                        case 'H': cnt = statMxN();                     break;
                                        default: ;
                                      }
                                    printf("Stored values %i\n", cnt);
                                  }
                                  #endif //STATDEBUG


    if(!GRAPHMODE) { //Change over hourglass to the left side
      clearScreenOld(clrStatusBar, !clrRegisterLines, !clrSoftkeys);
    }

//Removed - old portion ex WP43, replaced by graphs.c, where PLSTAT and EQN draw lives - remove when sure
//   Also, the 'D' reference in pltStatMx need to be surgically removed, bearing in mind that the PLSTAT and EQN draw in plotstat may need it.
//printf("XXXX4 plotStatMx=%s\n",plotStatMx);
//    calcMode = CM_GRAPH;
    hourGlassIconEnabled = true;       //clear the current portion of statusbar
    showHideHourGlass();
    refreshStatusBar();


    if((plotStatMx[0]=='S' && checkMinimumDataPoints(const_2)) ||
       (plotStatMx[0]=='D' && drawMxN() >= 2) ||
       (plotStatMx[0]=='H' && statMxN() >= 3) ) {
      plotStatScale = 0;              // per-mode default; SCATR/CENTRL set it

      if(!(lastPlotMode == PLOT_NOTHING || lastPlotMode == PLOT_START)) {
        plotMode = lastPlotMode;
      }
      calcMode = CM_PLOT_STAT;
      statGraphReset();

      if(plotMode == PLOT_START) {
        plotSelection = 0;
        roundedTicks = false;
      }
      else {
        if(plotMode == PLOT_LR && lrSelection != 0) {
          plotSelection = lrSelection;
          roundedTicks = false;
        }
          else {
            if(plotMode == H_PLOT || plotMode == H_NORM) {
               calcMode = CM_PLOT_STAT;
            }
          }
      }

        #if defined(DMCP_BUILD)
        lcd_refresh();
      #else // !DMCP_BUILD
        refreshLcd(NULL);
      #endif // DMCP_BUILD

        switch(plotMode) {
          case H_PLOT:
          case H_NORM:
            showSoftmenu(-MNU_HPLOT);
            break;
          case PLOT_LR:
            if(drawHistogram == 0) {
              showSoftmenu(-MNU_PLOT_ASSESS);
            }
            else {
              showSoftmenu(-MNU_HPLOT);
            }
            break;
        case PLOT_NXT:
        case PLOT_REV:
          showSoftmenu(-MNU_PLOT_ASSESS);
          break;
        case PLOT_ORTHOF:
        case PLOT_START:
          plotStatScale = 1;
          showSoftmenu(-MNU_PLOT_SCATR);
          break;
        case PLOT_NOTHING:
          break;
          default: ;
      }

      if((plotMode != PLOT_START) && (plotMode != H_PLOT) && (plotMode != H_NORM)) {
        fnPlotRegressionLine(plotMode);
      }
      else {
        lastPlotMode = plotMode;
      }
  }
  else {
    calcMode = CM_NORMAL;
    displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "There is no statistical/plot data available!");
      moreInfoOnError("In function fnPlotStat:", errorMessage, NULL, NULL);
    #endif
  }
#endif // OPTION_GRAPHICS
}


void fnPlotRegressionLine(uint16_t plotMode){
#if defined(OPTION_GRAPHICS)
  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("fnPlotRegressionLine: plotSelection = %u; Plotmode=%u\n", plotSelection, plotMode);
  #endif // STATDEBUG && PC_BUILD

  switch(plotMode) {
    case PLOT_ORTHOF: {
      plotSelection = CF_ORTHOGONAL_FITTING;
      lrChosen = CF_ORTHOGONAL_FITTING;
      break;
    }

      //Show data and one curve fit selected: Scans lrSelection from LSB and stop after the first one is found. If a chosen curve is there, override.
      //printf("#####X %u %u \n",plotSelection, lrSelection);
    case PLOT_NXT: {
      plotSelection = plotSelection << 1;
      if(plotSelection == 0){
        plotSelection = 1;
      }

      while((plotSelection != ( (lrSelection == 0 ? 1023 : lrSelection) & plotSelection)) && (plotSelection < 1024)){ //fast forward to selected LR
        plotSelection = plotSelection << 1;
      }

      if(plotSelection >= 1024) {
        plotSelection = 0;  //purposely change to zero graph display to give a no-line view
      }
      break;
    }

    case PLOT_REV: {
      if(plotSelection == 0){
        plotSelection = 1024; //wraparound, will still shift right 1
      }
      plotSelection = plotSelection >> 1;
      if(plotSelection >= 1024){
        plotSelection = 0;  //purposely change to zero graph display to give a no line view
      }

      while((plotSelection != ( (lrSelection == 0 ? 1023 : lrSelection) & plotSelection)) && (plotSelection < 1024) && (plotSelection > 0)){ //fast forward to selected LR
        plotSelection = plotSelection >> 1;
      }

      break;
    }

    case PLOT_LR: {
      //Show data and one curve fit selected: Scans lrSelection from LSB and stop after the first one is found. If a chosen curve is there, override.
      plotSelection = lrChosen;
      if(plotSelection == 0) {
        plotSelection = 1;
      }
      while((plotSelection != ( (lrSelection == 0 ? 1023 : lrSelection) & plotSelection)) && (plotSelection < 1024)){
        plotSelection = plotSelection << 1;
      }
      if(plotSelection >= 1024) {
        plotSelection = 0;  //purposely change to zero graph display
      }
      break;
    }

    case PLOT_START: {
      plotMode = PLOT_ORTHOF;
      break;
    }

    case PLOT_NOTHING: {
      break;
    }

    default: {
      break;
    }
  }
#endif // OPTION_GRAPHICS
}




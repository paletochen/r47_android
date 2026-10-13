// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//** //JM
 * \file graphs.c Graphing module
 ***********************************************/

#include "c47.h"

//#define STATDEBUG

bool_t    plotXYroute = false;      // plotXY: set by graph_stat (PLSTAT) only, so no other route into graph_plotmem takes the plotXY form; the option itself is FLAG_PL_2L (2LINE)

bool_t    invalid_intg = true;
bool_t    invalid_diff = true;
bool_t    invalid_rms  = true;

#if defined(STATDEBUG) && defined(PC_BUILD)
  static double dbl(const real_t *r) {                 // debug printf convenience only
    double d;
    realToDouble(r, &d);
    return d;
  }
#endif // STATDEBUG && PC_BUILD

// Graph range limits. Sized 34 so a reserved-variable real34 decodes in losslessly and ctxtReal39 writes fit (capacity rounds up to 39 digits); float dies below 1E-38 which these must support.
REAL_T_PTR(x_min, 34);
REAL_T_PTR(x_max, 34);
REAL_T_PTR(y_min, 34);
REAL_T_PTR(y_max, 34);
int8_t    PLOT_ZMY = 0;


void graphResetCommon() {
  realSetZero(x_min);
  realSetOne(x_max);
  realSetZero(y_min);
  realSetOne(y_max);

  clearSystemFlag(FLAG_CPXPLOT);
  clearSystemFlag(FLAG_SHOWY);
  clearSystemFlag(FLAG_SHOWX);
  clearSystemFlag(FLAG_VECT);
  clearSystemFlag(FLAG_NVECT);
  setSystemFlag(FLAG_PLINE);
  setSystemFlag(FLAG_PBOX);
  clearSystemFlag(FLAG_PL_2L);
  clearSystemFlag(FLAG_PCURVE);
  clearSystemFlag(FLAG_PCROS);
  clearSystemFlag(FLAG_PPLUS);
  clearSystemFlag(FLAG_PINTG);
  clearSystemFlag(FLAG_PDIFF);
  clearSystemFlag(FLAG_PRMS);
  clearSystemFlag(FLAG_PSHADE);


  real34SetZero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_UY));
  real34SetZero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_LY));

  PLOT_ZMY      = 0;
  PLOT_ZOOM     = 0;
  plotmode      = _SCAT;
  tick_int_x    = 0;
  tick_int_y    = 0;
  PLOT_AXIS     = false;

}


void graph_reset(void){
  graphResetCommon();
  clearSystemFlag(FLAG_SCALE);    //fnPlotSQ option; the stat plots (statGraphReset) run on plotStatScale and leave the user's flag alone
}


void fnClGrf(uint16_t unusedButMandatoryParameter) {
  graph_reset();
  fnClDrawMx(2);
  strcpy(plotStatMx, "DrwMX");
  fnRefreshState();                //jm
}


void fnPline(uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PLINE);
  fnPlotSQ(0);
}


void fnP2line(uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PL_2L);
  fnPlotSQ(0);
}


void fnPcros(uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PCROS);
  fnPlotSQ(0);
}

void fnPplus(uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PPLUS);
  fnPlotSQ(0);
}


void fnPbox (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PBOX);
  fnPlotSQ(0);
}

void fnPcurve (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PCURVE);
  fnPlotSQ(0);
}


void fnPintg (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PINTG);
  fnPlotSQ(0);
}


void fnPdiff (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PDIFF);
  fnPlotSQ(0);
}


void fnPrms (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PRMS);
  fnPlotSQ(0);
}


  void fnPMzoom (uint16_t param) { //param = 2: positive; param = 1: negative
    switch(calcMode){
      case CM_PLOT_STAT: {
        int8_t increment = param == 2 ? +1 : param == 1 ? -1 : 0;
        PLOT_ZOOM += increment;
        if(PLOT_ZOOM > statZoomRangeHi) {
          PLOT_ZOOM = statZoomRangeLo;
        }
        else if(PLOT_ZOOM < statZoomRangeLo) {
          PLOT_ZOOM = statZoomRangeHi;
        }
        if(PLOT_ZOOM != 0) {
           PLOT_AXIS = true;
        }
        else {
           PLOT_AXIS = false;
        }
        break;
      }
      case CM_GRAPH: {
        //real34SetZero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_UY));
        //real34SetZero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_LY));
        PLOT_AXIS = true;
        int8_t increment = param == 2 ? +1 : param == 1 ? -1 : 0;
        PLOT_ZMY += increment;
        if(PLOT_ZMY == zoomOverride-1 || PLOT_ZMY == zoomOverride+1) {
          PLOT_ZMY = 0;
        }
        else if(PLOT_ZMY > zoomOverride+1) {
          PLOT_ZMY = zoomRangeLo;
        }
        else if(PLOT_ZMY < zoomRangeLo) {
          PLOT_ZMY = zoomRangeHi;
        }
        fnRefreshState();
        fnPlotSQ(0);
        break;
      }
      default:break;
    }
  }


void fnPlotZoom(uint16_t unusedButMandatoryParameter){
    longInteger_t x;
    int32_t ii;

    if(!getRegisterAsLongInt(REGISTER_X, x, NULL)) {
      goto end;
    }

    longIntegerToInt32(x, ii);
    //the ZOOM command from outside the PLOT mode only works for PLSTAT
    PLOT_ZMY = ii;
end:
    longIntegerFree(x);
  }





// Multiplies *aa by stepBase^-zoomLevel. The zoom levels are small integers, so repeated double
// multiplication replaces libm's pow; stepBase carries the old 4.5^0.75 or 4.5^0.55 step per level.
static void calculateZoomFactor(double stepBase, int16_t zoomLevel, float *aa) {
  double m = 1.0;
  int16_t z = zoomLevel < 0 ? (int16_t)-zoomLevel : zoomLevel;
  while(z--) {
    m *= stepBase;
  }
  *aa = (float)(zoomLevel > 0 ? (double)*aa / m : (double)*aa * m);
}


static void multiplyZoomFactors(float plotzoomx, float plotzoomy, float histofactor, real_t *x_min, real_t *x_max, real_t *y_min, real_t *y_max, real_t *dx, real_t *dy) {
    real_t k, t, xavg, yavg;                           // ranges in real_t; the zoom scalars are dimensionless fractions, combined in float/double then converted once per use
    convertDoubleToReal(zoomfactor, &k, &ctxtReal39);    // k = zoomfactor
    realMultiply(dx, &k, &t, &ctxtReal39);               // t = dx * zoomfactor
    realSubtract(x_min, &t, x_min, &ctxtReal39);         // x_min = x_min - dx * zoomfactor
    realAdd(x_max, &t, x_max, &ctxtReal39);              // x_max = x_max + dx * zoomfactor
    realMultiply(dy, &k, &t, &ctxtReal39);               // t = dy * zoomfactor
    realSubtract(y_min, &t, y_min, &ctxtReal39);         // y_min = y_min - dy * zoomfactor
    realAdd(y_max, &t, y_max, &ctxtReal39);              // y_max = y_max + dy * zoomfactor
    realSubtract(x_max, x_min, dx, &ctxtReal39);         // dx = x_max - x_min
    realSubtract(y_max, y_min, dy, &ctxtReal39);         // dy = y_max - y_min
    realAdd(x_max, x_min, &xavg, &ctxtReal39);           // xavg = x_max + x_min
    realMultiply(&xavg, const_1on2, &xavg, &ctxtReal39); // xavg = (x_max + x_min) / 2
    realAdd(y_max, y_min, &yavg, &ctxtReal39);           // yavg = y_max + y_min
    realMultiply(&yavg, const_1on2, &yavg, &ctxtReal39); // yavg = (y_max + y_min) / 2
    convertDoubleToReal((double)plotzoomy * (double)histofactor / 2.0, &k, &ctxtReal39); // k = plotzoomy * histofactor / 2
    realMultiply(dy, &k, &t, &ctxtReal39);               // t = dy/2 * plotzoomy * histofactor
    realSubtract(&yavg, &t, y_min, &ctxtReal39);         // y_min = yavg - dy/2 * plotzoomy * histofactor
    realAdd(&yavg, &t, y_max, &ctxtReal39);              // y_max = yavg + dy/2 * plotzoomy * histofactor
    convertDoubleToReal((double)plotzoomx * (double)histofactor / 2.0, &k, &ctxtReal39); // k = plotzoomx * histofactor / 2
    realMultiply(dx, &k, &t, &ctxtReal39);               // t = dx/2 * plotzoomx * histofactor
    realSubtract(&xavg, &t, x_min, &ctxtReal39);         // x_min = xavg - dx/2 * plotzoomx * histofactor
    realAdd(&xavg, &t, x_max, &ctxtReal39);              // x_max = xavg + dx/2 * plotzoomx * histofactor
}


void fnPvect (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_VECT);
  fnPlotSQ(0);
}


void fnPNvect (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_NVECT);
  fnPlotSQ(0);
}


void fnScale (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_SCALE);
  fnRefreshState();
  fnPlotSQ(0);
}


void fnPshade (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PSHADE);
  fnPlotSQ(0);
}


void fnComplexPlot (uint16_t mode) {
  if(mode == ITM_CPXPLOT) {
    flipSystemFlag(FLAG_CPXPLOT);
  } else if(mode == ITM_IMPLOT) {
    flipSystemFlag(FLAG_IMPLOT);
  }
  fnEqSolvGraph(EQ_PLOT_LU);
  if(lastErrorCode == ERROR_NONE) { //same guard as fnPlotf
    fnPlotSQ(0);
  }
}


void fnPx (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_SHOWX);
  fnRefreshState();                //jm
  fnPlotSQ(0);
}


void fnPy (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_SHOWY);
  fnRefreshState();                //jm
  fnPlotSQ(0);
}


void fnPlotReset(uint16_t unusedButMandatoryParameter) {
  graph_reset();
  if(GRAPHMODE) {
    fnRefreshState();                //jm
    fnPlotSQ(0);
  }
}


void fnPlotf(uint16_t unusedButMandatoryParameter) {
  fnEqSolvGraph(EQ_PLOT); // will pick up X1 X2 from the stack
  if(lastErrorCode == ERROR_NONE) { //on a rejected range already set CM_NORMAL; prevent fnPlotSQ to force CM_GRAPH back and hide the error
    fnPlotSQ(NOPARAM);
  }
}


void fnPlotSQ(uint16_t unusedButMandatoryParameter) {
    #if defined(DMCP_BUILD)
      lcd_refresh();
    #else // !DMCP_BUILD
      refreshLcd(NULL);
    #endif // DMCP_BUILD

    PLOT_AXIS = true;

    if(GRAPHMODE) {
      previousCalcMode = CM_NORMAL;
    }
    else {
      previousCalcMode = calcMode;

// Removed due to interfering and unneccesry statusbar clear befoire the user can program SNAP after PLOTf.
//      0.5 % chance that removing it might cause remaining hourglass on the wrong side of screen.
//      LEaving this comment and original for a while to monitor performance.
//      clearScreenOld(clrStatusBar, !clrRegisterLines, !clrSoftkeys); //Change over hourglass to the left side

    }

    calcMode = CM_GRAPH;
    hourGlassIconEnabled = true;       //clear the current portion of statusbar
    showHideHourGlass();
    refreshStatusBar();

    if(programRunStop == PGM_RUNNING || programRunStop == PGM_PAUSED) {
      if(menu(0) != -MNU_SHOW) {           //blank menu under a programmed plot; EXIT pops it back
        showSoftmenu(-MNU_SHOW);
      }
    }
    else if(menu(0) != -MNU_PLOT_FUNC && plotStatMx[0] == 'D') {
      showSoftmenu(-MNU_PLOT_FUNC);
    }
    else if(menu(0) != -MNU_PLOT_STAT && plotStatMx[0] == 'S') {
      showSoftmenu(-MNU_PLOT_STAT);
    }
}


void fnListXY(uint16_t unusedButMandatoryParameter) {
  if((plotStatMx[0]=='D' ? (drawMxN() >= 1) : false)) {
    calcMode = CM_LISTXY; //Used to view graph/listing
    ListXYposition = 0;
  }
}


//added this, to add a new command to plot advanced from the STATS
  void fnPlotStatAdv(uint16_t unusedButMandatoryParameter) {
    lastPlotMode = PLOT_NOTHING;
    strcpy(plotStatMx, "STATS");
    plotXYroute = false;
    setSystemFlag(FLAG_PLINE);
    fnPlotSQ(0);
  }


#if defined(OPTION_MOREGRAPHICS)
  static void plotarrow(int16_t xo, int16_t yo, int16_t xn, int16_t yn) {              // Plots line from xo,yo to xn,yn; uses temporary x1,y1
    float dx, dy, ddx, dydx, zz, zzz;
    dydx = yn-yo;
    ddx = xn-xo;
    zz  = sqrtf(dydx*dydx + ddx*ddx);
    zzz = 3;
    dy  = dydx * (zzz/zz);
    dx  = ddx * (zzz/zz);
    #if defined(STATDEBUG)
      printf("%d %d  %d %d  ddx=%f, dydx=%f, zz=%f  zzz=%f, dx=%f, dy=%f \n", xo, yo, xn, yn, ddx, dydx, zz, zzz, dx, dy);
    #endif // STATDEBUG
    if(!(xo==xn && yo==yn)){
      plotline1(xn+(-3*dx +dy), yn+(-3*dy -dx), xn, yn);
      plotline1(xn+(-3*dx -dy), yn+(-3*dy +dx), xn, yn);
    }
    else {
      placePixel(xn, yn);
    }
  }
#endif // OPTION_MOREGRAPHICS


    typedef struct {              //JM VALUES DEMO
      int8_t valid;
      int8_t  xd1;
      int8_t  yd1;
      int8_t  xd2;
      int8_t  yd2;
    } plotdeltas;

    TO_QSPI const plotdeltas tabDeltaBig[] = {
      {1, +0, -2, +5, +6},
      {1, +5, +6, -5, +6},
      {1, -5, +6, +0, -2},
      {0,  0,  0,  0,  0},
    };
  static void plotdeltabig(int16_t xn, int16_t yn) {              // Plots ldifferential sign; uses temporary x1,y1
    int8_t ii=0;
    while(tabDeltaBig[ii].valid == 1) {
      plotline1(xn+tabDeltaBig[ii].xd1, yn+tabDeltaBig[ii].yd1, xn+tabDeltaBig[ii].xd2, yn+tabDeltaBig[ii].yd2);
      ii++;
    }
  }


#if defined(OPTION_MOREGRAPHICS)
    TO_QSPI const plotdeltas tabDelta[] = {
      {1, +0, -2, 0, 0},
      {1, -1, -1, 0, 0},
      {1, -1, +0, 0, 0},
      {1, -2, +1, 0, 0},
      {1, -2, +2, 0, 0},
      {1, +1, -1, 0, 0},
      {1, +1, -0, 0, 0},
      {1, +2, +1, 0, 0},
      {1, +2, +2, 0, 0},
      {1, -1, +2, 0, 0},
      {1, +0, +2, 0, 0},
      {1, +1, +2, 0, 0},
      {0,  0,  0, 0, 0},
    };
  static void plotdelta(int16_t xn, int16_t yn) {             // Plots ldifferential sign; uses temporary x1,y1
    int8_t ii=0;
    while(tabDelta[ii].valid == 1) {
      placePixel(xn+tabDelta[ii].xd1, yn+tabDelta[ii].yd1);
      ii++;
    }
  }
#endif // OPTION_MOREGRAPHICS


    TO_QSPI const plotdeltas tabDeltaIntBig[] = {
      {1, -0, -2+0, +3, -2+0},
      {1, -0, -2+1, +3, -2+1},
      {1, -3, -2+8, +0, -2+8},
      {1, -3, -2+9, +0, -2+9},
      {1, +0, -2+7, +0, -2+0},
      {1, +1, -2+7, +1, -2+0},
      {0,  0,    0,  0,    0},
    };
  static void plotintbig(int16_t xn, int16_t yn) {            // Plots integral sign; uses temporary x1,y1
    int8_t ii=0;
    while(tabDeltaIntBig[ii].valid == 1) {
      plotline1(xn+tabDeltaIntBig[ii].xd1, yn+tabDeltaIntBig[ii].yd1, xn+tabDeltaIntBig[ii].xd2, yn+tabDeltaIntBig[ii].yd2);
      ii++;
    }
  }


#if defined(OPTION_MOREGRAPHICS)
    TO_QSPI const plotdeltas tabDeltaInt[] = {
      {1, +0, +0, 0, 0},
      {1, +0, -1, 0, 0},
      {1, +0, -2, 0, 0},
      {1, +0, +1, 0, 0},
      {1, +0, +2, 0, 0},
      {1, +1, -2, 0, 0},
      {1, -1, +2, 0, 0},
      {0, 0, 0, 0, 0},
    };
  static void plotint(int16_t xn, int16_t yn) {               // Plots integral sign; uses temporary x1,y1
    int8_t ii=0;
    while(tabDeltaInt[ii].valid == 1) {
      placePixel(xn+tabDeltaInt[ii].xd1, yn+tabDeltaInt[ii].yd1);
      ii++;
    }
  }
#endif // OPTION_MOREGRAPHICS


    TO_QSPI const plotdeltas tabDeltaRms[] = {
      {1, +1, -1, 0, 0},
      {1, -1, -1, 0, 0},
      {1, -0, -1, 0, 0},
      {1, +1, +0, 0, 0},
      {1, -1, +0, 0, 0},
      {1, -0, +0, 0, 0},
      {1, +1, +1, 0, 0},
      {1, -1, +1, 0, 0},
      {1, -0, +1, 0, 0},
      {0,  0,  0, 0, 0},
    };
  static void plotrms(int16_t xn, int16_t yn) {               // Plots line from xo,yo to xn,yn; uses temporary x1,y1
    int8_t ii=0;
    while(tabDeltaRms[ii].valid == 1) {
      placePixel(xn+tabDeltaRms[ii].xd1, yn+tabDeltaRms[ii].yd1);
      ii++;
    }
  }


//###################################################################################

//PLSTAT; EQN Graph;

#define bufLen 40


  static void showGraphTickText1(double tick_int_x, double tick_int_y, int32_t xoff, int32_t yoff1, int32_t yoff2, uint16_t acc) {
    char buff[32];
    char outstr[bufLen];
    char tmpBuf[100];
    if(tick_int_y > 0) {                             // 0 only when the interval underflowed double, beyond 1e+-308: no ticks drawn, so no label
      snprintf(tmpString, bufLen, "  y %8s/tick  ", radixProcess(buff, formatCore(tick_int_y, acc, false, tmpBuf, 50)));
      convertDigits(smallE(buff, tmpString), outstr);
      showString(outstr, &standardFont, xoff, yoff1, vmNormal, true, true);
    }

    if(tick_int_x > 0) {
      snprintf(tmpString, bufLen, "  x %8s/tick  ", radixProcess(buff, formatCore(tick_int_x, acc, false, tmpBuf, 50)));
      convertDigits(smallE(buff, tmpString), outstr);
      showString(outstr, &standardFont, xoff, yoff2, vmNormal, true, true);
    }
  }


void graph_text(void) {
    uint32_t ypos = Y_POSITION_OF_REGISTER_T_LINE -11 + 12 * 5 -45;
    int16_t ii;
    char ss[100], tt[100];
    char tmpbuf[PLOT_TMP_BUF_SIZE];
    int32_t n;
    double xmaxd, ymaxd, xmind, ymind;
    realToDouble(x_max, &xmaxd);
    realToDouble(y_max, &ymaxd);
    realToDouble(x_min, &xmind);
    realToDouble(y_min, &ymind);
    grphNumFormatter(ss, "(", xmaxd, 2, "");
    uint16_t ssw = showStringEnhanced(padEquals(tmpbuf, ss), &standardFont, 0, 0, vmNormal, false, false, NO_compress, NO_raise, NO_Show, NO_Bold, NO_LF);
    grphNumFormatter(tt, radixProcess(tmpbuf, "#"), ymaxd, 2, ")");
    uint16_t ttw = showStringEnhanced(padEquals(tmpbuf, tt), &standardFont, 0, 0, vmNormal, false, false, NO_compress, NO_raise, NO_Show, NO_Bold, NO_LF);
    ypos += 38;
    n = showString(padEquals(tmpbuf, ss), &standardFont, 160-3-2-ssw-ttw, ypos, vmNormal, false, false);
    showString(padEquals(tmpbuf, tt), &standardFont, n+3, ypos, vmNormal, false, false);
    grphNumFormatter(ss, "(", xmind, 2, "");
    ypos += 19;
    n = showString(padEquals(tmpbuf, ss), &standardFont, 1, ypos, vmNormal, false, false);
    grphNumFormatter(ss, radixProcess(tmpbuf, "#"), ymind, 2, ")");
    showString(padEquals(tmpbuf, ss), &standardFont, n+3,  ypos, vmNormal, false, false);
    ypos -= 38;
    showGraphTickText1(tick_int_x, tick_int_y, 1, ypos, ypos-12, 3);
    ypos -= 24;

    uint32_t minnx, minny;
    minny = 0;
    minnx = SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH;
    tmpString[0] = 0;                                  //If the axis is on the edge supress it, and label accordingly
    uint8_t axisdisp =  (!(yzero == SCREEN_HEIGHT_GRAPH-1 || yzero == minny) ? 2 : 0)
                      + (!(xzero == SCREEN_WIDTH-1        || xzero == minnx) ? 1 : 0);
    switch(axisdisp) {
      case 0: strcpy(tmpString, "            ");                                       break;
      case 1: snprintf(tmpString, bufLen, "  y-axis x 0");                             break;
      case 2: snprintf(tmpString, bufLen, "  x-axis y 0");                             break;
      case 3: snprintf(tmpString, bufLen, "  axis 0%s0 ", radixProcess(tmpbuf, "."));  break;
      default: ;
    }

    //Change to the small characters and fabricate a small = char
    static char outstr[bufLen];
    convertDigits(tmpString, outstr);

    ii = showString(outstr, &standardFont, 1, ypos, vmNormal, true, true);  //JM
    if(tmpString[ stringByteLength(tmpString)-1 ] == '0') {
      #define sp 15
      plotline1((int16_t)(ii-17), (int16_t)(ypos+2+sp), (int16_t)(ii-11), (int16_t)(ypos+2+sp));
      plotline1((int16_t)(ii-17), (int16_t)(ypos+1+sp), (int16_t)(ii-11), (int16_t)(ypos+1+sp));
      plotline1((int16_t)(ii-17), (int16_t)(ypos-1+sp), (int16_t)(ii-11), (int16_t)(ypos-1+sp));
      plotline1((int16_t)(ii-17), (int16_t)(ypos-2+sp), (int16_t)(ii-11), (int16_t)(ypos-2+sp));
    }
    ypos += 48 + 2*19;

    if(getSystemFlag(FLAG_PINTG) && !invalid_intg) {
      snprintf(tmpString, bufLen, "  Trapezoid integral");
      showStringEnhanced(tmpString, &tinyFont, 1, ypos, vmNormal, false, false, NO_compress, NO_raise, DO_Show, DO_Bold, DO_LF);

      plotintbig(5, ypos+4+4-2-4);
      plotrect(5+4-1, (ypos+4+4-2+2)-1-4, 5+4+2, (ypos+4+4-2+2)+2-4);
      ypos += 20;
    }

    if(getSystemFlag(FLAG_PDIFF) && !invalid_diff) {
      snprintf(tmpString, bufLen, "  Numerical slope");
      showStringEnhanced(tmpString, &tinyFont, 1, ypos, vmNormal, false, false, NO_compress, NO_raise, DO_Show, DO_Bold, DO_LF);
      plotdeltabig(6, ypos+4+4-2-4);
      ypos += 20;
    }

    if(getSystemFlag(FLAG_PRMS) && !invalid_rms) {
      snprintf(tmpString, bufLen, "  Root Mean Square RMS");
      showStringEnhanced(tmpString, &tinyFont, 1, ypos, vmNormal, false, false, NO_compress, NO_raise, DO_Show, DO_Bold, DO_LF);
      plotrms(6, ypos+4+4-2-3);
      plotrect(6-1, (ypos+4+4-2)-1-3, 6+2, (ypos+4+4-2)+2-3);
      ypos += 20;
    }

    force_refresh(timed);
}



//####################################################
//######### PLOT MEM #################################
//####################################################


void graph_Include0(bool_t mode, uint16_t statnum) {
  //using global: FLAG_SHOWX, x_min, x_max, FLAG_SHOWY, y_min, y_max, FLAG_SCALE, PLOT_ZMY, zoomfactor

  real_t tmp, k;

  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("Axis1b: x_min = %f, y_min = %f, x_max = %f, y_max = %f\n", dbl(x_min), dbl(y_min), dbl(x_max), dbl(y_max));
    printf("PLOT_ZMY=%i  FLAG_SCALE=%i mode=%i\n", PLOT_ZMY, getSystemFlag(FLAG_SCALE), mode);
  #endif // STATDEBUG


  //Check and correct if min and max is swapped
  if(realCompareGreaterThan(x_min, const_0) && realCompareGreaterThan(x_min, x_max)) {     // x_min > 0 && x_min > x_max
    realSubtract(x_min, x_max, &tmp, &ctxtReal39);     // tmp = x_min - x_max
    convertDoubleToReal(1.1, &k, &ctxtReal39);         // k = 1.1
    realMultiply(&tmp, &k, &tmp, &ctxtReal39);         // tmp = (x_min - x_max) * 1.1
    realSubtract(x_min, &tmp, x_min, &ctxtReal39);     // x_min = x_min - (x_min - x_max) * 1.1
  }
  if(realCompareLessThan(x_min, const_0) && realCompareGreaterThan(x_min, x_max)) {        // x_min < 0 && x_min > x_max
    realSubtract(x_min, x_max, &tmp, &ctxtReal39);     // tmp = x_min - x_max
    convertDoubleToReal(1.1, &k, &ctxtReal39);         // k = 1.1
    realMultiply(&tmp, &k, &tmp, &ctxtReal39);         // tmp = (x_min - x_max) * 1.1
    realAdd(x_min, &tmp, x_min, &ctxtReal39);          // x_min = x_min + (x_min - x_max) * 1.1
  }


  //include the 0 axis
  convertDoubleToReal(-0.05, &k, &ctxtReal39);         // k = -0.05
  if(getSystemFlag(FLAG_SHOWX)) {
    if(realCompareGreaterThan(x_min, const_0) && realCompareGreaterThan(x_max, const_0)) { // x_min > 0 && x_max > 0
      if(realCompareLessEqual(x_min, x_max)) {         // x_min <= x_max
        realMultiply(x_max, &k, x_min, &ctxtReal39);   // x_min = -0.05 * x_max
      }
      else {
        realSetZero(x_min);                            // x_min = 0
      }
    }
    if(realCompareLessThan(x_min, const_0) && realCompareLessThan(x_max, const_0)) {       // x_min < 0 && x_max < 0
      if(realCompareGreaterEqual(x_min, x_max)) {      // x_min >= x_max
        realMultiply(x_max, &k, x_min, &ctxtReal39);   // x_min = -0.05 * x_max
      }
      else {
        realSetZero(x_max);                            // x_max = 0
      }
    }
  }
  if(getSystemFlag(FLAG_SHOWY)) {
    if(realCompareGreaterThan(y_min, const_0) && realCompareGreaterThan(y_max, const_0)) { // y_min > 0 && y_max > 0
      if(realCompareLessEqual(y_min, y_max)) {         // y_min <= y_max
        realMultiply(y_max, &k, y_min, &ctxtReal39);   // y_min = -0.05 * y_max
      }
      else {
        realSetZero(y_min);                            // y_min = 0
      }
    }
    if(realCompareLessThan(y_min, const_0) && realCompareLessThan(y_max, const_0)) {       // y_min < 0 && y_max < 0
      if(realCompareGreaterEqual(y_min, y_max)) {      // y_min >= y_max
        realMultiply(y_max, &k, y_min, &ctxtReal39);   // y_min = -0.05 * y_max
      }
      else {
        realSetZero(y_max);                            // y_max = 0
      }
    }
  }

  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("Axis2: x_min = %f, y_min = %f, x_max = %f, y_max = %f\n", dbl(x_min), dbl(y_min), dbl(x_max), dbl(y_max));
  #endif // STATDEBUG

  //modify the draw range if the min == max
  real_t dx, dy;
  realSubtract(x_max, x_min, &dx, &ctxtReal39);        // dx = x_max - x_min
  realSubtract(y_max, y_min, &dy, &ctxtReal39);        // dy = y_max - y_min
  if(realIsZero(&dy)) {                                // dy == 0: manufacture a 1-wide window around the value
    realAdd(y_min, const_1on2, y_max, &ctxtReal39);    // y_max = y_min + 0.5
    realSubtract(y_max, const_1, y_min, &ctxtReal39);  // y_min = y_max - 1
    realSubtract(y_max, y_min, &dy, &ctxtReal39);      // dy = y_max - y_min
  }
  else {
    graphRangeGuard(y_min, y_max);                     //a range below the graph working precision compresses it: widen as per  Ylo/Yhi
    realSubtract(y_max, y_min, &dy, &ctxtReal39);      // dy = y_max - y_min
  }
  if(realIsZero(&dx)) {                                // dx == 0: manufacture a 1-wide window around the value
    realAdd(x_min, const_1on2, x_max, &ctxtReal39);    // x_max = x_min + 0.5
    realSubtract(x_max, const_1, x_min, &ctxtReal39);  // x_min = x_max - 1
    realSubtract(x_max, x_min, &dx, &ctxtReal39);      // dx = x_max - x_min
  }

  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("Axis3a: x_min = %f, y_min = %f, x_max = %f, y_max = %f, dx=%f, dy=%f, \n", dbl(x_min), dbl(y_min), dbl(x_max), dbl(y_max), dbl(&dx), dbl(&dy));
  #endif // STATDEBUG

  //Calc zoom scales
  float plotzoomy = 1;
  float plotzoomx = 1;
  if(mode == PLOTSTAT) {
    //the ZOOM command from outside the PLOT mode only works for PLSTAT
//    const int8_t RangeHi = 0;
//    const int8_t RangeLo = -3;
//    if(PLOT_ZOOM > RangeHi) {
//      PLOT_ZOOM = RangeHi;
//    }
//    else if(PLOT_ZOOM < RangeLo) {
//      PLOT_ZOOM = RangeLo;
//    }
    //widen the histogram range to (n+2)/(n-1) of the bin-mid span: 1.5 bin widths of margin per side for the columns
    float histofactor = drawHistogram == 0 ? 1 : ((float)statnum + 2.0f)  /  ((float)(statnum) - 1.0f);
    calculateZoomFactor(3.0896507158606767, PLOT_ZOOM, &plotzoomx);   // 4.5^0.75 per zoom level
    plotzoomy = drawHistogram == 1 ? 1 : plotzoomx;
    multiplyZoomFactors(plotzoomx, plotzoomy, histofactor, x_min, x_max, y_min, y_max, &dx, &dy);
    if(drawHistogram == 1) {
      realSetZero(y_min);
    }
  }
  else { //mode != PLOTSTAT
    if(PLOT_ZMY != zoomOverride) {
      if(PLOT_ZMY == zoomOverride-1 || PLOT_ZMY == zoomOverride+1) {
        PLOT_ZMY = 0;
      }
      else if(PLOT_ZMY > zoomOverride+1) {
        PLOT_ZMY = zoomRangeLo;
      }
      else if(PLOT_ZMY < zoomRangeLo) {
        PLOT_ZMY = zoomRangeHi;
      }
      calculateZoomFactor(2.2870037808402090, PLOT_ZMY, &plotzoomy);  // 4.5^0.55 per zoom level
      //use this line if the x-display-range is to be the same as the y-display-range
      //plotzoomx = plotStatMx[0]=='D' ? 1 : plotzoomy;
      multiplyZoomFactors(plotzoomx, plotzoomy, 1/*histofactor*/, x_min, x_max, y_min, y_max, &dx, &dy);
      //printf("PLOT_ZMY=%i plotzoomx=%f, plotzoomy=%f\n",PLOT_ZMY, plotzoomx, plotzoomy);
    }
    else {

      //PLOT_ZMY = 18, special case to allow Ylo Yhi
      //_LY _UY override only if ZOOM is not set, AND Yup and Ylo are not zero, AND both are finite (NaN/infinite from old backups fall back to the default range)
      if(fabs(plotzoomx-1) < 0.00001 && fabs(plotzoomy-1) < 0.00001 && !(real34IsZero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_LY)) && real34IsZero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_UY)))
          && !real34IsSpecial(REGISTER_REAL34_DATA(RESERVED_VARIABLE_LY)) && !real34IsSpecial(REGISTER_REAL34_DATA(RESERVED_VARIABLE_UY))) {
        real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_LY), y_min);    // y_min = Ylo, the user's reserved variable
        real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_UY), y_max);    // y_max = Yhi
        graphRangeGuard(y_min, y_max);                 //swap reversed limits; widen Ylo == Yhi and spans below working precision
      }
      else {
        int32ToReal(-10, y_min);                       // y_min = -10
        int32ToReal(10, y_max);                        // y_max = 10
      }

    }
  }



  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("Axis3b: x_min = %f, y_min = %f, x_max = %f, y_max = %f, dx=%f, dy=%f \n", dbl(x_min), dbl(y_min), dbl(x_max), dbl(y_max), dbl(&dx), dbl(&dy));
  #endif // STATDEBUG



  //Cause scales to be the same: plotStatScale rules the fnPlotStat plots, the user's SCALE flag the fnPlotSQ plots
  if(mode == PLOTSTAT ? (plotStatScale != 0) : getSystemFlag(FLAG_SCALE)) {
    // if y >> x, then y simply takes on the X range and can be increased using ZMY
    if(mode == PLOTSTAT) {
      if(realCompareGreaterThan(x_min, y_min)) {       // x_min = min(x_min, y_min)
        realCopy(y_min, x_min);
      }
      if(realCompareLessThan(x_max, y_max)) {          // x_max = max(x_max, y_max)
        realCopy(y_max, x_max);
      }
      realCopy(x_min, y_min);                          // y_min = x_min
      realCopy(x_max, y_max);                          // y_max = x_max
    }
    else {  //new equal scale calculation to keep the grpah centre of screen
      realSubtract(x_max, x_min, &dx, &ctxtReal39);    // dx = |x_max - x_min|
      realSetPositiveSign(&dx);
      realSubtract(y_max, y_min, &dy, &ctxtReal39);    // dy = |y_max - y_min|
      realSetPositiveSign(&dy);
      convertDoubleToReal(1e-10, &k, &ctxtReal39);     // k = 1e-10
      realDivide(&dy, &dx, &tmp, &ctxtReal39);         // tmp = dy/dx (infinite when dx == 0; harmless, the dx > 1e-10 test excludes that case like the float original did)
      bool_t dxBigEnough = realCompareGreaterThan(&dx, &k);                        // dx > 1e-10
      int32ToReal(100000, &k);                         // k = 100000
      if(dxBigEnough && realCompareGreaterThan(&tmp, &k)) {                        // dx > 1e-10 && dy/dx > 100000: y takes on the x range
        realCopy(x_min, y_min);                        // y_min = x_min
        realCopy(x_max, y_max);                        // y_max = x_max
        realSubtract(x_max, x_min, &dx, &ctxtReal39);  // dx = |x_max - x_min|
        realSetPositiveSign(&dx);
        realSubtract(y_max, y_min, &dy, &ctxtReal39);  // dy = |y_max - y_min|
        realSetPositiveSign(&dy);
      }
      else {
        if(realCompareGreaterThan(&dx, &dy)) {         // the larger of dx and dy rules both axes
          realCopy(&dx, &dy);                          // dy = dx
        }
        else {
          realCopy(&dy, &dx);                          // dx = dy
        }
      }
      realAdd(x_min, x_max, &tmp, &ctxtReal39);        // tmp = x_min + x_max
      realMultiply(&tmp, const_1on2, &tmp, &ctxtReal39);                           // tmp = (x_min + x_max) / 2, the x centre
      realMultiply(&dx, const_1on2, &k, &ctxtReal39);  // k = dx/2
      realSubtract(&tmp, &k, x_min, &ctxtReal39);      // x_min = centre - dx/2
      realAdd(x_min, &dx, x_max, &ctxtReal39);         // x_max = x_min + dx
      realAdd(y_min, y_max, &tmp, &ctxtReal39);        // tmp = y_min + y_max
      realMultiply(&tmp, const_1on2, &tmp, &ctxtReal39);                           // tmp = (y_min + y_max) / 2, the y centre
      realMultiply(&dy, const_1on2, &k, &ctxtReal39);  // k = dy/2
      realSubtract(&tmp, &k, y_min, &ctxtReal39);      // y_min = centre - dy/2
      realAdd(y_min, &dy, y_max, &ctxtReal39);         // y_max = y_min + dy
    }
  }

  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("Axis3c: x_min = %f, y_min = %f, x_max = %f, y_max = %f, dx=%f, dy=%f \n", dbl(x_min), dbl(y_min), dbl(x_max), dbl(y_max), dbl(&dx), dbl(&dy));
  #endif // STATDEBUG


}





void graph_plotmem(void) {
  currentKeyCode = 255;
  #if defined(OPTION_MOREGRAPHICS)
      #if defined(STATDEBUG) && defined(PC_BUILD)
        uint16_t i;
        int16_t cnt1;
        cnt1 = drawMxN();
        printf("Stored values n=%i of matrix:%s\n", cnt1, plotStatMx);
        for(i = 0; i < cnt1; ++i) {
          printf("i = %3u x = %9f; y = %9f\n", i, grf_x(i), grf_y(i));
        }
      #endif // STATDEBUG && PC_BUILD

      if(!reDraw) {
        #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
          printf("graph_plotmem: Not reDrawing, text only\n");
        #endif // PC_BUILD &&MONITOR_CLRSCR
        clearScreenGraphs(1, clrTextArea, !clrGraphArea);
        graph_text();
        return;
      }
      else {
        #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
          printf("graph_plotmem: Drawing\n");
        #endif // PC_BUILD &&MONITOR_CLRSCR
        clearScreenGraphs(2, !clrTextArea, clrGraphArea);
        reDraw = false; //draw now and block reDraw in the next round
      } //continue with draw

      #if defined(LOW_GRAPH_ACC)
        int32_t s34 = ctxtReal34.digits, s39 = ctxtReal39.digits, s51 = ctxtReal51.digits, s75 = ctxtReal75.digits;
        //Change to SDIGS digit operation for graphs;
        ctxtReal34.digits = significantDigitsForScreen;
        ctxtReal39.digits = significantDigitsForScreen+3;
        ctxtReal51.digits = significantDigitsForScreen+3;
        ctxtReal75.digits = significantDigitsForScreen+3;
      #endif //LOW_GRAPH_ACC
      regStatsXY = findNamedVariable(plotStatMx);
      uint16_t cnt, ix, statnum;
      int16_t xo, xn, xN1;
      int16_t yo, yn;
      int16_t yN0 = 0, yN1 = 0;
      float x;
      float sx, sy;
      real_t xr, yr;
      float ddx = FLoatingMax;
      float dxx = FLoatingMax;
      float dydx = FLoatingMax;
      float inty = 0;
      float inty_off = 0;
      float rmsy = 0;

      statnum = 0;

      if((plotStatMx[0]=='S' ? statMxN() >= 2 : false) || (plotStatMx[0]=='D' ? drawMxN() >= 2 : false)) {
        if(plotStatMx[0]=='S') {
          statnum = statMxN();  //          realToInt32(SIGMA_N, statnum);
        }
        else {
          statnum = drawMxN();
        }
        #if defined(STATDEBUG)
          printf("points n=%d\n", statnum);
        #endif // STATDEBUG
      }

      if(statnum >= 2) {
        //GRAPH SETUP
        plotXYn = (getSystemFlag(FLAG_PL_2L) && plotXYroute && plotStatMx[0] == 'S') ? statnum : 0;    // plotXYn: count-vs-value: read N rows as 2N points
        statnum += plotXYn;                                                         // so scaling and drawing cover both series; reset to 0 at plotmemExit

        roundedTicks = true;
        graph_axis();                        //Draw the axis on any uncontrolled scale to start. Maybe optimize by remembering if there is an image on screen Otherwise double axis draw.
        if(PLOT_AXIS) {
          graph_text();
        }

      if(getSystemFlag(FLAG_VECT) || getSystemFlag(FLAG_NVECT)) {
        plotmode = _VECT;
      }
      else {
        plotmode = _SCAT;
      }

        if(getSystemFlag(FLAG_PINTG)) {
          rmsy = fabs(grf_y(0));
          for(ix = 0; (ix < statnum); ++ix) {
            rmsy = sqrtf((rmsy * rmsy * ix + grf_y(ix) * grf_y(ix)) / (ix+1.0f));      // Changed rmsy to use the standard RMS calc, and not shoft it to the trapezium x-centre
          }
        inty_off = rmsy;
        }

        //AUTOSCALE
        convertDoubleToReal(FLoatingMax, x_min, &ctxtReal39);              // seed the range impossibly wide open: x_min = +1e38, x_max = -1e38,
        convertDoubleToReal(FLoatingMin, x_max, &ctxtReal39);              //   so the first data point replaces both
        convertDoubleToReal(FLoatingMax, y_min, &ctxtReal39);
        convertDoubleToReal(FLoatingMin, y_max, &ctxtReal39);
        #if defined(STATDEBUG)
          printf("Axis0: x: %f -> %f y: %f -> %f   \n", dbl(x_min), dbl(x_max), dbl(y_min), dbl(y_max));
        #endif // STATDEBUG
        if(plotmode != _VECT) {
          invalid_intg = false;                                                      //integral scale
          invalid_diff = false;                                                      //Differential dydx scale
          invalid_rms  = false;                                                      //RMSy

//#################################################### vvv SCALING LOOP DIFF INTG RMS vvv #########################
/**/      if(getSystemFlag(FLAG_PDIFF) || getSystemFlag(FLAG_PINTG) || getSystemFlag(FLAG_PRMS)) {
/**/        inty = inty_off;                                                          //  integral starting constant co-incides with graph
/**/        if(getSystemFlag(FLAG_PRMS)) {
/**/          rmsy = fabs(grf_y(0));
/**/        }
/**/
/**/        for(ix = 0; (ix < statnum); ++ix) {
              if(doubleSpecialLabel(grf_x(ix)) != NULL || doubleSpecialLabel(grf_y(ix)) != NULL) {
                continue;
              }
/**/          if(ix != 0) {
/**/            ddx = grf_x(ix) - grf_x(ix-1);                                            //used in DIFF and INT
/**/            if(ddx<=0) {                                                              //Cannot get slop or area if x is not growing in positive dierection
/**/              convertDoubleToReal(FLoatingMax, x_min, &ctxtReal39);
/**/              convertDoubleToReal(FLoatingMin, x_max, &ctxtReal39);
/**/              convertDoubleToReal(FLoatingMax, y_min, &ctxtReal39);
/**/              convertDoubleToReal(FLoatingMin, y_max, &ctxtReal39);
/**/              invalid_diff = true;
/**/              invalid_intg = true;
/**/              invalid_rms  = true;
/**/              break;
/**/            }
/**/            else {
/**/              grf_x_r(ix, &xr);                    // xr = grf_x(ix)
/**/              if(realCompareLessThan(&xr, x_min)) {                    // if(grf_x(ix) < x_min) x_min = grf_x(ix)
/**/                realCopy(&xr, x_min);
/**/              }
/**/              if(realCompareGreaterThan(&xr, x_max)) {                 // if(grf_x(ix) > x_max) x_max = grf_x(ix)
/**/                realCopy(&xr, x_max);
/**/              }
/**/              if(getSystemFlag(FLAG_PDIFF)) {
/**/                //plotDiff(); //dydx                                            //Differential
/**/                if(ddx != 0) {
/**/                  if(ix == 1) {                               // only two samples available
/**/                    dydx = (grf_y(ix) - grf_y(ix-1)) / ddx;   // Differential
/**/                  }
/**/                  else if(ix >= 2) {                          // ix >= 2 three samples available 0 1 2
/**/                    dydx = (grf_y(ix-2) - 4.0 * grf_y(ix-1) + 3.0 * grf_y(ix)) / 2.0 / ddx; //ChE 205 — Formulas for Numerical Differentiation, formule 32
/**/                  }
/**/                }
/**/                else {
/**/                  dydx = FLoatingMax;
/**/                }
/**/
/**/                convertDoubleToReal(dydx, &yr, &ctxtReal39);        // yr = dydx, the float overlay value as y-range candidate
/**/                if(realCompareLessThan(&yr, y_min)) {                 // if(dydx < y_min) y_min = dydx
/**/                  realCopy(&yr, y_min);
/**/                }
/**/                if(realCompareGreaterThan(&yr, y_max)) {              // if(dydx > y_max) y_max = dydx
/**/                  realCopy(&yr, y_max);
/**/                }
/**/              }
/**/              if(getSystemFlag(FLAG_PINTG)) {
/**/                inty = inty + (grf_y(ix) + grf_y(ix-1)) / 2 * ddx;
/**/                convertDoubleToReal(inty, &yr, &ctxtReal39);        // yr = inty, the float overlay value as y-range candidate
/**/                if(realCompareLessThan(&yr, y_min)) {                 // if(inty < y_min) y_min = inty
/**/                  realCopy(&yr, y_min);
/**/                }
/**/                if(realCompareGreaterThan(&yr, y_max)) {              // if(inty > y_max) y_max = inty
/**/                  realCopy(&yr, y_max);
/**/                }
/**/              }
/**/              if(getSystemFlag(FLAG_PRMS)) {
/**/                rmsy = sqrtf((rmsy * rmsy * ix + grf_y(ix) * grf_y(ix)) / (ix+1.0f));      // Changed rmsy to use the standard RMS calc, and not shoft it to the trapezium x-centre
/**/                convertDoubleToReal(rmsy, &yr, &ctxtReal39);        // yr = rmsy, the float overlay value as y-range candidate
/**/                if(realCompareLessThan(&yr, y_min)) {                 // if(rmsy < y_min) y_min = rmsy
/**/                  realCopy(&yr, y_min);
/**/                }
/**/                if(realCompareGreaterThan(&yr, y_max)) {              // if(rmsy > y_max) y_max = rmsy
/**/                  realCopy(&yr, y_max);
/**/                }
/**/              }
/**/            }
/**/          }
/**/          if(exitKeyWaiting()) {
/**/             goto plotmemExit;
/**/          }
/**/        }
/**/      }
//#################################################### ^^^ SCALING LOOP ^^^ #########################

          #if defined(STATDEBUG)
            printf("Axis0b1: x: %f -> %f y: %f -> %f  %d \n", dbl(x_min), dbl(x_max), dbl(y_min), dbl(y_max), invalid_diff);
          #endif // STATDEBUG

//#################################################### vvv SCALING LOOP  vvv #########################
/**/      uint16_t y_maxcnt=2;
/**/      uint16_t y_mincnt=2;
/**/      float a0, a1, a2, a3, a4, a5, a6, a7, a8;   //Digital filter to get rid of short sharp peak like some asymptotes
/**/      float aa = 1;
/**/      a0 = 0;
/**/      a1 = 0;
/**/      a2 = 0;
/**/      a3 = 0;
/**/      a4 = 0;
/**/      a5 = 0;
/**/      a6 = 0;
/**/      a7 = 0;
/**/      a8 = 0;
/**/
/**/      float scaleRmsy = 0;
/**/
/**/      if(getSystemFlag(FLAG_PBOX) || getSystemFlag(FLAG_PLINE) || getSystemFlag(FLAG_PCROS) || getSystemFlag(FLAG_PPLUS) || !(getSystemFlag(FLAG_PDIFF) || getSystemFlag(FLAG_PINTG))) {  //XXXX
/**/
/**/        //pre-loop to cover trivial cases of symmetrical axis
/**/        for(cnt=0; (cnt < statnum); cnt++) {
              if((doubleSpecialLabel(grf_x(cnt)) != NULL) || (doubleSpecialLabel(grf_y(cnt)) != NULL)) {
                continue;
              }
/**/          #if defined(STATDEBUG)
/**/            printf("Axis0a: cnt/statnum: %i/%i  x: %f y: %f   \n", cnt, statnum, grf_x(cnt), grf_y(cnt));
/**/          #endif // STATDEBUG
/**/          grf_x_r(cnt, &xr);                       // xr = grf_x(cnt)
/**/          grf_y_r(cnt, &yr);                       // yr = grf_y(cnt)
/**/          if(realCompareLessThan(&xr, x_min)) {                        // if(grf_x(cnt) < x_min) x_min = grf_x(cnt)
/**/            realCopy(&xr, x_min);
/**/          }
/**/          if(realCompareGreaterThan(&xr, x_max)) {                     // if(grf_x(cnt) > x_max) x_max = grf_x(cnt)
/**/            realCopy(&xr, x_max);
/**/          }
/**/          if(realCompareLessThan(&yr, y_min)) {                        // if(grf_y(cnt) < y_min) y_min = grf_y(cnt)
/**/            realCopy(&yr, y_min);
/**/          }
/**/          if(realCompareGreaterThan(&yr, y_max)) {                     // if(grf_y(cnt) > y_max) y_max = grf_y(cnt)
/**/            realCopy(&yr, y_max);
/**/          }
/**/          scaleRmsy = sqrtf((scaleRmsy * scaleRmsy * cnt + grf_y(cnt) * grf_y(cnt)) / (cnt+1.0f));
/**/        }
/**/
/**/        //The peak filter and symmetry heuristics below are dimensionless float logic: run them on float mirrors of the
/**/        //real_t y range and commit only values they changed, so extreme-magnitude ranges pass through untouched.
/**/        float fy_min, fy_max;
/**/        {
/**/          double d;
/**/          realToDouble(y_min, &d);
/**/          fy_min = (float)d;
/**/          realToDouble(y_max, &d);
/**/          fy_max = (float)d;
/**/        }
/**/        float fy_min0 = fy_min;
/**/        float fy_max0 = fy_max;
/**/
/**/        //pre-loop to cover trivial quasi symmetrical axis
/**/        if(fy_max > 0 && fy_min < 0 && (fy_max > 4 * scaleRmsy)) { //force the RMS if large peaks occur
/**/          fy_max = scaleRmsy;
/**/        }
/**/        else if(fy_max > 0 && fy_min < 0 && (-fy_min > 4 * scaleRmsy)) {
/**/          fy_min = -scaleRmsy;
/**/        }
/**/        else if(fy_max > 0 && fy_min < 0 && (fy_max > -fy_min) && (fy_max / fy_min < 1.2)) { //make x-axis sit in the middle if close enough
/**/          fy_min = -fy_max;
/**/        }
/**/        else if(fy_max > 0 && fy_min < 0 && (fy_max < -fy_min) && (fy_min / fy_max < 1.2)) {
/**/          fy_max = -fy_min;
/**/        }
/**/
/**/
/**/         {
/**/          for(cnt=0; (cnt < statnum); cnt++) {
                if((doubleSpecialLabel(grf_x(cnt)) != NULL) || (doubleSpecialLabel(grf_y(cnt)) != NULL)) {
                  continue;
                }
/**/            #if defined(STATDEBUG)
/**/              printf("Axis0a: cnt/statnum: %i/%i  x: %f y: %f   \n", cnt, statnum, grf_x(cnt), grf_y(cnt));
/**/            #endif // STATDEBUG
/**/            a8 = a7;
/**/            a7 = a6;
/**/            a6 = a5;
/**/            a5 = a4;
/**/            a4 = a3;
/**/            a3 = a2;
/**/            a2 = a1;
/**/            a1 = a0;
/**/            a0 = grf_y(cnt);
/**/            if(cnt < 8) {
/**/              aa = a0;
/**/            }
/**/            else {
/**/              aa = a8*0.2 + a7 *0.2 + a6*0.1 + a5*0.1 + a4*0.1 + a3*0.1 + a2*0.1 + a1*0.1;
/**/            }
/**/       //     if(aa != 0 && fabs(a0/aa) < 3 && a0 != 0) {
/**/       //       aa = a0 * 1.1;
/**/       //     }
/**/            //printf("%f %f %f %f %f %f %f %f %f  %f\n", a8, a7, a6, a5, a4, a3, a2, a1, a0, aa);
/**/            if(aa < fy_min) {
/**/              y_mincnt++;
/**/              if(fabs(aa / fy_min) < 4 ) {//|| aa == a0 * 1.1) {
/**/                if(aa < fy_min) {
/**/                 fy_min = aa;
/**/                }
/**/                y_mincnt=0;
/**/              }
/**/              else if(y_mincnt==3) {
/**/                fy_min = aa;
/**/                y_mincnt=0;
/**/              }
/**/            }
/**/            else {
/**/             y_mincnt=0;
/**/            }
/**/
/**/            if(aa > fy_max) {
/**/              y_maxcnt++;
/**/              if(fabs(aa / fy_max) < 4 ) {//|| aa == a0 * 1.1) {
/**/                if(aa>fy_max) {
/**/                  fy_max = aa;
/**/                }
/**/                y_maxcnt=0;
/**/              }
/**/              else if(y_maxcnt==3) {
/**/                fy_max = aa;
/**/                y_maxcnt=0;
/**/              }
/**/            }
/**/            else {
/**/              y_maxcnt=0;
/**/            }
/**/
/**/            #if defined(STATDEBUG)
/**/              printf("Axis0b: x: %f -> %f y: %f -> %f   \n", dbl(x_min), dbl(x_max), fy_min, fy_max);
/**/            #endif // STATDEBUG
/**/            if(exitKeyWaiting()) {
/**/              goto plotmemExit;
/**/            }
/**/          }
/**/        }
/**/        if(fy_min != fy_min0) {
/**/          convertDoubleToReal(fy_min, y_min, &ctxtReal39);
/**/        }
/**/        if(fy_max != fy_max0) {
/**/          convertDoubleToReal(fy_max, y_max, &ctxtReal39);
/**/        }
/**/      }
/**/    }
/**/
/**/    else {                 //VECTOR
/**/      sx =0;
/**/      sy =0;
/**/      for(cnt=0; (cnt < statnum); cnt++) {            //### Note XXX E- will stuff up statnum!
/**/        sx = sx + (!getSystemFlag(FLAG_NVECT) ? grf_x(cnt) : grf_y(cnt));
/**/        sy = sy + (!getSystemFlag(FLAG_NVECT) ? grf_y(cnt) : grf_x(cnt));
            if((doubleSpecialLabel(sx) != NULL) || (doubleSpecialLabel(sy) != NULL)) {
              continue;
            }
/**/        convertDoubleToReal(sx, &xr, &ctxtReal39); // xr = sx, the running vector sum
/**/        convertDoubleToReal(sy, &yr, &ctxtReal39); // yr = sy
/**/        if(realCompareLessThan(&xr, x_min)) {                          // if(sx < x_min) x_min = sx
/**/          realCopy(&xr, x_min);
/**/        } else if(realCompareGreaterThan(&xr, x_max)) {                // else if(sx > x_max) x_max = sx
/**/          realCopy(&xr, x_max);
/**/        }
/**/        if(realCompareLessThan(&yr, y_min)) {                          // if(sy < y_min) y_min = sy
/**/          realCopy(&yr, y_min);
/**/        } else if(realCompareGreaterThan(&yr, y_max)) {                // else if(sy > y_max) y_max = sy
/**/          realCopy(&yr, y_max);
/**/        }
/**/        if(exitKeyWaiting()) {
/**/          goto plotmemExit;
/**/        }
/**/      }
/**/    }
//#################################################### ^^^ SCALING LOOP ^^^ #########################


        if(realCompareGreaterThan(x_min, x_max) || realCompareGreaterThan(y_min, y_max)) { //the +-1E38 seeds are untouched: not one finite sample in the range, nothing to draw
          calcMode = CM_NORMAL;
          displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "no plottable sample in the plot range");
            moreInfoOnError("In function graph_plotmem:", errorMessage, NULL, NULL);
          #endif // EXTRA_INFO_ON_CALC_ERROR == 1
          goto plotmemExit;
        }


        //Manipulate the obtained axes positions
        #if defined(STATDEBUG)
         printf("Axis1a: x_min = %f, y_min = %f, x_max = %f, y_max = %f, \n", dbl(x_min), dbl(y_min), dbl(x_max), dbl(y_max));
        #endif // STATDEBUG


        graph_Include0(!PLOTSTAT, 0);


        roundedTicks = true;
        graph_axis();
        if(PLOT_AXIS) {
          graph_text();
        }

        #if defined(STATDEBUG)
          printf("Axis3d: x_min = %f, y_min = %f, x_max = %f, y_max = %f \n", dbl(x_min), dbl(y_min), dbl(x_max), dbl(y_max));
        #endif // STATDEBUG


        if(plotmode != _VECT) {
          grf_y_r(0, &yr);
          grf_x_r(0, &xr);
        }
        else {
          realSetZero(&yr);
          realSetZero(&xr);
        }
        yn = screen_window_y_r(y_min, &yr, y_max);
        xn = screen_window_x_r(x_min, &xr, x_max);
        xN1 = xn;
        yN1 = yn;

        #if defined(STATDEBUG)
          printf("Axis3e: x_min = %f, y_min = %f, x_max = %f, y_max = %f \n", dbl(x_min), dbl(y_min), dbl(x_max), dbl(y_max));
        #endif // STATDEBUG

        sx = 0;
        sy = 0;
        //GRAPH
        ix = 0;
        inty = inty_off;                                                         //  integral starting constant co-incides with graph
        rmsy = 0;
        if(getSystemFlag(FLAG_PRMS)) {
          rmsy = fabs(grf_y(0));
        }

        //#################################################### vvv MAIN GRAPH LOOP vvv #########################
        bool_t plotInCurves = getSystemFlag(FLAG_PCURVE);

        static int16_t prev_y_unclipped = 0;
        if(plotInCurves) {
          plotline3(0, 0, 0, 0, true, false); //reset
        }
        for(ix = 0; (ix < statnum); ++ix) {
          if(plotmode != _VECT) {
            x = 0;

            if(ix !=0 && ( (getSystemFlag(FLAG_PDIFF) && !invalid_diff) || (getSystemFlag(FLAG_PINTG) && !invalid_intg) || (getSystemFlag(FLAG_PRMS) && !invalid_rms) )) {
              ddx = grf_x(ix) - grf_x(ix-1);
              if(getSystemFlag(FLAG_PDIFF) && ddx != 0) {
                if(ix == 1 || ( fabs( ((grf_x(ix) - grf_x(ix-1)) / (grf_x(ix-1) - grf_x(ix-2))) - 1) > 0.0001 )) {                               // only two samples available
                  dydx = (grf_y(ix) - grf_y(ix-1)) / ddx;   // Differential
                  dxx = (grf_x(ix) + grf_x(ix-1) )/2;
                }
                else { //if(ix >= 2)                        // ix >= 2 three samples available 0 1 2
                  dydx = ( grf_y(ix-2) - 4.0 * grf_y(ix-1) + 3.0 * grf_y(ix) ) / 2.0 / ddx; //ChE 205 — Formulas for Numerical Differentiation, formule 32
                  dxx = (grf_x(ix));
                }
              }
              else {
                dydx = FLoatingMax;
              }

              if(getSystemFlag(FLAG_PRMS))  {
                rmsy = sqrtf( (rmsy * rmsy * ix + grf_y(ix) * grf_y(ix)) / (ix+1.0f) );      // Changed rmsy to use the standard RMS calc, and not shoft it to the trapezium x-centre
              }
              if(getSystemFlag(FLAG_PINTG)) {
                inty = inty + (grf_y(ix) + grf_y(ix-1)) / 2 * ddx;
              }
            }

            x = grf_x(ix);                           // float x stays for the RMS overlay maths below
            grf_x_r(ix, &xr);                        // xr = grf_x(ix)
            grf_y_r(ix, &yr);                        // yr = grf_y(ix)

          }
          else { //_VECT
            sx = sx + (!getSystemFlag(FLAG_NVECT) ? grf_x(ix) : grf_y(ix));
            sy = sy + (!getSystemFlag(FLAG_NVECT) ? grf_y(ix) : grf_x(ix));
            x = sx;
            convertDoubleToReal(sx, &xr, &ctxtReal39);                     // xr = sx
            convertDoubleToReal(sy, &yr, &ctxtReal39);                     // yr = sy
          }
          xo = xN1;
          yo = yN1;
          yN0 = prev_y_unclipped;

          xN1 = screen_window_x_r(x_min, &xr, x_max);
          yN1 = screen_window_y_nolimit_r(y_min, &yr, y_max);
          int16_t current_y_unclipped = yN1;

          if(ix == 0 || ix == plotXYn) {   // plotXYn: first point: ix == plotXYn starts the second (y column) line
            if(plotInCurves && ix != 0) {
              plotline3(0, 0, 0, 0, false, true);  //clear the first series' last curve segment
              plotline3(0, 0, 0, 0, true, false);  //reset for the second series
            }
            xo = xN1;
            yo = yN1;
            yN0 = yN1;
            prev_y_unclipped = yN1;  // Initialize for next iteration
            if(plotmode != _VECT && xN1 < SCREEN_WIDTH_GRAPH && xN1 >= (SCREEN_WIDTH - SCREEN_HEIGHT_GRAPH) && yN1 < SCREEN_HEIGHT_GRAPH && yN1 >= 0) {
              plotPointGeneric(xN1, yN1, xN1, yN1,   // draw the first point's marker; no line (no previous point)
                                 (plotXYn && ix < plotXYn) ? (getSystemFlag(FLAG_PBOX) || getSystemFlag(FLAG_PPLUS)) : getSystemFlag(FLAG_PCROS), false, (plotXYn && ix < plotXYn) ? getSystemFlag(FLAG_PCROS) : getSystemFlag(FLAG_PBOX),
                                 (plotXYn && ix < plotXYn) ? false : getSystemFlag(FLAG_PPLUS), false);   // plotXYn: the x line takes the other marker, a cross for BOX or PLUS, a box for CROSS
            }
            continue;  // Skip clipping/line for first point (no previous point)
          }

          #if defined(STATDEBUG)
            printf("\n         xN1 = %d : (x_min = %f, x=%f, x_max = %f) ", xN1, dbl(x_min), x, dbl(x_max));
            printf("yN0 = %d yN1 = %d : (y_min = %f, y=%f, y_max = %f) \n", yN0, yN1, dbl(y_min), dbl(&yr), dbl(y_max));
            printf("plotting graph table[%d] = x:%f y:%f (dxx:%f dydx:%f) inty:%f xN1:%d yN1:%d ", ix, x, dbl(&yr), dxx, dydx, inty, xN1, yN1);
            printf("   ... x-ddx/2=%d dydx=%d inty=%d\n", screenX(x-ddx/2), screenY(dydx), screenY(inty));
          #endif // STATDEBUG

          int16_t minN_y, minN_x;
          minN_y = 0;
          minN_x = SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH;

          bool_t bothOutOfScreen01 = ((yN1 >= SCREEN_HEIGHT_GRAPH) && (yN0 >= SCREEN_HEIGHT_GRAPH)) || ((yN1 < minN_y) && (yN0 < minN_y));
          bool_t outOfScreen1  = (yN1 >= SCREEN_HEIGHT_GRAPH || yN1 < minN_y);
          bool_t outOfScreen0  = (yN0 >= SCREEN_HEIGHT_GRAPH || yN0 < minN_y);

          #if defined(STATDEBUG)
            printf("Before edge checking: 001 yN1 =%4i yN0=%4i minN_y=%4i : ", (int8_t)yN1,  (int8_t)yN0, (int8_t)minN_y);
            printf("    xN1 =%4i  xo=%4i minN_x=%i\n", (int16_t)xN1, (int16_t)xo, (int16_t)minN_x);
            if(!bothOutOfScreen01 && outOfScreen0 && !outOfScreen1) {
              printf("POTENTIAL ENTRY DEBUG: yN0=%d yN1=%d slope=%s from=%s\n", yN0, yN1, yN1 > yN0 ? "POSITIVE" : "NEGATIVE", yN0 >= SCREEN_HEIGHT_GRAPH ? "BOTTOM" : (yN0 < minN_y ? "TOP" : "MIDDLE"));
            }
          #endif // STATDEBUG


            if(!bothOutOfScreen01) {
              // Coming in from bottom - BOTH positive and negative slopes
              if(outOfScreen0 && !outOfScreen1 && yN0 >= SCREEN_HEIGHT_GRAPH) {
                //printf("ENTRY CLIP BOTTOM: yN0=%d yN1=%d xo=%d xN1=%d slope=%s\n", yN0, yN1, xo, xN1, yN1 > yN0 ? "POSITIVE" : "NEGATIVE");
                int16_t dY = abs(SCREEN_HEIGHT_GRAPH - 1 - yN0);
                if(yN1 != yN0) {
                  float dxN = fabs(((float)dY)*((float)(xN1-xo))/((float)(yN1-yN0)));
                  //printf("  -> Calculated dxN=%f, new xo=%f\n", dxN, xo + dxN);
                  xo = xo + dxN;
                  yN0 = SCREEN_HEIGHT_GRAPH - 1;
                  yo = yN0;
                }
              }
              // Coming in from top - BOTH positive and negative slopes
              else if(outOfScreen0 && !outOfScreen1 && yN0 < minN_y) {
                //printf("ENTRY CLIP TOP: yN0=%d yN1=%d xo=%d xN1=%d slope=%s\n", yN0, yN1, xo, xN1, yN1 > yN0 ? "POSITIVE" : "NEGATIVE");
                int16_t dY = abs(yN0 - minN_y);
                if(yN1 != yN0) {
                  float dxN = fabs(((float)dY)*((float)(xN1-xo))/((float)(yN1-yN0)));
                  //printf("  -> Calculated dxN=%f, new xo=%f\n", dxN, xo + dxN);
                  xo = xo + dxN;
                  yN0 = minN_y;
                  yo = yN0;
                }
              }
            }

          //exceeding the negative y-axis part or the bottom of the screen, use proportional triangle to determine the part of the line to be plotted to the edge of the plotting area
          if((yN1 > yN0 && xN1 > xo && yN1 >= SCREEN_HEIGHT_GRAPH && !bothOutOfScreen01 && outOfScreen1 && !outOfScreen0) ||
             (yN1 < yN0 && xN1 > xo && yN0 >= SCREEN_HEIGHT_GRAPH && !bothOutOfScreen01 && !outOfScreen1 && outOfScreen0)) {
            //printf("EXIT CLIP BOTTOM: yN0=%d yN1=%d\n", yN0, yN1);
            int16_t dY = abs(SCREEN_HEIGHT_GRAPH - 1 - yN0);
            if(yN1 == yN0) {
              continue; // Skip horizontal lines
            }
            float dxN = fabs(((float)dY)*((float)(xN1-xo))/((float)(yN1-yN0)));
            xN1 = xo + (int16_t)(dxN + 0.5);
            yN1 = SCREEN_HEIGHT_GRAPH - 1;
          }

          //exceeding the positive y-axis part or the top of the screen, use proportional triangle to determine the part of the line to be plotted to the edge of the plotting area
          else if((yN1 < yN0 && xN1 > xo && yN1 < minN_y && !bothOutOfScreen01 &&  outOfScreen1 && !outOfScreen0) ||
                  (yN1 > yN0 && xN1 > xo && yN0 < minN_y && !bothOutOfScreen01 && !outOfScreen1 &&  outOfScreen0)) {
            //printf("EXIT CLIP TOP: yN0=%d yN1=%d\n", yN0, yN1);
            int16_t dY = abs(yN0 - minN_y);
            if(yN1 == yN0) {
              continue; // Skip horizontal lines
            }
            float dxN = fabs(((float)dY)*((float)(xN1-xo))/((float)(yN1-yN0)));
            xN1 = xo + (int16_t)(dxN + 0.5);
            yN1 = minN_y;
          }

          #if defined(STATDEBUG)
            printf("After  edge checking: 002 yN1 =%4i yN0=%4i minN_y=%4i : ", (int8_t)yN1, (int8_t)yN0, (int8_t)minN_y);
            printf("    xN1 =%4i xo=%4i minN_x=%4i\n", (int16_t)xN1, (int16_t)xo, (int16_t)minN_x);
          #endif // STATDEBUG

          if((xN1 < SCREEN_WIDTH_GRAPH && xN1 >= minN_x && yN1 < SCREEN_HEIGHT_GRAPH && yN1 >= minN_y))  {
            yn = yN1;
            xn = xN1;

            #if defined(STATDEBUG)
              if(invalid_diff || invalid_intg || invalid_rms) {
                printf("invalid_diff=%d invalid_intg=%d invalid_rms=%d \n", invalid_diff, invalid_intg, invalid_rms);
              }
            #endif // STATDEBUG

            if(plotmode != _VECT) {
              #if defined(STATDEBUG)
                printf("Not _VECT\n");
              #endif // STATDEBUG

              plotPointGeneric(xn, yn, xo, yo,
                                 (plotXYn && ix < plotXYn) ? (getSystemFlag(FLAG_PBOX) || getSystemFlag(FLAG_PPLUS)) : getSystemFlag(FLAG_PCROS) /*cross*/ ,   // plotXYn: the x line takes the other marker,
                                 false                     /*fatbox*/,                                                                                        //   a cross for BOX or PLUS, a box for CROSS
                                 (plotXYn && ix < plotXYn) ? getSystemFlag(FLAG_PCROS) : getSystemFlag(FLAG_PBOX)  /*box*/   ,
                                 (plotXYn && ix < plotXYn) ? false : getSystemFlag(FLAG_PPLUS) /*plus*/  ,
                                 false                     /*line*/   );


              if(getSystemFlag(FLAG_PDIFF) && !invalid_diff && ix != 0) {
                #if defined(STATDEBUG)
                  printf("Plotting Delta x=%f dy=%f \n", dxx, dydx);
                #endif // STATDEBUG
                plotdelta(screenX(dxx), screenY(dydx));
              }


              if(getSystemFlag(FLAG_PRMS) && !invalid_rms && ix != 0) {
                #if defined(STATDEBUG)
                  printf("Plotting RMSy x=%f rmsy=%f \n", x - ddx/2, rmsy);
                #endif // STATDEBUG
                plotrms(screenX(x - ddx/2), screenY(rmsy));
              }


              if(getSystemFlag(FLAG_PINTG) && !invalid_intg && ix !=0) {
                #if defined(STATDEBUG)
                  printf("Plotting Integral x=%f intg(x)=%f\n", x-ddx/2, inty);
                #endif // STATDEBUG
                real_t xPrev;
                grf_x_r(ix-1, &xPrev);
                int16_t xN0   = screen_window_x_r(x_min, &xPrev, x_max);
                //uint16_t xN1   = screen_window_x_r(x_min, &xr, x_max);
                int16_t yNintg= screenY(inty);
                int16_t xAvg  = ((xN0+xN1) >> 1);

                if(abs((int16_t)(xN1-xN0)) >= 6) {
                  plotint( xAvg, yNintg );
                }
                else {
                  //placePixel(xAvg, yNintg);
                  plotrect(xAvg-1, yNintg-1, xAvg+1, yNintg+1);
                }

                if(abs((int16_t)(xN1-xN0)) >= 6) {
                  plotline1(xN1,    yNintg, xAvg+2, yNintg);
                  plotline1(xAvg-2, yNintg, xN0,    yNintg);
                }
                else if(abs((int16_t)(xN1-xN0)) >= 4) {
                  plotline1(xN1,    yNintg, xAvg+2, yNintg);
                  plotline1(xAvg-2, yNintg, xN0,    yNintg);
                }

                if(getSystemFlag(FLAG_PSHADE)) {
                  int16_t yNoff = screenY(0);
                  plotrect(xN0, yN0,   xN1, yN1);
                  plotrect(xN0, yNoff, xN1, yN0);
                  if(abs((int16_t)(xN1-xN0)) >= 6) {
                    plotline1(xN0, yN0,   xN1, yN1);
                  }
                }
              }

            }
            else { // _VECT
              #if defined(STATDEBUG)
                printf("Plotting arrow\n");
              #endif // STATDEBUG
              plotarrow(xo, yo, xn, yn);
            }

            if(getSystemFlag(FLAG_PLINE)) {
              #if defined(STATDEBUG)
                printf("######       Plotting line from xo=%d yo=%d to x=%d y=%d\n\n", xo, yo, xn, yn);
              #endif // STATDEBUG
              if(plotInCurves) {
                plotline3(xo, yo, xn, yn, false, false);
              }
              else {
                plotline2(xo, yo, xn, yn);
              }
            }

          }
          else {
            #if defined(PC_BUILD)
              printf("             Not plotted: ");
              if(!(xN1 < SCREEN_WIDTH_GRAPH)) {
                printf("NOT xN1 < SCREEN_WIDTH_GRAPH; ");
              }
              if(!(xN1 >= minN_x)) {
                printf("NOT xN1 >= minN_x; ");
              }
              if(!(yN1 < SCREEN_HEIGHT_GRAPH)) {
                printf("NOT yN1<SCREEN_HEIGHT_GRAPH");
              }
              if(!(yN1 >= minN_y)) {
                printf("NOT yN1>=minN_y; ");
              }
              printf(" : xN1=%d<SCREEN_WIDTH_GRAPH=%d && xN1=%d>=minN_x=%d && yN1=%d<SCREEN_HEIGHT_GRAPH=%d && yN1=%d>=minN_y=%d\n", xN1, SCREEN_WIDTH_GRAPH, xN1, minN_x, yN1, SCREEN_HEIGHT_GRAPH, yN1, minN_y);
            #endif // PC_BUILD
          }
          if(exitKeyWaiting()) {
            goto plotmemExit;
          }
          #if defined(STATDEBUG) && defined(PC_BUILD)
            fflush(stdout);
          #endif // STATDEBUG

          prev_y_unclipped = current_y_unclipped;
        }
        //#################################################### ^^^ MAIN GRAPH LOOP ^^^ #########################
        if(getSystemFlag(FLAG_PLINE) && plotInCurves) {
          #if defined(STATDEBUG)
            printf("######       Plotting last line segment from xo=%d yo=%d to x=%d y=%d\n\n", xo, yo, xn, yn);
          #endif // STATDEBUG
          plotline3(0, 0, 0, 0, false, true); //last line segment
        }

      }
      else {
        if(plotStatMx[0] == 'S') {   // "no statistical data" only applies to a stat plot. A draw matrix ('D') with <2 points is a function plot still being built (e.g. a mistimed refresh mid-build) - not an error, draw nothing.
          calcMode = CM_NORMAL;
          displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "There is no statistical data available!");
            moreInfoOnError("In function graph_plotmem:", errorMessage, NULL, NULL);
          #endif // EXTRA_INFO_ON_CALC_ERROR == 1
        }
      }

plotmemExit: ;
      plotXYn = 0;
      #if defined(LOW_GRAPH_ACC)
        //Change to normal operation for graphs;
        ctxtReal34.digits = s34;
        ctxtReal39.digits = s39;
        ctxtReal51.digits = s51;
        ctxtReal75.digits = s75;
      #endif //LOW_GRAPH_ACC
  #endif // OPTION_MOREGRAPHICS
}


//-----------------------------------------------------//-----------------------------------------------------

static void formatStatValue(double value, char *buf) {
  const char *special = doubleSpecialLabel(value);
  if(special != NULL) {
    snprintf(buf, 150, "%s", special); // "NaN", "+Inf" or "-Inf" for a special value, or NULL for a normal
  }
  else {
    snprintf(buf, 150, "%s", formatCore(value, 10, false, buf, 150));
  }
}


void fnStatList() {
    char tmpstr1[150], tmpstr2[150];
    int16_t ix, ixx, statnum;
    clearScreen(1);
    refreshStatusBar();
    if(regStatsXY != INVALID_VARIABLE && (plotStatMx[0]=='D' ? drawMxN() >= 1 : false)) {
      statnum = drawMxN();
      fnStatSum(0);
      sprintf(tmpString, "Graph data: N = %d", statnum);
      print_linestr(tmpString, true);
                                  #if defined(STATDEBUG)
                                    printf("Stat data %d - %d (%s)\n", statnum-1, max(0, statnum-1-6), tmpString );
                                  #endif // STATDEBUG
      if(ListXYposition > 0) {
        ListXYposition = 0;
      }
      else if(statnum - (min(10, statnum)-1) - 1 + ListXYposition < 0) {
        ListXYposition = - (statnum - (min(10, statnum)-1) - 1);
      }
      for(ix = 0; (ix < min(10, statnum)); ++ix) {
        ixx = statnum - ix - 1 + ListXYposition;
        char tmpBuf[150];
        formatStatValue(grf_x(ixx), tmpBuf);
        snprintf(tmpstr1, 150, "[%3d] x%4s%14.32s, ", ixx+1, "", tmpBuf);
        formatStatValue(grf_y(ixx), tmpBuf);
        snprintf(tmpstr2, 150, "y%4s%14.32s, ", "", tmpBuf);
        strcat(tmpstr1, tmpstr2);
        print_numberstr(tmpstr1, false);
        #if defined(STATDEBUG)
          printf("%d:%s\n", ixx, tmpstr1);
        #endif // STATDEBUG
      }
    }
}

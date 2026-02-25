// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//** //JM
 * \file graphs.c Graphing module
 ***********************************************/

#include "c47.h"

//#define STATDEBUG

bool_t    invalid_intg = true;
bool_t    invalid_diff = true;
bool_t    invalid_rms  = true;

float     x_min = 0;
float     x_max = 1;
float     y_min = 0;
float     y_max = 1;
int8_t    PLOT_ZMY = 0;


void graphResetCommon() {
  graph_dx      = 0;
  graph_dy      = 0;

  clearSystemFlag(FLAG_CPXPLOT);
  clearSystemFlag(FLAG_SHOWY);
  clearSystemFlag(FLAG_SHOWX);
  clearSystemFlag(FLAG_VECT);
  clearSystemFlag(FLAG_NVECT);
  clearSystemFlag(FLAG_SCALE);
  setSystemFlag(FLAG_PLINE);
  setSystemFlag(FLAG_PBOX);
  clearSystemFlag(FLAG_PCURVE);
  clearSystemFlag(FLAG_PCROS);
  clearSystemFlag(FLAG_PPLUS);

  real34Zero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_UY));
  real34Zero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_LY));

  PLOT_INTG     = false;
  PLOT_DIFF     = false;
  PLOT_RMS      = false;
  PLOT_SHADE    = false;
  PLOT_ZMY      = 0;
  PLOT_ZOOM     = 0;
  plotmode      = _SCAT;
  tick_int_x    = 0;
  tick_int_y    = 0;
  PLOT_AXIS     = false;

}


void graph_reset(void){
  graphResetCommon();
}


void fnClGrf(uint16_t unusedButMandatoryParameter) {
  graph_reset();
  fnClDrawMx(2);
  strcpy(plotStatMx,"DrwMX");
  fnRefreshState();                //jm
}


void fnPline(uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PLINE);
  if(!getSystemFlag(FLAG_PLINE) && !getSystemFlag(FLAG_PCROS) && !getSystemFlag(FLAG_PBOX) && !getSystemFlag(FLAG_PPLUS)) {
    setSystemFlag(FLAG_PBOX);
  }
  if(!getSystemFlag(FLAG_PLINE)) {
    clearSystemFlag(FLAG_PCURVE);
  }
  fnRefreshState();                //jm
  fnPlotSQ(0);
}


void fnPcros(uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PCROS);
  if(getSystemFlag(FLAG_PCROS)) {
    clearSystemFlag(FLAG_PBOX);
    clearSystemFlag(FLAG_PPLUS);
  }
  if(!getSystemFlag(FLAG_PCROS) && !getSystemFlag(FLAG_PBOX) && !getSystemFlag(FLAG_PPLUS)) {
    setSystemFlag(FLAG_PLINE);
  }
  fnRefreshState();                //jm
  fnPlotSQ(0);
}

void fnPplus(uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PPLUS);
  if(getSystemFlag(FLAG_PPLUS)) {
    clearSystemFlag(FLAG_PBOX);
    clearSystemFlag(FLAG_PCROS);
  }
  if(!getSystemFlag(FLAG_PCROS) && !getSystemFlag(FLAG_PBOX) && !getSystemFlag(FLAG_PPLUS)) {
    setSystemFlag(FLAG_PLINE);
  }
  fnRefreshState();                //jm
  fnPlotSQ(0);
}


void fnPbox (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PBOX);
  if(getSystemFlag(FLAG_PBOX)) {
    clearSystemFlag(FLAG_PCROS);
    clearSystemFlag(FLAG_PPLUS);
  }
  if(!getSystemFlag(FLAG_PCROS) && !getSystemFlag(FLAG_PBOX) && !getSystemFlag(FLAG_PPLUS)) {
    setSystemFlag(FLAG_PLINE);
  }
  fnRefreshState();                //jm
  fnPlotSQ(0);
}

void fnPcurve (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_PCURVE);
  if(getSystemFlag(FLAG_PCURVE)) {
    setSystemFlag(FLAG_PLINE);
  }
  fnRefreshState();                //jm
  fnPlotSQ(0);
}


void fnPintg (uint16_t unusedButMandatoryParameter) {
  PLOT_INTG = !PLOT_INTG;
  if(!PLOT_INTG) {
    PLOT_SHADE = false;
  }
  clearSystemFlag(FLAG_VECT);
  clearSystemFlag(FLAG_NVECT);
  fnRefreshState();                //jm
  fnPlotSQ(0);
}


void fnPdiff (uint16_t unusedButMandatoryParameter) {
  PLOT_DIFF  = !PLOT_DIFF;
  clearSystemFlag(FLAG_VECT);
  clearSystemFlag(FLAG_NVECT);
  fnRefreshState();                //jm
  fnPlotSQ(0);
}


void fnPrms (uint16_t unusedButMandatoryParameter) {
  PLOT_RMS   = !PLOT_RMS;
  clearSystemFlag(FLAG_VECT);
  clearSystemFlag(FLAG_NVECT);
  fnRefreshState();                //jm
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
        //real34Zero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_UY));
        //real34Zero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_LY));
        PLOT_AXIS = true;
        int8_t increment = param == 2 ? +1 : param == 1 ? -1 : 0;
        PLOT_ZMY += increment;
        if(PLOT_ZMY == zoomOverride-1 || PLOT_ZMY == zoomOverride+1) {
          PLOT_ZMY = 0;
        } else
        if(PLOT_ZMY > zoomOverride+1) {
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





static void calculateZoomFactor(float factor, float *aa) {
  #define basefactor 4.5f
  if(factor != 0) {
    (*aa) *= pow(basefactor,-factor);
  }
}


static void multiplyZoomFactors(float plotzoomx, float plotzoomy, float histofactor, float *x_min, float *x_max, float *y_min, float *y_max, float *dx, float *dy) {
    *x_min = *x_min - *dx * zoomfactor;
    *y_min = *y_min - *dy * zoomfactor;
    *x_max = *x_max + *dx * zoomfactor;
    *y_max = *y_max + *dy * zoomfactor;
    *dx = *x_max - *x_min;
    *dy = *y_max - *y_min;
    float xavg = (*x_max + *x_min)/2;
    float yavg = (*y_max + *y_min)/2;
    *y_min = yavg - *dy/2 * plotzoomy * histofactor;
    *y_max = yavg + *dy/2 * plotzoomy * histofactor;
    *x_min = xavg - *dx/2 * plotzoomx * histofactor;
    *x_max = xavg + *dx/2 * plotzoomx * histofactor;
}


void fnPvect (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_VECT);
  if(getSystemFlag(FLAG_VECT)) {
    clearSystemFlag(FLAG_NVECT);
  }
  PLOT_INTG    = false;
  PLOT_DIFF    = false;
  PLOT_RMS     = false;
  PLOT_SHADE   = false;
  fnRefreshState();                //jm
  fnPlotSQ(0);
}


void fnPNvect (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_NVECT);
  if(getSystemFlag(FLAG_NVECT)) {
    clearSystemFlag(FLAG_VECT);
  }
  PLOT_INTG   = false;
  PLOT_DIFF   = false;
  PLOT_RMS    = false;
  PLOT_SHADE  = false;
  fnRefreshState();                //jm
  fnPlotSQ(0);
}


void fnScale (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_SCALE);
  fnRefreshState();                //jm
  fnPlotSQ(0);
}


void fnPshade (uint16_t unusedButMandatoryParameter) {
  PLOT_SHADE = !PLOT_SHADE;
  if(PLOT_SHADE) {
    PLOT_INTG = true;
  }
  clearSystemFlag(FLAG_VECT);
  clearSystemFlag(FLAG_NVECT);
  fnRefreshState();                //jm
  fnPlotSQ(0);
}


void fnComplexPlot (uint16_t unusedButMandatoryParameter) {
  flipSystemFlag(FLAG_CPXPLOT);
  fnRefreshState();                //jm
  fnEqSolvGraph(EQ_PLOT_LU);
  fnPlotSQ(0);
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


void fnPlotSQ(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    #if defined(DMCP_BUILD)
      lcd_refresh();
    #else // !DMCP_BUILD
      refreshLcd(NULL);
    #endif // DMCP_BUILD

    PLOT_AXIS = true;

    if(GRAPHMODE) {
      previousCalcMode = CM_NORMAL;
    } else {
      previousCalcMode = calcMode;
      clearScreenOld(clrStatusBar, !clrRegisterLines, !clrSoftkeys); //Change over hourglass to the left side
    }

    calcMode = CM_GRAPH;
    hourGlassIconEnabled = true;       //clear the current portion of statusbar
    showHideHourGlass();
    refreshStatusBar();

    if(menu(0) != -MNU_PLOT_FUNC && plotStatMx[0] == 'D') {
      showSoftmenu(-MNU_PLOT_FUNC);
    }
    else if(menu(0) != -MNU_PLOT_STAT && plotStatMx[0] == 'S') {
      showSoftmenu(-MNU_PLOT_STAT);
    }
  #endif // !TESTSUITE_BUILD
}


void fnListXY(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
  if((plotStatMx[0]=='D' ? (drawMxN() >= 1) : false)) {
    calcMode = CM_LISTXY; //Used to view graph/listing
    ListXYposition = 0;
    }
  #endif // !TESTSUITE_BUILD
}


//added this, to add a new command to plot advanced from the STATS
#if !defined(TESTSUITE_BUILD)
  void fnPlotStatAdv(uint16_t unusedButMandatoryParameter) {
    lastPlotMode = PLOT_NOTHING;
    strcpy(plotStatMx, "STATS");
    setSystemFlag(FLAG_PLINE);
    PLOT_SHADE = true;
    fnPlotSQ(0);
  }
#endif // !TESTSUITE_BUILD


#if !defined(TESTSUITE_BUILD)
  static void plotarrow(int16_t xo, int16_t yo, int16_t xn, int16_t yn) {              // Plots line from xo,yo to xn,yn; uses temporary x1,y1
    float dx, dy, ddx, dydx, zz, zzz;
    dydx = yn-yo;
    ddx = xn-xo;
    zz  = sqrt(dydx*dydx + ddx*ddx);
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
      placePixel(xn,yn);
    }
  }


    typedef struct {              //JM VALUES DEMO
      int8_t valid;
      int8_t  xd1;
      int8_t  yd1;
      int8_t  xd2;
      int8_t  yd2;
    } plotdeltas;

    TO_QSPI const plotdeltas tabDeltaBig[] = {
      {1,+0,-2,+5,+6},
      {1,+5,+6,-5,+6},
      {1,-5,+6,+0,-2},
      {0,0,0,0,0},
    };
  static void plotdeltabig(int16_t xn, int16_t yn) {              // Plots ldifferential sign; uses temporary x1,y1
    int8_t ii=0;
    while(tabDeltaBig[ii].valid == 1) {
      plotline1(xn+tabDeltaBig[ii].xd1, yn+tabDeltaBig[ii].yd1, xn+tabDeltaBig[ii].xd2, yn+tabDeltaBig[ii].yd2);
      ii++;
    }
  }


    TO_QSPI const plotdeltas tabDelta[] = {
      {1,+0,-2,0,0},
      {1,-1,-1,0,0},
      {1,-1,+0,0,0},
      {1,-2,+1,0,0},
      {1,-2,+2,0,0},
      {1,+1,-1,0,0},
      {1,+1,-0,0,0},
      {1,+2,+1,0,0},
      {1,+2,+2,0,0},
      {1,-1,+2,0,0},
      {1,+0,+2,0,0},
      {1,+1,+2,0,0},
      {0,0,0,0,0},
    };
  static void plotdelta(int16_t xn, int16_t yn) {             // Plots ldifferential sign; uses temporary x1,y1
    int8_t ii=0;
    while(tabDelta[ii].valid == 1) {
      placePixel(xn+tabDelta[ii].xd1, yn+tabDelta[ii].yd1);
      ii++;
    }
  }


    TO_QSPI const plotdeltas tabDeltaIntBig[] = {
      {1,-0,-2+0,+3,-2+0},
      {1,-0,-2+1,+3,-2+1},
      {1,-3,-2+8,+0,-2+8},
      {1,-3,-2+9,+0,-2+9},
      {1,+0,-2+7,+0,-2+0},
      {1,+1,-2+7,+1,-2+0},
      {0,0,0,0,0},
    };
  static void plotintbig(int16_t xn, int16_t yn) {            // Plots integral sign; uses temporary x1,y1
    int8_t ii=0;
    while(tabDeltaIntBig[ii].valid == 1) {
      plotline1(xn+tabDeltaIntBig[ii].xd1, yn+tabDeltaIntBig[ii].yd1, xn+tabDeltaIntBig[ii].xd2, yn+tabDeltaIntBig[ii].yd2);
      ii++;
    }
  }


    TO_QSPI const plotdeltas tabDeltaInt[] = {
      {1,+0,+0,0,0},
      {1,+0,-1,0,0},
      {1,+0,-2,0,0},
      {1,+0,+1,0,0},
      {1,+0,+2,0,0},
      {1,+1,-2,0,0},
      {1,-1,+2,0,0},
      {0,0,0,0,0},
    };
  static void plotint(int16_t xn, int16_t yn) {               // Plots integral sign; uses temporary x1,y1
    int8_t ii=0;
    while(tabDeltaInt[ii].valid == 1) {
      placePixel(xn+tabDeltaInt[ii].xd1, yn+tabDeltaInt[ii].yd1);
      ii++;
    }
  }


    TO_QSPI const plotdeltas tabDeltaRms[] = {
      {1,+1,-1,0,0},
      {1,-1,-1,0,0},
      {1,-0,-1,0,0},
      {1,+1,+0,0,0},
      {1,-1,+0,0,0},
      {1,-0,+0,0,0},
      {1,+1,+1,0,0},
      {1,-1,+1,0,0},
      {1,-0,+1,0,0},
      {0,0,0,0,0},
    };
  static void plotrms(int16_t xn, int16_t yn) {               // Plots line from xo,yo to xn,yn; uses temporary x1,y1
    int8_t ii=0;
    while(tabDeltaRms[ii].valid == 1) {
      placePixel(xn+tabDeltaRms[ii].xd1, yn+tabDeltaRms[ii].yd1);
      ii++;
    }
  }

#endif  // !TESTSUITE_BUILD


//###################################################################################

//PLSTAT; EQN Graph;

#define bufLen 40


#if !defined(TESTSUITE_BUILD)
  static void showGraphTickText1(float tick_int_x, float tick_int_y, int32_t xoff, int32_t yoff1, int32_t yoff2, uint16_t acc) {
    char buff[32];
    char outstr[bufLen];
    char tmpBuf[100];
    snprintf(tmpString, bufLen, "  y %8s/tick  ", radixProcess(buff,formatCore(tick_int_y,acc,false,tmpBuf,50)));
    convertDigits(smallE(buff,tmpString), outstr);
    showString(outstr, &standardFont, xoff, yoff1, vmNormal, true, true);

    snprintf(tmpString, bufLen, "  x %8s/tick  ", radixProcess(buff,formatCore(tick_int_x,acc,false,tmpBuf,50)));
    convertDigits(smallE(buff,tmpString), outstr);
    showString(outstr, &standardFont, xoff, yoff2, vmNormal, true, true);
  }
#endif // !TESTSUITE_BUILD


void graph_text(void) {
  #if !defined(TESTSUITE_BUILD)
    uint32_t ypos = Y_POSITION_OF_REGISTER_T_LINE -11 + 12 * 5 -45;
    int16_t ii;
    char ss[100], tt[100];
    char tmpbuf[PLOT_TMP_BUF_SIZE];
    int32_t n;
    grphNumFormatter(ss, "(", x_max, 2, "");
    uint16_t ssw = showStringEnhanced(padEquals(tmpbuf, ss), &standardFont, 0, 0,vmNormal, false, false, NO_compress, NO_raise, NO_Show, NO_Bold, NO_LF);
    grphNumFormatter(tt, radixProcess(tmpbuf, "#"), y_max, 2, ")");
    uint16_t ttw = showStringEnhanced(padEquals(tmpbuf, tt), &standardFont, 0, 0,vmNormal, false, false, NO_compress, NO_raise, NO_Show, NO_Bold, NO_LF);
    ypos += 38;
    n = showString(padEquals(tmpbuf, ss), &standardFont, 160-3-2-ssw-ttw, ypos, vmNormal, false, false);
    showString(padEquals(tmpbuf, tt), &standardFont, n+3, ypos, vmNormal, false, false);
    grphNumFormatter(ss, "(", x_min, 2, "");
    ypos += 19;
    n = showString(padEquals(tmpbuf, ss), &standardFont,1, ypos, vmNormal, false, false);
    grphNumFormatter(ss, radixProcess(tmpbuf, "#"), y_min, 2, ")");
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
      case 0: strcpy(tmpString,"            ");                    break;
      case 1: snprintf(tmpString, bufLen, "  y-axis x 0"); break;
      case 2: snprintf(tmpString, bufLen, "  x-axis y 0"); break;
      case 3: snprintf(tmpString, bufLen, "  axis 0%s0 ", radixProcess(tmpbuf,"."));  break;
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

    if(PLOT_INTG && !invalid_intg) {
      snprintf(tmpString, bufLen, "  Trapezoid integral");
      showStringEnhanced(tmpString, &tinyFont, 1, ypos, vmNormal, false, false, NO_compress, NO_raise, DO_Show, DO_Bold, DO_LF);

      plotintbig(5, ypos+4+4-2-4);
      plotrect(5+4-1, (ypos+4+4-2+2)-1-4, 5+4+2, (ypos+4+4-2+2)+2-4);
      ypos += 20;
    }

    if(PLOT_DIFF && !invalid_diff) {
      snprintf(tmpString, bufLen, "  Numerical slope");
      showStringEnhanced(tmpString, &tinyFont, 1, ypos, vmNormal, false, false, NO_compress, NO_raise, DO_Show, DO_Bold, DO_LF);
      plotdeltabig(6, ypos+4+4-2-4);
      ypos += 20;
    }

    if(PLOT_RMS && !invalid_rms) {
      snprintf(tmpString, bufLen, "  Root Mean Square RMS");
      showStringEnhanced(tmpString, &tinyFont, 1, ypos, vmNormal, false, false, NO_compress, NO_raise, DO_Show, DO_Bold, DO_LF);
      plotrms(6, ypos+4+4-2-3);
      plotrect(6-1, (ypos+4+4-2)-1-3, 6+2, (ypos+4+4-2)+2-3);
      ypos += 20;
    }

    force_refresh(timed);
  #endif // !TESTSUITE_BUILD
}



//####################################################
//######### PLOT MEM #################################
//####################################################


void graph_Include0(bool_t mode, uint16_t statnum) {
  //using global: FLAG_SHOWX, x_min, x_max, FLAG_SHOWY, y_min, y_max, FLAG_SCALE, PLOT_ZMY, zoomfactor

  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("PLOT_ZMY=%i  FLAG_SCALE=%i mode=%i\n", PLOT_ZMY, getSystemFlag(FLAG_SCALE), mode);
    printf("Axis1b: x: %f -> %f y: %f -> %f   \n",x_min, x_max, y_min, y_max);
  #endif // STATDEBUG


  //Check and correct if min and max is swapped
  if(x_min>0.0f && x_min > x_max) {
    x_min = x_min - (-x_max+x_min)* 1.1f;
  }
  if(x_min<0.0f && x_min > x_max) {
    x_min = x_min + (-x_max+x_min)* 1.1f;
  }


  //include the 0 axis
  if(getSystemFlag(FLAG_SHOWX)) {
    if(x_min > 0.0f && x_max > 0.0f) {
      if(x_min <= x_max) {
        x_min = -0.05f * x_max;
      }
      else {
        x_min = 0.0f;
      }
    }
    if(x_min < 0.0f && x_max < 0.0f) {
      if(x_min >= x_max) {
        x_min = -0.05f * x_max;
      }
      else {
        x_max = 0.0f;
      }
    }
  }
  if(getSystemFlag(FLAG_SHOWY)) {
    if(y_min > 0.0f && y_max > 0.0f) {
      if(y_min <= y_max) {
        y_min = -0.05f * y_max;
      }
      else {
        y_min = 0.0f;
      }
    }
    if(y_min < 0.0f && y_max < 0.0f) {
      if(y_min >= y_max) {
        y_min = -0.05f * y_max;
      }
      else {
        y_max = 0.0f;
      }
    }
  }

  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("x_min=%f,y_min=%f,x_max=%f,y_max=%f\n", x_min,y_min,x_max,y_max);
    printf("Axis2: x: %f -> %f y: %f -> %f   \n",x_min, x_max, y_min, y_max);
  #endif // STATDEBUG

  //modify the draw range if the min == max
  float dx = x_max-x_min;
  float dy = y_max-y_min;
  if(dy == 0.0f) {
    dy = 1.0f;
    y_max = y_min + dy/2.0f;
    y_min = y_max - dy;
    dy = y_max-y_min;
  }
  if(dx == 0.0f) {
    dx = 1.0f;
    x_max = x_min + dx/2.0f;
    x_min = x_max - dx;
    dx = x_max-x_min;
  }

  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("x_min=%f,y_min=%f,x_max=%f,y_max=%f, dx=%f, dy=%f, \n", x_min,y_min,x_max,y_max, dx, dy);
    printf("Axis3a: x: %f -> %f y: %f -> %f   \n",x_min, x_max, y_min, y_max);
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
    float histofactor = drawHistogram == 0 ? 1 : 1/zoomfactor * (((float)statnum + 2.0f)  /  ((float)(statnum) - 1.0f) - 1)/2;     //Create space on the sides of the graph for the wider histogram columns
    calculateZoomFactor(PLOT_ZOOM * 0.75, &plotzoomx);
    plotzoomy = drawHistogram == 1 ? 1 : plotzoomx;
    multiplyZoomFactors(plotzoomx, plotzoomy, histofactor, &x_min, &x_max, &y_min, &y_max, &dx, &dy);
    if(drawHistogram == 1) {
      y_min = 0;
    }
  }
  else { //mode != PLOTSTAT
    if(PLOT_ZMY != zoomOverride) {
      if(PLOT_ZMY == zoomOverride-1 || PLOT_ZMY == zoomOverride+1) {
        PLOT_ZMY = 0;
      } else if(PLOT_ZMY > zoomOverride+1) {
        PLOT_ZMY = zoomRangeLo;
      }
      else if(PLOT_ZMY < zoomRangeLo) {
        PLOT_ZMY = zoomRangeHi;
      }
      calculateZoomFactor(PLOT_ZMY * 0.55, &plotzoomy);
      //use this line if the x-display-range is to be the same as the y-display-range
      //plotzoomx = plotStatMx[0]=='D' ? 1 : plotzoomy;
      multiplyZoomFactors(plotzoomx, plotzoomy, 1/*histofactor*/, &x_min, &x_max, &y_min, &y_max, &dx, &dy);
      //printf("PLOT_ZMY=%i plotzoomx=%f, plotzoomy=%f\n",PLOT_ZMY, plotzoomx, plotzoomy);
    }
    else {

      //PLOT_ZMY = 18, special case to allow Ylo Yhi
      //_LY _UY override only if ZOOM is not set, AND Yup and Ylo are not zero
      if(fabs(plotzoomx-1) < 0.00001 && fabs(plotzoomy-1) < 0.00001 && !(real34IsZero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_LY)) && real34IsZero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_UY)))) {
        y_min = convertRegisterToDouble(RESERVED_VARIABLE_LY);
        y_max = convertRegisterToDouble(RESERVED_VARIABLE_UY);
      } else {
        y_min = -10;
        y_max = 10;
      }

    }
  }



  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("x_min=%f,y_min=%f,x_max=%f,y_max=%f, dx=%f, dy=%f \n", x_min,y_min,x_max,y_max, dx, dy);
    printf("Axis3b: x: %f -> %f y: %f -> %f   \n",x_min, x_max, y_min, y_max);
  #endif // STATDEBUG



  //Cause scales to be the same
  if(getSystemFlag(FLAG_SCALE)) {
    // if y >> x, then y simply takes on the X range and can be increased using ZMY
    if(mode == PLOTSTAT) {
      x_min = min(x_min,y_min);
      x_max = max(x_max,y_max);
      y_min = x_min;
      y_max = x_max;
    }
    else {  //new equal scale calculation to keep the grpah centre of screen
      float dx = fabs(x_max - x_min);
      float dy = fabs(y_max - y_min);
      //printf("dx=%f dy=%f\n",dx,dy);
      if(dx > 1e-10 && dy/dx > 100000) {
        y_min = x_min;
        y_max = x_max;
        dx = fabs(x_max - x_min);
        dy = fabs(y_max - y_min);
      }
      else {
        if(dx > dy) {
          dy = dx;
        } else {
          dx = dy;
        }
      }
      x_min = (x_min + x_max)/2 - dx/2;
      x_max = x_min + dx;
      y_min = (y_min + y_max)/2 - dy/2;
      y_max = y_min + dy;
    }
  }

  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("x_min=%f,y_min=%f,x_max=%f,y_max=%f, dx=%f, dy=%f \n", x_min,y_min,x_max,y_max, dx, dy);
    printf("Axis3c: x: %f -> %f y: %f -> %f   \n",x_min, x_max, y_min, y_max);
  #endif // STATDEBUG


}





void graph_plotmem(void) {
  currentKeyCode = 255;
  #if !defined(SAVE_SPACE_DM42_13GRF_JM)
    #if !defined(TESTSUITE_BUILD)
      #if defined(STATDEBUG) && defined(PC_BUILD)
        uint16_t i;
        int16_t cnt1;
        cnt1 = drawMxN();
        printf("Stored values n=%i of matrix:%s\n",cnt1, plotStatMx);
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

      #if defined (LOW_GRAPH_ACC)
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
      float y;
      float sx, sy;
      float ddx = FLoatingMax;
      float dxx = FLoatingMax;
      float dydx = FLoatingMax;
      float inty = 0;
      float inty_off = 0;
      float rmsy = 0;

      statnum = 0;

      if((plotStatMx[0]=='S' ? statMxN() >= 2 : false) || (plotStatMx[0]=='D' ? drawMxN() >= 2:false)) {
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

        if(PLOT_INTG) {
          rmsy = fabs(grf_y(0));
          for(ix = 0; (ix < statnum); ++ix) {
            rmsy = sqrt((rmsy * rmsy * ix + grf_y(ix) * grf_y(ix)) / (ix+1.0));      // Changed rmsy to use the standard RMS calc, and not shoft it to the trapezium x-centre
          }
        inty_off = rmsy;
        }

        //AUTOSCALE
        x_min = FLoatingMax;
        x_max = FLoatingMin;
        y_min = FLoatingMax;
        y_max = FLoatingMin;
        #if defined(STATDEBUG)
          printf("Axis0: x: %f -> %f y: %f -> %f   \n", x_min, x_max, y_min, y_max);
        #endif // STATDEBUG
        if(plotmode != _VECT) {
          invalid_intg = false;                                                      //integral scale
          invalid_diff = false;                                                      //Differential dydx scale
          invalid_rms  = false;                                                      //RMSy

//#################################################### vvv SCALING LOOP DIFF INTG RMS vvv #########################
/**/      if(PLOT_DIFF || PLOT_INTG || PLOT_RMS) {
/**/        inty = inty_off;                                                          //  integral starting constant co-incides with graph
/**/        if(PLOT_RMS) {
/**/          rmsy = fabs(grf_y(0));
/**/        }
/**/
/**/        for(ix = 0; (ix < statnum); ++ix) {
/**/          if(ix != 0) {
/**/            ddx = grf_x(ix) - grf_x(ix-1);                                            //used in DIFF and INT
/**/            if(ddx<=0) {                                                              //Cannot get slop or area if x is not growing in positive dierection
/**/              x_min = FLoatingMax;
/**/              x_max = FLoatingMin;
/**/              y_min = FLoatingMax;
/**/              y_max = FLoatingMin;
/**/              invalid_diff = true;
/**/              invalid_intg = true;
/**/              invalid_rms  = true;
/**/              break;
/**/            }
/**/            else {
/**/              if(grf_x(ix) < x_min) {
/**/                x_min = grf_x(ix);
/**/              }
/**/              if(grf_x(ix) > x_max) {
/**/                x_max = grf_x(ix);
/**/              }
/**/              if(PLOT_DIFF) {
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
/**/                if(dydx < y_min) {
/**/                  y_min = dydx;
/**/                }
/**/                if(dydx > y_max) {
/**/                  y_max = dydx;
/**/                }
/**/              }
/**/              if(PLOT_INTG) {
/**/                inty = inty + (grf_y(ix) + grf_y(ix-1)) / 2 * ddx;
/**/                if(inty < y_min) {
/**/                  y_min = inty;
/**/                }
/**/                if(inty > y_max) {
/**/                  y_max = inty;
/**/                }
/**/              }
/**/              if(PLOT_RMS) {
/**/                rmsy = sqrt((rmsy * rmsy * ix + grf_y(ix) * grf_y(ix)) / (ix+1.0));      // Changed rmsy to use the standard RMS calc, and not shoft it to the trapezium x-centre
/**/                if(rmsy < y_min) {
/**/                  y_min = rmsy;
/**/                }
/**/                if(rmsy > y_max) {
/**/                  y_max = rmsy;
/**/                }
/**/              }
/**/            }
/**/          }
/**/          if(exitKeyWaiting()) {
/**/             return;
/**/          }
/**/        }
/**/      }
//#################################################### ^^^ SCALING LOOP ^^^ #########################

          #if defined(STATDEBUG)
            printf("Axis0b1: x: %f -> %f y: %f -> %f  %d \n", x_min, x_max, y_min, y_max, invalid_diff);
          #endif // STATDEBUG

//#################################################### vvv SCALING LOOP  vvv #########################
/**/      uint16_t y_maxcnt=2;
/**/      uint16_t y_mincnt=2;
/**/      float a0,a1,a2,a3,a4,a5,a6,a7,a8;   //Digital filter to get rid of short sharp peak like some asymptotes
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
/**/      if(getSystemFlag(FLAG_PBOX) || getSystemFlag(FLAG_PLINE) || getSystemFlag(FLAG_PCROS) || getSystemFlag(FLAG_PPLUS) || !(PLOT_DIFF || PLOT_INTG)) {  //XXXX
/**/
/**/        //pre-loop to cover trivial cases of symmetrical axis
/**/        for(cnt=0; (cnt < statnum); cnt++) {
/**/          #if defined(STATDEBUG)
/**/            printf("Axis0a: cnt/statnum: %i/%i  x: %f y: %f   \n", cnt, statnum, grf_x(cnt), grf_y(cnt));
/**/          #endif // STATDEBUG
/**/          if(grf_x(cnt) < x_min) {
/**/            x_min = grf_x(cnt);
/**/          }
/**/          if(grf_x(cnt) > x_max) {
/**/            x_max = grf_x(cnt);
/**/          }
/**/          if(grf_y(cnt) < y_min) {
/**/            y_min = grf_y(cnt);
/**/          }
/**/          if(grf_y(cnt) > y_max) {
/**/            y_max = grf_y(cnt);
/**/          }
/**/          scaleRmsy = sqrt((scaleRmsy * scaleRmsy * cnt + grf_y(cnt) * grf_y(cnt)) / (cnt+1.0));
/**/        }
/**/
/**/        //pre-loop to cover trivial quasi symmetrical axis
/**/        if(y_max > 0 && y_min < 0 && (y_max > 4 * scaleRmsy)) {y_max = scaleRmsy;} else                      //force the RMS if large peaks occur
/**/        if(y_max > 0 && y_min < 0 && (-y_min > 4 * scaleRmsy)) {y_min = -scaleRmsy;} else
/**/        if(y_max > 0 && y_min < 0 && (y_max > -y_min) && (y_max / y_min < 1.2)) { y_min = -y_max; } else     //make x-axis sit in the middle if close enough
/**/        if(y_max > 0 && y_min < 0 && (y_max < -y_min) && (y_min / y_max < 1.2)) { y_max = -y_min; } else
/**/
/**/
/**/         {
/**/          for(cnt=0; (cnt < statnum); cnt++) {
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
/**/            if(aa < y_min) {
/**/              y_mincnt++;
/**/              if(fabs(aa / y_min) < 4 ) {//|| aa == a0 * 1.1) {
/**/                if(aa < y_min) {
/**/                 y_min = aa;
/**/                }
/**/                y_mincnt=0;
/**/              }
/**/              else if(y_mincnt==3) {
/**/                y_min = aa;
/**/                y_mincnt=0;
/**/              }
/**/            }
/**/            else {
/**/             y_mincnt=0;
/**/            }
/**/
/**/            if(aa > y_max) {
/**/              y_maxcnt++;
/**/              if(fabs(aa / y_max) < 4 ) {//|| aa == a0 * 1.1) {
/**/                if(aa>y_max) {
/**/                  y_max = aa;
/**/                }
/**/                y_maxcnt=0;
/**/              }
/**/              else if(y_maxcnt==3) {
/**/                y_max = aa;
/**/                y_maxcnt=0;
/**/              }
/**/            }
/**/            else {
/**/              y_maxcnt=0;
/**/            }
/**/
/**/            #if defined(STATDEBUG)
/**/              printf("Axis0b: x: %f -> %f y: %f -> %f   \n", x_min, x_max, y_min, y_max);
/**/            #endif // STATDEBUG
/**/            if(exitKeyWaiting()) {
/**/              return;
/**/            }
/**/          }
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
/**/        if(sx < x_min) {
/**/          x_min = sx;
/**/        }
/**/        if(sx > x_max) {
/**/          x_max = sx;
/**/        }
/**/        if(sy < y_min) {
/**/          y_min = sy;
/**/        }
/**/        if(sy > y_max) {
/**/          y_max = sy;
/**/        }
/**/        if(exitKeyWaiting()) {
/**/          return;
/**/        }
/**/      }
/**/    }
//#################################################### ^^^ SCALING LOOP ^^^ #########################


        //Manipulate the obtained axes positions
        #if defined(STATDEBUG)
          printf("Axis1a: x: %f -> %f y: %f -> %f   \n",x_min, x_max, y_min, y_max);
        #endif // STATDEBUG


        graph_Include0(!PLOTSTAT, 0);


        roundedTicks = true;
        graph_axis();
        if(PLOT_AXIS) {
          graph_text();
        }

        #if defined(STATDEBUG)
          printf("Axis3b: x: %f -> %f y: %f -> %f   \n",x_min, x_max, y_min, y_max);
        #endif // STATDEBUG


        if(plotmode != _VECT) {
          yn = screen_window_y(y_min,grf_y(0),y_max);
          xn = screen_window_x(x_min,grf_x(0),x_max);
          xN1 = xn;
          yN1 = yn;
        }
        else {
          yn = screen_window_y(y_min,0,y_max);
          xn = screen_window_x(x_min,0,x_max);
          xN1 = xn;
          yN1 = yn;
        }

        #if defined(STATDEBUG)
          printf("Axis3c: x: %f -> %f y: %f -> %f   \n",x_min, x_max, y_min, y_max);
        #endif // STATDEBUG

        sx = 0;
        sy = 0;
        //GRAPH
        ix = 0;
        inty = inty_off;                                                         //  integral starting constant co-incides with graph
        rmsy = 0;
        if(PLOT_RMS) {
          rmsy = fabs(grf_y(0));
        }

        //#################################################### vvv MAIN GRAPH LOOP vvv #########################
        bool_t plotInCurves = getSystemFlag(FLAG_PCURVE);

        static int16_t prev_y_unclipped = 0;
        if(plotInCurves) {
          plotline3(0,0,0,0,true,false); //reset
        }
        for(ix = 0; (ix < statnum); ++ix) {
          if(plotmode != _VECT) {
            x = 0;
            y = 0;

            if(ix !=0 && ( (PLOT_DIFF && !invalid_diff) || (PLOT_INTG && !invalid_intg) || (PLOT_RMS && !invalid_rms) )) {
              ddx = grf_x(ix) - grf_x(ix-1);
              if(PLOT_DIFF && ddx != 0) {
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

              if(PLOT_RMS)  {
                rmsy = sqrt ( (rmsy * rmsy * ix + grf_y(ix) * grf_y(ix)) / (ix+1.0) );      // Changed rmsy to use the standard RMS calc, and not shoft it to the trapezium x-centre
              }
              if(PLOT_INTG) {
                inty = inty + (grf_y(ix) + grf_y(ix-1)) / 2 * ddx;
              }
            }

            x = grf_x(ix);
            y = grf_y(ix);

          }
          else { //_VECT
            sx = sx + (!getSystemFlag(FLAG_NVECT) ? grf_x(ix) : grf_y(ix));
            sy = sy + (!getSystemFlag(FLAG_NVECT) ? grf_y(ix) : grf_x(ix));
            x = sx;
            y = sy;
          }
          xo = xN1;
          yo = yN1;
          yN0 = prev_y_unclipped;

          xN1 = screen_window_x(x_min,x,x_max);
          yN1 = screen_window_y_nolimit(y_min,y,y_max);
          int16_t current_y_unclipped = yN1;

          if(ix == 0) {
            xo = xN1;
            yo = yN1;
            yN0 = yN1;
            prev_y_unclipped = yN1;  // Initialize for next iteration
            continue;  // Skip clipping for first point
          }

          #if defined(STATDEBUG)
            printf("\n         xN1 = %d : (x_min=%f,x=%f,x_max=%f) ", xN1, x_min,x,x_max);
            printf("yN0 = %d yN1 = %d : (y_min=%f,y=%f,y_max=%f) \n", yN0, yN1, y_min,y,y_max);
            printf("plotting graph table[%d] = x:%f y:%f (dxx:%f dydx:%f) inty:%f xN1:%d yN1:%d ", ix, x, y, dxx, dydx, inty, xN1, yN1);
            printf("   ... x-ddx/2=%d dydx=%d inty=%d\n", screen_window_x(x_min, x-ddx/2, x_max), screen_window_y(y_min, dydx, y_max), screen_window_y(y_min, inty, y_max));
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
            if (!bothOutOfScreen01 && outOfScreen0 && !outOfScreen1) {
              printf("POTENTIAL ENTRY DEBUG: yN0=%d yN1=%d slope=%s from=%s\n", yN0, yN1, yN1 > yN0 ? "POSITIVE" : "NEGATIVE", yN0 >= SCREEN_HEIGHT_GRAPH ? "BOTTOM" : (yN0 < minN_y ? "TOP" : "MIDDLE"));
            }
          #endif // STATDEBUG


            if (!bothOutOfScreen01) {
              // Coming in from bottom - BOTH positive and negative slopes
              if (outOfScreen0 && !outOfScreen1 && yN0 >= SCREEN_HEIGHT_GRAPH) {
                //printf("ENTRY CLIP BOTTOM: yN0=%d yN1=%d xo=%d xN1=%d slope=%s\n", yN0, yN1, xo, xN1, yN1 > yN0 ? "POSITIVE" : "NEGATIVE");
                int16_t dY = abs(SCREEN_HEIGHT_GRAPH - 1 - yN0);
                if (yN1 != yN0) {
                  float dxN = fabs(((float)dY)*((float)(xN1-xo))/((float)(yN1-yN0)));
                  //printf("  -> Calculated dxN=%f, new xo=%f\n", dxN, xo + dxN);
                  xo = xo + dxN;
                  yN0 = SCREEN_HEIGHT_GRAPH - 1;
                  yo = yN0;
                }
              }
              // Coming in from top - BOTH positive and negative slopes
              else if (outOfScreen0 && !outOfScreen1 && yN0 < minN_y) {
                //printf("ENTRY CLIP TOP: yN0=%d yN1=%d xo=%d xN1=%d slope=%s\n", yN0, yN1, xo, xN1, yN1 > yN0 ? "POSITIVE" : "NEGATIVE");
                int16_t dY = abs(yN0 - minN_y);
                if (yN1 != yN0) {
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
            if (yN1 == yN0) continue; // Skip horizontal lines
            float dxN = fabs(((float)dY)*((float)(xN1-xo))/((float)(yN1-yN0)));
            xN1 = xo + (int16_t)(dxN + 0.5);
            yN1 = SCREEN_HEIGHT_GRAPH - 1;
          }

          //exceeding the positive y-axis part or the top of the screen, use proportional triangle to determine the part of the line to be plotted to the edge of the plotting area
          else if((yN1 < yN0 && xN1 > xo && yN1 < minN_y && !bothOutOfScreen01 && outOfScreen1 && !outOfScreen0) ||
                  (yN1 > yN0 && xN1 > xo && yN0 < minN_y && !bothOutOfScreen01 && !outOfScreen1 && outOfScreen0)) {
            //printf("EXIT CLIP TOP: yN0=%d yN1=%d\n", yN0, yN1);
            int16_t dY = abs(yN0 - minN_y);
            if (yN1 == yN0) continue; // Skip horizontal lines
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
              if(invalid_diff || invalid_intg || invalid_rms)
                printf("invalid_diff=%d invalid_intg=%d invalid_rms=%d \n", invalid_diff, invalid_intg, invalid_rms);
            #endif // STATDEBUG

            if(plotmode != _VECT) {
              #if defined(STATDEBUG)
                printf("Not _VECT\n");
              #endif // STATDEBUG

              plotPointGeneric(xn, yn, xo, yo,
                                 getSystemFlag(FLAG_PCROS) /*cross*/ ,
                                 false                     /*fatbox*/,
                                 getSystemFlag(FLAG_PBOX)  /*box*/   ,
                                 getSystemFlag(FLAG_PPLUS) /*plus*/  ,
                                 false                     /*line*/   );


              if(PLOT_DIFF && !invalid_diff && ix != 0) {
                #if defined(STATDEBUG)
                  printf("Plotting Delta x=%f dy=%f \n", dxx, dydx);
                #endif // STATDEBUG
                plotdelta(screen_window_x( x_min, dxx, x_max), screen_window_y(y_min, dydx, y_max));
              }


              if(PLOT_RMS && !invalid_rms && ix != 0) {
                #if defined(STATDEBUG)
                  printf("Plotting RMSy x=%f rmsy=%f \n", x - ddx/2, rmsy);
                #endif // STATDEBUG
                plotrms(screen_window_x(x_min, x - ddx/2, x_max), screen_window_y(y_min, rmsy, y_max));
              }


              if(PLOT_INTG && !invalid_intg && ix !=0) {
                #if defined(STATDEBUG)
                  printf("Plotting Integral x=%f intg(x)=%f\n", x-ddx/2, inty);
                #endif // STATDEBUG
                int16_t xN0   = screen_window_x(x_min, grf_x(ix-1), x_max);
                //uint16_t xN1   = screen_window_x(x_min, grf_x(ix), x_max);
                int16_t yNintg= screen_window_y(y_min, inty, y_max);
                int16_t xAvg  = ((xN0+xN1) >> 1);

                if(abs((int16_t)(xN1-xN0)>=6)) {
                  plotint( xAvg, yNintg );
                }
                else {
                  //placePixel(xAvg, yNintg);
                  plotrect(xAvg-1, yNintg-1, xAvg+1, yNintg+1);
                }

                if(abs((int16_t)(xN1-xN0) >= 6)) {
                  plotline1(xN1,    yNintg, xAvg+2, yNintg);
                  plotline1(xAvg-2, yNintg, xN0,    yNintg);
                }
                else if(abs((int16_t)(xN1-xN0) >= 4)) {
                  plotline1(xN1,    yNintg, xAvg+2, yNintg);
                  plotline1(xAvg-2, yNintg, xN0,    yNintg);
                }

                if(PLOT_SHADE) {
                  int16_t yNoff = screen_window_y(y_min, 0, y_max);
                  plotrect(xN0, yN0,   xN1, yN1);
                  plotrect(xN0, yNoff, xN1, yN0);
                  if(abs((int16_t)(xN1-xN0) >= 6)) {
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
              } else {
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
            return;
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
          plotline3(0,0,0,0,false,true); //last line segment
        }

      }
      else {
        displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "There is no statistical data available!");
          moreInfoOnError("In function graph_plotmem:", errorMessage, NULL, NULL);
        #endif // EXTRA_INFO_ON_CALC_ERROR == 1
      }

      #if defined (LOW_GRAPH_ACC)
        //Change to normal operation for graphs;
        ctxtReal34.digits = 34;
        ctxtReal39.digits = 39;
        ctxtReal51.digits = 51;
        ctxtReal75.digits = 75;
      #endif //LOW_GRAPH_ACC
    #endif // !TESTSUITE_BUILD
  #endif // !SAVE_SPACE_DM42_13GRF_JM
}


//-----------------------------------------------------//-----------------------------------------------------
void fnStatList() {
  #if !defined(TESTSUITE_BUILD)
    char tmpstr1[100], tmpstr2[100];
    int16_t ix, ixx, statnum;

    clearScreen(1);
    refreshStatusBar();

    if(regStatsXY != INVALID_VARIABLE && (plotStatMx[0]=='D' ? drawMxN() >= 1 : false)) {
      statnum = drawMxN();
      fnStatSum(0);
      sprintf(tmpString, "Graph data: N = %d", statnum);
      print_linestr(tmpString, true);

                                  #if defined(STATDEBUG)
                                    printf("Stat data %d - %d (%s)\n",statnum-1, max(0, statnum-1-6), tmpString );
                                  #endif // STATDEBUG

      if(ListXYposition > 0) {
        ListXYposition = 0;
      }
      else if(statnum - (min(10,statnum)-1) - 1 + ListXYposition < 0) {
        ListXYposition = - (statnum - (min(10,statnum)-1) - 1);
      }

      for(ix = 0; (ix < min(10,statnum)); ++ix) {
        ixx = statnum - ix - 1 + ListXYposition;
        char tmpBuf[100];

          sprintf(tmpstr1,"[%3d] x%4s%14s, ",ixx+1, "", formatCore(grf_x(ixx),10,false,tmpBuf, 150));
          sprintf(tmpstr2,"y%4s%14s, ", "", formatCore(grf_y(ixx),10, false,tmpBuf, 150));
        strcat(tmpstr1,tmpstr2);

        print_numberstr(tmpstr1,false);
        #if defined(STATDEBUG)
          printf("%d:%s\n",ixx,tmpstr1);
        #endif // STATDEBUG
      }
    }
  #endif // !TESTSUITE_BUILD
}

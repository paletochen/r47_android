// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"


void fnPlotRegressionLine(uint16_t plotMode);


#if !defined(TESTSUITE_BUILD)
  static real_t RR,SMI,aa0,aa1,aa2,sa0, sa1; //L.R. variables
  static void drawline(uint16_t selection, real_t *RR, real_t *SMI, real_t *aa0, real_t *aa1, real_t *aa2, real_t *sa0, real_t *sa1);
#endif // !TESTSUITE_BUILD


float     graph_dx;
float     graph_dy;
bool_t    roundedTicks;
bool_t    PLOT_INTG;
bool_t    PLOT_DIFF;
bool_t    PLOT_RMS;
bool_t    PLOT_SHADE;
bool_t    PLOT_AXIS;
int8_t    PLOT_ZOOM;
uint8_t   drawHistogram;

int8_t    plotmode;
float     tick_int_x;
float     tick_int_y;
uint32_t  xzero;
uint32_t  yzero;




void statGraphReset(void){
  graphResetCommon();
  currentKeyCode = 255;
  roundedTicks  = true;
  clearSystemFlag(FLAG_SHOWX);
  clearSystemFlag(FLAG_PLINE);
  y_min         = 0;
  y_max         = 1;
}



#if !defined(TESTSUITE_BUILD)
  float grf_x(int i) {
                                  #ifdef STATDEBUG
                                    char prefix[100];
                                    memcpy(prefix, allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName + 1, allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName[0]);
                                    strcpy(prefix + allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName[0], " :");
                                    printf("X: Reading from Matrix %s\n",prefix);
                                  #endif //STATDEBUG
//    if(keyWaiting()) {
//       return 0;
//    }
    float xf=0;
    real_t xr;

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


  float grf_y(int i) {
                                  #ifdef STATDEBUG
                                    char prefix[100];
                                    memcpy(prefix, allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName + 1, allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName[0]);
                                    strcpy(prefix + allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName[0], " :");
                                    printf("Y: Reading from Matrix %s\n",prefix);
                                  #endif //STATDEBUG
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
      real34ToReal(&stats.matrixElements[i * cols + 1], &yr);
      realToFloat(&yr, &yf);
    }
    else {
      yf = 0;
    }
    return yf;
  }


#define ROUND_F2I(f) ((int)((f) >= 0 ? (f) + 0.5f : (f) - 0.5f))

int16_t screen_window_x(float x_min, float x, float x_max) {
    int16_t temp;
    float tempr;

    tempr = ((x - x_min) / (x_max - x_min) * (float)(SCREEN_HEIGHT_GRAPH - 1));

    if (tempr > 32766) {
        temp = 32767;
    } else if (tempr < -32766) {
        temp = -32767;
    } else {
        temp = (int16_t)ROUND_F2I(tempr);
    }

    if (temp > SCREEN_HEIGHT_GRAPH - 1) {
        temp = SCREEN_HEIGHT_GRAPH - 1;
    } else if (temp < 0) {
        temp = 0;
    }

    return temp + SCREEN_WIDTH - SCREEN_HEIGHT_GRAPH;
}


#define minn 0
int16_t _screen_window_y(float y_min, float y, float y_max, bool_t nolimit) {
    int32_t temp;
    float tempr;

    tempr = ((y - y_min) / (y_max - y_min) * (float)(SCREEN_HEIGHT_GRAPH - 1 - minn));

    if (tempr > 32766) {
        temp = 32767;
    } else if (tempr < -32766) {
        temp = -32767;
    } else {
        temp = (int16_t)ROUND_F2I(tempr);
    }

    if (!nolimit) {
        if (temp > SCREEN_HEIGHT_GRAPH - 1 - minn) {
            temp = SCREEN_HEIGHT_GRAPH - 1 - minn;
        } else if (temp < 0) {
            temp = 0;
        }
    }

    #if defined(PC_BUILD)
    if (SCREEN_HEIGHT_GRAPH - 1 - temp < 0) {
        printf("In function screen_window_y Y NEGATIVE %6d; ", SCREEN_HEIGHT_GRAPH - 1 - temp);
    }
    if (SCREEN_HEIGHT_GRAPH - 1 - temp > 239) {
        printf("In function screen_window_y Y EXCEEDED %6d; ", SCREEN_HEIGHT_GRAPH - 1 - temp);
    }
    #endif

    return (int16_t)(SCREEN_HEIGHT_GRAPH - 1 - temp);
}

  #define nolimit true
  #define limit false

  int16_t screen_window_y_nolimit(float y_min, float y, float y_max) {
    return _screen_window_y(y_min, y, y_max, nolimit);
  }

  int16_t screen_window_y(float y_min, float y, float y_max) {
    return _screen_window_y(y_min, y, y_max, limit);
  }



#endif // !TESTSUITE_BUILD


void placePixel(uint32_t x, uint32_t y) {
  #if !defined(TESTSUITE_BUILD)
  if(x < SCREEN_WIDTH_GRAPH && y < SCREEN_HEIGHT_GRAPH && y >= 1 + minn) {
    setBlackPixel(x, y);
  }
#endif //!TESTSUITE_BUILD
}


void removePixel(uint32_t x, uint32_t y) {
  #if !defined(TESTSUITE_BUILD)
  if(x < SCREEN_WIDTH_GRAPH && y < SCREEN_HEIGHT_GRAPH && y >= 1 + minn) {
    setWhitePixel(x, y);
  }
#endif //!TESTSUITE_BUILD
}


void clearScreenPixels(void) {
  #if !defined(TESTSUITE_BUILD)
    lcd_fill_rect(SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH, 0, SCREEN_HEIGHT_GRAPH, SCREEN_HEIGHT_GRAPH, LCD_SET_VALUE);
    lcd_fill_rect(0, Y_POSITION_OF_REGISTER_T_LINE, SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH, 171-5-Y_POSITION_OF_REGISTER_T_LINE+1, LCD_SET_VALUE);
    lcd_fill_rect(19, 171-5, SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH-19+1, 5, LCD_SET_VALUE);
  #endif //!TESTSUITE_BUILD
}


#if !defined(TESTSUITE_BUILD)
void plotcross(int16_t xn, int16_t yn) {              // Plots cross at xn,yn
  plotline1(max((int16_t)xn-2,0),max((int16_t)yn-2,0),xn+2,yn+2);                       //   PLOT a cross
  plotline1(max((int16_t)xn-2,0),yn+2,xn+2,max((int16_t)yn-2,0));
}


void plotplus(int16_t xn, int16_t yn) {              // Plots plus xn,yn
  plotline1(max((int16_t)xn-3,0),yn,xn+3,yn);          // PLOT a plus
  plotline1(xn,yn+3,xn,max((int16_t)yn-3,0));
}


void plotbox(int16_t xn, int16_t yn) {                // Plots line from xo,yo to xn,yn; uses temporary x1,y1
  plotline1   (max((int16_t)xn-2,0),max((int16_t)yn-2,0),max((int16_t)xn-2,0),max((int16_t)yn-1,0));                       //   PLOT a box
  placePixel  (max((int16_t)xn-1,0),max((int16_t)yn-2,0));
  plotline1   (max((int16_t)xn-2,0),yn+2,max((int16_t)xn-2,0),yn+1);
  placePixel  (max((int16_t)xn-1,0),yn+2);
  plotline1   (xn+2,max((int16_t)yn-2,0),xn+1,max((int16_t)yn-2,0));
  placePixel  (xn+2,max((int16_t)yn-1,0));
  plotline1   (xn+2,yn+2,xn+2,yn+1);
  placePixel  (xn+1,yn+2);
}


void plotrect(int16_t a, int16_t b, int16_t c, int16_t d) {                // Plots rectangle from xo,yo to xn,yn; uses temporary x1,y1
  plotline1(a, b, c, b);
  plotline1(a, b, a, d);
  plotline1(c, d, c, b);
  plotline1(c, d, a, d);
}


#if !defined(SAVE_SPACE_DM42_13GRF)
  static void plotHisto_coln(int16_t x, int16_t y, int16_t y_min, int16_t y_wid, int16_t colw) {  //x is 0..(n-1)
    plotrect(max((int16_t)x - colw,0), y_min + y_wid,  x + colw, y);
    }
#endif //SAVE_SPACE_DM42_13GRF



void plotbox_fat(int16_t xn, int16_t yn) {                                         // Plots line from xo,yo to xn,yn; uses temporary x1,y1
  plotrect(max((int16_t)xn-3,0),max((int16_t)yn-3,0),xn+3,yn+3);
  plotrect(max((int16_t)xn-2,0),max((int16_t)yn-2,0),xn+2,yn+2);
}


void plotline1(int16_t xo, int16_t yo, int16_t xn, int16_t yn) {                   // Plots line from xo,yo to xn,yn; uses temporary x1,y1
   pixelline(xo,yo,xn,yn,1);
}


void plotline2(int16_t xo, int16_t yo, int16_t xn, int16_t yn) {                   // Plots line from xo,yo to xn,yn; uses temporary x1,y1
   //printf("From (%8d,%8d) to (%8d, %8d)\n", xo,yo,xn,yn);
   pixelline(xo,yo,xn,yn,1);
   pixelline(max((int16_t)xo-1,0),yo,max((int16_t)xn-1,0),yn,1);
   pixelline(xo,max((int16_t)yo-1,0),xn,max((int16_t)yn-1,0),1);
   //   pixelline(xo+1,yo,xn+1,yn,1);   //Do not use the full doubling, without it give as nice profile if the slope changes
   //   pixelline(xo,yo+1,xn,yn+1,1);
}



// plotline3 does curve fitting every 2, 3, 4, and 5 points plotted.
//           it needs to be started prior to data using plotline3(0,0,0,0,true,false)
//           it needs to be stopped after the last data plotline3(0,0,0,0,false,true) which will plot the last segment provided there were more than 4 points

#if defined(USECURVES)
static void evalHermite(
    float t,
    float x1, float x2, float tx1, float tx2,
    float y1, float y2, float ty1, float ty2,
    float *ox, float *oy)
{
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
        if(px[rr] < 1 || px[rr] > 398 || py[rr] < 1 || py[rr] > 238) return true;
    }
    return false;
}
#endif //USECURVES


void plotline3(int16_t xo, int16_t yo, int16_t xn, int16_t yn, bool_t first_time, bool_t final_segment) {

  #if !defined(USECURVES)
    plotline2(xo,yo,xn,yn);
  #endif //USECURVES


  #if defined(USECURVES)
    //printf("plotline3: %10d %10d %10d %10d ", xo,yo, xn, yn);
    #define maxSteps 6
    static uint16_t px[5];
    static uint16_t  py[5];
    static int count = 0;
    static int z[5] = {0};

    if (first_time) {
        count = 0;
        memset(z, 0, sizeof(z));
        return;
    }

    if (!final_segment) {
        if (count < 5) {
            px[count] = xn;
            py[count] = yn;

            if (count > 0) {
                int dx = abs((int)px[count] - (int)px[count - 1]);
                int dy = abs((int)py[count] - (int)py[count - 1]);
                z[count] = (int)(sqrtf((float)(dx*dx + dy*dy)) + 0.5f);
            }

            count++;
        } else {
            for (int i = 0; i < 4; i++) {
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

        if (count < 2) return;

        if (count == 2) {
            float prev_x = px[0], prev_y = py[0];
            if(ifAnyMax(px, py, count)) {
                int steps = min(1,max(maxSteps,z[1]));
                for (int i = 1; i <= steps; i++) {
                    float t = (float)i / steps;
                    float sx = (1 - t) * px[0] + t * px[1];
                    float sy = (1 - t) * py[0] + t * py[1];
                    plotline2(ROUND_F2I(prev_x), ROUND_F2I(prev_y), ROUND_F2I(sx), ROUND_F2I(sy));
                    prev_x = sx; prev_y = sy;
                }
            } else {
                    plotline2(ROUND_F2I(px[0]), ROUND_F2I(py[0]), ROUND_F2I(px[1]), ROUND_F2I(py[1]));
            }



        } else if (count == 3) {
            float prev_x = px[0], prev_y = py[0];
            if(!ifAnyMax(px, py, count)) {
                int steps = min(1,max(maxSteps,z[1] + z[2]));
                for (int i = 1; i <= steps; i++) {
                    float t = (float)i / steps;
                    float u = 1 - t;
                    float sx = u*u*px[0] + 2*u*t*px[1] + t*t*px[2];
                    float sy = u*u*py[0] + 2*u*t*py[1] + t*t*py[2];
                    plotline2(ROUND_F2I(prev_x), ROUND_F2I(prev_y), ROUND_F2I(sx), ROUND_F2I(sy));
                    prev_x = sx; prev_y = sy;
                }
            } else {
                    plotline2(ROUND_F2I(px[1]), ROUND_F2I(py[1]), ROUND_F2I(px[2]), ROUND_F2I(py[2]));
            }



        } else if (count == 4) {
            float t1x = (px[2] - px[0]) / 2.0f;
            float t1y = (py[2] - py[0]) / 2.0f;
            float t2x = (px[3] - px[1]) / 2.0f;
            float t2y = (py[3] - py[1]) / 2.0f;

            float prev_x = px[1], prev_y = py[1];
            if(!ifAnyMax(px, py, count)) {
                int steps = min(1,max(maxSteps,z[1] + z[2] + z[3]));
                for (int i = 1; i <= steps; i++) {
                    float sx, sy;
                    evalHermite((float)i / steps,
                                px[1], px[2], t1x, t2x,
                                py[1], py[2], t1y, t2y,
                                &sx, &sy);
                    plotline2(ROUND_F2I(prev_x), ROUND_F2I(prev_y), ROUND_F2I(sx), ROUND_F2I(sy));
                    prev_x = sx; prev_y = sy;
                }
            } else {
                    plotline2(ROUND_F2I(px[2]), ROUND_F2I(py[2]), ROUND_F2I(px[3]), ROUND_F2I(py[3]));
            }


        } else { // count == 5
            {
                float t1x = (px[2] - px[0]) / 2.0f;
                float t1y = (py[2] - py[0]) / 2.0f;
                float t2x = (px[3] - px[1]) / 2.0f;
                float t2y = (py[3] - py[1]) / 2.0f;

                float prev_x = px[1], prev_y = py[1];
                if(!ifAnyMax(px, py, 5)) {
                    int steps = min(1,max(maxSteps,z[1] + z[2]));
                    for (int i = 1; i <= steps; i++) {
                        float sx, sy;
                        evalHermite((float)i / steps, px[1], px[2], t1x, t2x, py[1], py[2], t1y, t2y, &sx, &sy);
                        plotline2(ROUND_F2I(prev_x), ROUND_F2I(prev_y), ROUND_F2I(sx), ROUND_F2I(sy));
                        prev_x = sx; prev_y = sy;
                    }
                } else {
                        plotline2(ROUND_F2I(px[2]), ROUND_F2I(py[2]), ROUND_F2I(px[3]), ROUND_F2I(py[3]));
                }
            }

            {
                float t1x = (px[3] - px[1]) / 2.0f;
                float t1y = (py[3] - py[1]) / 2.0f;
                float t2x = (px[4] - px[2]) / 2.0f;
                float t2y = (py[4] - py[2]) / 2.0f;

                float prev_x = px[2], prev_y = py[2];
                if(!ifAnyMax(px, py, 5)) {
                    int steps = min(1,max(maxSteps,z[2] + z[3] + z[4]));
                    for (int i = 1; i <= steps; i++) {
                        float sx, sy;
                        evalHermite((float)i / steps, px[2], px[3], t1x, t2x, py[2], py[3], t1y, t2y, &sx, &sy);
                        plotline2(ROUND_F2I(prev_x), ROUND_F2I(prev_y), ROUND_F2I(sx), ROUND_F2I(sy));
                        prev_x = sx; prev_y = sy;
                    }
                } else {
                    ;
                }
            }
        }


    } else if (final_segment && count == 5) {
        float t1x = (px[4] - px[2]) / 2.0f;
        float t1y = (py[4] - py[2]) / 2.0f;
        float t2x = (px[4] - px[3]) / 2.0f;
        float t2y = (py[4] - py[3]) / 2.0f;

        if(!ifAnyMax(px, py, count)) {
            float prev_x = px[3], prev_y = py[3];
            int steps = min(1,max(maxSteps,z[3] + z[4]));
            for (int i = 1; i <= steps; i++) {
                float sx, sy;
                evalHermite((float)i / steps,
                            px[3], px[4], t1x, t2x,
                            py[3], py[4], t1y, t2y,
                            &sx, &sy);
                plotline2(ROUND_F2I(prev_x), ROUND_F2I(prev_y),ROUND_F2I(sx), ROUND_F2I(sy));

                prev_x = sx; prev_y = sy;
            }
        } else {
                plotline2(ROUND_F2I(px[2]), ROUND_F2I(py[2]), ROUND_F2I(px[3]), ROUND_F2I(py[3]));
        }
    }
  #endif //USECURVES
}
#endif //TESTSUITE_BUILD





//Exhange the name of this routine with pixelline() above to try Bresenham
void pixelline(int16_t xo, int16_t yo, int16_t xn, int16_t yn, bool_t vmNormal) { // Plots line from xo,yo to xn,yn; uses temporary x1,y1
  #if defined(STATDEBUG_VERBOSE) && defined(PC_BUILD)
    printf("pixelline 1: xo,yo,xn,yn: %d %d   %d %d \n",xo,yo,xn,yn);
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
#if !defined(SAVE_SPACE_DM42_13GRF)
  #if !defined(TESTSUITE_BUILD)
    if(x_min <= FLoatingMin || x_min >= FLoatingMax || x_max <= FLoatingMin || x_max >= FLoatingMax || y_min <= FLoatingMin || y_min >= FLoatingMax || y_max <= FLoatingMin || y_max >= FLoatingMax) {
      return;
    }
  uint32_t cnt;

  clearScreenPixels();
  //GRAPH ZERO AXIS
  yzero = screen_window_y(y_min,0,y_max);
  xzero = screen_window_x(x_min,0,x_max);

  uint32_t minnx, minny;
  minny = 0;
  minnx = SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH;


  //SEPARATING LINE
  cnt = minny;
  while(cnt!=SCREEN_HEIGHT_GRAPH) {
    setBlackPixel(minnx-1,cnt);
    setBlackPixel(minnx-2,cnt);
    cnt++;
  }

    #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("xzero=%d yzero=%d   \n",(int)xzero,(int)yzero);
    #endif // STATDEBUG && PC_BUILD

  float x;
  float y;

  if( PLOT_AXIS && !(yzero == SCREEN_HEIGHT_GRAPH-1 || yzero == minny)) {
    //DRAW XAXIS
    cnt = minnx;

    while(cnt != SCREEN_WIDTH_GRAPH - 1) {
      setBlackPixel(cnt,yzero);
        #if defined(STATDEBUG) && defined(PC_BUILD)
        printf("cnt=%d   \n",(int)cnt);
        #endif // STATDEBUG && PC_BUILD
      cnt++;
    }

   force_refresh(timed);

   if(0<x_max && 0>x_min) {
     for(x=0; x<=x_max; x+=tick_int_x) {                         //draw x ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
         printf(">> x=%d   \n",(int)x);
          #endif // STATDEBUG && PC_BUILD
       cnt = screen_window_x(x_min,x,x_max);
       //printf(">>>>>A %f %d ",x,cnt);
       setBlackPixel(cnt,min(yzero+1,SCREEN_HEIGHT_GRAPH-1));    //tick
       setBlackPixel(cnt,max(yzero-1,minny));                    //tick
     }
     for(x=0; x>=x_min; x+=-tick_int_x) {                        //draw x ticks
       cnt = screen_window_x(x_min,x,x_max);
       //printf(">>>>>B %f %d ",x,cnt);
       setBlackPixel(cnt,min(yzero+1,SCREEN_HEIGHT_GRAPH-1));    //tick
       setBlackPixel(cnt,max(yzero-1,minny));                    //tick
     }
     for(x=0; x<=x_max; x+=tick_int_x*5) {                       //draw x ticks
       cnt = screen_window_x(x_min,x,x_max);
       setBlackPixel(cnt,min(yzero+2,SCREEN_HEIGHT_GRAPH-1));    //tick
       setBlackPixel(cnt,max(yzero-2,minny));                    //tick
       setBlackPixel(cnt,min(yzero+3,SCREEN_HEIGHT_GRAPH-1));    //tick
       setBlackPixel(cnt,max(yzero-3,minny));                    //tick
     }
     for(x=0; x>=x_min; x+=-tick_int_x*5) {                      //draw x ticks
       cnt = screen_window_x(x_min,x,x_max);
       setBlackPixel(cnt,min(yzero+2,SCREEN_HEIGHT_GRAPH-1));    //tick
       setBlackPixel(cnt,max(yzero-2,minny));                    //tick
       setBlackPixel(cnt,min(yzero+3,SCREEN_HEIGHT_GRAPH-1));    //tick
       setBlackPixel(cnt,max(yzero-3,minny));                    //tick
     }
   }
   else {
     for(x=x_min; x<=x_max; x+=tick_int_x) {                     //draw x ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>x=%d   \n",(int)x);
          #endif // STATDEBUG && PC_BUILD
        cnt = screen_window_x(x_min,x,x_max);
        //printf(">>>>>A %f %d ",x,cnt);
          setBlackPixel(cnt,min(yzero+1,SCREEN_HEIGHT_GRAPH-1)); //tick
          setBlackPixel(cnt,max(yzero-1,minny));                 //tick
       }
      for(x=x_min; x<=x_max; x+=tick_int_x*5) {                  //draw x ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>>x=%d   \n",(int)x);
          #endif // STATDEBUG && PC_BUILD
        cnt = screen_window_x(x_min,x,x_max);
          setBlackPixel(cnt,min(yzero+2,SCREEN_HEIGHT_GRAPH-1)); //tick
          setBlackPixel(cnt,max(yzero-2,minny));                 //tick
          setBlackPixel(cnt,min(yzero+3,SCREEN_HEIGHT_GRAPH-1)); //tick
          setBlackPixel(cnt,max(yzero-3,minny));                 //tick
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
    lcd_fill_rect(xzero,minny,1,SCREEN_HEIGHT_GRAPH-minny,LCD_EMPTY_VALUE);

    //printf("PLOT_ZMY=%i tick_int_x=%f, tick_int_y=%f\n",PLOT_ZMY, tick_int_x, tick_int_y);


    force_refresh(timed);
    if(0<y_max && 0>y_min) {
      for(y=0; y<=y_max; y+=tick_int_y) {                     //draw y ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>y=%d   \n",(int)y);
          #endif // STATDEBUG && PC_BUILD
        cnt = screen_window_y(y_min,y,y_max);
        setBlackPixel(max(xzero-1,0),cnt);                    //tick
        setBlackPixel(min(xzero+1,SCREEN_WIDTH_GRAPH-1),cnt); //tick
      }
      for(y=0; y>=y_min; y+=-tick_int_y) {                    //draw y ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>y=%d   \n",(int)y);
          #endif // STATDEBUG && PC_BUILD
        cnt = screen_window_y(y_min,y,y_max);
        setBlackPixel(max(xzero-1,0),cnt);                    //tick
        setBlackPixel(min(xzero+1,SCREEN_WIDTH_GRAPH-1),cnt); //tick
      }
      for(y=0; y<=y_max; y+=tick_int_y*5) {                   //draw y ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>>y=%d   \n",(int)y);
          #endif // STATDEBUG && PC_BUILD
        cnt = screen_window_y(y_min,y,y_max);
        setBlackPixel(max(xzero-2,0),cnt);                    //tick
        setBlackPixel(min(xzero+2,SCREEN_WIDTH_GRAPH-1),cnt); //tick
        setBlackPixel(max(xzero-3,0),cnt);                    //tick
        setBlackPixel(min(xzero+3,SCREEN_WIDTH_GRAPH-1),cnt); //tick
      }
      for(y=0; y>=y_min; y+=-tick_int_y*5) {                  //draw y ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>>>y=%d   \n",(int)y);
          #endif // STATDEBUG && PC_BUILD
        cnt = screen_window_y(y_min,y,y_max);
        setBlackPixel(max(xzero-2,0),cnt);                    //tick
        setBlackPixel(min(xzero+2,SCREEN_WIDTH_GRAPH-1),cnt); //tick
        setBlackPixel(max(xzero-3,0),cnt);                    //tick
        setBlackPixel(min(xzero+3,SCREEN_WIDTH_GRAPH-1),cnt); //tick
      }
    }
    else {
      for(y=y_min; y<=y_max; y+=tick_int_y) {                 //draw y ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>>>>>y=%d   \n",(int)y);
          #endif // STATDEBUG && PC_BUILD
        cnt = screen_window_y(y_min,y,y_max);
        setBlackPixel(max(xzero-1,0),cnt);                    //tick
        setBlackPixel(min(xzero+1,SCREEN_WIDTH_GRAPH-1),cnt); //tick
      }
      for(y=y_min; y<=y_max; y+=tick_int_y*5) {               //draw y ticks
          #if defined(STATDEBUG) && defined(PC_BUILD)
          printf(">>>>>>>>y=%d   \n",(int)y);
          #endif // STATDEBUG && PC_BUILD
        cnt = screen_window_y(y_min,y,y_max);
        setBlackPixel(max(xzero-2,0),cnt);                    //tick
        setBlackPixel(min(xzero+2,SCREEN_WIDTH_GRAPH-1),cnt); //tick
        setBlackPixel(max(xzero-3,0),cnt);                    //tick
        setBlackPixel(min(xzero+3,SCREEN_WIDTH_GRAPH-1),cnt); //tick
      }
    }
  }
  //printf("PLOT_ZMY=%i tick_int_x=%f, tick_int_y=%f\n",PLOT_ZMY, tick_int_x, tick_int_y);
  force_refresh(timed);
  #endif
#endif //SAVE_SPACE_DM42_13GRF
}


float auto_tick(float tick_int_f) {
  #if !defined(SAVE_SPACE_DM42_13GRF)
    char tmpString2[100];

    if(!roundedTicks) {
      return tick_int_f;
    }
    //Obtain scaling of ticks, to about 20 intervals left to right.
    //graphtype tick_int_f = (x_max-x_min)/20;                                                 //printf("tick interval:%f ",tick_int_f);
    snprintf(tmpString2, 100, "%.1e", fabs(tick_int_f));
    char tx[4];
    //get mantissa
    tx[0] = tmpString2[0]; //expecting the form "6.5e+01"
    tx[1] = tmpString2[1]; //the decimal radix is copied over, so region setting should not affect it
    tx[2] = tmpString2[2]; //the exponent is stripped
    tx[3] = 0;
    tick_int_f = strtof (tx, NULL);  //creating the form "6.5"
    //printf("tick0 %f orgstr %s ==> tx %s \n",tick_int_f, tmpString2, tx);
    //get exponent
    tmpString2[0] = '1';
    tmpString2[2] = '0';  //creating "1.0e+01"
    float tick_int_f_mult = strtof (tmpString2, NULL);;
    //tick_int_f = (float)(tx[0]-48) + (float)(tx[2]-48)/10.0f;
    //printf("tick1 %f x %f, orgstr %s ==> tx %s \n",tick_int_f, tick_int_f_mult, tmpString2, tx);

    if(tick_int_f > 0) {
      //if(tick_int_f <= 0.3) {
      //  tick_int_f = 0.2f;
      //}
      //else if(tick_int_f <= 0.6) {
      //  tick_int_f = 0.5f;
      //}
      //else
      if(tick_int_f <= 1.3) {
        tick_int_f = 1.0f;
      }
      else if(tick_int_f <= 1.7) {
        tick_int_f = 1.5f;
      }
      else if(tick_int_f <= 3.0) {
        tick_int_f = 2.0f;
      }
      else if(tick_int_f <= 6.5) {
        tick_int_f = 5.0f;
      }
      else if(tick_int_f <= 9.9) {
        tick_int_f = 7.5f;
      }
      //no higher values than 9.9 possible
    }
    else { //tick_int_f == 0
      tick_int_f = 1;
    }
    tick_int_f *= tick_int_f_mult;

    //printf("tick2 %f\n",tick_int_f);
  #endif // !SAVE_SPACE_DM42_13GRF

return tick_int_f;
}


void graph_axis (void){
#if !defined(SAVE_SPACE_DM42_13GRF)
  #if !defined(TESTSUITE_BUILD)
    graph_dx = 0; //XXX override manual setting from GRAPH to auto, temporarily. Can program these to fixed values.
    graph_dy = 0;

    if(graph_dx == 0) {
      tick_int_x = auto_tick((x_max-x_min)/20);
    }
    else {
      tick_int_x = graph_dx;
    }

    if(graph_dy == 0) {
      tick_int_y = auto_tick((y_max-y_min)/20);
    }
    else {
      tick_int_y = graph_dy;
    }

    #if defined(STATDEBUG)
     printf("TICKS X=%f Y=%f\n",tick_int_x,tick_int_y);
    #endif // STATDEBUG


  #endif // !TESTSUITE_BUILD
  graphAxisDraw();
#endif //SAVE_SPACE_DM42_13GRF
}


char * radixProcess(char *output, const char * ss) {  //  .  HIERDIE WERK GLAD NIE
  int8_t ix = 0, iy = 0;

  while( ss[ix] != 0 ){
    if(ss[ix]==',' || ss[ix]=='.') {
      output[iy++] = RADIX34_MARK_CHAR;
    }
    else
    if(ss[ix]=='#') {
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
    for (int ix = 2; s02[ix]!=0; ix++) {
      if(s02[ix]=='n' && s02[ix-1]=='a' && s02[ix-2]=='n') { //check for nan
        if (s02[0] == '(' && s02[ix+1] != 0) {
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

#if !defined(TESTSUITE_BUILD)

  static int checkWidthWithPrefix(const char* itemName, const char* numStr, uint32_t max_width) {
    char test_buffer[128];
    snprintf(test_buffer, sizeof(test_buffer), "%s%s", itemName, numStr);
    #if !defined(TESTSUITE_BUILD)
      //printf("ssss=%d\n",stringWidthC47(test_buffer, stdNoEnlarge, !nocompress, false, false));
      return stringWidthC47(test_buffer, stdNoEnlarge, !nocompress, false, false) < max_width;
    #else //TESTSUITE_BUILD
      return 0;
    #endif //TESTSUITE_BUILD
  }


  static void cleanupTrailingZeros(char* str) {
    char* e_pos = strchr(str, 'E');
    if (!e_pos) {
      e_pos = strchr(str, 'e');
    }
    if (e_pos && *e_pos == 'e') {
      *e_pos = 'E';
    }
    if (e_pos) {
      char* decimal_pos = strchr(str, '.');
      if (decimal_pos && decimal_pos < e_pos) {
        char* p = e_pos - 1;
        while (p > decimal_pos && *p == '0') {
          p--;
        }
        if (p == decimal_pos) {
          memmove(decimal_pos, e_pos, strlen(e_pos) + 1);
        } else {
          memmove(p + 1, e_pos, strlen(e_pos) + 1);
        }
      }
      e_pos = strchr(str, 'E');
      if (e_pos) {
        char* exp_start = e_pos + 1;
        if (*exp_start == '+') {
          exp_start++;
        }
        if (*exp_start == '-') {
          exp_start++;
        }
        while (*exp_start == '0' && *(exp_start + 1) != '\0') {
          memmove(exp_start, exp_start + 1, strlen(exp_start + 1) + 1);
        }
      }
    } else {
      char* decimal_pos = strchr(str, '.');
      if (decimal_pos) {
        int len = strlen(str);
        int i = len - 1;
        while (i > decimal_pos - str && str[i] == '0') {
          str[i] = '\0';
          i--;
        }
        if (i == decimal_pos - str) {
          str[i] = '\0';
        }
      }
    }
  }
#endif //TESTSUITE_BUILD


//success flag set if convertion was done and maxwidth is NOT overreached. Additionally ?? is output when no conversion is done.
char* formatDoubleWidth(real34_t *real34, int digits, char* itemName, bool_t* success, int actual_max_width, char* buf, int digitswidthLimit) {
  #if !defined(TESTSUITE_BUILD)
    uint8_t savedDisplayFormatDigits = displayFormatDigits;
    uint8_t saveddisplayFormat = displayFormat;
    bool_t  ovrENG = getSystemFlag(FLAG_ENGOVR);
    clearSystemFlag(FLAG_ENGOVR);
    if (real34IsZero(real34)) {
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
    if (realCompareAbsGreaterThan(&real, &threshold9E99)) {
      strcpy(buf, isNegative ? STD_GAUSS_WHITE_L STD_GAUSS_WHITE_L STD_GAUSS_WHITE_L /* <<< */ : STD_GAUSS_WHITE_R STD_GAUSS_WHITE_R STD_GAUSS_WHITE_R /* >>> */);
      *success = 1;
      goto done;
    }
    if (realCompareAbsLessThan(&real, &threshold1E_99)) {
      strcpy(buf, isNegative ? STD_GAUSS_WHITE_L STD_SUB_0 : STD_GAUSS_WHITE_R STD_SUB_0);   //  "<0" : ">0");
      *success = 1;
      goto done;
    }
    realLog10(&real, &reall10, &ctxtReal39);
    if(realToInt32C47(&reall10) < digits) {
      displayFormat = DF_SF;
      displayFormatDigits = digits;
    } else {
      displayFormat = DF_FIX;
      displayFormatDigits = 0;
    }
    for (int ddd = 8; ddd >= 2; ddd--) {
      updateDisplayValueX = true;
      displayValueX[0] = 0;
      real34ToDisplayString(real34, amNone, buf, &standardFont, digitswidthLimit == 0 ? 60 : digitswidthLimit, ddd, LIMITEXP, !FRONTSPACE, NOIRFRAC);
      updateDisplayValueX = false;
      strcpy(buf, displayValueX);
      cleanupTrailingZeros(buf);

      if (checkWidthWithPrefix(itemName, buf, actual_max_width)) {
         *success = false;
         goto done;
      }
    }
    strcpy(buf, "??");
    *success = 0;
done:
    displayFormatDigits = savedDisplayFormatDigits;
    displayFormat = saveddisplayFormat;
    if(ovrENG) setSystemFlag(FLAG_ENGOVR); else clearSystemFlag(FLAG_ENGOVR);
  #endif //TESTSUITE_BUILD
  return buf;
}

char* formatCore(double value, int digits, bool handle_zero, char* buf, int widthLimit) {
    const char* sign = (value < 0.0) ? "-" : "";
    if (value < 0.0) value = -value;

    if (handle_zero && value == 0.0) {
      sprintf(buf, "%s0.0", sign);
    } else {
      #if !defined(TESTSUITE_BUILD)
        real34_t value34;
        real_t valueR;
        bool_t success;
        char tmpBuf[128];
        convertDoubleToReal(value, &valueR, &ctxtReal39);
        realToReal34(&valueR, &value34);
        strcpy(buf, sign);
        strcat(buf, formatDoubleWidth(&value34, digits, "", &success, widthLimit == 0 ? 50 : widthLimit, tmpBuf, widthLimit == 0 ? 50 : widthLimit));
      #else
        buf[0] = '\0';
      #endif //TESTSUITE_BUILD
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
//      for (int d = 2; d <= 8; d++) {
//          printf("formatCore(%d)\tgrphNumFormatter(%d)\t", d, d);
//      }
//      printf("\n");
//
//      printf("===========\t\t");
//      for (int d = 2; d <= 8; d++) {
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
//      for (int i = 0; i < num_values; i++) {
//          printf("%.3e\t\t", test_values[i]);
//
//          for (int digits = 2; digits <= 8; digits++) {
//              formatCore(test_values[i], digits, false, result_buf, 256);
//              printf("%s\t", result_buf);
//              grphNumFormatter(result_buf, "[", test_values[i], digits, "]");
//              printf("%s\t", result_buf);
//          }
//          printf("\n");
//
//          if (i % 15 == 14) printf("\n"); // Add spacing every 15 values
//      }
//
//      printf("\nTotal test cases: %d values × 7 digit settings × 2 functions = %d tests\n",
//             num_values, num_values * 7 * 2);
//  }



#define horOffsetR 109+5 //digit righ side aliognment
#define autoinc 19 //text line spacing
#define autoshift -4 //text line offset
#define horOffset 1 //labels from the left


#if !defined(TESTSUITE_BUILD)
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
        if(isStatsMatrix(&rows,plotStatMx)) {
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
      printf("Plotting cross to x=%d y=%d\n",xn,yn);
    #endif // STATDEBUG && PC_BUILD
    plotcross(xn,yn);
  }
  else if(PLOT_BOXFAT) {
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("Plotting box to x=%d y=%d\n",xn,yn);
    #endif // STATDEBUG && PC_BUILD
    plotbox_fat(xn,yn);
  }
  else if(PLOT_BOX) {
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("Plotting box to x=%d y=%d\n",xn,yn);
    #endif // STATDEBUG && PC_BUILD
    plotbox(xn,yn);
  }
  else if(PLOT_PLUS) {
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("Plotting plus to x=%d y=%d\n",xn,yn);
    #endif // STATDEBUG && PC_BUILD
    plotplus(xn,yn);
  }

  if(PLOT_LINE) {
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("Plotting line to x=%d y=%d\n",xn,yn);
    #endif // STATDEBUG && PC_BUILD
    plotline1(xo, yo, xn, yn);
  }
}
#endif // !TESTSUITE_BUILD




//ASSESS; HISTO; SCATR

void graphPlotstat(uint16_t selection){
currentKeyCode = 255;
#if !defined(SAVE_SPACE_DM42_13GRF)
  #if defined(STATDEBUG) && defined(PC_BUILD)
    printf("#####>>> graphPlotstat: selection:%u:%s  lastplotmode:%u  lrSelection:%u lrChosen:%u\n",selection, getCurveFitModeName(selection), lastPlotMode, lrSelection, lrChosen);
  #endif // STATDEBUG && PC_BUILD
  #if !defined(TESTSUITE_BUILD)
  uint16_t  cnt, ix, numberOfPlotPoints;
  int16_t  xo, xn, xN;
  int16_t  yo, yn, yN;
  float x;
  float y;

  numberOfPlotPoints = 0;
  roundedTicks = false;

  if((plotStatMx[0]=='S' && checkMinimumDataPoints(const_2)) ||
     (plotStatMx[0]=='D' && drawMxN() >= 2) ||
     (plotStatMx[0]=='H' && statMxN() >= 3)) {
    switch(plotStatMx[0]) {
      case 'S': numberOfPlotPoints = realToInt32C47(SIGMA_N); break;
      case 'D': numberOfPlotPoints = drawMxN();               break;
      case 'H': numberOfPlotPoints = statMxN();               break;
      default: ;
    }
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("graphPlotstat: numberOfPlotPoints n=%d\n",numberOfPlotPoints);
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
      x_min = FLoatingMax;
      x_max = FLoatingMin;
      y_min = FLoatingMax;
      y_max = FLoatingMin;
      #if defined(STATDEBUG) && defined(PC_BUILD)
        printf("Axis0: x: %f -> %f y: %f -> %f   \n",x_min, x_max, y_min, y_max);
      #endif // STATDEBUG && PC_BUILD


    //#################################################### vvv SCALING LOOP  vvv
    for(cnt=0; (cnt < numberOfPlotPoints); cnt++) {
      #if defined(STATDEBUG) && defined(PC_BUILD)
        printf("Axis0a: x: %f y: %f   \n",grf_x(cnt), grf_y(cnt));
      #endif // STATDEBUG && PC_BUILD
      if(grf_x(cnt) < x_min) {
        x_min = grf_x(cnt);
      }
      if(grf_x(cnt) > x_max) {
        x_max = grf_x(cnt);
      }
      if(grf_y(cnt) < y_min) {
        y_min = grf_y(cnt);
      }
      if(grf_y(cnt) > y_max) {
        y_max = grf_y(cnt);
      }
      #if defined(STATDEBUG) && defined(PC_BUILD)
        printf("Axis0b: x: %f -> %f y: %f -> %f   \n",x_min, x_max, y_min, y_max);
      #endif // STATDEBUG && PC_BUILD
      if(exitKeyWaiting()) {
        return;
      }
    }
    //##  ################################################## ^^^ SCALING LOOP ^^^
      #if defined(STATDEBUG) && defined(PC_BUILD)
        printf("Axis1a: x: %f -> %f y: %f -> %f   \n",x_min, x_max, y_min, y_max);
      #endif // STATDEBUG && PC_BUILD

      if(x_min <= FLoatingMin || x_max <= FLoatingMin || y_min <= FLoatingMin || y_max <= FLoatingMin) {
        goto scaleMinusInfinity;
      }
      if(x_min >= FLoatingMax || x_max >= FLoatingMax || y_min >= FLoatingMax || y_max >= FLoatingMax) {
        goto scalePlusInfinity;
      }


    graph_Include0(PLOTSTAT, numberOfPlotPoints);



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

    if(x_min <= FLoatingMin || x_max <= FLoatingMin || y_min <= FLoatingMin || y_max <= FLoatingMin) {
       goto scaleMinusInfinity;
     }
     if(x_min >= FLoatingMax || x_max >= FLoatingMax || y_min >= FLoatingMax || y_max >= FLoatingMax) {
       goto scalePlusInfinity;
     }



    graph_axis();
    yn = screen_window_y(y_min,grf_y(0),y_max);
    xn = screen_window_x(x_min,grf_x(0),x_max);
    xN = xn;
    yN = yn;
      #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("Axis3c: x: %f -> %f y: %f -> %f   \n",x_min, x_max, y_min, y_max);
      #endif // STATDEBUG && PC_BUILD

      int16_t colw = (int16_t) (
                                 (  (screen_window_x(x_min,grf_x(1),x_max) - screen_window_x(x_min,grf_x(0),x_max))  / 2.0f  )
                                ) - 1;
        //#################################################### vvv MAIN GRAPH LOOP vvv #########################
      for(ix = 0; (ix < numberOfPlotPoints); ++ix) {
        x = grf_x(ix);
        y = grf_y(ix);
        xo = xN;
        yo = yN;
        xN = screen_window_x(x_min,x,x_max);
        yN = screen_window_y(y_min,y,y_max);

        #if defined(STATDEBUG) && defined(PC_BUILD)
          printf("plotting graph table[%d] = x:%f y:%f xN:%d yN:%d drawHistogram:%d ", ix, x, y, xN, yN, drawHistogram);
        #endif // STATDEBUG && PC_BUILD

        int16_t minN_y,minN_x;
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
            printf("Not plotted point: (%u %u) ",xN,yN);
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
            //printf("Not plotted: xN=%d<SCREEN_WIDTH_GRAPH=%d && xN=%d>minN_x=%d && yN=%d<SCREEN_HEIGHT_GRAPH=%d && yN=%d>1+minN_y=%d\n",xN,SCREEN_WIDTH_GRAPH,xN,minN_x,yN,SCREEN_HEIGHT_GRAPH,yN,1+minN_y);
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
      real34ToReal(&loBinR ,&lBr);
      real34ToReal(&hiBinR ,&hBr);
      real34ToReal(&nBins  ,&nBr);
      realToFloat(&lBr, &lB);
      realToFloat(&hBr, &hB);
      realToFloat(&nBr, &nB);

      strcpy(ss, "Histogram(");
      strcat(ss,histElementXorY == 1 ? "y)" : "x)");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffset + 17, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -7 +autoshift, vmNormal, false, false);

      grphNumFormatter(ss, "(", x_max, 2, "");
      grphNumFormatter(tt,radixProcess(tmpbuf, "#"),y_max,2,")");
      strcat(tt, padEquals(tmpbuf, ss));
      n = showString(padEquals(tmpbuf, ss), &standardFont, 160-2-3-2 - stringWidth(tt, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index + 2   -3  +autoshift, vmNormal, false, false);
      grphNumFormatter(ss, radixProcess(tmpbuf, "#"), y_max, 2, ")");
      showString(padEquals(tmpbuf, ss), &standardFont, n+3,           Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++  -3  + autoshift + 2, vmNormal, false, false);

      grphNumFormatter(ss, "(", x_min, 2, "");
      n = showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index    -6  + autoshift + 2, vmNormal, false, false);
      grphNumFormatter(ss, radixProcess(tmpbuf, "#"), y_min, 2, ")");
      showString(padEquals(tmpbuf, ss), &standardFont, n+3,           Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++  -6  + autoshift + 2, vmNormal, false, false);

      strcpy(ss,"Bin centres:");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -4 +autoshift, vmNormal, false, false);
      grphNumFormatter(ss,"",lB,3,"");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -4 +autoshift, vmNormal, false, false);
      strcpy(ss,STD_DOWN_ARROW "BIN" "=");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -4 +autoshift, vmNormal, false, false);

      grphNumFormatter(ss,"",hB,3,"");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -1 +autoshift, vmNormal, false, false);
      strcpy(ss,STD_UP_ARROW "BIN" "=");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -1 +autoshift, vmNormal, false, false);

      grphNumFormatter(ss,"",nB,3,"");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -1 +autoshift, vmNormal, false, false);
      strcpy(ss,"nBINS" "=");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -1 +autoshift, vmNormal, false, false);
      grphNumFormatter(ss,"",(hB-lB)/nB,3,"");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -1 +autoshift, vmNormal, false, false);
      strcpy(ss,"Width" "=");
      showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -1 +autoshift, vmNormal, false, false);

    }




  }
  else {
    displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "There is no statistical data available!");
      moreInfoOnError("In function graphPlotstat:", errorMessage, NULL, NULL);
    #endif
  }
 return;


  scalePlusInfinity:
  displayCalcErrorMessage(ERROR_OVERFLOW_PLUS_INF, ERR_REGISTER_LINE, REGISTER_X);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    sprintf(errorMessage, "Plus Infinity encountered!");
    moreInfoOnError("In function graphPlotstat:", errorMessage, NULL, NULL);
  #endif
  return;

  scaleMinusInfinity:
  displayCalcErrorMessage(ERROR_OVERFLOW_MINUS_INF, ERR_REGISTER_LINE, REGISTER_X);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    sprintf(errorMessage, "Minus Infinity encountered!");
    moreInfoOnError("In function graphPlotstat:", errorMessage, NULL, NULL);
  #endif


 #endif // !TESTSUITE_BUILD)
#endif //SAVE_SPACE_DM42_13GRF
}


#if !defined(TESTSUITE_BUILD)
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
      int32ToReal34(mv+rand()%4-2,REGISTER_REAL34_DATA(REGISTER_X));
      // reading 1 has additional +0 to +3 variance to the said random number

      setSystemFlag(FLAG_ASLIFT);
      liftStack();
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      int32ToReal34(mv+rand()%4-2,REGISTER_REAL34_DATA(REGISTER_X));
      // reading 2 has additional +0 to +3 variance to the said random number

      runFunction(ITM_SIGMAPLUS);
    }
  }
#endif // !TESTSUITE_BUILD


void graphDrawLRline(uint16_t selection) {
  #if !defined(TESTSUITE_BUILD)
    //demo_plot();
    if(selection != 0) {
      processCurvefitSelection(selection, &RR, &SMI, &aa0, &aa1, &aa2);
      realMultiply(&RR,&RR,&RR,&ctxtReal39);
      if(orOrtho(selection) == CF_ORTHOGONAL_FITTING) {
        processCurvefitSA(&sa0, &sa1);
      }
      drawline(selection, &RR, &SMI, &aa0, &aa1, &aa2, &sa0, &sa1);
    }
  #endif // !TESTSUITE_BUILD
}


#if !defined(TESTSUITE_BUILD)
 static void drawline(uint16_t selection, real_t *RR, real_t *SMI, real_t *aa0, real_t *aa1, real_t *aa2, real_t *sa0, real_t *sa1) {
#if !defined(SAVE_SPACE_DM42_13GRF)
    int32_t n = 0;
    uint16_t NN;
    char tmpbuf[PLOT_TMP_BUF_SIZE];

    switch(plotStatMx[0]) {
      case 'S': n = realToInt32C47(SIGMA_N); break;
      case 'D': n = drawMxN();               break;
      case 'H': n = statMxN();               break;
      default: ;
    }
    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("drawline: n=%d\n",n);
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
    if(!selection) {
      return;
    }

    realToFloat(RR , &rr );
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
      double    x = x_min;
      double    y = 0.0;
      int16_t   Intervals = numberIntervals; //starting point to calculate dx
      uint16_t  iterations = 0;
      double    intervalW = (double)(x_max-x_min)/(double)(Intervals);

      int16_t minN_y,minN_x;
      minN_y = 0;
      minN_x = SCREEN_WIDTH-SCREEN_HEIGHT_GRAPH;
      for(ix = (double)x_min-intervalW; iterations < 2000 && x < x_max+(x_max-x_min)*0.5 && xN < SCREEN_WIDTH-1; iterations++) {       //Variable accuracy line plot
        xo = xN;
        yo = yN;
        uint16_t xx;
        for( xx=0; xx<14; xx++) {      //the starting point is ix + dx where dx=2^-0*interval and reduce it to dx=2^-31*interval until dy<=2
          x = ix + intervalW / ((double)((uint16_t) 1 << xx));
          if(USEFLOATING == useREAL4) {
            //TODO create REAL from x (double) if REALS will be used
            //snprintf(ss,100,"%f",x); stringToReal(ss,&XX,&ctxtReal4);
            convertDoubleToReal(x, &XX, &ctxtReal4);
          }
          else {
            if(USEFLOATING == useREAL39) {
              //TODO create REAL from x (double) if REALS will be used
              //snprintf(ss,100,"%f",x); stringToReal(ss,&XX,&ctxtReal39);
              convertDoubleToReal(x, &XX, &ctxtReal39);
            }
          }
          yIsFnx( USEFLOATING, selection, x, &y, a0, a1, a2, &XX, &YY, RR, SMI, aa0, aa1, aa2);
          xN = screen_window_x(x_min,(float)x,x_max);
          yN = screen_window_y(y_min,(float)y,y_max);
          if((abs((int)yN-(int)yo)<=2 /*&& abs((int)xN-(int)xo)<=2*/) || iterations == 0 || xN <= minN_x) {
            break;
          }
        }
        ix = x;
        //printf("### iter:%u ix=%20lf xx=%i x=%lf y=%lf xN=%u yN=%u\n",iterations,ix,xx,x,y,xN,yN);
        if(iterations > 0) {  //Allow for starting values to accumulate in the registers at ix = 0
          #if defined(STATDEBUG) && defined(PC_BUILD)
            printf("plotting graph: iter:%u ix:%f I.vals:%u ==>xmin:%f (x:%f) xmax:%f ymin:%f (y:%f) ymax:%f xN:%d yN:%d \n",iterations,ix,Intervals,x_min,x,x_max,y_min,y,y_max,  xN,yN);
          #endif // STATDEBUG && PC_BUILD
          #define tol 4
          if(xN<SCREEN_WIDTH_GRAPH && xN>minN_x && yN<SCREEN_HEIGHT_GRAPH-tol && yN>minN_y) {
            yn = yN;
            xn = xN;
            #if defined(STATDEBUG_VERBOSE) && defined(PC_BUILD)
              printf("Plotting box to x=%d y=%d\n",xn,yn);
            #endif // STATDEBUG && PC_BUILD
            if(fittedcurveboxes) {
              plotbox(xn,yn);
            }
            if(xo < SCREEN_WIDTH_GRAPH && xo > minN_x && yo < SCREEN_HEIGHT_GRAPH-tol && yo > minN_y) {
              #if defined(STATDEBUG_VERBOSE) && defined(PC_BUILD)
                printf("Plotting line to x=%d y=%d\n",xn,yn);
              #endif // STATDEBUG && PC_BUILD
              plotline2(xo, yo, xn, yn);
            }
          }
          else {
            #if defined(STATDEBUG) && defined(PC_BUILD)
              printf("Not plotted line: (%u %u) ",xN,yN);
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
        printf("Drawline: %u / 2000 iterations\n",iterations);
      #endif // PC_BUILD
    }


    int16_t index = -1;
    if(selection!=0) {
      strcpy(ss,eatSpacesEnd(getCurveFitModeName(selection)));
      if(lrCountOnes(lrSelection)>1 && selection == lrChosen) {
        strcat(ss,lrChosen == 0 ? "" : STD_SUP_ASTERISK);
      }
      showString(ss, &standardFont, horOffset + 17, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -10 +autoshift, vmNormal, false, false);
      if(selection != CF_GAUSS_FITTING && selection != CF_CAUCHY_FITTING) {
        strcpy(ss,"y="); strcat(ss,getCurveFitModeFormula(selection));
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7 +autoshift, vmNormal, false, false);
      }
      else {
        strcpy(ss,"y="); strcat(ss,getCurveFitModeFormula(selection));
        compressString = 1;
        showString(          ss,  &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7 +autoshift, vmNormal, false, false);
      }
    }

    if(isValidDraw) {
      if(softmenu[softmenuStack[0].softmenuId].menuItem != -MNU_PLOT_SCATR) {
        sprintf(ss, "%u",NN);
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
        strcpy(ss,formatCore(rr,5,false,tmpBuf, 50));
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  +2  +autoshift, vmNormal, false, false);
        strcpy(ss, "r" STD_SUP_2 "=");
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   +2 +autoshift, vmNormal, false, false);

        grphNumFormatter(ss,"(",x_max,2,"");
        uint16_t ssw = showStringEnhanced(padEquals(tmpbuf, ss),&standardFont,0,0,vmNormal, false, false, NO_compress, NO_raise, NO_Show, NO_Bold, NO_LF);
        grphNumFormatter(tt,radixProcess(tmpbuf, "#"),y_max,2,")");
        uint16_t ttw = showStringEnhanced(padEquals(tmpbuf, tt),&standardFont,0,0,vmNormal, false, false, NO_compress, NO_raise, NO_Show, NO_Bold, NO_LF);
        n = showString(padEquals(tmpbuf, ss), &standardFont,160-3-2-ssw-ttw, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index + 2       +autoshift, vmNormal, false, false);
        showString(padEquals(tmpbuf, tt), &standardFont,n+3, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++ +autoshift + 2, vmNormal, false, false);
        grphNumFormatter(ss, "(", x_min, 2, "");
        n = showString(padEquals(tmpbuf, ss), &standardFont,horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index    -2  +autoshift + 2, vmNormal, false, false);
        grphNumFormatter(ss,radixProcess(tmpbuf, "#"),y_min,2,")");
        showString(padEquals(tmpbuf, ss), &standardFont,n+3,       Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++  -2  +autoshift + 2, vmNormal, false, false);

      }
      else {                          //ORTHOF
        char tmpBuf[100];
        strcpy(ss,formatCore(a0,3,false,tmpBuf, 50));
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -4 +autoshift, vmNormal, false, false);
        strcpy(ss, "a" STD_SUB_0 "=");
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -4 +autoshift, vmNormal, false, false);

        strcpy(ss,formatCore(ssa0,3, false,tmpBuf, 50));
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -4 +autoshift, vmNormal, false, false);
        strcpy(ss, "    " STD_PLUS_MINUS);
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -4 +autoshift, vmNormal, false, false);

        strcpy(ss,formatCore(a1,3,false,tmpBuf, 50));
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -1 +autoshift, vmNormal, false, false);
        strcpy(ss, "a" STD_SUB_1 "=");
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -1 +autoshift, vmNormal, false, false);

        strcpy(ss,formatCore(ssa1,3,false,tmpBuf, 50));
        showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  -1 +autoshift, vmNormal, false, false);
        strcpy(ss, "    " STD_PLUS_MINUS);
        showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   -1 +autoshift, vmNormal, false, false);

        if(softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_PLOT_SCATR) {
          if(n >= 30) {
            grphNumFormatter(ss, "", smi, 3, "");
          }
          else {
            strcpy(ss,"  | n < 30");
          }

          showString(padEquals(tmpbuf, ss), &standardFont, horOffsetR - stringWidth(ss, &standardFont, false, false), Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index  +1 +autoshift, vmNormal, false, false);
          strcpy(ss,"s" STD_SUB_m STD_SUB_i "=");
          showString(padEquals(tmpbuf, ss), &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc*index++   +1 +autoshift, vmNormal, false, false);
        }
        else {
          strcpy(ss,formatCore(rr,3,false,tmpBuf, 50));
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
        sprintf(ss," %u < %u", NN,minLRDataPoints(selection));
        showString(ss, &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7+2 +autoshift, vmNormal, false, false);
        }
      else if(selection == 0) {
        showString("No Valid L.R.", &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7+2 +autoshift, vmNormal, false, false);
      }
      else {
        showString("L.R. error", &standardFont, horOffset, Y_POSITION_OF_REGISTER_Z_LINE + autoinc * index++ -7+2 +autoshift, vmNormal, false, false);
    }
  }
#endif // !SAVE_SPACE_DM42_13GRF
  }
#endif // !TESTSUITE_BUILD


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
#if !defined(TESTSUITE_BUILD)
#if !defined(SAVE_SPACE_DM42_13GRF)
    //restoreStats();
  switch(plotMode) {
    case PLOT_ORTHOF:
    case PLOT_START:
    case PLOT_REV:
    case PLOT_NXT:
    case PLOT_LR: {
      drawHistogram = 0;
      strcpy(plotStatMx, "STATS");
      break;
    }
    case H_PLOT: {
      drawHistogram = 1;
      strcpy(plotStatMx, "HISTO");
      break;
    }
    case H_NORM: {
      drawHistogram = 1;
      strcpy(statMx,"HISTO");
      calcSigma(0);
      plotMode = PLOT_LR;
      lastPlotMode = PLOT_START;
      lrSelectionHistobackup = lrSelection;
      lrChosenHistobackup = lrChosen;
      fnCurveFitting(CF_GAUSS_FITTING);
      break;
    }
    default: {
      break;
    }
  }

                                  #if defined(STATDEBUG) && defined(PC_BUILD)
                                  printf("fnPlotStat1: plotSelection = %u; Plotmode=%u\n",plotSelection,plotMode);
                                  printf("#####>>> fnPlotStat1: plotSelection:%u:%s  Plotmode:%u lastplotmode:%u  lrSelection:%u lrChosen:%u plotStatMx:%s\n",plotSelection, getCurveFitModeName(plotSelection), plotMode, lastPlotMode, lrSelection, lrChosen, plotStatMx);
                                    if( (plotStatMx[0]=='S' && checkMinimumDataPoints(const_2)) ||
                                        (plotStatMx[0]=='D' && drawMxN() >= 2) ||
                                        (plotStatMx[0]=='H' && statMxN() >= 3) ) {
                                      int16_t cnt = 0;
                                      switch(plotStatMx[0]) {
                                        case 'S': cnt = realToInt32C47(SIGMA_N); break;
                                        case 'D': cnt = drawMxN();               break;
                                        case 'H': cnt = statMxN();               break;
                                        default: ;
                                      }
                                    printf("Stored values %i\n",cnt);
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
      clearSystemFlag(FLAG_SCALE);

      #if !defined(TESTSUITE_BUILD)
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
          setSystemFlag(FLAG_SCALE);
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
      #endif // !TESTSUITE_BUILD

  }
  else {
    calcMode = CM_NORMAL;
    displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "There is no statistical/plot data available!");
      moreInfoOnError("In function fnPlotStat:", errorMessage, NULL, NULL);
    #endif
  }
#endif // SAVE_SPACE_DM42_13GRF
#endif // !TESTSUITE_BUILD
}


void fnPlotRegressionLine(uint16_t plotMode){
#if !defined(SAVE_SPACE_DM42_13GRF)
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
#endif // !SAVE_SPACE_DM42_13GRF
}




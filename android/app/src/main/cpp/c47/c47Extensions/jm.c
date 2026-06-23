// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"



#if defined(PC_BUILD)
  #if defined(PC_BUILD_TELLTALE)
    static char * getCalcModeName1(uint16_t cm) {
      if(cm == CM_NORMAL)           return "normal ";
      if(cm == CM_AIM)              return "aim    ";
      if(cm == CM_EIM)              return "eim    ";
      if(cm == CM_PEM)              return "pem    ";
      if(cm == CM_NIM)              return "nim    ";
      if(cm == CM_ASSIGN)           return "assign ";
      if(cm == CM_REGISTER_BROWSER) return "reg.bro";
      if(cm == CM_ASN_BROWSER)      return "asn.bro";
      if(cm == CM_FLAG_BROWSER)     return "flg.bro";
      if(cm == CM_FONT_BROWSER)     return "fnt.bro";
      if(cm == CM_PLOT_STAT)        return "plot.st";
      if(cm == CM_GRAPH)            return "plot.gr";
      if(cm == CM_ERROR_MESSAGE)    return "err.msg";
      if(cm == CM_BUG_ON_SCREEN)    return "bug.scr";
      if(cm == CM_MIM)              return "mim    ";
      if(cm == CM_EIM)              return "eim    ";
      if(cm == CM_TIMER)            return "timer  ";
      if(cm == CM_CONFIRMATION)     return "confirm";
      if(cm == CM_LISTXY)           return "listxy ";    //JM
      return "???    ";
    }

    static char * getAlphaCaseName1(uint16_t ac) {
      if(ac == AC_LOWER) return "lower";
      if(ac == AC_UPPER) return "upper";
      return "???  ";
    }
  #endif // PC_BUILD_TELLTALE


  void jm_show_calc_state(char comment[]) {
    #if defined(PC_BUILD_TELLTALE)
      printf("\n%s--------------------------------------------------------------------------------\n", comment);
      printf(".  calcMode: %s   last_CM=%s  AlphaCase=%s  doRefreshSoftMenu=%d    lastErrorCode=%d fnAsnDisplayUSER=%d TI=%u\n", getCalcModeName1(calcMode), getCalcModeName1(last_CM), getAlphaCaseName1(alphaCase), doRefreshSoftMenu, lastErrorCode, fnAsnDisplayUSER, temporaryInformation);
      printf(".  softmenuStack[0].softmenuId=%d      softmenu[softmenuStack[0].softmenuId].menuItem=%d -MNU_ALPHA=%d temporaryInformation=%d currentSolverStatus=%d\n",
                 softmenuStack[0].softmenuId,        softmenu[softmenuStack[0].softmenuId].menuItem,   -MNU_ALPHA,   temporaryInformation, currentSolverStatus);

      printf(".  ");
      int8_t ix=0;
      while(ix < SOFTMENU_STACK_SIZE) {
        printf("(%d)=%5d ", ix, softmenuStack[ix].softmenuId);
        ix++;
      }
      printf("\n");

      printf(".  ");
      ix=0;
      while(ix < SOFTMENU_STACK_SIZE) {
        printf("%9s ", indexOfItems[-softmenu[softmenuStack[ix].softmenuId].menuItem].itemSoftmenuName  );
        ix++;
      }
      printf("\n");
      printf(".  ");
      ix=0;
      while(ix < SOFTMENU_STACK_SIZE) {
        printf("%9s ", getCalcModeName1(softmenuStack[ix].calcMode));
        ix++;
      }
      printf("\n");

      printf(".  (tam.mode=%d, catalog=%d)   \n",
                  tam.mode,    catalog    );
      jm_show_comment("calcstate END:");
    #endif //PC_BUILD_TELLTALE
  }


  void jm_show_comment(char comment[]) {
    #if defined(PC_BUILD_VERBOSE2)
    char tmp[600];
    tmp[0]=0;
      strcat(tmp, "                                                                                                                                                                ");
      tmp[100]=0;
      printf("....%s %s calcMode=%4d last_CM=%4d tam.mode=%5d catalog=%5d Id=%4d Name=%8s f=%d g=%d \n", tmp, comment, calcMode, last_CM, tam.mode, catalog, softmenuStack[0].softmenuId, indexOfItems[-softmenu[softmenuStack[0].softmenuId].menuItem].itemSoftmenuName, shiftF, shiftG);
    #endif // PC_BUILD_VERBOSE2
    //  printf("....%s\n",tmp);
  }
#endif // PC_BUILD



/********************************************//** XXX
 * \brief Set Norm_Key_00
 *
 * \param[in] sigmaAssign uint16_t
 * \return void
 ***********************************************/
void fnSigmaAssign(uint16_t sigmaAssign) {             //DONE
  if(Norm_Key_00_key != -1) {
    int16_t tt = (int16_t)sigmaAssign;
    Norm_Key_00.func = tt - 16384;
    Norm_Key_00.funcParam[0] = 0;
    Norm_Key_00.used = Norm_Key_00.func != kbd_std[Norm_Key_00_key].primary;
    fnRefreshState();                                 //drJM
    fnClearFlag(FLAG_USER);
  }
  else {
    Norm_Key_00.used = false;
    displayCalcErrorMessage(ERROR_CANNOT_ASSIGN_HERE, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnSigmaAssign: ", "the NRM key is not available.", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

}

// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors

#include "c47.h"

#define PROGRAM_VERSION                     01  // Original version
#define EXPORT_VERSION                      03  // 02 Modified export version to indent LBL; 03 Add RTF method and revise fixed table
#define OLDEST_COMPATIBLE_PROGRAM_VERSION   01  // Original version
#define BACKUP_FORMAT                       00  // Same program format as in backup file
#define TEXT_FORMAT                         01  // Text program format - for future use

// Structure of the program file.
// Format: ASCII
// Each line after line 6 contains the decimal value of a program byte
//
//  +-----+---------------------------+
//  | Line|         Content           |
//  +-----+---------------------------+
//  |  1  |   "PROGRAM_FILE_FORMAT"   |
//  |  2  |       <file format>       |
//  |  3  |"WP43_program_file_version"|
//  |  4  |   <program file version>  |
//  |  5  |         "PROGRAM"         |
//  |  6  |       <program size>      |
//  |  7  |    <first program byte>   |
//  | ... |            ...            |
//  |  n  |    <last program byte>    |
//  +-----+---------------------------+
//



#if !defined(TESTSUITE_BUILD)
  static void _addSpaceAfterPrograms(uint16_t size) {
    if(freeProgramBytes < size) {
      uint8_t *oldBeginOfProgramMemory = beginOfProgramMemory;
      uint32_t programSizeInBlocks = RAM_SIZE_IN_BLOCKS - TO_C47MEMPTR(beginOfProgramMemory);
      uint32_t newProgramSizeInBlocks = TO_BLOCKS(TO_BYTES(programSizeInBlocks) - freeProgramBytes + size);
      freeProgramBytes      += TO_BYTES(newProgramSizeInBlocks - programSizeInBlocks);
      resizeProgramMemory(newProgramSizeInBlocks);
      currentStep           = currentStep           - oldBeginOfProgramMemory + beginOfProgramMemory;
      firstDisplayedStep    = firstDisplayedStep    - oldBeginOfProgramMemory + beginOfProgramMemory;
      beginOfCurrentProgram = beginOfCurrentProgram - oldBeginOfProgramMemory + beginOfProgramMemory;
      endOfCurrentProgram   = endOfCurrentProgram   - oldBeginOfProgramMemory + beginOfProgramMemory;
    }

    firstFreeProgramByte   += size;
    freeProgramBytes       -= size;
  }


  static bool_t _addEndNeeded(void) {
    if(firstFreeProgramByte <= beginOfProgramMemory) {
      return false;
    }
    if(firstFreeProgramByte == beginOfProgramMemory + 1) {
      return true;
    }
    if(isAtEndOfProgram(firstFreeProgramByte - 2)) {
      return false;
    }
    return true;
  }
#endif // !TESTSUITE_BUILD


    typedef struct {
      char     *itemName;
      uint8_t  _previousNewLine;
      uint8_t  _indent;
      uint8_t  _addnextLineIndent;
    } indentType;

    TO_QSPI const indentType indents[] = {
    //Note that the first condition satisfied, stops the find process
    //"NNNNN" = Text,
    //      A = Newline on previous line 0/1,
    //      B = Indented number of spaces on the current line,
    //      C = Indented number of spaces on the next line.
    //"NNNNN", A, B, C
    {"REM",    0, 0, 0 },
    {"ENDP",   0, 0, 0 },
    {STD_LEFT_SINGLE_QUOTE, 0,0,0},
    //[TEST]
    {"ENTRY?", 0, 0, +2},
    {"KEY?",   0, 0, +2},
    {"LBL?",   0, 0, +2},
    {"STRI?",  0, 0, +2},
    {"CONVG?", 0, 0, +2},
    {"TOP?",   0, 0, +2},
    {"INT?",   0, 0, +2},
    {"EVEN?",  0, 0, +2},
    {"ODD?",   0, 0, +2},
    {"PRIME?", 0, 0, +2},
    {"LEAP?",  0, 0, +2},
    {"FP?",    0, 0, +2},
    {"x* ?",   0, 0, +2},
    {"x** ?",  0, 0, +2},
    {"x=*0?",  0, 0, +2},
    {"SPEC?",  0, 0, +2},
    {"NaN?",   0, 0, +2},
    {"M.SQR?", 0, 0, +2},
    {"MATR?",  0, 0, +2},
    {"CPX?",   0, 0, +2},
    {"REAL?",  0, 0, +2},
    //[LOOP]
    {"DSE",    0, 0, +2},
    {"DSZ",    0, 0, +2},
    {"DSL",    0, 0, +2},
    {"ISE",    0, 0, +2},
    {"ISZ",    0, 0, +2},
    {"ISG",    0, 0, +2},
    //[P.FN1]
    {"LBL",    1,-2,  0},
    {"GTO",    0,-2,  0},
    {"XEQ",    0,-2,  0},
    {"RTN",    0,-2,  0},
    {"END",    0,-2,  0},
    {".END.",  0,-2,  0},

    {"",       0, 0,  0}
    };



#ifndef TESTSUITE_BUILD
#if !defined(SAVE_SPACE_DM42_10)
  static bool_t subStrWildCardCompare(const char *in1, const char *in2) { //wild card is '*', active from the second character being compared
    int16_t i = 0;
    bool_t areEqual = true;
    while(areEqual && in1[i] != 0 && in2[i] != 0) {
      if(!(i >= 1 && (in1[i] == '*' || in2[i] == '*'))) {
        if(in1[i] != in2[i]) {
          areEqual = false;
          return false;
        }
      }
      i++;
    }
    if(in1[i] == 0 && in2[i] != 0) {    //If in1 runs to an end, before in2 is at its end, override the result to FALSE. This is to prevent RE in in1, to falsely agree with REAL?.
      areEqual = false;
    }
    return areEqual;
  }


//uses tmpString
static int16_t findIndents(bool_t *newLine, int8_t *indent, int8_t *addnextLineIndent){
        int16_t jj = 0;
        while((indents[jj].itemName[0]) != 0) {
          if(subStrWildCardCompare(tmpString, (char*)(indents[jj].itemName))) {
            *newLine = (bool_t)(indents[jj]._previousNewLine);
            *indent += indents[jj]._indent;
            *addnextLineIndent = indents[jj]._addnextLineIndent;
            break;
          }
          jj++;
        }
        return jj;
      }

#endif //SAVE_SPACE_DM42_10
#endif //TESTSUITE_BUILD


void fnPExport(void) {
#if !defined(SAVE_SPACE_DM42_10)
  #if !defined(TESTSUITE_BUILD)
    ///////////////////////////////////////////////////////////////////////////////////////
    // For details, see fnPem(). This is a modified copy.
    //
    uint16_t line, firstLine;
    uint8_t *step, *nextStep;
    uint16_t numberOfSteps = getNumberOfSteps();
    char asciiString[448];           //cannot use errorMessage buffer in disk write operations
                                     //solution is to use a local variable of sufficient length to contain a step string.

    firstDisplayedLocalStepNumber = 0;
    defineFirstDisplayedStep();

    step                     = firstDisplayedStep;
    programListEnd           = false;
    lastProgramListEnd       = false;

    if(firstDisplayedLocalStepNumber == 0) {
      sprintf(tmpString, "0000: { Prgm #%" PRIu16 "/%" PRIu16 ": %" PRIu32 " bytes / %" PRIu16 " step%s }", currentProgramNumber, numberOfPrograms, _getProgramSize(), numberOfSteps, numberOfSteps == 1 ? "" : "s");
      stringCopy(tmpString + stringByteLength(tmpString), " \\par");
      stringCopy(tmpString + stringByteLength(tmpString), "\n");
      ioFileWrite(tmpString, strlen(tmpString));
      firstLine = 1;
    }
    else {
      firstLine = 0;
    }

    int lineOffset = 0, lineOffsetTam = 0;
    int8_t  indent;;
    bool_t  newLine;
    int8_t  addnextLineIndent = 0;
    int16_t lastCommandFound = 0;


    for(line=firstLine; line<9999; line++) {
      nextStep = findNextStep(step);


      //Decode
      decodeOneStep_XPORT(step);
      //printf("§§=%s",tmpString);

      indent = 2;
      newLine = false;
      if(addnextLineIndent == 0) {
        lastCommandFound = findIndents(&newLine, &indent, &addnextLineIndent); //uses tmpString as inpur
      }
      else {
        int8_t rubbish = 0;
        bool_t rubbishb = false;
        if(lastCommandFound != findIndents(&rubbishb, &rubbish, &rubbish)) { //only use the indents if the last two commands are not the same
          indent += addnextLineIndent;
        }
        addnextLineIndent = 0;
      }


//MAKE THIS MORE EFFICIENT!
      //additional indents prepended
      if(indent > 0) {
        uint16_t ii = 0;
        stringCopy(asciiString, tmpString);
        stringCopy(tmpString + indent, asciiString);
        while(ii < indent) {
          tmpString[ii++]= ' ';
        }
      }


      char tmpp[30];
      asciiString[0]=0;

      //Add extra blank line before LBL
      if(newLine){
        stringCopy(asciiString + stringByteLength(asciiString), "\\par\n");
      }

      //Line Number and base indent ==> asciiString
      sprintf(tmpp, "%04d:  " , firstDisplayedLocalStepNumber + line - lineOffset + lineOffsetTam);
      stringCopy(asciiString + stringByteLength(asciiString), tmpp);

      //Add instruction
      stringCopy(asciiString + stringByteLength(asciiString), tmpString);        //add number + instruction: 0000:  1/X

      //Add end line
      stringCopy(asciiString + stringByteLength(asciiString), "\\par\n");

      //Convert unprintable characters
      stringToRTF(asciiString, tmpString);

      ioFileWrite(tmpString, strlen(tmpString));

      if(isAtEndOfProgram(step)) {
        programListEnd = true;
        if(*nextStep == 255 && *(nextStep + 1) == 255) {
          lastProgramListEnd = true;
        }
        break;
      }
      if((*step == 255) && (*(step + 1) == 255)) {
        programListEnd = true;
        lastProgramListEnd = true;
        break;
      }
      step = nextStep;
    }
  #endif // !TESTSUITE_BUILD
#endif // !SAVE_SPACE_DM42_10
}


void _fnExportProgram(ioFilePath_t path) {
    uint32_t programVersion = PROGRAM_VERSION;
    uint32_t exportVersion = EXPORT_VERSION;
    int ret;

    ret = ioFileOpen(path, ioModeWrite);

    if(ret != FILE_OK ) {
      if(ret == FILE_CANCEL ) {
        return;
      }
      else {
        #if !defined(DMCP_BUILD)
          printf("Cannot export program!\n");
        #endif
        displayCalcErrorMessage(ERROR_CANNOT_WRITE_FILE, ERR_REGISTER_LINE, REGISTER_X);
        return;
      }
    }


    //stringCopy(tmpString, "{\\rtf1\\ansi\\ansicpg1252\\deff0\\nouicompat{\\fonttbl{\\f0\\fnil\\fcharset0 C47__StandardFont;}}{\\pard\\sl240\\sa0\\sa200\\slmult1\\f0\\fs24\\lang9\\f0\\loch\n");
    stringCopy(tmpString, "{\\rtf1\\ansi\\ansicpg1252\\deff0\\nouicompat{\\fonttbl{\\f0\\fnil\\fcharset0 C47__StandardFont;}}{\\pard\\sl240\\slmult1\\f0\\fs24\\lang9\\f0\\loch\n");
    ioFileWrite(tmpString, strlen(tmpString));

    // PROGRAM file version
    sprintf(tmpString, "C47 Program file export: Export format version %" PRIu32 ", C47 program version %" PRIu32 ".\n", (uint32_t)exportVersion, (uint32_t)programVersion);
    ioFileWrite(tmpString, strlen(tmpString));

    stringCopy(tmpString, " \\par\n");
    ioFileWrite(tmpString, strlen(tmpString));

    fnPExport();

    stringCopy(tmpString, "}}\n");
    ioFileWrite(tmpString, strlen(tmpString));

    ioFileClose();

}


static void _selectProgram(uint16_t label) {
  #if !defined(TESTSUITE_BUILD)
    dynamicMenuItem = -1;
    // Find program boundaries
    // no argument – need to save current program
    if(label == 0 && !tam.alpha && tam.digitsSoFar == 0) {
        // find the first global label in the current program
        uint16_t currentLabel = 0;
        strcpy(tmpStringLabelOrVariableName, "untitled");
        while(currentLabel < numberOfLabels) {
          if(labelList[currentLabel].program == currentProgramNumber) {
            break;
          }
          currentLabel++;
        }
        // get the first global label name
        while(currentLabel < numberOfLabels) {
          if(labelList[currentLabel].step > 0) {  // global label
            // get current label name (to be used as default file name)
            xcopy(tmpStringLabelOrVariableName, labelList[currentLabel].labelPointer + 1, *(labelList[currentLabel].labelPointer));
            tmpStringLabelOrVariableName[*(labelList[currentLabel].labelPointer)] = 0;
            break;
          }
          currentLabel++;
        }
    }
    // Existing global label
    else if(label >= FIRST_LABEL && label <= LAST_LABEL) {
      fnGoto(label);
      // get current label name (to be used as default file name)
      xcopy(tmpStringLabelOrVariableName, labelList[label - FIRST_LABEL].labelPointer + 1, *(labelList[label - FIRST_LABEL].labelPointer));
      tmpStringLabelOrVariableName[*(labelList[label - FIRST_LABEL].labelPointer)] = 0;
    }
    // Invalid label
    else {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "label %" PRIu16 " is not a global label", label);
        moreInfoOnError("In function fnSaveProgram/fnExportProgram (_selectProgram):", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  #endif // !TESTSUITE_BUILD
}


void _exportProgram(uint16_t label, ioFilePath_t path) {
    const uint16_t savedCurrentLocalStepNumber = currentLocalStepNumber;
    uint16_t savedCurrentProgramNumber = currentProgramNumber;

    #if defined(DMCP_BUILD)
      // Don't pass through if the power is insufficient
      if(power_check_screen()) {
        return;
      }
    #endif // DMCP_BUILD

    _selectProgram(label);
    _fnExportProgram(path);

    currentLocalStepNumber = savedCurrentLocalStepNumber;
    currentProgramNumber = savedCurrentProgramNumber;
    temporaryInformation = TI_SAVED;
}


void fnExportProgram(uint16_t label) {
  #if !defined(TESTSUITE_BUILD)
    _exportProgram(label, ioPathExportRTFProgram);
  #endif // !TESTSUITE_BUILD
}



void _saveProgram(uint16_t label, ioFilePath_t path) {
  #if !defined(TESTSUITE_BUILD)
    uint32_t programVersion = PROGRAM_VERSION;
    //char tmpString[3000];           //The concurrent use of the global tmpString
    //                                //as target does not work while the source is at
    //                                //tmpRegisterString = tmpString + START_REGISTER_VALUE;
    //                                //Temporary solution is to use a local variable of sufficient length for the target.
    uint32_t i;
    int ret;

    #if defined(DMCP_BUILD)
      // Don't pass through if the power is insufficient
      if(power_check_screen()) {
        return;
      }
    #endif // DMCP_BUILD

    const uint16_t savedCurrentLocalStepNumber = currentLocalStepNumber;
    uint16_t savedCurrentProgramNumber = currentProgramNumber;

    _selectProgram(label);

    ret = ioFileOpen(path, ioModeWrite);

    if(ret != FILE_OK ) {
      if(ret == FILE_CANCEL ) {
        return;
      }
      else {
        #if !defined(DMCP_BUILD)
          printf("Cannot save program!\n");
        #endif
        displayCalcErrorMessage(ERROR_CANNOT_WRITE_FILE, ERR_REGISTER_LINE, REGISTER_X);
        return;
      }
    }

    // PROGRAM file version
    sprintf(tmpString, "PROGRAM_FILE_FORMAT\n%" PRIu8 "\n", (uint8_t)BACKUP_FORMAT);
    ioFileWrite(tmpString, strlen(tmpString));
    sprintf(tmpString, "C47_program_file_version\n%" PRIu32 "\n", (uint32_t)programVersion);
    ioFileWrite(tmpString, strlen(tmpString));

    // Program
    size_t currentSizeInBytes = endOfCurrentProgram - ((currentProgramNumber == numberOfPrograms) ? 2 : 0) - beginOfCurrentProgram;
    sprintf(tmpString, "PROGRAM\n%" PRIu32 "\n", (uint32_t)currentSizeInBytes);
    ioFileWrite(tmpString, strlen(tmpString));

    // Save program bytes
    for(i=0; i<currentSizeInBytes; i++) {
      sprintf(tmpString, "%" PRIu8 "\n", beginOfCurrentProgram[i]);
      ioFileWrite(tmpString, strlen(tmpString));
    }
    // If last program in memory then add .END. statement
    if(currentProgramNumber == numberOfPrograms) {
      sprintf(tmpString, "255\n255\n");
      ioFileWrite(tmpString, strlen(tmpString));
    }

    ioFileClose();

    currentLocalStepNumber = savedCurrentLocalStepNumber;
    currentProgramNumber = savedCurrentProgramNumber;

    temporaryInformation = TI_SAVED;
  #endif // !TESTSUITE_BUILD
}


void fnSaveProgram(uint16_t label) {
  #if !defined(TESTSUITE_BUILD)
    _saveProgram(label, ioPathSaveProgram);
  #endif // !TESTSUITE_BUILD
}


void fnSaveAllPrograms(uint16_t unusedButMandatoryParameter) {
  #if defined(PC_BUILD)
    const uint16_t savedCurrentLocalStepNumber = currentLocalStepNumber;
    uint16_t savedCurrentProgramNumber = currentProgramNumber;
    uint16_t oldCurrentProgramNumber;

    uint16_t label;
    char labelName[16];
    char labelName1[500];
        for(int i=0; i<numberOfLabels; i++) {
          if(labelList[i].step > 0) { // Global label
            xcopy(labelName, labelList[i].labelPointer + 1, labelList[i].labelPointer[0]);
            labelName[labelList[i].labelPointer[0]]=0;
            label = findNamedLabel(labelName);
            //printf("#### labelnumber=%i, labelname=%s\n",label, labelName);
            oldCurrentProgramNumber = currentProgramNumber;

            _selectProgram(label);

            stringToASCII(labelName, labelName1);
            //printf("----X %6u ? old=%6u name=%30s  ",currentProgramNumber, oldCurrentProgramNumber, labelName1);
            if(currentProgramNumber != oldCurrentProgramNumber) {
              printf("Export & saving labelnumber %5i in program number %5u: Files %s.p47 %s.rtf\n",label, currentProgramNumber, labelName1, labelName1);
              fflush(stdout);
              _saveProgram  (label, ioPathSaveAllPrograms);
              _exportProgram(label, ioPathExportRTFAllPrograms);
            } else {
              printf("   Not saved: %s is not the first label in program %5u.\n", labelName1, currentProgramNumber);
              fflush(stdout);
            }
          }
        }
    currentLocalStepNumber = savedCurrentLocalStepNumber;
    currentProgramNumber = savedCurrentProgramNumber;
  #endif //PC_BUILD
}


void fnLoadProgram(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    ioFilePath_t path;
    uint32_t pgmSizeInByte;
    uint32_t i;
    uint8_t *startOfProgram;
    int ret;

    path = ioPathLoadProgram;
    ret = ioFileOpen(path, ioModeRead);

    if(ret != FILE_OK ) {
      if(ret == FILE_CANCEL ) {
        return;
      }
      else {
        displayCalcErrorMessage(ERROR_CANNOT_READ_FILE, ERR_REGISTER_LINE, REGISTER_X);
        return;
      }
    }

    //Check save file version
    uint32_t loadedVersion = 0;
    readLine(tmpString);
    if(strcmp(tmpString, "PROGRAM_FILE_FORMAT") == 0) {
      readLine(aimBuffer); // Format of program instructions (ignore now, there is only one format)
    }
    else {
      #if !defined(TESTSUITE_BUILD)
        sprintf(tmpString," \nThis is not a C47 program\n\nIt will not be loaded.");
        show_warning(tmpString);
      #endif // TESTSUITE_BUILD
      ioFileClose();
      return;
    }
    readLine(aimBuffer); // param
    readLine(tmpString); // value
    if(strcmp(aimBuffer, "C47_program_file_version") == 0) {
      loadedVersion = stringToUint32(tmpString);
      if(loadedVersion < OLDEST_COMPATIBLE_PROGRAM_VERSION) { // Program incompatibility
        #if !defined(TESTSUITE_BUILD)
          sprintf(tmpString, " \n   !!! Program version is too old !!!\nNot compatible with current version\n \nIt will not be loaded.");
          show_warning(tmpString);
        #endif // TESTSUITE_BUILD
        ioFileClose();
        return;
      }
    }
    else {
      if(strcmp(aimBuffer, "WP43_program_file_version") == 0) {
        loadedVersion = stringToUint32(tmpString);
        #if !defined(TESTSUITE_BUILD)
          sprintf(tmpString," \nThis is a WP43 program\nWP43 program support is experimental\nSome instructions may not be \ncompatible with the C47 and may\ncrash the calculator.");
          show_warning(tmpString);
        #endif // TESTSUITE_BUILD
      }
      else {
        #if !defined(TESTSUITE_BUILD)
          sprintf(tmpString, " \nThis is not a C47 program\n \nIt will not be loaded.");
          show_warning(tmpString);
        #endif // TESTSUITE_BUILD
        ioFileClose();
        return;
      }
    }
    readLine(aimBuffer); // param
    readLine(tmpString); // value
    if(strcmp(aimBuffer, "PROGRAM") == 0) {
      pgmSizeInByte = stringToUint32(tmpString);
    }
    else {
      ioFileClose();
      return;
    }

    if(_addEndNeeded()) {
      _addSpaceAfterPrograms(2);
      *(firstFreeProgramByte - 2) = (ITM_END >> 8) | 0x80;
      *(firstFreeProgramByte - 1) =  ITM_END       & 0xff;
      *(firstFreeProgramByte    ) = 0xffu;
      *(firstFreeProgramByte + 1) = 0xffu;
      scanLabelsAndPrograms();
    }

    _addSpaceAfterPrograms(pgmSizeInByte);
    startOfProgram = firstFreeProgramByte - pgmSizeInByte;
    for(i=0; i<pgmSizeInByte; i++) {
      readLine(tmpString); // One byte
      startOfProgram[i] = stringToUint8(tmpString);
    }

    *(firstFreeProgramByte    ) = 0xffu;
    *(firstFreeProgramByte + 1) = 0xffu;
    scanLabelsAndPrograms();

    goToGlobalStep(programList[numberOfPrograms - 1].step);   // Set active program to the loaded program and display the first step

    ioFileClose();

    if(loadedVersion < OLDEST_COMPATIBLE_PROGRAM_VERSION) { // Program incompatibility
      sprintf(tmpString," \n"
                        "   !!! Program version is too old !!!\n"
                        "Not compatible with current version\n"
                        " \n"
                        "It will not be loaded.");
      #if !defined(TESTSUITE_BUILD)
        show_warning(tmpString);
      #endif // TESTSUITE_BUILD
      return;
    }

    temporaryInformation = TI_PROGRAM_LOADED;
  #endif // !TESTSUITE_BUILD
}

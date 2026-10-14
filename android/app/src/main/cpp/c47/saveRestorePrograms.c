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
//  |  3  |"C47_program_file_version" |
//  |  4  |   <program file version>  |
//  |  5  |         "PROGRAM"         |
//  |  6  |       <program size>      |
//  |  7  |    <first program byte>   |
//  | ... |            ...            |
//  |  n  |    <last program byte>    |
//  +-----+---------------------------+
//



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

  static const uint8_t *_findLastStep(void) {
    uint8_t *step = programList[numberOfPrograms - 1].instructionPointer;
    while(!(isAtEndOfProgram(step) || isAtEndOfPrograms(step))) { // END or .END.
      step = findNextStep(step);
    }
    return step;
  }

  static bool_t _addEndNeeded(void) {
    if(firstFreeProgramByte <= beginOfProgramMemory) {
      return false;
    }
    if(firstFreeProgramByte == beginOfProgramMemory + 1) {
      return true;
    }
    if(isAtEndOfProgram(_findLastStep())) {
      return false;
    }
    return true;
  }

  static bool_t _delEndNeeded(void) {
    if(firstFreeProgramByte <= beginOfProgramMemory) {
      return false;
    }
    if(isAtEndOfProgram(programList[numberOfPrograms - 1].instructionPointer)) {  // the last program's own first step. A step ending 133 178 not taken for a 2nd END
      return true;
    }
    return false;
  }


  // First-pass screen of a program file, before anything is loaded, using the grammar tables paramTailBytes() and literalTailBytes() in programming/nextStep.c.
  // It refuses a declared label name longer than MAX_LABEL_NAME_LENGTH, and any non-item opcode, which the in-memory walker decodes as a zero-parameter step,
  // so quitting silently there would let a crafted file hide an overlong label behind one. A step the walker cannot otherwise decode ends the screening,
  // without refusing, because scanLabelsAndPrograms() truncates the program area there anyway.

  static bool_t _readFileProgramByte(uint32_t *remaining, uint8_t *value) {
    if(*remaining == 0) {
      return false;
    }
    readLine(tmpString, TMP_STR_LENGTH);
    *value = stringToUint8(tmpString);
    (*remaining)--;
    return true;
  }


  static bool_t _skipFileProgramBytes(uint32_t *remaining, uint32_t count) {
    uint8_t value;
    while(count-- > 0) {
      if(!_readFileProgramByte(remaining, &value)) {
        return false;
      }
    }
    return true;
  }


  // Consumes one step; returns false at end of stream, on an undecodable step, or with *refuse set.
  // allowKeyParam limits the embedded second parameter of KEY and 42KEY to one level, exactly like findNextStep().
  static bool_t _screenFileStep(uint32_t *remaining, bool_t *refuse, bool_t allowKeyParam) {
    uint8_t byte, length;
    uint16_t op;
    int16_t tail;
    bool_t declaredLabel = false;

    if(!_readFileProgramByte(remaining, &byte)) {
      return false;
    }
    op = byte;
    if(byte & 0x80) {
      if(!_readFileProgramByte(remaining, &byte)) {
        return false;
      }
      op = (uint16_t)((op & 0x7f) << 8) | byte;
    }
    if(op == 0x7fff) { // .END.
      return false;
    }
    if(op >= LAST_ITEM) { // non-item opcode: only a corrupt or crafted file contains one
      *refuse = true;
      return false;
    }

    uint16_t ptp = indexOfItems[op].status & PTP_STATUS;
    if(ptp == PTP_NONE || ptp == PTP_DISABLED) {
      tail = 0;
    }
    else if(ptp == PTP_LITERAL || ptp == PTP_REM) {
      if(!_readFileProgramByte(remaining, &byte)) { // literal type
        return false;
      }
      tail = literalTailBytes(byte);
      if(tail == PARAM_TAIL_BASE_LENGTH_PREFIXED) {
        if(!_readFileProgramByte(remaining, &byte)) { // base
          return false;
        }
        tail = PARAM_TAIL_LENGTH_PREFIXED;
      }
    }
    else {
      uint16_t paramMode = (ptp == PTP_KEYG_KEYX) ? PARAM_NUMBER_8 : ptp >> 9;
      if(!_readFileProgramByte(remaining, &byte)) { // first parameter byte
        return false;
      }
      tail = paramTailBytes(paramMode, op, byte);
      declaredLabel = (paramMode == PARAM_DECLARE_LABEL); // its length-prefixed tails are exactly the named-label forms
    }

    bool_t ok;
    if(tail == PARAM_TAIL_INVALID) {
      return false;
    }
    else if(tail == PARAM_TAIL_LENGTH_PREFIXED) {
      if(!_readFileProgramByte(remaining, &length)) {
        return false;
      }
      if(declaredLabel && length > MAX_LABEL_NAME_LENGTH) { // the check this screen exists for
        *refuse = true;
        return false;
      }
      ok = _skipFileProgramBytes(remaining, length);
    }
    else {
      ok = _skipFileProgramBytes(remaining, (uint32_t)tail);
    }
    if(ok && allowKeyParam && (op == ITM_KEY || op == ITM_42KEY)) {
      return _screenFileStep(remaining, refuse, false);
    }
    return ok;
  }


  static bool_t _programFileRefused(uint32_t pgmSizeInByte) {
    uint32_t remaining = pgmSizeInByte;
    bool_t refuse = false;
    while(remaining > 0 && _screenFileStep(&remaining, &refuse, true)) {
    }
    return refuse;
  }


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
    //"NNNNN", A,  B,  C
    {"REM",    0,  0,  0 },
    {"ENDP",   0,  0,  0 },
    {STD_LEFT_SINGLE_QUOTE, 0, 0, 0},
    //[TEST]
    {"ENTRY?", 0,  0, +2},
    {"KEY?",   0,  0, +2},
    {"LBL?",   0,  0, +2},
    {"STRI?",  0,  0, +2},
    {"CONVG?", 0,  0, +2},
    {"TOP?",   0,  0, +2},
    {"INT?",   0,  0, +2},
    {"EVEN?",  0,  0, +2},
    {"ODD?",   0,  0, +2},
    {"PRIME?", 0,  0, +2},
    {"LEAP?",  0,  0, +2},
    {"FP?",    0,  0, +2},
    {"x* ?",   0,  0, +2},
    {"x** ?",  0,  0, +2},
    {"x=*0?",  0,  0, +2},
    {"SPEC?",  0,  0, +2},
    {"NaN?",   0,  0, +2},
    {"M.SQR?", 0,  0, +2},
    {"MATR?",  0,  0, +2},
    {"CPX?",   0,  0, +2},
    {"REAL?",  0,  0, +2},
    //[LOOP]
    {"DSE",    0,  0, +2},
    {"DSZ",    0,  0, +2},
    {"DSL",    0,  0, +2},
    {"ISE",    0,  0, +2},
    {"ISZ",    0,  0, +2},
    {"ISG",    0,  0, +2},
    //[STRUCT]
#if defined(OPTION_STRUCT_INDENT)
    {"IF",     0,  0,  0}, // an opener stands at the column of the step above it and its body two columns in; see structLevel in fnPExport below
    {"ELSE",   0, -2,  0}, // ELSE and WHILE stand on their opener's column while the count still holds the body in
    {"ENDIF",  0,  0,  0},
    {"ENDDO",  0,  0,  0}, // before END below, which it would otherwise match
    {"WHILE",  0, -2,  0},
    {"DOT",    0,  0,  0}, // the dot product, listed here only so the DO row below does not claim it: the compare takes a prefix
    {"DO",     0,  0,  0},
    {"NEXTP",  0,  0,  0}, // the next prime, listed here only so the NEXT row below does not claim it, the same prefix case as DOT
    {"NEXT",   0,  0,  0},
    {"FOR",    0,  0,  0},
    {"REPEAT", 0,  0,  0},
    {"UNTIL",  0,  0,  0},
#endif // OPTION_STRUCT_INDENT
    //[P.FN1]
    {"LBL",    1, -2,  0},
#if defined(OPTION_STRUCT_INDENT)
    {"GTO",    0,  0,  0},
    {"XEQ",    0,  0,  0},
    {"RTN",    0,  0,  0},
#else // OPTION_STRUCT_INDENT
    {"GTO",    0, -2,  0},
    {"XEQ",    0, -2,  0},
    {"RTN",    0, -2,  0},
#endif // OPTION_STRUCT_INDENT
    {"END",    0, -2,  0},
    {".END.",  0, -2,  0},

    {"",       0,  0,  0}
    };



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



void fnPExport(void) {
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
    int8_t  indent;
    bool_t  newLine;
    int8_t  addnextLineIndent = 0;
    #if defined(OPTION_STRUCT_INDENT)
      int8_t  structLevel = 0;
    #endif // OPTION_STRUCT_INDENT
    int16_t lastCommandFound = 0;


    for(line=firstLine; line<9999; line++) {
      nextStep = findNextStep(step);


      //Decode
      decodeOneStep_XPORT(step);
      //printf("§§=%s",tmpString);

      #if defined(OPTION_STRUCT_INDENT)
        indent = XPORTP_STRUCT_INDENT; // every step hangs one indent in; only the first LBL, END and .END. take the outdent below and stand on the left margin
      #else // OPTION_STRUCT_INDENT
        indent = 2;
      #endif // OPTION_STRUCT_INDENT
      newLine = false;
      #if defined(OPTION_STRUCT_INDENT)
        if(structStepClosesIndent(step) && structLevel > 0) {
          structLevel--;         // the body ends here, so the closer prints on the column its opener printed on
        }
        if(addnextLineIndent != 0 && structDisplayOutdent(step)) {
          addnextLineIndent = 0; // the STRUCT tokens take their own column even directly after a test; asked by opcode, since a name compare here is a prefix match
        }
      #endif // OPTION_STRUCT_INDENT
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
      #if defined(OPTION_STRUCT_INDENT)
        if(line != 1 && checkOpCodeOfStep(step, ITM_LBL)) {
          indent += XPORTP_STRUCT_INDENT; // only the first label of the program stands on the left margin; every label below it hangs with the ordinary steps
        }
        indent += XPORTP_STRUCT_INDENT * (structLevel > XPORTP_MAX_STRUCT_LEVELS ? XPORTP_MAX_STRUCT_LEVELS : structLevel);
        if(structStepOpensIndent(step)) {
          structLevel++;         // the body steps below stand one level further in
        }
      #endif // OPTION_STRUCT_INDENT


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
        displayCalcErrorMessage(ERROR_CANNOT_WRITE_FILE, ERR_REGISTER_LINE);
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
            uint8_t lblNameLen = boundProgramNameLength(labelList[currentLabel].labelPointer + 1, *(labelList[currentLabel].labelPointer));
            xcopy(tmpStringLabelOrVariableName, labelList[currentLabel].labelPointer + 1, lblNameLen);
            tmpStringLabelOrVariableName[lblNameLen] = 0;
            break;
          }
          currentLabel++;
        }
    }
    // Existing global label
    else if(label >= FIRST_LABEL && label <= LAST_LABEL) {
      fnGoto(label);
      // get current label name (to be used as default file name)
      uint8_t lblNameLen = boundProgramNameLength(labelList[label - FIRST_LABEL].labelPointer + 1, *(labelList[label - FIRST_LABEL].labelPointer));
      xcopy(tmpStringLabelOrVariableName, labelList[label - FIRST_LABEL].labelPointer + 1, lblNameLen);
      tmpStringLabelOrVariableName[lblNameLen] = 0;
    }
    // Invalid label
    else {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "label %" PRIu16 " is not a global label", label);
        moreInfoOnError("In function fnSaveProgram/fnExportProgram (_selectProgram):", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
}


// _selectProgram() uses fnGoto(), which moves the step, the step number, the program number, the displayed step, its number,
// and the program bounds. This restores it for WRITEP and XPORTP
typedef struct {
  uint8_t *step;
  uint8_t *firstDisplayed;
  uint16_t localStepNumber;
  uint16_t firstDisplayedLocalStepNumber;
  uint16_t programNumber;
} editorPosition_t;

static void _saveEditorPosition(editorPosition_t *position) {
  position->step                          = currentStep;
  position->firstDisplayed                = firstDisplayedStep;
  position->localStepNumber               = currentLocalStepNumber;
  position->firstDisplayedLocalStepNumber = firstDisplayedLocalStepNumber;
  position->programNumber                 = currentProgramNumber;
}

static void _restoreEditorPosition(const editorPosition_t *position) {
  currentStep = position->step;
  defineCurrentProgramFromCurrentStep(); // the program bounds follow the step
  currentLocalStepNumber        = position->localStepNumber;
  currentProgramNumber          = position->programNumber;
  firstDisplayedStep            = position->firstDisplayed;
  firstDisplayedLocalStepNumber = position->firstDisplayedLocalStepNumber;
}


void _exportProgram(uint16_t label, ioFilePath_t path) {
    editorPosition_t savedPosition;
    if(calcMode == CM_PEM) { // _selectProgram below reaches fnGoto, which inserts a GTO step rather than moving while the editor is open
      displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE);
      return;
    }
    _saveEditorPosition(&savedPosition);

    #if defined(DMCP_BUILD)
      // Don't pass through if the power is insufficient
      bool_t lowPower = power_check_screen();
      screenUpdatingMode = SCRUPD_AUTO;
      if(lowPower) {
        return;
      }
    #endif // DMCP_BUILD

    _selectProgram(label);

    // A program holding a 0 can be neither stored nor run; VALID puts the numbers in. The developer bulk export writes whatever is in memory and is
    // not the user exporting one program, so it is not refused.
    if(path != ioPathExportRTFAllPrograms && structProgramHasUnnumbered()) {
      displayCalcErrorMessage(ERROR_STRUCTURE_NOT_NUMBERED, ERR_REGISTER_LINE);
      _restoreEditorPosition(&savedPosition);
      return;
    }

    if((getSystemFlag(FLAG_PRTACT)) && (lastFunc == ITM_PRINTERPROG)) {     // If printer active and command is to print program then print to IR printer
    #if defined(OPTION_IR_PRINTING)
      printProgram(PROG, 0);
      temporaryInformation = TI_PRINT_COMPLETE;
    #endif //OPTION_IR_PRINTING
    }
    else {                                                                  // else print to file
      _fnExportProgram(path);
      temporaryInformation = TI_SAVED;
    }

    _restoreEditorPosition(&savedPosition);
}


void fnExportProgram(uint16_t label) {
  _exportProgram(label, ioPathExportRTFProgram);
}



void _saveProgram(uint16_t label, ioFilePath_t path) {
    if(calcMode == CM_PEM) { // as in _exportProgram above: _selectProgram reaches fnGoto, which inserts a GTO step while the editor is open
      displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE);
      return;
    }
    uint32_t programVersion = PROGRAM_VERSION;
    //char tmpString[3000];           //The concurrent use of the global tmpString
    //                                //as target does not work while the source is at
    //                                //tmpRegisterString = tmpString + START_REGISTER_VALUE;
    //                                //Temporary solution is to use a local variable of sufficient length for the target.
    uint32_t i;
    int ret;

    #if defined(DMCP_BUILD)
      // Don't pass through if the power is insufficient
      bool_t lowPower = power_check_screen();
      screenUpdatingMode = SCRUPD_AUTO;
      if(lowPower) {
        return;
      }
    #endif // DMCP_BUILD

    editorPosition_t savedPosition;
    _saveEditorPosition(&savedPosition);

    _selectProgram(label);

    // A program holding a 0 can be neither stored nor run; VALID puts the numbers in. The developer bulk export writes whatever is in memory and is
    // not the user storing one program, so it is not refused.
    if(path != ioPathSaveAllPrograms && structProgramHasUnnumbered()) {
      displayCalcErrorMessage(ERROR_STRUCTURE_NOT_NUMBERED, ERR_REGISTER_LINE);
      _restoreEditorPosition(&savedPosition);
      return;
    }

    ret = ioFileOpen(path, ioModeWrite);

    if(ret != FILE_OK ) {
      if(ret == FILE_CANCEL ) {
        _restoreEditorPosition(&savedPosition);
        return;
      }
      else {
        #if !defined(DMCP_BUILD)
          printf("Cannot save program!\n");
        #endif
        displayCalcErrorMessage(ERROR_CANNOT_WRITE_FILE, ERR_REGISTER_LINE);
        _restoreEditorPosition(&savedPosition);
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

    _restoreEditorPosition(&savedPosition);

    temporaryInformation = TI_SAVED;
}


void fnSaveProgram(uint16_t label) {
  _saveProgram(label, ioPathSaveProgram);
}


void fnSaveAllPrograms(uint16_t unusedButMandatoryParameter) {
  if(calcMode == CM_PEM) { // it selects each program itself, so it needs the same refusal the two above make
    displayCalcErrorMessage(ERROR_OPERATION_UNDEFINED, ERR_REGISTER_LINE);
    return;
  }
  #if defined(PC_BUILD)
    const uint16_t savedCurrentLocalStepNumber = currentLocalStepNumber;
    uint16_t savedCurrentProgramNumber = currentProgramNumber;
    uint16_t oldCurrentProgramNumber = 0;

    uint16_t label;
    char labelName[256]; // a global label name is a 1-byte-length string, so up to 255 bytes
    char labelName1[500];
        for(int i=0; i<numberOfLabels; i++) {
          if(labelList[i].step > 0) { // Global label
            uint8_t nameLength = boundProgramNameLength(labelList[i].labelPointer + 1, labelList[i].labelPointer[0]);
            xcopy(labelName, labelList[i].labelPointer + 1, nameLength);
            labelName[nameLength]=0;
            label = findNamedLabel(labelName, GLOBAL_LABELS);
            _selectProgram(label);
            stringToASCII(labelName, labelName1);
            //printf("#### labelNo=%5i, labelName=%11s, ----Pr no=%6u, old Pr no=%6u name=%30s,  ",label, labelName, currentProgramNumber, oldCurrentProgramNumber, labelName1);
            if(currentProgramNumber != oldCurrentProgramNumber) {
              printf("Export & saving labelnumber %5i in program number %5u: Files %s.p47 %s.rtf\n", label, currentProgramNumber, labelName1, labelName1);
              fflush(stdout);
              _saveProgram  (label, ioPathSaveAllPrograms);
              _exportProgram(label, ioPathExportRTFAllPrograms);
            }
            else {
              printf("   Not saved: %s is not the first label in program %5u.\n", labelName1, currentProgramNumber);
              fflush(stdout);
            }
            oldCurrentProgramNumber = currentProgramNumber;
          }
        }
    currentLocalStepNumber = savedCurrentLocalStepNumber;
    currentProgramNumber = savedCurrentProgramNumber;
  #endif //PC_BUILD
}


void fnLoadProgram(uint16_t unusedButMandatoryParameter) {
    ioFilePath_t path;
    uint32_t pgmSizeInByte;
    uint32_t i;
    uint8_t *startOfProgram;
    int ret;

    temporaryInformation = TI_NO_INFO; // only a completed load sets TI_PROGRAM_LOADED, so a refusal cannot leave the previous load's value standing.
    path = ioPathLoadProgram;
    ret = ioFileOpen(path, ioModeRead);

    if(ret != FILE_OK ) {
      if(ret == FILE_CANCEL ) {
        return;
      }
      else {
        displayCalcErrorMessage(ERROR_CANNOT_READ_FILE, ERR_REGISTER_LINE);
        return;
      }
    }

    //Check save file version
    uint32_t loadedVersion = 0;
    readLine(tmpString, TMP_STR_LENGTH);
    if(strcmp(tmpString, "PROGRAM_FILE_FORMAT") == 0) {
      readLine(aimBuffer, AIM_BUFFER_LENGTH); // Format of program instructions (ignore now, there is only one format)
    }
    else {
      sprintf(tmpString, " \nThis is not a C47 program\n\nIt will not be loaded.");
      show_warning(tmpString);
      ioFileClose();
      return;
    }
    readLine(aimBuffer, AIM_BUFFER_LENGTH); // param
    readLine(tmpString, TMP_STR_LENGTH); // value
    if(strcmp(aimBuffer, "C47_program_file_version") == 0) {
      loadedVersion = stringToUint32(tmpString);
      if(loadedVersion < OLDEST_COMPATIBLE_PROGRAM_VERSION) { // Program incompatibility
        sprintf(tmpString, " \n   !!! Program version is too old !!!\nNot compatible with current version\n \nIt will not be loaded.");
        show_warning(tmpString);
        ioFileClose();
        return;
      }
    }
    else {
      if(strcmp(aimBuffer, "WP43_program_file_version") == 0) {
        loadedVersion = stringToUint32(tmpString);
        sprintf(tmpString, " \nThis is a WP43 program\nWP43 program support is experimental\nSome instructions may not be \ncompatible with the C47 and may\ncrash the calculator.");
        show_warning(tmpString);
      }
      else {
        sprintf(tmpString, " \nThis is not a C47 program\n \nIt will not be loaded.");
        show_warning(tmpString);
        ioFileClose();
        return;
      }
    }
    readLine(aimBuffer, AIM_BUFFER_LENGTH); // param
    readLine(tmpString, TMP_STR_LENGTH); // value
    if(strcmp(aimBuffer, "PROGRAM") == 0) {
      pgmSizeInByte = stringToUint32(tmpString);
    }
    else {
      ioFileClose();
      return;
    }

    // Refuse what cannot fit, before reserving anything. The second bound is the program area's own accounting: freeProgramBytes is a uint16_t,
    // and _addSpaceAfterPrograms() takes a uint16_t size, so a larger claim would reserve only its low 16 bits while the read loop writes them all.
    if((uint64_t)pgmSizeInByte + 2 > (uint64_t)freeProgramBytes + getFreeRamMemory() || (uint64_t)pgmSizeInByte + 2 > UINT16_MAX) {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      ioFileClose();
      return;
    }
    // First pass: screen the file before loading anything, so a refusal needs no rollback.
    if(_programFileRefused(pgmSizeInByte)) {
      displayCalcErrorMessage(ERROR_INVALID_CORRUPTED_DATA, ERR_REGISTER_LINE);
      ioFileClose();
      return;
    }
    ioFileSeek(0);
    for(i=0; i<6; i++) { // skip the header lines the screening pass already validated
      readLine(tmpString, TMP_STR_LENGTH);
    }

    if(_addEndNeeded()) {
      _addSpaceAfterPrograms(2);
      *(firstFreeProgramByte - 2) = (ITM_END >> 8) | 0x80;
      *(firstFreeProgramByte - 1) =  ITM_END       & 0xff;
      *(firstFreeProgramByte    ) = 0xffu;
      *(firstFreeProgramByte + 1) = 0xffu;
      scanLabelsAndPrograms();
    }
    if(_delEndNeeded()) {
      firstFreeProgramByte -= 2;
      freeProgramBytes += 2;
      *(firstFreeProgramByte    ) = 0xffu;
      *(firstFreeProgramByte + 1) = 0xffu;
    }

    _addSpaceAfterPrograms(pgmSizeInByte);
    startOfProgram = firstFreeProgramByte - pgmSizeInByte;
    for(i=0; i<pgmSizeInByte; i++) {
      readLine(tmpString, TMP_STR_LENGTH); // One byte
      startOfProgram[i] = stringToUint8(tmpString);
    }

    *(firstFreeProgramByte    ) = 0xffu;
    *(firstFreeProgramByte + 1) = 0xffu;
    scanLabelsAndPrograms();

    dynamicMenuItem = -1;
    goToGlobalStep(programList[numberOfPrograms - 1].step);   // Set active program to the loaded program and display the first step

    ioFileClose();

    if(loadedVersion < OLDEST_COMPATIBLE_PROGRAM_VERSION) { // Program incompatibility
      sprintf(tmpString, " \n"
                         "   !!! Program version is too old !!!\n"
                         "Not compatible with current version\n"
                         " \n"
                         "It will not be loaded.");
      show_warning(tmpString);
      return;
    }

    temporaryInformation = TI_PROGRAM_LOADED;

    #if defined(OPTION_STRUCTURED_PGM)
      if(getSystemFlag(FLAG_AUTOVALID)) {
        fnValid(NOPARAM); // the load leaves its own program open, so the setting numbers and checks it here, as it does on leaving the program editor
        if(lastErrorCode == ERROR_NONE) { // VALID clears the confirmation as it leaves, so put it back; where it found a fault that message stands instead
          temporaryInformation = TI_PROGRAM_LOADED;
        }
      }
    #endif // OPTION_STRUCTURED_PGM
}

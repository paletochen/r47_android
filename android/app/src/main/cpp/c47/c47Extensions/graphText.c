// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

//#define DISPLOADING

void preventFilenameTimeout(void){
  uint32_t tmp__32;
  tmp__32 = getUptimeMs();
  mem__32 = tmp__32;
  cancelFilename = false;
}

#if defined(DMCP_BUILD)
  /*-DMCP-*/     typedef struct {              //JM VALUES DEMO
  /*-DMCP-*/       uint8_t  count;
  /*-DMCP-*/       char     itemName[40];
  /*-DMCP-*/     } nstr;
  /*-DMCP-*/     TO_QSPI const nstr IOMsgs[] = {
  /*-DMCP-*/       { 0,  "" },
  /*-DMCP-*/       { 1,  "Write error ID001--> " },
  /*-DMCP-*/       { 2,  "File open error ID002--> " },
  /*-DMCP-*/       { 3,  "Seek error ID003--> " },
  /*-DMCP-*/       { 4,  "Write error ID004--> " },
  /*-DMCP-*/       { 5,  "Close error ID005--> " },
  /*-DMCP-*/       { 6,  "Not found ID006 --> " },
  /*-DMCP-*/       { 7,  "No path ID007 --> " },
  /*-DMCP-*/       { 8,  ". Using fallback." },
  /*-DMCP-*/       { 9,  "ERROR too long file using fallback" },
  /*-DMCP-*/       {100,"Msg List"}
  /*-DMCP-*/     };
  //###################################################################################
  /*-DMCP-*/ TCHAR *f_gets(TCHAR* buff /* Pointer to the buffer to store read string */, int len /* Size of string buffer (items) */, FIL* fp /* Pointer to the file object */) {
  /*-DMCP-*/   int nc = 0;
  /*-DMCP-*/   TCHAR *p = buff;
  /*-DMCP-*/   BYTE s[4];
  /*-DMCP-*/   UINT rc;
  /*-DMCP-*/   DWORD dc;
  /*-DMCP-*/
  /*-DMCP-*/   // Byte-by-byte read without any conversion (ANSI/OEM API)
  /*-DMCP-*/   len -= 1; // Make a room for the terminator
  /*-DMCP-*/   while(nc < len) {
  /*-DMCP-*/     f_read(fp, s, 1, &rc);  // Get a byte
  /*-DMCP-*/     if(rc != 1) { // EOF?
  /*-DMCP-*/       break;
  /*-DMCP-*/     }
  /*-DMCP-*/     dc = s[0];
  /*-DMCP-*/     if(dc == '\r') {
  /*-DMCP-*/       continue;
  /*-DMCP-*/     }
  /*-DMCP-*/     *p++ = (TCHAR)dc;
  /*-DMCP-*/     nc++;
  /*-DMCP-*/     if(dc == '\n') {
  /*-DMCP-*/       break;
  /*-DMCP-*/     }
  /*-DMCP-*/   }
  /*-DMCP-*/
  /*-DMCP-*/   *p = 0; // Terminate the string
  /*-DMCP-*/   return (nc ? buff : 0); // When no data read due to EOF or error, return with error.
  /*-DMCP-*/ }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/ TCHAR *f_getsline (TCHAR* buff /* Pointer to the buffer to store read string */, int len /* Size of string buffer (items) */, FIL* fp /* Pointer to the file object */) {
  /*-DMCP-*/   int nc = 0;
  /*-DMCP-*/   TCHAR *p = buff;
  /*-DMCP-*/   BYTE s[2];
  /*-DMCP-*/   UINT rc;
  /*-DMCP-*/   BYTE sc;
  /*-DMCP-*/   s[1] = 0;
  /*-DMCP-*/       // Byte-by-byte read without any conversion (ANSI/OEM API)
  /*-DMCP-*/   len -= 1; // Make a room for the terminator
  /*-DMCP-*/   while(nc < len) {
  /*-DMCP-*/     f_read(fp, s, 1, &rc);  // Get a byte
  /*-DMCP-*/     if(rc != 1) { // EOF?
  /*-DMCP-*/       break;
  /*-DMCP-*/     }
  /*-DMCP-*/     sc = s[0];
  /*-DMCP-*/     *p++ = (TCHAR)sc;
  /*-DMCP-*/     nc++;
  /*-DMCP-*/   }
  /*-DMCP-*/
  /*-DMCP-*/   *p = 0; // Terminate the string
  /*-DMCP-*/   return (nc ? buff : 0); // When no data read due to EOF or error, return with error.
  /*-DMCP-*/ }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/ /*-----------------------------------------------------------------------*/
  /*-DMCP-*/ /* Put a Character to the File (sub-functions)                           */
  /*-DMCP-*/ /*-----------------------------------------------------------------------*/
  /*-DMCP-*/ // Putchar output buffer and work area
  /*-DMCP-*/ typedef struct {
  /*-DMCP-*/   FIL *fp;       // Ptr to the writing file
  /*-DMCP-*/   int idx, nchr; // Write index of buf[] (-1:error), number of encoding units written
  /*-DMCP-*/   BYTE buf[64];  // Write buffer
  /*-DMCP-*/ } putbuff;
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/ // Buffered write with code conversion
  /*-DMCP-*/ static void putc_bfd(putbuff* pb, TCHAR c) {
  /*-DMCP-*/   UINT n;
  /*-DMCP-*/   int i, nc;
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/   if(c == '\n') {    // LF -> CRLF conversion
  /*-DMCP-*/     putc_bfd(pb, '\r');
  /*-DMCP-*/   }
  /*-DMCP-*/
  /*-DMCP-*/   i = pb->idx;        // Write index of pb->buf[]
  /*-DMCP-*/   if(i < 0) {
  /*-DMCP-*/     return;
  /*-DMCP-*/   }
  /*-DMCP-*/   nc = pb->nchr;      // Write unit counter
  /*-DMCP-*/
  /*-DMCP-*/   // ANSI/OEM input (without re-encoding)
  /*-DMCP-*/   pb->buf[i++] = (BYTE)c;
  /*-DMCP-*/
  /*-DMCP-*/   if(i >= (int)(sizeof pb->buf) - 4) { // Write buffered characters to the file
  /*-DMCP-*/     f_write(pb->fp, pb->buf, (UINT)i, &n);
  /*-DMCP-*/     i = (n == (UINT)i ? 0 : -1);
  /*-DMCP-*/   }
  /*-DMCP-*/   pb->idx = i;
  /*-DMCP-*/   pb->nchr = nc + 1;
  /*-DMCP-*/ }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/ // Flush remaining characters in the buffer
  /*-DMCP-*/ static int putc_flush(putbuff* pb) {
  /*-DMCP-*/   UINT nw;
  /*-DMCP-*/
  /*-DMCP-*/   if(pb->idx >= 0 && f_write(pb->fp, pb->buf, (UINT)pb->idx, &nw) == FR_OK && (UINT)pb->idx == nw) {
  /*-DMCP-*/     return pb->nchr;
  /*-DMCP-*/   }
  /*-DMCP-*/   return EOF;
  /*-DMCP-*/ }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/ // Initialize write buffer
  /*-DMCP-*/ // Fill memory block
  /*-DMCP-*/ static void mem_set(void* dst, int val, UINT cnt) {
  /*-DMCP-*/   BYTE *d = (BYTE*)dst;
  /*-DMCP-*/
  /*-DMCP-*/   do {
  /*-DMCP-*/     *d++ = (BYTE)val;
  /*-DMCP-*/   } while(--cnt);
  /*-DMCP-*/ }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/ static void putc_init(putbuff* pb, FIL* fp) {
  /*-DMCP-*/   mem_set(pb, 0, sizeof (putbuff));
  /*-DMCP-*/   pb->fp = fp;
  /*-DMCP-*/ }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/ int f_putc(TCHAR c /* A character to be output */, FIL* fp /* Pointer to the file object */) {
  /*-DMCP-*/   putbuff pb;
  /*-DMCP-*/
  /*-DMCP-*/   putc_init(&pb, fp);
  /*-DMCP-*/   putc_bfd(&pb, c); // Put the character
  /*-DMCP-*/   return putc_flush(&pb);
  /*-DMCP-*/ }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/ /*-----------------------------------------------------------------------*/
  /*-DMCP-*/ /* Put a String to the File                                              */
  /*-DMCP-*/ /*-----------------------------------------------------------------------*/
  /*-DMCP-*/ int f_puts(const TCHAR *str /* Pointer to the string to be output */, FIL *fp /* Pointer to the file object */) {
  /*-DMCP-*/   putbuff pb;
  /*-DMCP-*/
  /*-DMCP-*/   putc_init(&pb, fp);
  /*-DMCP-*/   while(*str) {
  /*-DMCP-*/     putc_bfd(&pb, *str++);   // Put the string
  /*-DMCP-*/   }
  /*-DMCP-*/   return putc_flush(&pb);
  /*-DMCP-*/ }
  /*-DMCP-*/
  /*-DMCP-*/
  //###################################################################################
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/  int16_t export_append_string_to_file(const char line1[TMP_STR_LENGTH], uint8_t mode, const char filedir[40]) {
  /*-DMCP-*/    char line[200]; // Line buffer
  /*-DMCP-*/    FIL fil;        // File object
  /*-DMCP-*/    int fr;         // FatFs return code
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/    // Prepare to write
  /*-DMCP-*/    sys_disk_write_enable(1);
  /*-DMCP-*/    fr = sys_is_disk_write_enable();
  /*-DMCP-*/    if(fr==0) {
  /*-DMCP-*/      sprintf(line,"%s%d    \n",IOMsgs[1].itemName, fr); //Write error ID001-->
  /*-DMCP-*/      print_linestr(line,true);
  /*-DMCP-*/      f_close(&fil);
  /*-DMCP-*/      sys_disk_write_enable(0);
  /*-DMCP-*/      return (int)fr;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    // Opens an existing file. If not exist, creates a new file.
  /*-DMCP-*/    if(mode == APPEND) {
  /*-DMCP-*/      fr = f_open(&fil, filedir, FA_OPEN_APPEND | FA_WRITE);
  /*-DMCP-*/    }
  /*-DMCP-*/    else {
  /*-DMCP-*/      fr = f_open(&fil, filedir, FA_WRITE | FA_CREATE_ALWAYS);
  /*-DMCP-*/    }
  /*-DMCP-*/    if(fr) {
  /*-DMCP-*/      sprintf(line,"%s%d    \n",IOMsgs[2].itemName,  fr); //File open error ID002-->
  /*-DMCP-*/      print_linestr(line,false);
  /*-DMCP-*/      f_close(&fil);
  /*-DMCP-*/      sys_disk_write_enable(0);
  /*-DMCP-*/      return (int)fr;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    // Seek to end of the file to append data
  /*-DMCP-*/    if(mode == APPEND) {
  /*-DMCP-*/      fr = f_lseek(&fil, f_size(&fil));
  /*-DMCP-*/      if(fr) {
  /*-DMCP-*/        sprintf(line,"%s%d    \n",IOMsgs[3].itemName, fr); //Seek error ID003-->
  /*-DMCP-*/        print_linestr(line,false);
  /*-DMCP-*/        f_close(&fil);
  /*-DMCP-*/        sys_disk_write_enable(0);
  /*-DMCP-*/        return (int)fr;
  /*-DMCP-*/      }
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    // Create string and output
  /*-DMCP-*/    fr = f_puts(line1, &fil);
  /*-DMCP-*/    if(fr == EOF) {
  /*-DMCP-*/      sprintf(line,"%s%d    \n",IOMsgs[4].itemName, fr); //Write error ID004-->
  /*-DMCP-*/      print_linestr(line,false);
  /*-DMCP-*/      f_close(&fil);
  /*-DMCP-*/      sys_disk_write_enable(0);
  /*-DMCP-*/      return (int)fr;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    // close the file
  /*-DMCP-*/    fr = f_close(&fil);
  /*-DMCP-*/    if(fr) {
  /*-DMCP-*/      sprintf(line,"%s%d    \n",IOMsgs[4].itemName,  fr); //Write error ID004-->
  /*-DMCP-*/      print_linestr(line,false);
  /*-DMCP-*/      f_close(&fil);
  /*-DMCP-*/      sys_disk_write_enable(0);
  /*-DMCP-*/      return (int)fr;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    sys_disk_write_enable(0);
  /*-DMCP-*/
  /*-DMCP-*/    return 0;
  /*-DMCP-*/  }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/  static FIL fil;               // File object
  /*-DMCP-*/  int16_t export_append_string_to_file_n(const char *line1, uint8_t mode, const char filedir[40]) {
  /*-DMCP-*/  char line[200];               // Line buffer
  /*-DMCP-*/      int fr;                   // FatFs return code
  /*-DMCP-*/
  /*-DMCP-*/    switch(mode) {
  /*-DMCP-*/      case OPEN:
  /*-DMCP-*/      case APPEND:
  /*-DMCP-*/        /* Prepare to write */
  /*-DMCP-*/        sys_disk_write_enable(1);
  /*-DMCP-*/        fr = sys_is_disk_write_enable();
  /*-DMCP-*/        if(fr == 0) {
  /*-DMCP-*/          sprintf(line,"%s%d    \n",IOMsgs[1].itemName,  fr); //Write error ID001-->
  /*-DMCP-*/          print_linestr(line,true);
  /*-DMCP-*/          f_close(&fil);
  /*-DMCP-*/          sys_disk_write_enable(0);
  /*-DMCP-*/          return (int)fr;
  /*-DMCP-*/        }
  /*-DMCP-*/        /* Opens an existing file. If not exist, creates a new file. */
  /*-DMCP-*/        if(mode == APPEND) {
  /*-DMCP-*/          fr = f_open(&fil, filedir, FA_OPEN_APPEND | FA_WRITE);
  /*-DMCP-*/        }
  /*-DMCP-*/        else {
  /*-DMCP-*/          fr = f_open(&fil, filedir, FA_WRITE|FA_CREATE_ALWAYS);
  /*-DMCP-*/        }
  /*-DMCP-*/        if(fr) {
  /*-DMCP-*/          sprintf(line,"%s%d    \n",IOMsgs[2].itemName,  fr); //File open error ID002-->
  /*-DMCP-*/          print_linestr(line,false);
  /*-DMCP-*/          f_close(&fil);
  /*-DMCP-*/          sys_disk_write_enable(0);
  /*-DMCP-*/          return (int)fr;
  /*-DMCP-*/        }
  /*-DMCP-*/        /* Seek to end of the file to append data */
  /*-DMCP-*/        if(mode == APPEND) {
  /*-DMCP-*/          fr = f_lseek(&fil, f_size(&fil));
  /*-DMCP-*/          if(fr) {
  /*-DMCP-*/            sprintf(line,"%s%d    \n",IOMsgs[3].itemName,  fr); print_linestr(line,false); //Seek error ID003-->
  /*-DMCP-*/            f_close(&fil);
  /*-DMCP-*/            sys_disk_write_enable(0);
  /*-DMCP-*/            return (int)fr;
  /*-DMCP-*/          }
  /*-DMCP-*/        }
  /*-DMCP-*/        break;
  /*-DMCP-*/
  /*-DMCP-*/      case WRITE:
  /*-DMCP-*/        /* Create string and output */
  /*-DMCP-*/        fr = f_puts(line1, &fil);
  /*-DMCP-*/        if(fr == EOF) {
  /*-DMCP-*/          sprintf(line,"%s%d    \n",IOMsgs[4].itemName,  fr); print_linestr(line,false); //Write error ID004-->
  /*-DMCP-*/          f_close(&fil);
  /*-DMCP-*/          sys_disk_write_enable(0);
  /*-DMCP-*/          return (int)fr;
  /*-DMCP-*/        }
  /*-DMCP-*/        break;
  /*-DMCP-*/
  /*-DMCP-*/      case CLOSE:
  /*-DMCP-*/        /* close the file */
  /*-DMCP-*/        fr = f_close(&fil);
  /*-DMCP-*/        if(fr) {
  /*-DMCP-*/          sprintf(line,"%s%d    \n",IOMsgs[5].itemName,  fr); print_linestr(line,false); //Close error ID005-->
  /*-DMCP-*/          f_close(&fil);
  /*-DMCP-*/          sys_disk_write_enable(0);
  /*-DMCP-*/          return (int)fr;
  /*-DMCP-*/        }
  /*-DMCP-*/        sys_disk_write_enable(0);
  /*-DMCP-*/        break;
  /*-DMCP-*/
  /*-DMCP-*/      default:
  /*-DMCP-*/        break;
  /*-DMCP-*/      }
  /*-DMCP-*/
  /*-DMCP-*/      return 0;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/  int16_t open_text(const char *dirname, const char *dirfile) {
  /*-DMCP-*/    char rr[2];
  /*-DMCP-*/    rr[0] = 0;
  /*-DMCP-*/    check_create_dir(dirname);
  /*-DMCP-*/    if(export_append_string_to_file_n(rr, OPEN, dirfile)) {
  /*-DMCP-*/      return 1;
  /*-DMCP-*/    }
  /*-DMCP-*/    return 0;
  /*-DMCP-*/  }
  /*-DMCP-*/
  /*-DMCP-*/  int16_t close_text(const char *dirfile) {
  /*-DMCP-*/    char rr[2];
  /*-DMCP-*/    rr[0] = 0;
  /*-DMCP-*/    if(export_append_string_to_file_n(rr, CLOSE, dirfile)) {
  /*-DMCP-*/      return 1;
  /*-DMCP-*/    }
  /*-DMCP-*/    return 0;
  /*-DMCP-*/  }
  /*-DMCP-*/
  /*-DMCP-*/  int16_t save_text(const char *line1, uint8_t mode1, uint8_t mode2, uint8_t mode3, int16_t  nn, const char *dirfile) {
  /*-DMCP-*/    char rr[2];
  /*-DMCP-*/    rr[1] = 0;
  /*-DMCP-*/    switch(mode1) {
  /*-DMCP-*/      case OPEN:
  /*-DMCP-*/      case APPEND:
  /*-DMCP-*/        if(export_append_string_to_file_n(rr, mode1, dirfile)) {
  /*-DMCP-*/          return 1;
  /*-DMCP-*/        }
  /*-DMCP-*/        break;
  /*-DMCP-*/      default: break;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    int16_t ii, jj;
  /*-DMCP-*/    switch(mode2) {
  /*-DMCP-*/      case WRITE:
  /*-DMCP-*/        ii = min(nn,(int16_t)(strlen(line1)));
  /*-DMCP-*/        jj = ii;
  /*-DMCP-*/        while(ii != 0) {
  /*-DMCP-*/          rr[0] = line1[jj-ii];
  /*-DMCP-*/          if(export_append_string_to_file_n(rr, mode2, dirfile) != 0) {
  /*-DMCP-*/            return 1;
  /*-DMCP-*/          }
  /*-DMCP-*/        ii--;
  /*-DMCP-*/        }
  /*-DMCP-*/        break;
  /*-DMCP-*/      default: break;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    switch(mode3) {
  /*-DMCP-*/      case CLOSE:
  /*-DMCP-*/        if(export_append_string_to_file_n(rr, mode3, dirfile)) {
  /*-DMCP-*/          return 1;
  /*-DMCP-*/        }
  /*-DMCP-*/        break;
  /*-DMCP-*/      default: break;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    return 0;
  /*-DMCP-*/  }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/int16_t export_string_to_filename(const char line1[TMP_STR_LENGTH], uint8_t mode, const char *dirname, const char *filename) {
  /*-DMCP-*/char dirfile[40];
  /*-DMCP-*/    //Create file name
  /*-DMCP-*/    strcpy(dirfile,dirname);
  /*-DMCP-*/    strcat(dirfile,"\\");
  /*-DMCP-*/    strcat(dirfile,filename);
  /*-DMCP-*/
  /*-DMCP-*/    sprintf(tmpString,"%s%s",line1,CSV_NEWLINE);
  /*-DMCP-*/    check_create_dir(dirname);
  /*-DMCP-*/    if(export_append_string_to_file(tmpString, mode, dirfile) != 0) {
  /*-DMCP-*/      //ERROR ALREADY ANNOUNCED
  /*-DMCP-*/      return 1;
  /*-DMCP-*/    }
  /*-DMCP-*/  return 0;
  /*-DMCP-*/}
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/int16_t import_string_from_filename(char *line1,  char *dirname,  char *filename_short,  char *filename,  char *fallback, bool_t scanning) {
                #if (VERBOSE_LEVEL >= 2)
  /*-DMCP-*/      print_inlinestr("From dir:",false);
  /*-DMCP-*/      print_inlinestr(dirname,false);
  /*-DMCP-*/      print_inlinestr(", ",true);
  /*-DMCP-*/    #endif
  /*-DMCP-*/
                #if (VERBOSE_LEVEL >= 2)
  /*-DMCP-*/      char line[300];         // Line buffer
  /*-DMCP-*/    #endif
  /*-DMCP-*/
  /*-DMCP-*/    char dirfile[200];
  /*-DMCP-*/    dirfile[0]=0;
  /*-DMCP-*/    FIL fil;                  // File object
  /*-DMCP-*/    int fr;                   // FatFs return code
  /*-DMCP-*/
  /*-DMCP-*/    //Create dir name
  /*-DMCP-*/    check_create_dir(dirname);
  /*-DMCP-*/
  /*-DMCP-*/    strcpy(dirfile, dirname);
  /*-DMCP-*/    strcat(dirfile, "\\");
  /*-DMCP-*/    strcat(dirfile, filename_short);
  /*-DMCP-*/
                #if (VERBOSE_LEVEL >= 1)
  /*-DMCP-*/      print_inlinestr("1: reading:", false);
  /*-DMCP-*/      print_inlinestr(dirfile, false);
  /*-DMCP-*/      print_inlinestr(" ", true);
  /*-DMCP-*/    #endif
  /*-DMCP-*/
  /*-DMCP-*/    // Opens an existing file.
  /*-DMCP-*/    fr = f_open(&fil, dirfile, FA_READ);   // | FA_OPEN_EXISTING
  /*-DMCP-*/    if(fr != FR_OK) {
                  #if (VERBOSE_LEVEL >= 1)
  /*-DMCP-*/        print_inlinestr("Not open. ",false);
                    #if (VERBOSE_LEVEL >= 2)
  /*-DMCP-*/          if(fr == 4) {
  /*-DMCP-*/            sprintf(line,"%s%d ",IOMsgs[6].itemName,   fr); //Not found ID006 -->
  /*-DMCP-*/            print_inlinestr(line, false);
  /*-DMCP-*/            sprintf(line,"File: %s \n", dirfile);
  /*-DMCP-*/            print_inlinestr(line, false);
  /*-DMCP-*/          }
  /*-DMCP-*/          else if(fr == 5) {
  /*-DMCP-*/            sprintf(line,"%s%d ",IOMsgs[7].itemName,   fr); //No path ID007 -->
  /*-DMCP-*/            print_inlinestr(line, false);
  /*-DMCP-*/            sprintf(line,"File: %s \n", dirfile);
  /*-DMCP-*/            print_inlinestr(line, false);
  /*-DMCP-*/          }
  /*-DMCP-*/          else {
  /*-DMCP-*/            sprintf(line,"%s%d ",IOMsgs[2].itemName,   fr); //File open error ID002-->
  /*-DMCP-*/            print_inlinestr(line, false);
  /*-DMCP-*/            sprintf(line,"File: %s \n", dirfile);
  /*-DMCP-*/            print_inlinestr(line, false);
  /*-DMCP-*/          }
                    #endif // VERBOSE_LEVEL >= 2
                  #endif // VERBOSE_LEVEL >= 1
                  #if (VERBOSE_LEVEL >= 2)
  /*-DMCP-*/        print_inlinestr(".", true);
                  #endif
  /*-DMCP-*/      f_close(&fil);
  /*-DMCP-*/
  /*-DMCP-*/      if(filename[0] != 0) {
  /*-DMCP-*/        strcpy(dirfile, dirname);
  /*-DMCP-*/        strcat(dirfile, "\\");
  /*-DMCP-*/        strcat(dirfile, filename);
                   #if (VERBOSE_LEVEL >= 1)
  /*-DMCP-*/          print_inlinestr("2: reading:", false);
  /*-DMCP-*/          print_inlinestr(dirfile, false);
  /*-DMCP-*/          print_inlinestr(" ", true);
                   #endif
  /*-DMCP-*/
  /*-DMCP-*/        /* Opens an existing file. */
  /*-DMCP-*/        fr = f_open(&fil, dirfile, FA_READ );   //| FA_OPEN_EXISTING
  /*-DMCP-*/        if(fr != FR_OK) {
                      #if (VERBOSE_LEVEL >= 1)
  /*-DMCP-*/            print_inlinestr("Not open. ",false);
                        #if (VERBOSE_LEVEL >= 2)
  /*-DMCP-*/              if(fr == 4) {
  /*-DMCP-*/                sprintf(line,"%s%d ",IOMsgs[7].itemName,  fr); //No path ID007 -->
  /*-DMCP-*/                print_inlinestr(line, false);
  /*-DMCP-*/                sprintf(line,"File: %s \n",dirfile);
  /*-DMCP-*/                print_inlinestr(line, false);
  /*-DMCP-*/              }
  /*-DMCP-*/              else {
  /*-DMCP-*/                sprintf(line,"%s%d ",IOMsgs[2].itemName,   fr); //File open error ID002-->
  /*-DMCP-*/                print_inlinestr(line, false);
  /*-DMCP-*/              }
                        #endif // VERBOSE_LEVEL >= 2
                      #endif // VERBOSE_LEVEL >= 1
                      #if (VERBOSE_LEVEL >= 1)
  /*-DMCP-*/            print_inlinestr((char*)IOMsgs[8].itemName,   true); //. Using fallback.
                      #endif // VERBOSE_LEVEL >= 1
  /*-DMCP-*/          f_close(&fil);
  /*-DMCP-*/          strcpy(line1, fallback);
  /*-DMCP-*/          return 1;
  /*-DMCP-*/        }
  /*-DMCP-*/      }
  /*-DMCP-*/      else {
                    #if (VERBOSE_LEVEL >= 1)
  /*-DMCP-*/          print_inlinestr((char*)IOMsgs[8].itemName,   true); //. Using fallback.
                    #endif
  /*-DMCP-*/        strcpy(line1, fallback);
  /*-DMCP-*/        return 1;
  /*-DMCP-*/      }
  /*-DMCP-*/    }
  /*-DMCP-*/
                #if (VERBOSE_LEVEL >= 1)
  /*-DMCP-*/      print_inlinestr("reading...",false);
                #endif
  /*-DMCP-*/
  /*-DMCP-*/    /* Read if open */
  /*-DMCP-*/    line1[0]=0;
  /*-DMCP-*/    f_getsline(line1, (scanning ? min(100,TMP_STR_LENGTH) : TMP_STR_LENGTH), &fil);
  /*-DMCP-*/    f_close(&fil);
  /*-DMCP-*/
                #if (VERBOSE_LEVEL >= 1)
  /*-DMCP-*/      print_inlinestr("read:",true);
  /*-DMCP-*/      print_inlinestr(line1,true);
                #endif
  /*-DMCP-*/
  /*-DMCP-*/    if(stringByteLength(line1) >= TMP_STR_LENGTH-1) {
  /*-DMCP-*/      strcpy(line1, fallback);
                  #if (VERBOSE_LEVEL >= 1)
  /*-DMCP-*/        print_inlinestr((char*)IOMsgs[9].itemName, true); //ERROR too long file using fallback
                  #endif
  /*-DMCP-*/      return 1;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    return 0;
  /*-DMCP-*/  }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/  void create_filename(char *fn){ //fn must be in format ".STAT.TSV"
  /*-DMCP-*/    uint32_t tmp__32;                                                 //JM_CSV
  /*-DMCP-*/
  /*-DMCP-*/    tmp__32 = getUptimeMs();                                      //KEEP PERSISTENT FILE NAME FOR A PERIOD
  /*-DMCP-*/    if(cancelFilename || (mem__32 == 0) || (tmp__32 > mem__32 + 120000) || (stringByteLength(filename_csv) > 10 && compareString(filename_csv + (stringByteLength(filename_csv) - 9),fn, CMP_NAME) != 0)) {
  /*-DMCP-*/      //Create file name
  /*-DMCP-*/      check_create_dir("DATA");
  /*-DMCP-*/      make_date_filename(filename_csv, "DATA\\", fn);
  /*-DMCP-*/      check_create_dir("DATA");
  /*-DMCP-*/    }
  /*-DMCP-*/    mem__32 = tmp__32;
  /*-DMCP-*/    cancelFilename = false;
  /*-DMCP-*/  }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/  int16_t export_xy_to_file(float x, float y) {
  /*-DMCP-*/    char line[TMP_STR_LENGTH]; // Line buffer
  /*-DMCP-*/    create_filename(".STAT.TSV");
  /*-DMCP-*/    sprintf(line,"%.16e%s%.16e%s", x, CSV_TAB, y, CSV_NEWLINE);
  /*-DMCP-*/    if(export_append_string_to_file(line, APPEND, filename_csv) != 0) {
  /*-DMCP-*/      //ERROR ALREADY ANNOUNCED
  /*-DMCP-*/      return 1;
  /*-DMCP-*/    }
  /*-DMCP-*/    return 0;
  /*-DMCP-*/  }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/  int16_t export_append_line_short(const char *inputstring) {
  /*-DMCP-*/    char line[200]; // Line buffer
  /*-DMCP-*/    FIL fil;        // File object
  /*-DMCP-*/    int fr;         // FatFs return code
  /*-DMCP-*/
  /*-DMCP-*/    // Prepare to write
  /*-DMCP-*/    sys_disk_write_enable(1);
  /*-DMCP-*/    fr = sys_is_disk_write_enable();
  /*-DMCP-*/    if(fr==0) {
  /*-DMCP-*/      sprintf(line,"%s%d    \n",IOMsgs[4].itemName,  fr); //Write error ID004-->
  /*-DMCP-*/      print_linestr(line, false);
  /*-DMCP-*/      f_close(&fil);
  /*-DMCP-*/      sys_disk_write_enable(0);
  /*-DMCP-*/      return (int)fr;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/    // Opens an existing file. If not exist, creates a new file.
  /*-DMCP-*/    fr = f_open(&fil, filename_csv, FA_OPEN_APPEND | FA_WRITE);
  /*-DMCP-*/    if(fr) {
  /*-DMCP-*/      sprintf(line,"%s%d    \n",IOMsgs[2].itemName,  fr); //File open error ID002-->
  /*-DMCP-*/      print_linestr(line, false);
  /*-DMCP-*/      f_close(&fil);
  /*-DMCP-*/      sys_disk_write_enable(0);
  /*-DMCP-*/      return (int)fr;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    // Seek to end of the file to append data
  /*-DMCP-*/    fr = f_lseek(&fil, f_size(&fil));
  /*-DMCP-*/    if(fr) {
  /*-DMCP-*/      sprintf(line,"%s%d    \n",IOMsgs[3].itemName, fr); //Seek error ID003-->
  /*-DMCP-*/      print_linestr(line, false);
  /*-DMCP-*/      f_close(&fil);
  /*-DMCP-*/      sys_disk_write_enable(0);
  /*-DMCP-*/      return (int)fr;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    fr = f_puts(inputstring, &fil);
  /*-DMCP-*/
               #if (VERBOSE_LEVEL >= 1)
  /*-DMCP-*/      sprintf(line,"wrote %d, %s\n", fr, inputstring);
  /*-DMCP-*/      print_linestr(line, false);
               #endif // VERBOSE_LEVEL >= 1
  /*-DMCP-*/
  /*-DMCP-*/    if(fr == 0) {
  /*-DMCP-*/      sprintf(line,"%s%d    \n",IOMsgs[4].itemName,  fr); //Write error ID004-->
  /*-DMCP-*/      print_linestr(line, false);
  /*-DMCP-*/      f_close(&fil);
  /*-DMCP-*/      sys_disk_write_enable(0);
  /*-DMCP-*/      return (int)fr;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    // close the file
  /*-DMCP-*/    fr = f_close(&fil);
  /*-DMCP-*/    if(fr) {
  /*-DMCP-*/      sprintf(line,"%s%d    \n",IOMsgs[5].itemName,  fr); //Close error ID005-->
  /*-DMCP-*/      print_linestr(line, false);
  /*-DMCP-*/      f_close(&fil);
  /*-DMCP-*/      sys_disk_write_enable(0);
  /*-DMCP-*/      return (int)fr;
  /*-DMCP-*/    }
  /*-DMCP-*/
  /*-DMCP-*/    sys_disk_write_enable(0);
  /*-DMCP-*/
               #if (VERBOSE_LEVEL >= 1)
  /*-DMCP-*/      print_linestr("-closed return-",false);
               #endif // VERBOSE_LEVEL >= 1
  /*-DMCP-*/
  /*-DMCP-*/    return 0;
  /*-DMCP-*/  }
  /*-DMCP-*/
  /*-DMCP-*/
  /*-DMCP-*/int16_t export_append_line(const char *inputstring) {
  /*-DMCP-*/  int ix = 0;
  /*-DMCP-*/  char tmp[200];
  /*-DMCP-*/  int fr;
  /*-DMCP-*/  #define linesize 128
  /*-DMCP-*/
  /*-DMCP-*/  while(stringByteLength(inputstring + ix) > linesize) {
  /*-DMCP-*/    xcopy(tmp, inputstring + ix, linesize);
  /*-DMCP-*/    tmp[linesize] = 0;
  /*-DMCP-*/    fr = export_append_line_short(tmp);
  /*-DMCP-*/    if(fr) {
  /*-DMCP-*/      return (int)fr;
  /*-DMCP-*/    }
  /*-DMCP-*/    ix += linesize;
  /*-DMCP-*/  }
  /*-DMCP-*/  if(stringByteLength(inputstring + ix) > 0) {
  /*-DMCP-*/    fr = export_append_line_short(inputstring + ix);
  /*-DMCP-*/    if(fr) {
  /*-DMCP-*/      return (int)fr;
  /*-DMCP-*/    }
  /*-DMCP-*/  }
  /*-DMCP-*/  return 0;
  /*-DMCP-*/}
  /*-DMCP-*/

//**********************************************************************************************************
#elif PC_BUILD // PC_BUILD
  int16_t export_string_to_filename(const char line1[TMP_STR_LENGTH], uint8_t mode, const char *dirname, const char *filename) {
    FILE *outfile;
    char dirfile[40];
    int frr = 0;
    char line[200]; // Line buffer

    strcpy(dirfile, dirname);
    strcat(dirfile, "/");
    strcat(dirfile, filename);

    if(mode == APPEND) {
      outfile = fopen(dirfile, "ab");
    }
    else {
      outfile = fopen(dirfile, "wb");
    }

    if(outfile == NULL) {
      printf("Cannot open ID008: %s %s\n", dirfile, line1);
      fflush(stdout);
      return 1;
    }

    sprintf(tmpString, "%s%s", line1, CSV_NEWLINE);
    frr = fputs(tmpString, outfile);
    if(frr == EOF) {
      sprintf(line, "Write error ID009 --> %i    \n", frr);
      //print_linestr(line,false);
      printf("%s", line1);
      fflush(stdout);
      if(outfile != NULL) fclose(outfile);
      return frr;
    }
    else {
      printf("Exported %i chars to %s: %s\n", frr, dirfile, line1);
      fflush(stdout);
      fclose(outfile);
    }
    return 0;
  }


  int16_t import_string_from_filename(char *line1,  char *dirname,   char *filename_short,  char *filename,  char *fallback, bool_t scanning) {
    #if (VERBOSE_LEVEL >= 2)
      print_inlinestr("From dir:", false);
      print_inlinestr(dirname, false);
      print_inlinestr(", ", true);
    #endif // VERBOSE_LEVEL >= 2

    char dirfile[200];
    dirfile[0] = 0;
    FILE *infile;
    char onechar[2];

    strcpy(dirfile, dirname);
    strcat(dirfile, "/");
    strcat(dirfile, filename_short);

    #if (VERBOSE_LEVEL >= 1)
      print_inlinestr("1: reading:", false);
      print_inlinestr(dirfile, false);
      print_inlinestr(" ", true);
    #endif // VERBOSE_LEVEL >= 1

    // Opens an existing file.
    infile = fopen(dirfile, "rb");
    if(infile == NULL) {
      #if (VERBOSE_LEVEL >= 1)
        #if defined(PC_BUILD)
          printf("Cannot load ID010 %s\n", dirfile);
          fflush(stdout);
        #endif // PC_BUILD
        print_inlinestr("Not open. ", true);
      #endif // VERBOSE_LEVEL >= 1

      if(filename[0] != 0) {
        strcpy(dirfile, dirname);
        strcat(dirfile, "/");
        strcat(dirfile, filename);
        #if (VERBOSE_LEVEL >= 1)
          print_inlinestr("2: reading:", false);
          print_inlinestr(dirfile, false);
          print_inlinestr(" ", true);
        #endif // VERBOSE_LEVEL >= 1

        /* Opens an existing file. */
        infile = fopen(dirfile, "rb");
        if(infile == NULL) {
          #if (VERBOSE_LEVEL >= 1)
            #if defined(PC_BUILD)
              printf("Cannot load %s\n", dirfile);
              fflush(stdout);
            #endif // PC_BUILD
            print_inlinestr("Fallback.", true);
          #endif // VERBOSE_LEVEL >= 1
          strcpy(line1, fallback);
          return 1;
        }
      }
      else {
        #if (VERBOSE_LEVEL >= 1)
          #if defined(PC_BUILD)
            printf("Cannot load %s\n", dirfile);
            fflush(stdout);
          #endif // PC_BUILD
          print_inlinestr("Fallback.", true);
        #endif // VERBOSE_LEVEL >= 1
        strcpy(line1, fallback);
        return 1;
      }
    }

    #if (VERBOSE_LEVEL >= 1)
      print_inlinestr("reading...", false);
    #endif // VERBOSE_LEVEL >= 1

    /* Read if open */
    line1[0] = 0;
    onechar[1] = 0;
    while(fread(&onechar, 1, 1, infile)) {
      strcat(line1, onechar);
    }
    fclose(infile);

    #if (VERBOSE_LEVEL >= 1)
      #if defined(PC_BUILD)
        printf("Loaded >>> %s\n", dirfile);
        fflush(stdout);
      #endif // PC_BUILD
      print_inlinestr("read:", true);
      print_inlinestr(line1, true);
    #endif // VERBOSE_LEVEL >= 1

    #if (VERBOSE_LEVEL >= 2)
      #if defined(PC_BUILD)
        printf("Loaded %s |%s|\n", dirfile, line1);
        fflush(stdout);
      #endif // PC_BUILD
    #endif // VERBOSE_LEVEL >= 2


    if(stringByteLength(line1) >= TMP_STR_LENGTH-1) {
      strcpy(line1, fallback);
      #if (VERBOSE_LEVEL >= 1)
        print_inlinestr("ERROR too long file using fallback", true);
      #endif // VERBOSE_LEVEL >= 1
      printf("ERROR too long file using fallback\n");
      fflush(stdout);
      return 1;
    }

    return 0;
  }


  int16_t export_append_line(const char *inputstring) {
    FILE *outfile;

    //strcpy(dirfile, "PROGRAMS/C47_LOG.TXT");
    outfile = fopen(filename_csv, "ab");
    if(outfile == NULL) {
      printf("Cannot open to append ID011: %s %s\n", filename_csv, inputstring);
      fflush(stdout);
      return 1;
    }
    int frr = 0;
    char line[200]; // Line buffer
    frr = fputs(inputstring, outfile);
    if(frr == EOF) {
      sprintf(line,"Write error ID012 --> %i %s\n", frr, inputstring);
      fflush(stdout);
      //print_linestr(line, false);
      printf("%s", line);
      fflush(stdout);
      if(outfile != NULL) fclose(outfile);
      return frr;
    }
    else {
      printf("Exported %i chars to %s: %s\n", frr, filename_csv, inputstring);
      fflush(stdout);
      fclose(outfile);
    }
    return 0;
  }


  void create_filename(char *fn){ //fn must be in format ".STAT.TSV"
      uint32_t tmp__32;                                                 //JM_CSV
      time_t rawTime;
      struct tm *timeInfo;

      tmp__32 = getUptimeMs();                                          //KEEP PERSISTENT FILE NAME FOR A PERIOD
      if(cancelFilename || (mem__32 == 0) || (tmp__32 > mem__32 + 120000)  || (stringByteLength(filename_csv) > 10 && compareString(filename_csv + (stringByteLength(filename_csv) - 9),fn, CMP_NAME) != 0)) {
        //Create file name
        time(&rawTime);
        timeInfo = localtime(&rawTime);
        strftime(filename_csv, FILENAMELEN, "%Y%m%d-%H%M%S00", timeInfo);
        strcat(filename_csv, fn);
      }
      mem__32 = tmp__32;
      cancelFilename = false;
  }


  int16_t export_xy_to_file(float x, float y) {
    char line[200]; // Line buffer
    create_filename(".STAT.TSV");
    sprintf(line, "%.16e%s%.16e%s", x, CSV_TAB, y, CSV_NEWLINE);
    export_append_line(line);
    return 0;
  }
#endif // DMCP_BUILD


// BOTH vvv *** ################################################################################################

uint32_t ttt = 0;
void printStatus(uint8_t row, const char *line1, uint8_t forced) {
  #if defined (PC_BUILD)
    if(ttt==0) ttt = (uint32_t)(g_get_monotonic_time());
    printf("Status: %10u, %s\n", (uint32_t)(g_get_monotonic_time())-ttt, line1);
    fflush(stdout);
  #endif //PC_BUILD
  #if !defined(TESTSUITE_BUILD)
    int16_t g_line_x, g_line_y;
    g_line_y = Y_POSITION_OF_REGISTER_T_LINE; //20
    g_line_x = 20 * row;

//  refreshRegisterLine(REGISTER_T);
    clearRegisterLine(REGISTER_T, true, true);

    //g_line_x = showStringEnhanced(line1, &standardFont, g_line_x, g_line_y, vmNormal, false, false, NO_compress, NO_raise, DO_Show, NO_LF);
    g_line_x = showString(line1, &standardFont, (uint32_t) g_line_x, (uint32_t) g_line_y, vmNormal, true, true);

//  clearRect(g_line_x, g_line_y);

    if(forced == force) {
      force_refresh(force);
    }
    else {
      force_refresh(timed);
    }
  #endif // !TESTSUITE_BUILD
}




int16_t g_line_x, g_line_y;

void print_linestr(const char *line1, bool_t line_init) {        //prints one line at a time, filled with dots on remaining line
  #if !defined(TESTSUITE_BUILD)
    char l1[200];
    l1[0] = 0;
    int16_t ix = 0;
    int16_t ixx;

    if(line_init) {
      g_line_y = 20;
      g_line_x = 0;
    }
    strcat(l1, "-");
    ixx = stringByteLength(line1);
    while(ix < ixx && ix < 98 && stringWidth(l1, &standardFont, true, true) < SCREEN_WIDTH-12-g_line_x) {
      xcopy(l1, line1, ix+1);
      l1[ix+1+1] = 0;
      ix = stringNextGlyph(line1, ix);
    }
    while(stringByteLength(l1) < 200 && stringWidth(l1, &standardFont, true, true) < SCREEN_WIDTH-12-g_line_x) {
      strcat(l1, ".");
    }

    if(g_line_y < SCREEN_HEIGHT) {
      ixx = showString(l1, &standardFont, (uint32_t) g_line_x, (uint32_t) g_line_y, vmNormal, true, true);
    }
    if(!line_init) {
      g_line_y += 20;
    }
    if(g_line_y > SCREEN_HEIGHT - 20) {
      g_line_y = 40;
      g_line_x += 4;
    }
    force_refresh(timed);
    #if defined(DMCP_BUILD)
      lcd_refresh_wait();
    #endif //DMCP_BUILD
  #endif // !TESTSUITE_BUILD
}

void print_numberstr(const char *line1, bool_t line_init) {     //ONLY N=ASCII NUMBERS AND E AND . //FIXED FONT
  #if !defined(TESTSUITE_BUILD)
    if(line_init || g_line_y >= SCREEN_HEIGHT) {
      g_line_y = 0;
      g_line_x = 0;
    }
    g_line_y += 20;
    if(g_line_y < SCREEN_HEIGHT) {
        int16_t cnt = 0;
        char tt[2];
        while(line1[cnt] != 0 && g_line_x < SCREEN_WIDTH-8 +1) {
          tt[0]=line1[cnt]; tt[1]=0;
          g_line_x = showString(tt, &standardFont, (uint32_t)cnt * 8, (uint32_t) g_line_y, vmNormal, true, true);
          cnt++;
        }
    }
    g_line_x = 0;
    force_refresh(timed);
    #if defined(DMCP_BUILD)
      lcd_refresh_wait();
    #endif //DMCP_BUILD

  #endif // !TESTSUITE_BUILD
}



uint32_t t_line_x, t_line_y;

void print_inlinestr(const char *line1, bool_t endline) {  //prints with or without newline at the end of the line
    #if !defined(TESTSUITE_BUILD)

    char l1[100];    //Clip the string at 40
    l1[0] = 0;
    int16_t ix = 0;
    int16_t ixx;
    ixx = stringByteLength(line1);
    while(ix < ixx && ix < 98 && t_line_x + stringWidth(l1, &standardFont, true, true) < SCREEN_WIDTH-12) {
      xcopy(l1, line1, ix+1);
      l1[ix+1] = 0;
      ix = stringNextGlyph(line1, ix);
    }
    if(t_line_y < SCREEN_HEIGHT) {
      t_line_x = showString(l1, &standardFont, t_line_x, t_line_y, vmNormal, true, true);
    }
    if(endline) {
      t_line_y += 20;
      t_line_x = 0;
    }
    force_refresh(force);
    #if defined(DMCP_BUILD)
      lcd_refresh_wait();
    #endif //DMCP_BUILD
  #endif // !TESTSUITE_BUILD
}


void print_Register_line(calcRegister_t regist, char *before, char *after, bool_t line_init) {
  #if !defined(TESTSUITE_BUILD)
    char str[TMP_STR_LENGTH];

    copyRegisterToClipboardString2(regist, str);
    addStrBothSides(str, before, after);

    print_numberstr(str, line_init);
  #endif // !TESTSUITE_BUILD
}

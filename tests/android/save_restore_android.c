#include "c47.h"
#include "hal/io.h"
extern void doSave(uint16_t saveType);
extern void setConfirmationMode(void (*func)(uint16_t));
  void fnSaveAuto(uint16_t unusedButMandatoryParameter) {
  #if defined(DMCP_BUILD) || defined(ANDROID_BUILD)
    doSave(autoSave);
  #endif // DMCP_BUILD
  }
void fnDeleteBackup(uint16_t confirmation) {
  if(confirmation == NOT_CONFIRMED) {
    setConfirmationMode(fnDeleteBackup);
  }
  else {
    #if defined(DMCP_BUILD)
      FRESULT result;
      sys_disk_write_enable(1);
      result = f_unlink(SAVE_DIR "\\" SAVE_FILE);
      if(result != FR_OK && result != FR_NO_FILE && result != FR_NO_PATH) {
        displayCalcErrorMessage(ERROR_IO, ERR_REGISTER_LINE);
      }
      result = f_unlink(SAVE_DIR "\\" AUTO_SAVE_FILE);
      if(result != FR_OK && result != FR_NO_FILE && result != FR_NO_PATH) {
        displayCalcErrorMessage(ERROR_IO, ERR_REGISTER_LINE);
      }
      sys_disk_write_enable(0);
    #elif defined(ANDROID_BUILD)
      extern int ioFileRemove(ioFilePath_t path, uint32_t *errorNumber);
      uint32_t err;
      ioFileRemove(ioPathManualSave, &err);
      ioFileRemove(ioPathAutoSave, &err);
    #else // !DMCP_BUILD
      int result = remove(SAVE_DIR "/" SAVE_FILE);
      if(result == -1) {
        int e = errno;
        if(e != ENOENT) {
          displayCalcErrorMessage(ERROR_IO, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "removing the backup failed with error code %d", e);
            moreInfoOnError("In function fnDeleteBackup:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
      }
    #endif // DMCP_BUILD
  }
}

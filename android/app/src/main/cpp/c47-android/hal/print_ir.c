#include "c47.h"

static uint16_t android_ir_line_delay = 0;

uint32_t getLineDelay(void) {
    return android_ir_line_delay;
}

void setLineDelay(uint16_t delay) {
    android_ir_line_delay = delay;
}

void sendByteIR(uint8_t byte) {
    (void)byte;
}
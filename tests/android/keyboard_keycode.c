#include "c47.h"
extern uint8_t lastKeyCode;
extern uint8_t calcModel;
void setLastKeyCode(int key) {             // key = 10      setLastKeyCode = 10-6 +30 = 34 row 3, col 4
  if(1 <= key && key <= 43) {              // key = 27      setLastKeyCode = 27-22+60 = 65 row 6, col 5
    if(isR47FAM) {
      switch(key) {
        case 23: {
          key = 18;                        // UP
          break;
        }
        case 28: {
          key = 23;                        // DOWN
          break;
        }
        case 11: {
          key = 28;                        // SHIFT
          break;
        }
        case 18: {
          key = 11;                        // COS
          break;
        }
        case 16: {
          key = 15;                        // +/-
          break;
        }
        case 15: {
          key = 16;                        // E
          break;
        }
        default: {
          break;
        }
      }
    }
    if(key <=  6) {                        // key = 28 EXIT setLastKeyCode = 28-27+70 = 71 row 7, col 1
      lastKeyCode = key      + 20;
    }
    else if(key <= 12) {
      lastKeyCode = key -  6 + 30;
    }
    else if(key <= 17) {
      lastKeyCode = key - 12 + 40;
    }
    else if(key <= 22) {
      lastKeyCode = key - 17 + 50;
    }
    else if(key <= 27) {
      lastKeyCode = key - 22 + 60;
    }
    else if(key <= 32) {
      lastKeyCode = key - 27 + 70;
    }
    else if(key <= 37) {
      lastKeyCode = key - 32 + 80;
    }
    else if(key <= 43) {
      lastKeyCode = key - 37 + 10; // function keys
    }
  }
}

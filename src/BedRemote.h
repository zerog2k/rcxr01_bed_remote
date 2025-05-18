#ifndef BED_REMOTE_H
#define BED_REMOTE_H

#include <Arduino.h>
#include "RadioManager.h"
#include "DisplayManager.h"
#include "PowerManager.h"
#include "KeypadManager.h"
// Don't include custom_keypad.h here - it's already included in KeypadManager.h

// Learning/teaching mode states
typedef enum {
  T_NORMAL,
  T_LEARN_START,
  T_LEARN_COMPLETE,
  T_TEACH_WAIT,
  T_TEACH_START,
  T_TEACH_COMPLETE
} learn_mode_t;

class BedRemote {
public:
    BedRemote();
    void begin();
    void loop();
    
private:
    // Component managers
    RadioManager _radioManager;
    DisplayManager _displayManager;
    PowerManager _powerManager;
    KeypadManager _keypadManager;
    
    // State tracking
    learn_mode_t _learnMode;
    
    // Process user input
    void handleKeypress(uint8_t key);
    
    // Handle learning/teaching modes
    void processLearnTeachMode();
    
    // Constants
    const uint8_t RF_CE = 0;    // PB0
    const uint8_t RF_CSN = 17;  // PC1
    const uint8_t USB_DETECT_PIN = 2; // PB2
    const char* VERSION = "v0.2";
};

#endif // BED_REMOTE_H
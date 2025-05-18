#ifndef KEYPAD_MANAGER_H
#define KEYPAD_MANAGER_H

#include <Arduino.h>
#include "custom_keypad.h"

// Keypad command mapping
class KeypadManager {
public:
    KeypadManager();
    
    // Process keypad input
    uint8_t getKey();
    uint32_t getBedCommandFromKey(uint8_t key);
    
private:
    // Key definitions from custom_keypad.h are used directly
};

#endif // KEYPAD_MANAGER_H
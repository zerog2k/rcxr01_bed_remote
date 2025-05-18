#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "RF24.h"

// System operation modes
typedef enum {
  M_SLEEP,
  M_WAKEUP,
  M_ACTIVE
} system_mode_t;

class PowerManager {
public:
    PowerManager(uint8_t usbDetectPin, RF24* radio);
    void begin();
    
    // Power state management
    void enterSleep();
    void exitSleep();
    void checkPowerState();
    
    // Sleep timer management
    void resetSleepTimer();
    bool shouldSleep();
    
    // Getters and setters
    system_mode_t getMode() { return _mode; }
    void setMode(system_mode_t mode) { _mode = mode; }
    bool isOnBattery() { return _onBattery; }
    
private:
    system_mode_t _mode;
    uint8_t _usbDetectPin;
    uint32_t _sleepTimer;
    bool _onBattery;
    RF24* _radio;
    
    const uint8_t SLEEP_DELAY_TIME = 8; // seconds before sleep
};

#endif // POWER_MANAGER_H
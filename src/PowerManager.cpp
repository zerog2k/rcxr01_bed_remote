#include "PowerManager.h"
#include "sleep.h"
#include "rtc.h"
#include "custom_keypad.h"
#include "lcd.h"

extern volatile uint32_t seconds;

PowerManager::PowerManager(uint8_t usbDetectPin, RF24* radio) :
    _mode(M_ACTIVE),
    _usbDetectPin(usbDetectPin),
    _sleepTimer(0),
    _onBattery(true),
    _radio(radio) {
}

void PowerManager::begin() {
    pinMode(_usbDetectPin, INPUT_PULLUP);
    setupWatchDogTimer();
    _sleepTimer = seconds;
}

void PowerManager::enterSleep() {
    _mode = M_SLEEP;
    u8x8.setPowerSave(1);
    _radio->powerDown();
    keypad_enter_sleep();
}

void PowerManager::exitSleep() {
    keypad_exit_sleep();
    _sleepTimer = seconds;
    u8x8.setPowerSave(0);
    _radio->powerUp();
    _mode = M_ACTIVE;
}

void PowerManager::checkPowerState() {
    _onBattery = digitalRead(_usbDetectPin);
    
    if (!_onBattery) {
        // On USB power, reset sleep timer or wake
        _sleepTimer = seconds;
        if (_mode == M_SLEEP) {
            _mode = M_WAKEUP;
        }
    }
    
    // Check if it's time to sleep
    if (_mode == M_ACTIVE && shouldSleep()) {
        enterSleep();
    }
}

void PowerManager::resetSleepTimer() {
    _sleepTimer = seconds;
}

bool PowerManager::shouldSleep() {
    return (seconds - _sleepTimer) > SLEEP_DELAY_TIME;
}
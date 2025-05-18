#include "BedRemote.h"
#include "lcd.h"
#include "rtc.h"
#include "sleep.h"
#include "serno.h"
#include <avr/power.h>
#include <printf.h>

BedRemote::BedRemote() :
    _radioManager(RF_CE, RF_CSN),
    _displayManager(),
    _powerManager(USB_DETECT_PIN, &_radioManager._radio),
    _keypadManager(),
    _learnMode(T_NORMAL) {
}

void BedRemote::begin() {
    // Change clock from 8 to 1MHz to ensure safe operating range down to
    // brown-out battery voltage of 1.8V
    clock_prescale_set(clock_div_8);
    
    delay(100);
    
    Serial.begin(9600);
    printf_begin();
    Serial.println("starting...");
    
    // Initialize display
    _displayManager.begin();
    u8g2.clearBuffer();
    u8g2.setCursor(0, 8);
    u8g2.print("bed remote");
    u8g2.setCursor(0, 16);
    u8g2.print(VERSION);
    u8g2.sendBuffer();
    
    // Initialize radio
    _radioManager.begin();
    _radioManager._radio.printDetails();
    _displayManager.showChannel(_radioManager.getChannel());
    
    // Initialize power management
    _powerManager.begin();
    
    // Initialize RTC
    Serial.print("rtc_init start.");
    rtc_init();
    Serial.println("rtc_init done.");
    
    // Print serial number and lot code
    print_ser_num();
    print_lot_code();
    
    // Enter sleep mode for keypad
    keypad_enter_sleep();
    
    // Enable interrupts
    sei();
}

void BedRemote::loop() {
    system_mode_t currentMode = _powerManager.getMode();
    
    if (currentMode == M_WAKEUP) {
        // Transition from WAKE to ACTIVE
        _powerManager.exitSleep();
        _displayManager.setDisplayMode(D_NORMAL);
        _learnMode = T_NORMAL;
        return;
    }
    
    // Check power state (USB/battery)
    _powerManager.checkPowerState();
    
    // Main operational area
    if (currentMode == M_ACTIVE) {
        // Check for keypress
        uint8_t keypress = _keypadManager.getKey();
        if (keypress) {
            _powerManager.resetSleepTimer();
            handleKeypress(keypress);
        }
        
        // Update display if needed
        if (_displayManager.needsUpdate()) {
            _displayManager.updateDisplay();
            if (_displayManager.getDisplayMode() == D_NORMAL) {
                _displayManager.showAddress(_radioManager.getTxAddress());
            }
            _powerManager.resetSleepTimer();
        }
        
        // Process learn/teach mode
        processLearnTeachMode();
    }
    
    // Enter sleep if needed
    if (_powerManager.getMode() == M_SLEEP) {
        do_sleep(); // Wake on WDT and RTC interrupts
    }
}

void BedRemote::handleKeypress(uint8_t key) {
    switch (key) {
        case K_ENTER:
            if (_displayManager.getDisplayMode() == D_MAIN_MENU) {
                _displayManager.selectMenuItem();
            }
            if (_displayManager.getDisplayMode() == D_TEACH_START && _learnMode == T_TEACH_WAIT) {
                _learnMode = T_TEACH_START;
            }
            break;
            
        case K_PREV:
            _displayManager.prevMenuItem();
            break;
            
        case K_NEXT:
            _displayManager.nextMenuItem();
            break;
            
        case K_MENU:
            _displayManager.showMainMenu();
            break;
            
        default: {
            uint32_t cmd = _keypadManager.getBedCommandFromKey(key);
            if (cmd != 0) {
                _radioManager.sendCommand(cmd);
                lcd_animate_sig(MAX_SIGNAL, 2);
            }
            break;
        }
    }
}

void BedRemote::processLearnTeachMode() {
    switch (_learnMode) {
        case T_LEARN_START:
            _radioManager.doLearn();
            _learnMode = T_LEARN_COMPLETE;
            _displayManager.setNeedsUpdate(true);
            break;
            
        case T_LEARN_COMPLETE:
            if (_displayManager.getDisplayMode() == D_LEARN_COMPLETE) {
                lcd_animate_sig(MAX_SIGNAL, 2);
                delay(2000);
                _displayManager.showMainMenu();
                _learnMode = T_NORMAL;
            } else {
                _displayManager.setDisplayMode(D_LEARN_COMPLETE);
            }
            break;
            
        case T_TEACH_START:
            _radioManager.doTeach();
            lcd_animate_sig(MAX_SIGNAL, 2);
            _learnMode = T_TEACH_COMPLETE;
            _displayManager.setNeedsUpdate(true);
            break;
            
        case T_TEACH_COMPLETE:
            if (_displayManager.getDisplayMode() == D_TEACH_COMPLETE) {
                lcd_animate_sig(MAX_SIGNAL, 2);
                delay(2000);
                _displayManager.showMainMenu();
                _learnMode = T_NORMAL;
            } else {
                _displayManager.setDisplayMode(D_TEACH_COMPLETE);
            }
            break;
            
        case T_NORMAL:
            if (_displayManager.getDisplayMode() == D_LEARN_START) {
                _learnMode = T_LEARN_START;
            } else if (_displayManager.getDisplayMode() == D_TEACH_START) {
                _learnMode = T_TEACH_WAIT;
            }
            break;
            
        case T_TEACH_WAIT:
            // Just wait for enter key press
            break;
    }
}
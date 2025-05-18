#include "DisplayManager.h"
#include "lcd.h"
#include "vcc.h"

DisplayManager::DisplayManager() : 
    _displayMode(D_NORMAL),
    _menuSelection(SELECT_NONE),
    _updateDisplay(true) {
}

void DisplayManager::begin() {
    lcd_init();
    // Use a smaller font that fits better on a 96x32 display
    u8g2.setFont(u8g2_font_profont10_tf);
}

void DisplayManager::updateDisplay() {
    u8g2.clearBuffer();
    updateBatteryDisplay(readVcc());
    
    switch (_displayMode) {
        case D_NORMAL: {
            u8g2.setCursor(0, 8); // Adjusted Y positions for smaller display
            u8g2.print("normal mode:");
            u8g2.setCursor(0, 16);
            u8g2.print("remote");
            u8g2.setCursor(0, 24);
            u8g2.print("control addr");
            break;
        }
        case D_MAIN_MENU: {
            u8g2.setCursor(0, 8);
            u8g2.print("Main Menu:");
            
            // Draw menu items with proper highlighting
            if (_menuSelection == SELECT_LEARN) {
                // Draw highlighted background for Learn Remote
                u8g2.setDrawColor(1); // Set to draw in white
                u8g2.drawBox(0, 9, 96, 8);
                u8g2.setDrawColor(0); // Set to draw in black (for text on white background)
                u8g2.setCursor(0, 16);
                u8g2.print("Learn Remote");
                u8g2.setDrawColor(1); // Reset to normal
            } else {
                u8g2.setDrawColor(1); // Normal drawing mode
                u8g2.setCursor(0, 16);
                u8g2.print("Learn Remote");
            }
            
            if (_menuSelection == SELECT_TEACH) {
                // Draw highlighted background for Teach Base
                u8g2.setDrawColor(1); // Set to draw in white
                u8g2.drawBox(0, 17, 96, 8);
                u8g2.setDrawColor(0); // Set to draw in black (for text on white background)
                u8g2.setCursor(0, 24);
                u8g2.print("Teach Base");
                u8g2.setDrawColor(1); // Reset to normal
            } else {
                u8g2.setDrawColor(1); // Normal drawing mode
                u8g2.setCursor(0, 24);
                u8g2.print("Teach Base");
            }
            
            if (_menuSelection == SELECT_BACK) {
                // Draw highlighted background for Back
                u8g2.setDrawColor(1); // Set to draw in white
                u8g2.drawBox(0, 25, 96, 8);
                u8g2.setDrawColor(0); // Set to draw in black (for text on white background)
                u8g2.setCursor(0, 32);
                u8g2.print("Back");
                u8g2.setDrawColor(1); // Reset to normal
            } else {
                u8g2.setDrawColor(1); // Normal drawing mode
                u8g2.setCursor(0, 32);
                u8g2.print("Back");
            }
            break;
        }
        case D_LEARN_START: {
            u8g2.setCursor(0, 8);
            u8g2.print("Learn Mode:");
            u8g2.setCursor(0, 16);
            u8g2.print("Now press");
            u8g2.setCursor(0, 24);
            u8g2.print("sync on remote");
            break;
        }
        case D_LEARN_COMPLETE: {
            u8g2.setCursor(0, 8);
            u8g2.print("Learn Mode:");
            u8g2.setCursor(0, 24);
            u8g2.print("Completed!");
            break;
        }
        case D_TEACH_START: {
            u8g2.setCursor(0, 8);
            u8g2.print("Teach Base:");
            u8g2.setCursor(0, 16);
            u8g2.print("Start sync on");
            u8g2.setCursor(0, 24);
            u8g2.print("base then");
            u8g2.setCursor(0, 32);
            u8g2.print("press enter");
            break;
        }
        case D_TEACH_COMPLETE: {
            u8g2.setCursor(0, 8);
            u8g2.print("Teach Base:");
            u8g2.setCursor(0, 24);
            u8g2.print("Completed!");
            break;
        }
        default:
            break;
    }
    
    u8g2.sendBuffer(); // Send the buffer to the display
    _updateDisplay = false;
}

void DisplayManager::updateBatteryDisplay(uint16_t batteryVoltage) {
    lcd_set_bat_shell(BAT_SHELL_ON); 
    
    if (batteryVoltage > BATT_LEVEL_HIGH) {
        lcd_set_bat(3);
    } else if (batteryVoltage > BATT_LEVEL_MED) {
        lcd_set_bat(2);
    } else if (batteryVoltage > BATT_LEVEL_LOW) {
        lcd_set_bat(1);
    } else {
        lcd_set_bat(0);
        lcd_set_bat_shell(BAT_SHELL_BLINK_SLOW);
    }
}

void DisplayManager::nextMenuItem() {
    if (_displayMode == D_MAIN_MENU) {
        switch (_menuSelection) {
            case SELECT_BACK:
            case SELECT_NONE:
                _menuSelection = SELECT_LEARN;
                break;              
            case SELECT_LEARN:
                _menuSelection = SELECT_TEACH;
                break;
            case SELECT_TEACH:
                _menuSelection = SELECT_BACK;
                break;
        }
        _updateDisplay = true;
    }
}

void DisplayManager::prevMenuItem() {
    if (_displayMode == D_MAIN_MENU) {
        switch (_menuSelection) {
            case SELECT_NONE:
            case SELECT_LEARN:
                _menuSelection = SELECT_BACK;
                break;
            case SELECT_TEACH:
                _menuSelection = SELECT_LEARN;
                break;
            case SELECT_BACK:
                _menuSelection = SELECT_TEACH;
                break;
        }
        _updateDisplay = true;
    }
}

void DisplayManager::selectMenuItem() {
    if (_displayMode == D_MAIN_MENU) {
        switch (_menuSelection) {
            case SELECT_LEARN:
                setDisplayMode(D_LEARN_START);
                break;
            case SELECT_TEACH:
                setDisplayMode(D_TEACH_START);
                break;
            case SELECT_BACK:
                setDisplayMode(D_NORMAL);
                break;
            case SELECT_NONE:
                // Do nothing
                break;
        }
    }
}

void DisplayManager::showMainMenu() {
    _displayMode = D_MAIN_MENU;
    _updateDisplay = true;
}

void DisplayManager::setDisplayMode(display_menu_t mode) {
    _displayMode = mode;
    _updateDisplay = true;
}

void DisplayManager::showAddress(uint8_t* address) {
    char addr[12];
    sprintf(addr, "%02X%02X%02X%02X%02X",
        address[0],
        address[1],
        address[2],
        address[3],
        address[4]);
    u8g2.setCursor(0, 32);
    u8g2.print(addr);
    u8g2.sendBuffer(); // Update the display
}

void DisplayManager::showChannel(uint8_t channel) {
    lcd_set_chan(channel);
}
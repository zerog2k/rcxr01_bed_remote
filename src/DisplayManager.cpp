#include "DisplayManager.h"
#include "lcd.h"
#include "vcc.h"

DisplayManager::DisplayManager() : 
    _displayMode(D_NORMAL),
    _menuSelection(SELECT_NONE),
    _updateDisplay(true) {
}

void DisplayManager::begin() {
    u8x8.begin();
    u8x8_c = u8x8.getU8x8(); // low-level c access for custom lcd funcs
    lcd_clear_all_symbols();
    u8x8.setPowerSave(0);
    u8x8.setFont(u8x8_font_victoriamedium8_r);
}

void DisplayManager::updateDisplay() {
    u8x8.clearDisplay();
    updateBatteryDisplay(readVcc());
    u8x8.setCursor(0, 0);
    
    switch (_displayMode) {
        case D_NORMAL: {
            u8x8.drawString(0, 0, "normal mode:");
            u8x8.drawString(0, 1, "remote");
            u8x8.drawString(0, 2, "control addr");
            break;
        }
        case D_MAIN_MENU: {
            u8x8.drawString(0, 0, "Main Menu:");
            u8x8.setInverseFont((_menuSelection == SELECT_LEARN) ? 1 : 0);
            u8x8.drawString(0, 1, "Learn Remote");
            u8x8.setInverseFont((_menuSelection == SELECT_TEACH) ? 1 : 0);
            u8x8.drawString(0, 2, "Teach Base");
            u8x8.setInverseFont((_menuSelection == SELECT_BACK) ? 1 : 0);
            u8x8.drawString(0, 3, "Back");
            u8x8.setInverseFont(0);
            break;
        }
        case D_LEARN_START: {
            u8x8.drawString(0, 0, "Learn Mode:");
            u8x8.drawString(0, 1, "Now press");
            u8x8.drawString(0, 2, "sync on");
            u8x8.drawString(0, 3, "remote");
            break;
        }
        case D_LEARN_COMPLETE: {
            u8x8.drawString(0, 0, "Learn Mode:");
            u8x8.drawString(0, 2, "Completed!");
            break;
        }
        case D_TEACH_START: {
            u8x8.drawString(0, 0, "Teach Base:");
            u8x8.drawString(0, 1, "Start sync");
            u8x8.drawString(0, 2, "on base then");
            u8x8.drawString(0, 3, "press enter");
            break;
        }
        case D_TEACH_COMPLETE: {
            u8x8.drawString(0, 0, "Teach Base:");
            u8x8.drawString(0, 2, "Completed!");
            break;
        }
        default:
            break;
    }
    
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
    u8x8.drawString(0, 3, addr);
}

void DisplayManager::showChannel(uint8_t channel) {
    lcd_set_chan(channel);
}
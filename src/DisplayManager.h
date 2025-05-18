#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include "RadioManager.h"

// Display menu states
typedef enum {
  D_NORMAL,
  D_MAIN_MENU,
  D_LEARN_START,
  D_LEARN_COMPLETE,
  D_TEACH_START,
  D_TEACH_COMPLETE,
  D_UNDEFINED = 0xFF
} display_menu_t;

// Menu selection states
typedef enum {
  SELECT_NONE,
  SELECT_LEARN,
  SELECT_TEACH,
  SELECT_BACK
} menu_selection_t;

class DisplayManager {
public:
    DisplayManager();
    void begin();
    
    // Display update methods
    void updateDisplay();
    void updateBatteryDisplay(uint16_t batteryVoltage);
    
    // Menu navigation
    void nextMenuItem();
    void prevMenuItem();
    void selectMenuItem();
    void showMainMenu();
    
    // Getters and setters
    display_menu_t getDisplayMode() { return _displayMode; }
    void setDisplayMode(display_menu_t mode);
    menu_selection_t getMenuSelection() { return _menuSelection; }
    bool needsUpdate() { return _updateDisplay; }
    void setNeedsUpdate(bool update) { _updateDisplay = update; }
    
    // Display helpers
    void showAddress(uint8_t* address);
    void showChannel(uint8_t channel);
    
private:
    display_menu_t _displayMode;
    menu_selection_t _menuSelection;
    bool _updateDisplay;
    
    // Battery level thresholds
    const uint16_t BATT_LEVEL_HIGH = 2500;
    const uint16_t BATT_LEVEL_MED = 2200;
    const uint16_t BATT_LEVEL_LOW = 1900;
};

#endif // DISPLAY_MANAGER_H
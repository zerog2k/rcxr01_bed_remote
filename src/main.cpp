// cloning bed remote for RCXR-01

#include <Arduino.h>
#include "BedRemote.h"
#include "PowerManager.h"

// Create the main bed remote controller
BedRemote bedRemote;

// Global variable for system mode, used by ISR in custom_keypad.h
volatile system_mode_t mode = M_ACTIVE;

void setup() {
  bedRemote.begin();
}

void loop() {
  bedRemote.loop();
}
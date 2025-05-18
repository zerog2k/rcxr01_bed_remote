#ifndef CUSTOM_KEYPAD_H
#define CUSTOM_KEYPAD_H

#include <Keypad.h>
#include "PowerManager.h"

/// keypad stuff
extern const uint8_t ROWS;
extern const uint8_t COLS;
extern uint8_t hexaKeys[5][4];
extern uint8_t rowPins[5];
extern uint8_t colPins[4];

// initialize an instance of class NewKeypad
extern Keypad customKeypad;

// Global variable for system mode, used by ISR
extern volatile system_mode_t mode;

// Function declarations
void keypad_enter_sleep();
void keypad_exit_sleep();

/// keypad layout section
// XR vs XR2 have slightly different keypad labeling

#define VARIANT_XR
// #define VARIANT_XR2

#ifdef VARIANT_XR

#define K_YES       02
#define K_PREV      K_YES
#define K_MENU      03
#define K_NO        04
#define K_NEXT      K_NO
#define K_SEL       08
#define K_Q         09
#define K_QUESTION  K_Q
#define K_PEN       13

#elif   VARIANT_XR2

#define K_PREV      02
#define K_SEL       03
#define K_NEXT      04
#define K_CHAN      13
#define K_abc       K_SEL

#endif

#define K_ENTER     01
#define K_1A        05
#define K_2B        06
#define K_3C        07

#define K_4D        10
#define K_5E        11
#define K_CLR       12
#define K_DEL       K_CLR
#define K_8H        14
#define K_9I        15
#define K_6F        16
#define K_7G        17
#define K_PLUS      18
#define K_PLUSMINUS K_PLUS
#define K_0J        19
#define K_DOT       20

#endif // CUSTOM_KEYPAD_H
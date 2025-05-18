#include "custom_keypad.h"

/// keypad stuff
const uint8_t ROWS = 5; //four rows
const uint8_t COLS = 4; //four columns

uint8_t hexaKeys[ROWS][COLS] = {
  { 1,  2,  3,  4},
  { 5,  6,  7,  8},
  { 9, 10, 11, 12},
  {13, 14, 15, 16},
  {17, 18, 19, 20}
};

uint8_t rowPins[ROWS] = {25, 27, 30, 31, 28}; //connect to the row pinouts of the keypad
uint8_t colPins[COLS] = {29, 26, 11, 12}; //connect to the column pinouts of the keypad

// initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// setup pin change interrupt for rows to wake from sleep
ISR(PCINT0_vect)
{
  if (mode == M_SLEEP)
    mode = M_WAKEUP;
}

void keypad_enter_sleep()
{
  // set columns as outputs pulled low
  DDRA |= _BV(PA5) | _BV(PA2);
  DDRD |= _BV(PD3) | _BV(PD4);
  PORTA &= ~(_BV(PA5) | _BV(PA2));
  PORTD &= ~(_BV(PD3) | _BV(PD4));

  // set rows as pulled up inputs
  DDRA &= ~(_BV(PA1) | _BV(PA3) | _BV(PA4) | _BV(PA6) | _BV(PA7));
  // set pc int on rows
  PCMSK0 |= _BV(PCINT1) | _BV(PCINT3) | _BV(PCINT4) | _BV(PCINT6) | _BV(PCINT7);
  PCIFR  |= _BV (PCIF0);
  PCICR |= _BV(PCIE0);
}

void keypad_exit_sleep()
{
  // restore colums to inputs to prevent triggering keypress event after wakeup
  DDRA &= ~(_BV(PA5) | _BV(PA2));
  DDRD &= ~(_BV(PD3) | _BV(PD4));  
}
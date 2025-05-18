#ifndef SLEEP_H
#define SLEEP_H

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/cpufunc.h>

/*
  contains various functions for sleep & wdt
  from: http://donalmorrissey.blogspot.com/2010/04/sleeping-arduino-part-5-wake-up-via.html
*/

void enable_prr();
void disable_prr();
void do_sleep(void);
void setupWatchDogTimer();

#endif // SLEEP_H
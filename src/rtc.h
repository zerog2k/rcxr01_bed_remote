#ifndef RTC_H
#define RTC_H

// rtc using timer2 and 32.768khz watch crystal
// ref: AVR130 & AVR134 app notes

#include <avr/interrupt.h>

extern volatile uint32_t seconds;

// rtc 1-sec tick
ISR(TIMER2_OVF_vect);

void rtc_init();

#endif // RTC_H
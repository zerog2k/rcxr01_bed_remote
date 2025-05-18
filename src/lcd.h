#ifndef LCD_H
#define LCD_H

/// LCD stuff for RCXR-01
#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>

#define CS 13 //PD5
#define RST 14 //PD6
#define RS 3 //PB3

// Define the U8G2 display object
extern U8G2_ST7565_RCXR01_F_4W_HW_SPI u8g2;

// custom lcd symbols on RCXR-01
// custom symbols on page 8
#define PAGENUM     0x08

// symbol addresses
// channel number ("ch#")
#define SYM_CH      0x04
// channel number lcd segments
#define CH_TENS_a   0x05
#define CH_TENS_f   0x06
#define CH_TENS_g   0x07
#define CH_TENS_e   0x08
#define CH_TENS_d   0x09
#define CH_TENS_c   0x0a
#define CH_TENS_b   0x0b

#define CH_ONES_f   0x0c
#define CH_ONES_e   0x0d
#define CH_ONES_d   0x0e
#define CH_ONES_c   0x0f
#define CH_ONES_g   0x10
#define CH_ONES_b   0x11
#define CH_ONES_a   0x12

#define NUM_SEGS  7
extern const uint8_t ch_tens_map[NUM_SEGS];
extern const uint8_t ch_ones_map[NUM_SEGS];

// digit to segment map: a,b,c,d,e,f
extern const uint8_t digits[][NUM_SEGS];

// some other symbols
#define SYM_1       0x27 
#define SYM_23      0x2d
#define SYM_A       0x37
#define SYM_BC      0x3b

// signal bar graph
#define SIG_1       0x55
#define SIG_2       0x56
#define SIG_3       0x57
#define SIG_4       0x58
#define SIG_5       0x59
#define SIG_6       0x5f
#define MAX_SIGNAL    6
extern const uint8_t signal_map[MAX_SIGNAL];

// battery bar graph
#define BAT_3       0x5a
#define BAT_2       0x5b
#define BAT_1       0x5c

#define BAT_SHELL_OFF         0x00
#define BAT_SHELL_BLINK_SLOW  0x01
#define BAT_SHELL_BLINK_FAST  0x02
#define BAT_SHELL_ON          0x03

// no (crossed circle)
#define SYM_NO      0x5d
// yes (check mark)
#define SYM_YES     0x5e

// function prototypes
void lcd_init();
void lcd_set_bat(uint8_t level);
void lcd_set_bat_shell(uint8_t state);
void lcd_set_sig(uint8_t level);
void lcd_animate_sig(uint8_t level, uint8_t loop_delay);
void lcd_clear_all_symbols();
void lcd_set_yes();
void lcd_set_no();
void lcd_set_chan(uint8_t chan);
void lcd_write(uint8_t addr, uint8_t data);

#endif // LCD_H
#include "lcd.h"

// Define global variables
U8G2_ST7565_RCXR01_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ CS, /* dc=*/ RS, /* reset=*/ RST);

const uint8_t ch_tens_map[NUM_SEGS] = { CH_TENS_a, CH_TENS_b, CH_TENS_c, CH_TENS_d, CH_TENS_e, CH_TENS_f, CH_TENS_g };
const uint8_t ch_ones_map[NUM_SEGS] = { CH_ONES_a, CH_ONES_b, CH_ONES_c, CH_ONES_d, CH_ONES_e, CH_ONES_f, CH_ONES_g };

// digit to segment map: a,b,c,d,e,f
const uint8_t digits[][NUM_SEGS] = {
//  a, b, c, d, e, f, g
  { 1, 1, 1, 1, 1, 1, 0 }, // 0
  { 0, 1, 1, 0, 0, 0, 0 }, // 1
  { 1, 1, 0, 1, 1, 0, 1 }, // 2
  { 1, 1, 1, 1, 0, 0, 1 }, // 3
  { 0, 1, 1, 0, 0, 1, 1 }, // 4
  { 1, 0, 1, 1, 0, 1, 1 }, // 5
  { 1, 0, 1, 1, 1, 1, 1 }, // 6
  { 1, 1, 1, 0, 0, 0, 0 }, // 7
  { 1, 1, 1, 1, 1, 1, 1 }, // 8
  { 1, 1, 1, 1, 0, 1, 1 }  // 9
};

const uint8_t signal_map[MAX_SIGNAL] = { SIG_1, SIG_2, SIG_3, SIG_4, SIG_5, SIG_6 };

void lcd_init() {
  u8g2.begin();
  u8g2.setPowerSave(0);
  u8g2.setFont(u8g2_font_victoriamedium8_8r);
  lcd_clear_all_symbols();
}

void lcd_set_bat(uint8_t level) {
  switch (level) {
    case 0:
      lcd_write(BAT_1, 0);
      lcd_write(BAT_2, 0);
      lcd_write(BAT_3, 0);
      break;
    case 1:
      lcd_write(BAT_1, 1);
      lcd_write(BAT_2, 0);
      lcd_write(BAT_3, 0);
      break;
    case 2:
      lcd_write(BAT_1, 1);
      lcd_write(BAT_2, 1);
      lcd_write(BAT_3, 0);
      break;
    case 3:
    default:
      lcd_write(BAT_1, 1);
      lcd_write(BAT_2, 1);
      lcd_write(BAT_3, 1);
      break;                    
  }
}

void lcd_set_bat_shell(uint8_t state) {
  if (state > BAT_SHELL_ON) {
    state = BAT_SHELL_ON;
  }
  
  // Using u8g2's command method
  u8g2.sendF("ca", 0xAD, state);
}

void lcd_set_sig(uint8_t level) {
  uint8_t i, val;

  for (i=0; i < MAX_SIGNAL; i++) {
    if (i < level)
      val = 1;
    else
      val = 0;
    lcd_write(signal_map[i], val);
  }
}

void lcd_animate_sig(uint8_t level, uint8_t loop_delay) {
  uint8_t i;
  if (level > MAX_SIGNAL)
    level = MAX_SIGNAL; 
  for (i=0; i <= level; i++) {
    lcd_set_sig(i);
    delay(loop_delay);
  }
  delay(loop_delay*2);
  for (i=level; i>0; i--) {
    lcd_set_sig(i-1);
    delay(loop_delay);    
  }
}

void lcd_clear_all_symbols() {
  // Using u8g2's command method
  u8g2.sendF("c", 0xb0 | PAGENUM);
  u8g2.sendF("c", 0x10 | 0);
  u8g2.sendF("c", 0x00 | 0);
  
  // Clear the entire page
  for (uint8_t i=0; i<255; i++) {
    u8g2.sendF("c", 0);
  }
}

void lcd_set_yes() {
  lcd_write(SYM_YES, 1);
  lcd_write(SYM_NO, 0);
}

void lcd_set_no() {
  lcd_write(SYM_NO, 1);
  lcd_write(SYM_YES, 0);
}

void lcd_set_chan(uint8_t chan) {
  uint8_t i, n;
  lcd_write(SYM_CH, 1);
  // process tens
  n = (chan / 10) % 10;
  for (i=0; i < NUM_SEGS; i++) {
    lcd_write(ch_tens_map[i], digits[n][i]);
  }
  // process ones
  n = chan % 10;
  for (i=0; i < NUM_SEGS; i++) {
    lcd_write(ch_ones_map[i], digits[n][i]);
  }
}

void lcd_write(uint8_t addr, uint8_t data) {
  // Using u8g2's command method
  u8g2.sendF("c", 0xb0 | PAGENUM);
  u8g2.sendF("c", 0x10 | (addr >> 4));
  u8g2.sendF("c", 0x00 | (addr & 0xF));
  u8g2.sendF("c", data);
}
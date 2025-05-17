// cloning bed remote for RCXR-01

#include <Arduino.h>

#include "lcd.h"
#include "vcc.h"
#include <printf.h>
#include "RF24.h"
#include "sleep.h"
#include "rtc.h"
#include "serno.h"

#include <EEPROM.h>


#define VERSION "v0.1"
#define EEPROM_MAGIC 0xDEADBEEF

/// radio stuff
#define   RF_CE   0  //PB0
#define   RF_CSN  17 //PC1

RF24 radio(RF_CE, RF_CSN);


typedef struct {
  uint8_t txaddr[5];
  uint8_t rxaddr[5];
  uint8_t channel;
} radio_config_t;

radio_config_t radio_config;

uint8_t payload[8];

// display/sleep mode stuff
uint32_t sleep_timer = 0;

typedef enum {
  M_SLEEP,
  M_WAKEUP,
  M_ACTIVE
} system_mode;

volatile system_mode mode = M_ACTIVE;

#define SLEEP_DELAY_TIME 8
#include "custom_keypad.h"

typedef enum {
  D_NORMAL,
  D_MAIN_MENU,
  D_LEARN_START,
  D_LEARN_COMPLETE,
  D_TEACH_START,
  D_TEACH_COMPLETE,
  D_UNDEFINED = 0xFF
} display_menu_t;

display_menu_t display_mode = D_NORMAL;
bool update_display = true;

typedef enum {
  SELECT_NONE,
  SELECT_LEARN,
  SELECT_TEACH,
  SELECT_BACK
} menu_selection_t;

menu_selection_t menu_selection = SELECT_NONE;

typedef enum {
  T_NORMAL,
  T_LEARN_START,
  T_LEARN_COMPLETE,
  T_TEACH_WAIT,
  T_TEACH_START,
  T_TEACH_COMPLETE
} learn_mode_t;

learn_mode_t learn_mode = T_NORMAL;

/// usb voltage detection stuff
#define USB_DETECT_PIN  2   // PB2
uint8_t on_battery;
#define BATT_LEVEL_HIGH 2500
#define BATT_LEVEL_MED  2200
#define BATT_LEVEL_LOW  1900

uint8_t i;

// fetches radio config struct from eeprom, if stored
// otherwise returns default configuration

radio_config_t load_radio_config()
{
  uint32_t magic_check = 0;
  radio_config_t radio_config = {
    // defaults
    .txaddr = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01 },
    .rxaddr = { 0x94, 0x69, 0x96, 0x69, 0x96 },
    .channel = 1
  };
  EEPROM.get(0, magic_check);
  if (magic_check == EEPROM_MAGIC)
  {
    EEPROM.get(sizeof(magic_check), radio_config);
  }
  return radio_config;
}

// stores radio config struct in eeprom
uint8_t save_radio_config(radio_config_t radio_config)
{
   uint32_t magic = EEPROM_MAGIC;
   EEPROM.put(0, magic);
   EEPROM.put(sizeof(magic), radio_config);
}

void update_battery_display()
{
    uint16_t batt_mv = readVcc();
    lcd_set_bat_shell(BAT_SHELL_ON); 
    if (batt_mv > BATT_LEVEL_HIGH)
      lcd_set_bat(3);
    else if (batt_mv > BATT_LEVEL_MED)
      lcd_set_bat(2);
    else if (batt_mv > BATT_LEVEL_LOW)
      lcd_set_bat(1);
    else
    {
      lcd_set_bat(0);
      lcd_set_bat_shell(BAT_SHELL_BLINK_SLOW);
    }
}

void do_display_update(display_menu_t dm)
{
  u8x8.clearDisplay();
  update_battery_display();
  u8x8.setCursor(0,0);
  switch (dm)
  {
    case D_NORMAL:
      char addr[12];
      sprintf(addr, "%02X%02X%02X%02X%02X",
        radio_config.txaddr[0],
        radio_config.txaddr[1],
        radio_config.txaddr[2],
        radio_config.txaddr[3],
        radio_config.txaddr[4]);
      u8x8.drawString(0,3, addr);
      u8x8.drawString(0,0, "normal mode:");
      u8x8.drawString(0,1, "remote");
      u8x8.drawString(0,2, "control addr");
      break;
    case D_MAIN_MENU:
      u8x8.drawString(0,0, "Main Menu:");
      u8x8.setInverseFont((menu_selection == SELECT_LEARN) ? 1 : 0);
      u8x8.drawString(0,1, "Learn Remote");
      u8x8.setInverseFont((menu_selection == SELECT_TEACH) ? 1 : 0);
      u8x8.drawString(0,2, "Teach Base");
      u8x8.setInverseFont((menu_selection == SELECT_BACK) ? 1 : 0);
      u8x8.drawString(0,3, "Back");
      u8x8.setInverseFont(0);
      break;
    case D_LEARN_START:
      u8x8.drawString(0,0, "Learn Mode:");
      u8x8.drawString(0,1, "Now press");
      u8x8.drawString(0,2, "sync on");
      u8x8.drawString(0,3, "remote");
      break;
    case D_LEARN_COMPLETE:
      u8x8.drawString(0,0, "Learn Mode:");
      u8x8.drawString(0,2, "Completed!");
      break;
    case D_TEACH_START:
      u8x8.drawString(0,0, "Teach Base:");
      u8x8.drawString(0,1, "Start sync");
      u8x8.drawString(0,2, "on base then");
      u8x8.drawString(0,3, "press enter");
      break;
    case D_TEACH_COMPLETE:
      u8x8.drawString(0,0, "Teach Base:");
      u8x8.drawString(0,2, "Completed!");
      delay(2000);
      break;
    default:
    break;
  }
}

uint32_t get_bed_cmd_from_key(uint8_t key)
{
  uint32_t val = 0;
  switch (key)
  {
    case K_4D:
      val = 0x01000000; // head up
      break;
    case K_5E:
      val = 0x00000200; // lamp
      break;
    case K_6F:
      val = 0x04000000; // feet up
      break;
    case K_7G:
      val = 0x02000000; // head down
      break;
    case K_8H:
      val = 0x00800000; // snore/zzz
    case K_9I:
      val = 0x08000000; // feet down
      break;
    case K_PLUSMINUS:
      val = 0x00400000; // TV
      break;
    case K_0J:
      val = 0x00000008; // flat
      break;      
    case K_DOT:
      val = 0x00000100; // star
      break;
  }
  return val;
}

// bed remote stuff
/// message header bytes
// 0: message body size
// 1: message counter
// 2: message type?
// 3+: message body

// known message types
#define   MT_NORMAL       0x03
#define   MT_PAIR_INIT    0x04
#define   MT_PAIR_DONE    0x05
#define   MT_BASE_STATUS  0x06

#define MAX_CHANNELS  82
const uint8_t pairaddr[5] = {0x94, 0x69, 0x96, 0x69, 0x96};
uint8_t buf[32];
uint8_t msgbuf[32];
uint8_t sendctr = 0;

bool do_teach()
{
  // transmit pair offer to waiting base in pairing mode
  uint32_t pair_timeout = seconds + 20;
  const uint8_t pipeaddr_offset = 0x04;
  bool base_pair_done = false;
  while (! base_pair_done && pair_timeout > seconds && customKeypad.getKey() != K_MENU )
  {
    for (uint8_t i = 0; i < MAX_CHANNELS; i++)
    {
      // roll through channels and broadcast pairing request
      // until we find a base accepting our requests
      radio.setChannel(i);
      radio.stopListening();
      radio.openWritingPipe(pairaddr); // 0x9669966994
      // msg header    
      buf[0] = 0x04; // msg body size
      buf[1] = 0x00; // msg counter, dont care
      buf[2] = MT_NORMAL; // msg type, normal ?
      // msg body
      buf[3] = 0;    // dont care
      buf[4] = 0;
      buf[5] = 0;
      buf[6] = 0; 
      // Send a dummy message. If ack'ed we know
      // we are on the channel where base is listening
      // some bases want to see multiple requests
      if (radio.write(buf, 7) && radio.write(buf, 7))
      {
        // msg header
        buf[0] = 0x07; // msg body size
        buf[1] = 0x00; // msg counter, dont care
        buf[2] = MT_PAIR_INIT; // msg type: pair init
        // msg body
        buf[3] = radio_config.channel; // desired channel
        buf[4] = pipeaddr_offset; // desired base pipe addr offset
        buf[5] = radio_config.txaddr[0] - pipeaddr_offset;
        buf[6] = radio_config.txaddr[1];
        buf[7] = radio_config.txaddr[2];
        buf[8] = radio_config.txaddr[3];
        buf[9] = radio_config.txaddr[4];
        //printf("send pair init. ");
        // bases likes it sent multiple times ?
        if (radio.write(buf, 10) && radio.write(buf, 10) && radio.write(buf, 10))
        {
          // msg header
          buf[0] = 0x01; // msg body size
          buf[1] = 0x00; // msg counter, dont care
          buf[2] = MT_PAIR_DONE; // msg type
          // msg body
          buf[3] = 1; // success?         
          //printf("send pair done. ");
          // some bases like it multiple times ?
          if (radio.write(buf, 4) && radio.write(buf, 4))
          {
            //printf("ack'd successfully.\n");
            base_pair_done = true;
          }
          else
          {
            //printf("NO ack from receiver.\n");
          }
        }
      }
    }
    delay(100);
  }

  // ensure we are back on remotes chan & addr
  radio.setChannel(radio_config.channel); // move back to remote chan
  lcd_set_chan(radio_config.channel);
  radio.openWritingPipe(radio_config.txaddr);
  //radio.openReadingPipe(1, pipeaddr);
  return base_pair_done;
}

bool do_learn()
{
  // listen on pairing address
  // wait for remote in pairing mode to send pair offer
  uint8_t pipenum, pairing_ctr;
  uint32_t pair_timeout = seconds + 20;
  uint8_t pipeaddr[5];
  uint8_t pipeaddr_offset;
  bool remote_pair_started = false;
  bool remote_pair_done = false;
  radio.openReadingPipe(1, pairaddr);
  radio.startListening();
  lcd_write(SYM_1, 0);
  lcd_write(SYM_23, 0);
  while (pair_timeout > seconds && customKeypad.getKey() != K_MENU )
  {
    if (radio.available(&pipenum))
    {
      // read msg header
      uint8_t dplsz = radio.getDynamicPayloadSize();
      radio.read(&buf, dplsz);
      uint8_t msgsize = buf[0];
      uint8_t msgcounter = buf[1];
      uint8_t msgtype = buf[2];

      // read msg body
      memcpy(msgbuf, &buf[3], msgsize);

      if (msgtype == MT_PAIR_INIT) 
      {
        // received a message type for pair init
        // a remote decides the channel and pipe to move to
        // as a receiver, we just follow orders
        // get channel & rx pipe addr from message
        // also track msgctr - should not change over pairing sequence
        remote_pair_started = true;   
        lcd_write(SYM_1, 1);
    
        pairing_ctr = msgcounter;
        radio_config.channel = msgbuf[0];
        // I suspect the remote pipe address is the last 5 bytes, i.e. msgbuf[2] through [5]
        // and msgbuf[1] is the "offset" above base address which bed receiver should listen
        // but this "offset" always seems to be 4, in my limited testing
        pipeaddr_offset = msgbuf[1];
        radio_config.txaddr[0] = msgbuf[2] + pipeaddr_offset;
        radio_config.txaddr[1] = msgbuf[3];
        radio_config.txaddr[2] = msgbuf[4];
        radio_config.txaddr[3] = msgbuf[5];
        radio_config.txaddr[4] = msgbuf[6];
        //printf("remote pair init request\n");
      } 
      else if (msgtype == MT_PAIR_DONE && remote_pair_started && pairing_ctr == msgcounter) 
      {
        // remote acked our autoack to pair init, he expects us to 
        // change pipe addr & chan, persist
        radio.openReadingPipe(1, radio_config.txaddr);  
        radio.setChannel(radio_config.channel);
        lcd_set_chan(radio_config.channel);     
        //printf("remote pair done, switched to chan: %02d, rxpipe: ", chan);
        // for (uint8_t i=0; i < sizeof(pipeaddr); i++) {
        //   printf("%02X ", pipeaddr[i]);
        // }
        //printf("\n");
        remote_pair_started = false;
        remote_pair_done = true;
        lcd_write(SYM_23, 1);
        save_radio_config(radio_config);
        break;
        // if (pipenum == 0)
        // {
        //   // this came from pairing address
        //   // we probably re-paired with a new remote
        //   // so initiate re-pair with base
        //   base_pair_done = false;      
        // }
      }
      else if (remote_pair_started && pairing_ctr != msgcounter)
      {
        // we missed pairing message sequence, reset state
        remote_pair_started = false;
        lcd_write(SYM_1, 0);
        lcd_write(SYM_23, 0);
        //printf("remote pairing state reset\n");
      }
    }
  }
  radio.stopListening();
  return remote_pair_done;
}

bool send_tx_command(uint32_t cmd)
{
    bool result = false;
    radio.setChannel(radio_config.channel);
    radio.openWritingPipe(radio_config.txaddr);
    // msg header    
    buf[0] = 0x04; // msg body size
    buf[1] = sendctr++; // msg counter, dont care
    buf[2] = MT_NORMAL; // msg type, normal ?
    // msg body
    buf[3] = cmd;
    buf[4] = cmd >>  8;
    buf[5] = cmd >> 16;
    buf[6] = cmd >> 24;

    result = radio.write(buf, 7);
    return result;
}

void setup()
{
  // change clock from 8 to 1MHz to ensure safe operating range down to
  // brown-out battery voltage of 1.8V
  clock_prescale_set(clock_div_8);
  
  delay(100);

  Serial.begin(9600);
  
  u8x8.begin();
  u8x8_c = u8x8.getU8x8(); // low-level c access for custom lcd funcs
  lcd_clear_all_symbols();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_victoriamedium8_r);
  u8x8.drawString(0,0,"bed remote");
  u8x8.drawString(0,1, VERSION);
  u8x8.setCursor(0,1);

  // usb detect
  pinMode(USB_DETECT_PIN, INPUT_PULLUP);


  radio_config = load_radio_config();
  // radio setup
  radio.begin();
  radio.openWritingPipe(radio_config.txaddr);
  //radio.openReadingPipe(1, radio_config.rxaddr);
  radio.enableDynamicPayloads(); // esb
  radio.setAutoAck(true);
  radio.setChannel(radio_config.channel);
  radio.setCRCLength(RF24_CRC_16);
  //radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_1MBPS);
  printf_begin();
  Serial.println("starting...");
  radio.printDetails();
  //radio.startListening();  // dont use this unless we need to - consumes about 13mA
  lcd_set_chan(radio.getChannel());
  // set up wdt as wake-up timer
  setupWatchDogTimer();
  Serial.print("rtc_init start.");
  rtc_init();
  Serial.println("rtc_init done.");
  print_ser_num();
  print_lot_code();
  keypad_enter_sleep();
  sei();
}
  
void loop()
{
  if (mode == M_WAKEUP)
  {
    // transition from WAKE to ACTIVE
    keypad_exit_sleep();
    sleep_timer = seconds;
    u8x8.setPowerSave(0);
    radio.powerUp();
    mode = M_ACTIVE;
    display_mode = D_NORMAL;
    learn_mode = T_NORMAL;
    update_display = true;
    delay(10);
    return;
  }

  on_battery = digitalRead(USB_DETECT_PIN);
  if (! on_battery)
  {
    // on usb power, reset sleep timer or wake
    sleep_timer = seconds;
    if (mode == M_SLEEP)
      mode = M_WAKEUP;
  }

  if ( mode == M_ACTIVE && 
      ( seconds - sleep_timer ) > SLEEP_DELAY_TIME)
  {
    // transition from ACTIVE to SLEEP
    mode = M_SLEEP;
    u8x8.setPowerSave(1);
    radio.powerDown();
    keypad_enter_sleep();
  }

  // main operational area
  if (mode == M_ACTIVE)
  {
    // check for keypress
    uint8_t keypress = customKeypad.getKey();
    if (keypress) 
    {
      sleep_timer = seconds; // reset sleep timer
      switch (keypress)
      {
        case K_ENTER:
          if (display_mode == D_MAIN_MENU)
          {
            switch (menu_selection)
            {
              case SELECT_LEARN:
                display_mode = D_LEARN_START;
                update_display = true;
                break;
              case SELECT_TEACH:
                display_mode = D_TEACH_START;
                update_display = true;
                break;
              case SELECT_BACK:
                display_mode = D_NORMAL;
                update_display = true;
                break;
            }
          }
          if (display_mode == D_TEACH_START && learn_mode == T_TEACH_WAIT)
            learn_mode = T_TEACH_START;
          break;
        case K_PREV:
          if (display_mode == D_MAIN_MENU)
            switch (menu_selection)
            {
              case SELECT_NONE:
              case SELECT_LEARN:
                menu_selection = SELECT_BACK;
                update_display = true;
                break;
              case SELECT_TEACH:
                menu_selection = SELECT_LEARN;
                update_display = true;
                break;
              case SELECT_BACK:
                menu_selection = SELECT_TEACH;
                update_display = true;
                break;
            }
          break;
        case K_NEXT:
          if (display_mode == D_MAIN_MENU)
            switch (menu_selection)
            {
              case SELECT_BACK:
              case SELECT_NONE:
                menu_selection = SELECT_LEARN;
                update_display = true;
                break;              
              case SELECT_LEARN:
                menu_selection = SELECT_TEACH;
                update_display = true;
                break;
              case SELECT_TEACH:
                menu_selection = SELECT_BACK;
                update_display = true;
                break;
            }
          break;        
        case K_MENU:
          display_mode = D_MAIN_MENU;
          update_display = true;
          break;
        default:
          uint32_t cmd = get_bed_cmd_from_key(keypress);
          send_tx_command(cmd);
          lcd_animate_sig();
          break;
      }
    }
    // check & redraw display
    if (update_display)
    {
      do_display_update(display_mode);
      update_display = false;
      sleep_timer = seconds; // reset sleep timer
    }

    // check menu state for teach/learn mode
    switch (learn_mode)
    {
      case T_LEARN_START:
        do_learn();
        //lcd_animate_sig();
        //delay(2000);
        learn_mode = T_LEARN_COMPLETE;
        update_display = true;
        break;
      case T_LEARN_COMPLETE:
        if (display_mode == D_LEARN_COMPLETE)
        {
          lcd_animate_sig();
          delay(2000);
          display_mode = D_MAIN_MENU;
          learn_mode = T_NORMAL;
          update_display = true;
        }
        else
        {
          display_mode = D_LEARN_COMPLETE;
          update_display = true;
        }
        break;
      case T_TEACH_START:
        do_teach();
        lcd_animate_sig();
        //delay(2000);
        learn_mode = T_TEACH_COMPLETE;
        update_display = true;
        break;
      case T_TEACH_COMPLETE:
        if (display_mode == D_TEACH_COMPLETE)
        {
          lcd_animate_sig();
          delay(2000);
          display_mode = D_MAIN_MENU;
          learn_mode = T_NORMAL;
          update_display = true;
        }
        else
        {
          display_mode = D_TEACH_COMPLETE;
          update_display = true;
        }
        break;
      case T_NORMAL:
        if (display_mode == D_LEARN_START)
          learn_mode = T_LEARN_START;
        else if (display_mode == D_TEACH_START)
          learn_mode = T_TEACH_WAIT;
        break;
    }
  }

  if (mode == M_SLEEP)
    do_sleep(); // wake on wdt and rtc interrupts
}

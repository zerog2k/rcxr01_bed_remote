#include "RadioManager.h"
#include "lcd.h"
#include "rtc.h"

extern volatile uint32_t seconds;

RadioManager::RadioManager(uint8_t cePin, uint8_t csnPin) : _radio(cePin, csnPin), _sendctr(0) {
    // Initialize with default values
    memset(&_config, 0, sizeof(_config));
}

void RadioManager::begin() {
    // Load configuration from EEPROM
    _config = loadConfig();
    
    // Initialize radio
    _radio.begin();
    _radio.openWritingPipe(_config.txaddr);
    _radio.enableDynamicPayloads();
    _radio.setAutoAck(true);
    _radio.setChannel(_config.channel);
    _radio.setCRCLength(RF24_CRC_16);
    _radio.setDataRate(RF24_1MBPS);
}

RadioManager::radio_config_t RadioManager::loadConfig() {
    uint32_t magic_check = 0;
    radio_config_t config = {
        // defaults
        .txaddr = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01 },
        .rxaddr = { 0x94, 0x69, 0x96, 0x69, 0x96 },
        .channel = 1
    };
    
    EEPROM.get(0, magic_check);
    if (magic_check == EEPROM_MAGIC) {
        EEPROM.get(sizeof(magic_check), config);
    }
    return config;
}

bool RadioManager::saveConfig(radio_config_t config) {
    uint32_t magic = EEPROM_MAGIC;
    EEPROM.put(0, magic);
    EEPROM.put(sizeof(magic), config);
    _config = config;
    return true;
}

bool RadioManager::sendCommand(uint32_t cmd) {
    bool result = false;
    _radio.setChannel(_config.channel);
    _radio.openWritingPipe(_config.txaddr);
    
    // msg header    
    _buf[0] = 0x04;           // msg body size
    _buf[1] = _sendctr++;     // msg counter
    _buf[2] = MT_NORMAL;      // msg type, normal
    
    // msg body
    _buf[3] = cmd;
    _buf[4] = cmd >> 8;
    _buf[5] = cmd >> 16;
    _buf[6] = cmd >> 24;

    result = _radio.write(_buf, 7);
    return result;
}

bool RadioManager::doTeach() {
    // transmit pair offer to waiting base in pairing mode
    uint32_t pair_timeout = seconds + 20;
    const uint8_t pipeaddr_offset = 0x04;
    bool base_pair_done = false;
    
    while (!base_pair_done && pair_timeout > seconds) {
        for (uint8_t i = 0; i < MAX_CHANNELS; i++) {
            // roll through channels and broadcast pairing request
            // until we find a base accepting our requests
            _radio.setChannel(i);
            _radio.stopListening();
            _radio.openWritingPipe(_pairaddr);
            
            // msg header    
            _buf[0] = 0x04;       // msg body size
            _buf[1] = 0x00;       // msg counter, dont care
            _buf[2] = MT_NORMAL;  // msg type, normal
            
            // msg body
            _buf[3] = 0;    // dont care
            _buf[4] = 0;
            _buf[5] = 0;
            _buf[6] = 0; 
            
            // Send a dummy message. If ack'ed we know
            // we are on the channel where base is listening
            if (_radio.write(_buf, 7) && _radio.write(_buf, 7)) {
                // msg header
                _buf[0] = 0x07;           // msg body size
                _buf[1] = 0x00;           // msg counter, dont care
                _buf[2] = MT_PAIR_INIT;   // msg type: pair init
                
                // msg body
                _buf[3] = _config.channel;  // desired channel
                _buf[4] = pipeaddr_offset;  // desired base pipe addr offset
                _buf[5] = _config.txaddr[0] - pipeaddr_offset;
                _buf[6] = _config.txaddr[1];
                _buf[7] = _config.txaddr[2];
                _buf[8] = _config.txaddr[3];
                _buf[9] = _config.txaddr[4];
                
                // bases likes it sent multiple times
                if (_radio.write(_buf, 10) && _radio.write(_buf, 10) && _radio.write(_buf, 10)) {
                    // msg header
                    _buf[0] = 0x01;           // msg body size
                    _buf[1] = 0x00;           // msg counter, dont care
                    _buf[2] = MT_PAIR_DONE;   // msg type
                    
                    // msg body
                    _buf[3] = 1;              // success?
                    
                    // some bases like it multiple times
                    if (_radio.write(_buf, 4) && _radio.write(_buf, 4)) {
                        base_pair_done = true;
                        break;
                    }
                }
            }
        }
        delay(100);
    }

    // ensure we are back on remotes chan & addr
    _radio.setChannel(_config.channel);
    lcd_set_chan(_config.channel);
    _radio.openWritingPipe(_config.txaddr);
    
    return base_pair_done;
}

bool RadioManager::doLearn() {
    // listen on pairing address
    // wait for remote in pairing mode to send pair offer
    uint8_t pipenum, pairing_ctr;
    uint32_t pair_timeout = seconds + 20;
    uint8_t pipeaddr[5];
    uint8_t pipeaddr_offset;
    bool remote_pair_started = false;
    bool remote_pair_done = false;
    
    _radio.openReadingPipe(1, _pairaddr);
    _radio.startListening();
    lcd_write(SYM_1, 0);
    lcd_write(SYM_23, 0);
    
    while (pair_timeout > seconds) {
        if (_radio.available(&pipenum)) {
            // read msg header
            uint8_t dplsz = _radio.getDynamicPayloadSize();
            _radio.read(&_buf, dplsz);
            uint8_t msgsize = _buf[0];
            uint8_t msgcounter = _buf[1];
            uint8_t msgtype = _buf[2];

            // read msg body
            memcpy(_msgbuf, &_buf[3], msgsize);

            if (msgtype == MT_PAIR_INIT) {
                // received a message type for pair init
                remote_pair_started = true;   
                lcd_write(SYM_1, 1);
        
                pairing_ctr = msgcounter;
                _config.channel = _msgbuf[0];
                
                pipeaddr_offset = _msgbuf[1];
                _config.txaddr[0] = _msgbuf[2] + pipeaddr_offset;
                _config.txaddr[1] = _msgbuf[3];
                _config.txaddr[2] = _msgbuf[4];
                _config.txaddr[3] = _msgbuf[5];
                _config.txaddr[4] = _msgbuf[6];
            } 
            else if (msgtype == MT_PAIR_DONE && remote_pair_started && pairing_ctr == msgcounter) {
                // remote acked our autoack to pair init
                _radio.openReadingPipe(1, _config.txaddr);  
                _radio.setChannel(_config.channel);
                lcd_set_chan(_config.channel);     
                
                remote_pair_started = false;
                remote_pair_done = true;
                lcd_write(SYM_23, 1);
                saveConfig(_config);
                break;
            }
            else if (remote_pair_started && pairing_ctr != msgcounter) {
                // we missed pairing message sequence, reset state
                remote_pair_started = false;
                lcd_write(SYM_1, 0);
                lcd_write(SYM_23, 0);
            }
        }
    }
    
    _radio.stopListening();
    return remote_pair_done;
}
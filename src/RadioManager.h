#ifndef RADIO_MANAGER_H
#define RADIO_MANAGER_H

#include <Arduino.h>
#include <RF24.h>
#include <EEPROM.h>

#define EEPROM_MAGIC 0xDEADBEEF
#define MAX_CHANNELS 82

// Known message types
#define MT_NORMAL       0x03
#define MT_PAIR_INIT    0x04
#define MT_PAIR_DONE    0x05
#define MT_BASE_STATUS  0x06

class RadioManager {
public:
    typedef struct {
        uint8_t txaddr[5];
        uint8_t rxaddr[5];
        uint8_t channel;
    } radio_config_t;

    RadioManager(uint8_t cePin, uint8_t csnPin);
    void begin();
    
    // Configuration management
    radio_config_t loadConfig();
    bool saveConfig(radio_config_t config);
    
    // Radio operations
    bool sendCommand(uint32_t cmd);
    bool doTeach();
    bool doLearn();
    
    // Getters
    uint8_t getChannel() { return _config.channel; }
    uint8_t* getTxAddress() { return _config.txaddr; }

    // Make radio public for direct access
    RF24 _radio;

private:
    radio_config_t _config;
    uint8_t _buf[32];
    uint8_t _msgbuf[32];
    uint8_t _sendctr;
    const uint8_t _pairaddr[5] = {0x94, 0x69, 0x96, 0x69, 0x96};
};

#endif // RADIO_MANAGER_H
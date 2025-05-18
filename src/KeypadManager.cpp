#include "KeypadManager.h"

KeypadManager::KeypadManager() {
}

uint8_t KeypadManager::getKey() {
    return customKeypad.getKey();
}

uint32_t KeypadManager::getBedCommandFromKey(uint8_t key) {
    uint32_t val = 0;
    switch (key) {
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
            break;
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
/*  Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Sysbotbase3_ControllerState_H
#define PokemonAutomation_Sysbotbase3_ControllerState_H

#include <stdint.h>

namespace PokemonAutomation{
namespace NintendoSwitch{



struct Sysbotbase3_ControllerState{
    uint64_t buttons = 0;
    int16_t left_joystick_x = 0;
    int16_t left_joystick_y = 0;
    int16_t right_joystick_x = 0;
    int16_t right_joystick_y = 0;

    bool is_neutral() const{
        return buttons == 0
            && left_joystick_x == 0
            && left_joystick_y == 0
            && right_joystick_x == 0
            && right_joystick_y == 0;
    }
    void clear(){
        buttons = 0;
        left_joystick_x = 0;
        left_joystick_y = 0;
        right_joystick_x = 0;
        right_joystick_y = 0;
    }
};



struct Sysbotbase3_ControllerCommand{
    uint64_t seqnum;
    uint64_t milliseconds;
    Sysbotbase3_ControllerState state;

    void write_to_hex(char str[64]) const{
        const char HEX_DIGITS[] = "0123456789abcdef";
        const char* ptr = (const char*)this;
        for (size_t c = 0; c < 64; c += 2){
            uint8_t hi = (uint8_t)ptr[0] >> 4;
            uint8_t lo = (uint8_t)ptr[0] & 0x0f;
            str[c + 0] = HEX_DIGITS[hi];
            str[c + 1] = HEX_DIGITS[lo];
            ptr++;
        }
    }
    void parse_from_hex(const char str[64]){
        char* ptr = (char*)this;
        for (size_t c = 0; c < 64; c += 2){
            char hi = str[c + 0];
            char lo = str[c + 1];
            hi = hi < 'a' ? hi - '0' : hi - 'a' + 10;
            lo = lo < 'a' ? lo - '0' : lo - 'a' + 10;
            ptr[0] = hi << 4 | lo;
            ptr++;
        }
    }
};




}
}
#endif

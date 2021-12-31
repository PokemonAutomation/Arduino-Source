/*  Digit Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Messages_DigitEntry_H
#define PokemonAutomation_NintendoSwitch_Messages_DigitEntry_H

#include <string.h>
#include "Common/NintendoSwitch/NintendoSwitch_Protocol_DigitEntry.h"
#include "ClientSource/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class DeviceRequest_enter_digits : public BotBaseRequest{
public:
    pabb_enter_digits params;
    DeviceRequest_enter_digits(uint8_t count, const uint8_t* digits)
        : BotBaseRequest(true)
    {
        params.count = count;
        memset(params.digit_pairs, 0, sizeof(params.digit_pairs));
        for (uint8_t c = 0; c < count; c++){
            params.digit_pairs[c/2] |= convert_digit(digits[c]) << 4 * (c & 1);
        }
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_ENTER_DIGITS, params);
    }
private:
    static uint8_t convert_digit(uint8_t digit){
        if (digit >= '0'){
            digit -= '0';
        }
        if (digit > 9){
            digit = 0;
        }
        return digit;
    }
};



}
}
#endif

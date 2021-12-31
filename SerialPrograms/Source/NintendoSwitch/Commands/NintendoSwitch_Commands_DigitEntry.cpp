/*  Digit Entry
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <cstring>
#include <sstream>
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch_Commands_DigitEntry.h"
#include "NintendoSwitch_Messages_DigitEntry.h"


#if 0
void enter_digits_str(uint8_t count, const char* digits){
    enter_digits(count, (const uint8_t*)digits);
}
void enter_digits(uint8_t count, const uint8_t* digits){
    enter_digits(*PokemonAutomation::global_connection, count, digits);
}
#endif


namespace PokemonAutomation{
namespace NintendoSwitch{



void enter_digits_str(const BotBaseContext& context, uint8_t count, const char* digits){
    enter_digits(context, count, (const uint8_t*)digits);
}
uint8_t convert_digit(uint8_t digit){
    if (digit >= '0'){
        digit -= '0';
    }
    if (digit > 9){
        digit = 0;
    }
    return digit;
}
void enter_digits(const BotBaseContext& context, uint8_t count, const uint8_t* digits){
    context.issue_request(DeviceRequest_enter_digits(count, digits));
}



int register_message_converters_switch_digit_entry(){
    register_message_converter(
        PABB_MSG_COMMAND_ENTER_DIGITS,
        [](const std::string& body){
            std::stringstream ss;
            ss << "enter_digits() - ";
            if (body.size() != sizeof(pabb_enter_digits)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_enter_digits*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", count = " << (unsigned)params->count;
            ss << ", digits = ";
            for (uint8_t c = 0; c < params->count; c++){
                ss << ((params->digit_pairs[c / 2] >> 4 * (c & 1)) & 0x0f);
            }
            return ss.str();
        }
    );
    return 0;
}
int init_SwitchDigitEntry = register_message_converters_switch_digit_entry();


}
}

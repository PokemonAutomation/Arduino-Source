/*  Digit Entry
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "SwitchDigitEntry.h"

#include <string.h>
#include <sstream>
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/SwitchRoutines/SwitchDigitEntry.h"
#include "Common/PokemonSwSh/PokemonSwShAutoHosts.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Libraries/MessageConverter.h"


#if 0
void enter_digits_str(uint8_t count, const char* digits){
    enter_digits(count, (const uint8_t*)digits);
}
void enter_digits(uint8_t count, const uint8_t* digits){
    enter_digits(*PokemonAutomation::global_connection, count, digits);
}
#endif


namespace PokemonAutomation{



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
    pabb_enter_digits params;
    params.count = count;
    memset(params.digit_pairs, 0, sizeof(params.digit_pairs));
    for (uint8_t c = 0; c < count; c++){
        params.digit_pairs[c/2] |= convert_digit(digits[c]) << 4 * (c & 1);
    }
    context->issue_request<PABB_MSG_COMMAND_ENTER_DIGITS>(&context.cancelled_bool(), params);
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

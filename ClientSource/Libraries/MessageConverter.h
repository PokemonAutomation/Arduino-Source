/*  Message Pretty Printing
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <string>
#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{


using MessageConverter = std::string (*)(const std::string& body);
void register_message_converter(uint8_t type, MessageConverter converter);

std::string message_to_string(const BotBaseMessage& message);


std::string program_name(uint8_t id);


}


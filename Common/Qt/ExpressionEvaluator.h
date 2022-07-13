/*  Simple (and incomplete) Expression Evaluator
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <stdint.h>
#include <string>

namespace PokemonAutomation{

uint32_t parse_ticks_ui32(const std::string& expression);

int32_t parse_ticks_i32(const std::string& expression);


}

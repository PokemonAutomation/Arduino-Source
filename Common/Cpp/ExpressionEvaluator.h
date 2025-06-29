/*  Simple (and incomplete) Expression Evaluator
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include <string>
#include <map>

namespace PokemonAutomation{



int64_t parse_expression(
    const std::map<std::string, int64_t>& variables,
    const std::string& expression
);

uint32_t parse_ticks_ui32(const std::string& expression);
int32_t parse_ticks_i32(const std::string& expression);


}

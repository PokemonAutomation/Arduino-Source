/*  Pretty Printing
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_PrettyPrint_H
#define PokemonAutomation_PrettyPrint_H

#include <string>
#include <set>

namespace PokemonAutomation{

std::string tostr_u_commas(uint64_t x);
std::string tostr_fixed(double x, int precision);
std::string ticks_to_time(uint64_t ticks);

std::string now_to_filestring();

std::string set_to_str(const std::set<std::string>& set);

}
#endif


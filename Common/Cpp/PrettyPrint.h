/*  Pretty Printing
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_PrettyPrint_H
#define PokemonAutomation_PrettyPrint_H

#include <string>
#include <set>
#include <chrono>

namespace PokemonAutomation{

std::string to_string_padded(size_t digits, uint64_t x);
std::string tostr_u_commas(uint64_t x);
std::string tostr_fixed(double x, int precision);

std::string now_to_filestring();

std::string set_to_str(const std::set<std::string>& set);

std::string duration_to_string(std::chrono::milliseconds milliseconds);

}
#endif


/*  Pretty Printing
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_PrettyPrint_H
#define PokemonAutomation_PrettyPrint_H

#include <string>
#include <set>
#include <chrono>

namespace PokemonAutomation{

std::string tostr_padded(size_t digits, uint64_t x);
std::string tostr_u_commas(int64_t x);
std::string tostr_bytes(uint64_t bytes);

// Convert double to string using the default precision on ostream.
std::string tostr_default(double x);
// Convert double to string with fixed precision.
// The precision specifies the number of digits after the decimal point.
std::string tostr_fixed(double x, int precision);

// Format current time to a string to be used as filenames.
// e.g. "20220320-044444408355"
std::string now_to_filestring();

std::string set_to_str(const std::set<std::string>& set);

std::string duration_to_string(std::chrono::milliseconds milliseconds);

std::string tostr_hex(uint64_t x);

}
#endif


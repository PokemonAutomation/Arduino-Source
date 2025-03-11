/*  String Tools (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StringToolsQt_H
#define PokemonAutomation_StringToolsQt_H

#include <string>

namespace PokemonAutomation{


std::u32string to_utf32(const std::string& str);
std::string to_utf8(const std::u32string& str);

std::string to_lower(const std::string& str);
void to_lower(std::u32string& str);

bool is_alphanumberic(char32_t ch);

bool has_extension(const std::string& str, const std::string& extension);


}
#endif

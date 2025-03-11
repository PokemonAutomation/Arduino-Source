/*  Unicode
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_Unicode_H
#define PokemonAutomation_Unicode_H

#include <string>

namespace PokemonAutomation{

std::u16string utf8_to_utf16(const std::string& str);

#ifdef _WIN32
std::wstring utf8_to_wstr(const std::string& str);
#endif

}
#endif

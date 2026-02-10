/*  Unicode
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_Unicode_H
#define PokemonAutomation_Unicode_H

#include <string>

namespace PokemonAutomation{



//
//  char* and std::string are UTF-8.
//

std::u8string utf8_to_utf8(const std::string& str);
std::string utf8_to_str(const std::u8string& str);

std::u16string utf8_to_utf16(const std::string& str);
std::u16string utr8_to_utf16(const std::u8string& str);
std::string utr16_to_str(const std::u16string& str);
std::u8string utr16_to_utf8(const std::u16string& str);

std::u32string utf8_to_utf32(const std::string& str);
std::u32string utf8_to_utf32(const std::u8string& str);
std::string utf32_to_str(const std::u32string& str);
std::u8string utf32_to_utf8(const std::u32string& str);



#ifdef _WIN32
std::wstring utf8_to_wstr(const std::string& str);
#endif



}
#endif

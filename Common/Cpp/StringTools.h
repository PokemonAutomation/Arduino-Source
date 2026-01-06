/*  String Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StringTools_H
#define PokemonAutomation_StringTools_H

#include <string>
#include <vector>

namespace PokemonAutomation{
namespace StringTools{


std::string replace(const std::string& str, const std::string& desired, const std::string& replace_with);

// Trim leading and trailing white spaces
std::string strip(const std::string& str);

// Parse str to size_t. Return SIZE_MAX if parsing fails
size_t to_size_t(const std::string& str);

// Break a string with a delimiter sub-string.
// e.g. split("a; b; c", "; ") -> ["a", "b", "c"]
// Note, if the string ends with a delimiter, the last element in the returned vector is an empty sub-string.
// e.g. split("a; b; ", "; ") -> ["a", "b", ""]
std::vector<std::string> split(const std::string& str, const std::string& delimiter);

// Capitalize each word in str. Words are separated by space characters.
std::string capitalize(const std::string& str);
// Uncapitalize each word in str. Words are separated by space characters.
std::string uncapitalize(const std::string& str);

// Convert std::u32string to utf8 std::string.
// We cannot print or cout std::u32string directly until we upgrade to C++23.
// There used to be a converter in STL (std::wstring_convert) but that was deprecated since C++17.
std::string u32string_to_utf8(const std::u32string& u32str);


}
}
#endif

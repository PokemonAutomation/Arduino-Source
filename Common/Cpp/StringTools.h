/*  String Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StringTools_H
#define PokemonAutomation_StringTools_H

#include <string>

namespace PokemonAutomation{
namespace StringTools{


std::string replace(const std::string& str, const std::string& desired, const std::string& replace_with);

// Trim leading and trailing white spaces
std::string strip(const std::string& str);

// Parse str to size_t. Return SIZE_MAX if parsing fails
size_t to_size_t(const std::string& str);

std::vector<std::string> split(const std::string& str, const std::string& delimiter);

}
}
#endif

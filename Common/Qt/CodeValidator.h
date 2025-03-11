/*  Raid/Trade Code Validator
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CodeValidator_H
#define PokemonAutomation_CodeValidator_H

#include <string>

namespace PokemonAutomation{


bool validate_code(size_t digits, const std::string& code);
std::string sanitize_code(size_t digits, const std::string& code);


}
#endif

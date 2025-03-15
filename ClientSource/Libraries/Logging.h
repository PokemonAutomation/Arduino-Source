/*  Pokemon Automation Bot Base - Client Example
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_Logging_H
#define PokemonAutomation_Logging_H

#include <string>
#include <sstream>

namespace PokemonAutomation{


void log(const std::ostringstream& ss);
void log(const std::string& msg);

std::string current_time_to_str();



}
#endif

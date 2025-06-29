/*  Program Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ProgramDumper_H
#define PokemonAutomation_ProgramDumper_H

#include <string>
#include "Common/Cpp/AbstractLogger.h"

namespace PokemonAutomation{


void setup_crash_handler();
bool program_dump(Logger* logger, const std::string& filename);


}
#endif

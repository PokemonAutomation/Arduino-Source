/*  Controller Type Strings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerTypeStrings_H
#define PokemonAutomation_Controllers_ControllerTypeStrings_H

#include "Common/Cpp/EnumStringMap.h"
#include "ControllerTypes.h"

namespace PokemonAutomation{



extern const EnumStringMap<ControllerInterface> CONTROLLER_INTERFACE_STRINGS;
extern const EnumStringMap<ControllerType> CONTROLLER_TYPE_STRINGS;
const EnumStringMap<ControllerClass>& CONTROLLER_CLASS_STRINGS();



}
#endif

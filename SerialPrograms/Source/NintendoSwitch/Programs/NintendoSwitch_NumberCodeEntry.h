/*  Number Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_NumberCodeEntry_H
#define PokemonAutomation_NintendoSwitch_NumberCodeEntry_H

#include <string>
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{



void numberpad_enter_code(
    Logger& logger, ProControllerContext& context,
    const std::string& code,
    bool include_plus
);



}
}
#endif

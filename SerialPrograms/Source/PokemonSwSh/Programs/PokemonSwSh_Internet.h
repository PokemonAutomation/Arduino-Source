/*  Internet
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Internet_H
#define PokemonAutomation_PokemonSwSh_Internet_H

#include <chrono>
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSwSh{


bool connect_to_internet_with_inference(
    ConsoleHandle& console, ControllerContext& context,
    std::chrono::milliseconds post_wait_time = std::chrono::seconds(3),
    uint16_t timeout_ticks = 120 * TICKS_PER_SECOND
);



}
}
}
#endif

/*  Internet
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Internet_H
#define PokemonAutomation_PokemonSwSh_Internet_H

#include <chrono>
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace std::chrono_literals;


bool connect_to_internet_with_inference(
    VideoStream& stream, SwitchControllerContext& context,
    Milliseconds post_wait_time = 3000ms,
    Milliseconds timeout_ticks = 120s
);



}
}
}
#endif

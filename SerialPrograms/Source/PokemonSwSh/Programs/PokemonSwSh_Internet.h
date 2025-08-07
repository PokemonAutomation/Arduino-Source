/*  Internet
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Internet_H
#define PokemonAutomation_PokemonSwSh_Internet_H

#include <chrono>
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace std::chrono_literals;


bool connect_to_internet_with_inference(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    Milliseconds post_wait_time = 3000ms,
    Milliseconds timeout_ticks = 120s
);



}
}
}
#endif

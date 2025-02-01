/*  Area Zero
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AreaZero_H
#define PokemonAutomation_PokemonSV_AreaZero_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{



//  After just entering the Zero Gate, go to the specified station.
void inside_zero_gate_to_station(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    int station,    //  1 - 4
    bool heal_at_station
);

//  You are inside Area Zero having traveled there via the Zero Gate. Return to
//  the zero gate fly spot.
void return_to_outside_zero_gate(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context
);

//  You are inside Area Zero having traveled there via the Zero Gate. Return to
//  inside the zero gate to setup for a subsequent call to "inside_zero_gate_to_station()".
void return_to_inside_zero_gate(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context
);
void return_to_inside_zero_gate_from_picnic(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context
);



void inside_zero_gate_to_secret_cave_entrance(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    bool heal_at_station
);



}
}
}
#endif

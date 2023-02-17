/*  Area Zero
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AreaZero_H
#define PokemonAutomation_PokemonSV_AreaZero_H


namespace PokemonAutomation{
    struct ProgramInfo;
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSV{



//  After just entering the Zero Gate, go to the specified station.
void inside_zero_gate_to_station(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    int station //  1 - 4
);

//  You are inside Area Zero having traveled there via the Zero Gate. Return to
//  the zero gate fly spot.
void return_to_outside_zero_gate(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  You are inside Area Zero having traveled there via the Zero Gate. Return to
//  inside the zero gate to setup for a subsequent call to "inside_zero_gate_to_station()".
void return_to_inside_zero_gate(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);



void inside_zero_gate_to_secret_cave_entrance(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context
);



}
}
}
#endif

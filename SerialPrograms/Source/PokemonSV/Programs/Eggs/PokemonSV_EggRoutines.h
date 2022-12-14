/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Various functions to do egg related automation.
 *  This file is used to prevent main function like EggAutonomous from becoming too long.
 */

#ifndef PokemonAutomation_PokemonSV_EggRoutines_H
#define PokemonAutomation_PokemonSV_EggRoutines_H

#include <functional>

namespace PokemonAutomation{

    class ConsoleHandle;
    class BotBaseContext;

namespace NintendoSwitch{
namespace PokemonSV{

// While entering Gastronome En Famille, order the dish Compote du Fils.
// This gives Egg Power Lv. 2 at price of 2800.
// Will throw OperationFailedException.
void order_compote_du_fils(ConsoleHandle& console, BotBaseContext& context);

// Standing in front of basket during picnic, check basket and update egg count.
// Will throw OperationFailedException.
void collect_eggs_from_basket(ConsoleHandle& console, BotBaseContext& context, size_t max_eggs, size_t& num_eggs_collected);

// check the five slots in the party column, after party lead.
// return how many eggs in the five slots, and how many non-egg pokemon in the five slots.
// Will throw OperationFailedException.
std::pair<uint8_t, uint8_t> check_egg_party_column(ConsoleHandle& console, BotBaseContext& context);


// Start at Zero Gate flying spot, go in circles in front of the lab to hatch eggs.
// `egg_hatched_callback` will be called after each egg hatched, with egg index (0-indexed) 
// Will throw OperationFailedException.
void hatch_eggs_at_zero_gate(ConsoleHandle& console, BotBaseContext& context,
    uint8_t num_eggs_in_party, std::function<void(uint8_t)> egg_hatched_callback = nullptr);


// When hatching at Zero Gate, use this function to reset player character position back to Zero Gate flying spot
// Will throw OperationFailedException.
void reset_position_at_zero_gate(ConsoleHandle& console, BotBaseContext& context);


}
}
}
#endif

/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Various functions to do egg related automation.
 *  This file is used to prevent main function like EggAutonomous from becoming too long.
 */

#ifndef PokemonAutomation_PokemonSV_EggRoutines_H
#define PokemonAutomation_PokemonSV_EggRoutines_H

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


}
}
}
#endif

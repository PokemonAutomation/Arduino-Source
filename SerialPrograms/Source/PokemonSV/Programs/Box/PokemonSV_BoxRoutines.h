/*  Box Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Various functions to operate in box system, like release pokemon.
 */

#ifndef PokemonAutomation_PokemonSV_BoxRoutines_H
#define PokemonAutomation_PokemonSV_BoxRoutines_H

namespace PokemonAutomation{
    
    class ProgramEnvironment;
    class ConsoleHandle;
    class BotBaseContext;

namespace NintendoSwitch{
namespace PokemonSV{

// Hold one column of five pokemon
void hold_one_column(BotBaseContext& context);

// Release current selected pokemon in the box system. May thow OperationFailedException.
// If `ensure_empty` is true, it will make sure the pokemon is release and throw OperationFailedException if releasing failed.
// Note: if `ensure_empty` is true, this function will fail if releasing a party pokemon that is not the last of the party!
//       Because game will automatically fill the hole in the party. To release party pokemon, set `ensure_empty` to false.
bool release_one_pokemon(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context, bool ensure_empty = false);

// In box system, check if the five party slots after the party lead are all empty
bool check_party_empty(ConsoleHandle& console, BotBaseContext& context);

void load_one_column_to_party(ConsoleHandle& console, BotBaseContext& context, uint8_t column_index);

void unload_one_column_from_party(ConsoleHandle& console, BotBaseContext& context, uint8_t column_index);

}
}
}
#endif

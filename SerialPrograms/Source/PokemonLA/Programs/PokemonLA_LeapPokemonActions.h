/*  Leap Pokemon Actions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_LeapPokemonActions_H
#define PokemonAutomation_PokemonLA_LeapPokemonActions_H

#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"
#include "PokemonLA/Options/PokemonLA_MiscOptions.h"
#include "PokemonLA_RegionNavigation.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
    class ConsoleHandle;
namespace PokemonLA{

enum class LeapPokemon
{
    Aipom,
    Burmy,
    Cherrim,
    Cherubi,
    Combee,
    Heracross,
    Pachirisu,
    Vespiquen,
    Wormadam,
    Geodude,
    Graveler,
    Bonsly,
    Bronzor,
    Nosepass,
    Bergimite
};

void setup(ProgramEnvironment& env, ConsoleHandle& console, SwitchControllerContext& context);
void route(ProgramEnvironment& env, ConsoleHandle& console, SwitchControllerContext& context, LeapPokemon pokemon);
bool check_tree_or_ore_for_battle(ConsoleHandle& console, SwitchControllerContext& context);
void exit_battle(ConsoleHandle& console, SwitchControllerContext& context, ExitBattleMethod exit_method);
void return_to_jubilife(ProgramEnvironment& env, ConsoleHandle& console, SwitchControllerContext& context, LeapPokemon pokemon);
PokemonDetails get_pokemon_details(ConsoleHandle& console, SwitchControllerContext& context, Language language);


}
}
}
#endif

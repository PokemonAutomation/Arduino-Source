/*  Leap Pokemon Actions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_LeapPokemonActions_H
#define PokemonAutomation_PokemonLA_LeapPokemonActions_H

#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"
#include "CommonFramework/OCR/OCR_StringMatchResult.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA_RegionNavigation.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
    class ConsoleHandle;
namespace NintendoSwitch{
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

void setup(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context);
void route(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context, size_t pokemon);
bool check_tree_or_ore_for_battle(ConsoleHandle& console, BotBaseContext& context);
void exit_battle(ConsoleHandle& console, BotBaseContext& context, bool mash_A_to_kill = false);
void return_to_jubilife(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context, size_t pokemon);
PokemonDetails get_pokemon_details(ConsoleHandle& console, BotBaseContext& context, Language language);


}
}
}
#endif

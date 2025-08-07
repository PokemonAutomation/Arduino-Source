/*  Leap Pokemon Actions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_LeapPokemonActions_H
#define PokemonAutomation_PokemonLA_LeapPokemonActions_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"
#include "PokemonLA/Options/PokemonLA_MiscOptions.h"
#include "PokemonLA_RegionNavigation.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
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

void setup(ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context);
void route(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    LeapPokemon pokemon,
    bool fresh_from_reset
);
bool check_tree_or_ore_for_battle(VideoStream& stream, ProControllerContext& context);
void exit_battle(VideoStream& stream, ProControllerContext& context, ExitBattleMethod exit_method);
void return_to_jubilife(ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context, LeapPokemon pokemon);
PokemonDetails get_pokemon_details(VideoStream& stream, ProControllerContext& context, Language language);


}
}
}
#endif

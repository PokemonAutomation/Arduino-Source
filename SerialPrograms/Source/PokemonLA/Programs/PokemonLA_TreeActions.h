/*  Tree Actions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_TreeActions_H
#define PokemonAutomation_PokemonLA_TreeActions_H

#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"
#include "PokemonLA/Inference/PokemonLA_SelectedRegionDetector.h"
#include "CommonFramework/OCR/OCR_StringMatchResult.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA_RegionNavigation.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
    class ConsoleHandle;
namespace NintendoSwitch{
namespace PokemonLA{


void setup(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context);
bool check_tree_for_battle(ConsoleHandle& console, BotBaseContext& context);
void exit_battle(BotBaseContext& context);
PokemonDetails get_pokemon_details(ConsoleHandle& console, BotBaseContext& context, Language language);


}
}
}
#endif

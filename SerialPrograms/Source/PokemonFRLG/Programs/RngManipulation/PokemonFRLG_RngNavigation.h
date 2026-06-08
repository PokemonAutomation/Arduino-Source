/*  Rng Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RngNavigation_H
#define PokemonAutomation_PokemonFRLG_RngNavigation_H

#include "CommonFramework/Language.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "Pokemon/Pokemon_AdvRng.h"
#include "PokemonFRLG_BlindNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
    class ConsoleHandle;
    class ProController;
    using ProControllerContext = ControllerContext<ProController>;
namespace PokemonFRLG{

using namespace Pokemon;

static const std::set<std::string> ROAMER_ROUTE1_SUBSET = { 
    "raikou", "suicune", "entei", "rattata", "pidgey" 
};
    
void go_to_summary(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    int from_last = 0, 
    StartMenuContext menu_context = StartMenuContext::STANDARD
);

bool shiny_check_summary(
    ConsoleHandle& console, ProControllerContext& context, 
    int from_last = 0, StartMenuContext menu_context = StartMenuContext::STANDARD
);

AdvObservedPokemon read_summary(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    const Language& language, 
    std::set<std::string> species = {}
);

int auto_catch(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    const uint64_t& max_ball_throws,
    bool safari_zone = false
);

bool use_rare_candy(
    ConsoleHandle& console, 
    ProControllerContext& context,
    const Language& language, 
    AdvObservedPokemon& pokemon,
    AdvRngFilters& filters,
    const BaseStats& base_stats,
    AdvRngMethod method = AdvRngMethod::Method1,
    bool safari_zone = false,
    bool first = false,
    int from_last = 0
);

int watch_for_shiny_encounter(ConsoleHandle& console, ProControllerContext& context);

int encounter_roamer(
    ConsoleHandle& console, ProControllerContext& context, 
    Language language, const std::set<std::string>& subset
);

bool check_for_shiny(
    ConsoleHandle& console, ProControllerContext& context, 
    PokemonFRLG_RngTarget TARGET,
    Language language = Language::English, 
    const std::set<std::string>& subset = ROAMER_ROUTE1_SUBSET
);

void hatch_daycare_egg(ConsoleHandle& console, ProControllerContext& context);

}
}
}
#endif

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
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
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


// Daycare (Four Island) navigation for egg RNG.
// bool methods return true when they fail to start surfing
void use_max_repel(ConsoleHandle& console, ProControllerContext& context);
void daycare_steps(ConsoleHandle& console, ProControllerContext& context);
bool walk_from_daycare_to_pond(ConsoleHandle& console, ProControllerContext& context);
void walk_from_pond_to_daycare_man(ConsoleHandle& console, ProControllerContext& context);
void egg_pickup(ConsoleHandle& console, ProControllerContext& context);
bool walk_from_daycare_man_to_pond(ConsoleHandle& console, ProControllerContext& context);

}
}
}
#endif

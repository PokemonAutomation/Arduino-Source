/*  RNG Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "VideoGames/PokemonFRLG/PokemonFRLG_Prechecks.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_RngProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


PrecheckList rng_prechecks(size_t free_party_slots_needed){
    PrecheckList checks;
    checks.emplace_back(make_black_border_precheck());
    if (free_party_slots_needed > 0){
        checks.emplace_back(make_free_party_slots_precheck(free_party_slots_needed));
    }
    return checks;
}

PrecheckList RngProgramInstance::prechecks() const{
    return rng_prechecks(free_party_slots_needed());
}

void RngProgramInstance::run_precheck(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    const SkippablePrechecks skippable = GameSettings::instance().RUN_RNG_START_CHECKS
        ? SkippablePrechecks::RUN
        : SkippablePrechecks::SKIP;
    run_prechecks(env, context, prechecks(), skippable);
}

}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation

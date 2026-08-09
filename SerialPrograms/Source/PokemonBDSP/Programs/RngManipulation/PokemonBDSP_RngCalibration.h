/*  BDSP RNG Calibration
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_RngCalibration_H
#define PokemonAutomation_PokemonBDSP_RngCalibration_H

#include <stdint.h>
#include <string>
#include "Pokemon/Pokemon_BdspRng.h"
#include "Pokemon/Pokemon_NatureChecker.h"
#include "Pokemon/Pokemon_StatsCalculation.h"
#include "PokemonBDSP_StarterNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


Pokemon::NatureAdjustment bdsp_nature_adjustment(uint8_t nature, size_t stat_index);

Pokemon::StatReads bdsp_expected_stats(
    const Pokemon::BdspPokemonResult& generated,
    const Pokemon::BaseStats& base_stats,
    uint8_t level
);

struct BdspObservedStarter{
    Pokemon::NatureCheckerValue nature = Pokemon::NatureCheckerValue::UnableToDetect;
    Pokemon::BdspGender gender = Pokemon::BdspGender::Genderless;
    bool gender_known = false;
    Pokemon::StatReads stats;
    Pokemon::BaseStats base_stats;
    uint8_t level = 5;
    bool shiny = false;
    bool shiny_known = false;
};


struct BdspHitIdentification{
    bool success = false;
    uint64_t advance = 0;
    int64_t offset = 0;
    size_t candidates = 0;
    std::string failure_reason;
};


BdspHitIdentification identify_hit_advance(
    const Pokemon::Xorshift128State& state,
    const Pokemon::BdspStaticTemplate& tmpl,
    uint64_t intended_advance,
    uint64_t radius,
    const BdspObservedStarter& observed
);


bool advances_between(
    const Pokemon::Xorshift128State& from,
    const Pokemon::Xorshift128State& to,
    uint64_t search_max,
    uint64_t& advances
);


const Pokemon::BaseStats& starter_base_stats(BdspStarter starter);

bool consistent_with(
    const Pokemon::BdspPokemonResult& generated,
    const BdspObservedStarter& observed
);


}
}
}
#endif

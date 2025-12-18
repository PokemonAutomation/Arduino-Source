/*  Collected Pokemon Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_CollectedPokemonInfo_H
#define PokemonAutomation_Pokemon_CollectedPokemonInfo_H

#include <cstdint>
#include <string>
#include <optional>
#include <vector>
#include <ostream>
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
#include "Pokemon/Options/Pokemon_BoxSortingTable.h"

namespace PokemonAutomation{
namespace Pokemon{


struct CollectedPokemonInfo{
    const std::vector<SortingRule>* preferences;

    // When adding any new member here, do not forget to modify the operators below (ctrl-f "new struct members")
    uint16_t national_dex_number = 0;
    bool shiny = false;
    bool gmax = false;
    bool alpha = false;
    std::string ball_slug = "";
    StatsHuntGenderFilter gender = StatsHuntGenderFilter::Genderless;
    uint32_t ot_id = 0; // original trainer ID
};

bool operator==(const CollectedPokemonInfo& lhs, const CollectedPokemonInfo& rhs);

// Sort two pokemon slot. "Smaller" pokemon is placed closer to front.
// If a slot is empty, it is always larger than non-empty slot so all the empty slots are at end after sorting.
// For two pokemon, check each preference rule. If we cannot determine the order based on the first rule, go
// to the second rule, and so on.
// Available rules:
// - National dex number: smaller ID should be at front.
// - Shiny: shiny pokemon should be at front.
// - Gigantamax: Gigantamax pokemon should be at front.
// - Alpha: Alpha pokemon should be at front.
// - Ball slug: pokemon with a ball slug that's smaller in the alphabetical order should be at front.
// - Gender: Male should be at front, then comes female, and genderless is last
// Each rule type also has a "reverse" which if true, reverses above ordering for that rule type.
// If user does not give a preference ruleset, sort by national dex number.
bool operator<(const std::optional<CollectedPokemonInfo>& lhs, const std::optional<CollectedPokemonInfo>& rhs);

// Print pokemon info into ostream
std::ostream& operator<<(std::ostream& os, const std::optional<CollectedPokemonInfo>& pokemon);

// Create short info of a pokemon for overlay log display
std::string create_overlay_info(const CollectedPokemonInfo& pokemon);


}
}
#endif

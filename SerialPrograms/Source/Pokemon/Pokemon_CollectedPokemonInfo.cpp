/*  Collected Pokemon Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon_CollectedPokemonInfo.h"

namespace PokemonAutomation{
namespace Pokemon{


bool operator==(const CollectedPokemonInfo& lhs, const CollectedPokemonInfo& rhs){
    // NOTE edit when adding new struct members
    return lhs.national_dex_number == rhs.national_dex_number &&
           lhs.shiny == rhs.shiny &&
           lhs.gmax == rhs.gmax &&
           lhs.alpha == rhs.alpha &&
           lhs.ball_slug == rhs.ball_slug &&
           lhs.gender == rhs.gender &&
           lhs.ot_id == rhs.ot_id;
}

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
bool operator<(const std::optional<CollectedPokemonInfo>& lhs, const std::optional<CollectedPokemonInfo>& rhs){
    if (!lhs.has_value()){ // lhs is empty
        return false;
    }
    if (!rhs.has_value()){ // lhs not empty but rhs is empty
        return true;
    }
    // both sides are not empty:
    for (const SortingRule& preference : *lhs->preferences){
        switch(preference.sort_type){
        // NOTE edit when adding new struct members
        case SortingRuleType::NationalDexNo:
            if (lhs->national_dex_number != rhs->national_dex_number){
                // we use (boolean != reverse) to apply the reverse effect
                return (lhs->national_dex_number < rhs->national_dex_number) != preference.reverse;
            }
            break;
        case SortingRuleType::Shiny:
            if (lhs->shiny != rhs->shiny){
                return lhs->shiny != preference.reverse;
            }
            break;
        case SortingRuleType::Gigantamax:
            if (lhs->gmax != rhs->gmax){
                return lhs->gmax != preference.reverse;
            }
            break;
        case SortingRuleType::Alpha:
            if (lhs->alpha != rhs->alpha){
                return lhs->gmax != preference.reverse;
            }
            break;
        case SortingRuleType::Ball_Slug:
            if (lhs->ball_slug != rhs->ball_slug){
                return (lhs->ball_slug < rhs->ball_slug) != preference.reverse;
            }
            break;
        case SortingRuleType::Gender:
            if (lhs->gender != rhs->gender){
                return (lhs->gender < rhs->gender) != preference.reverse;
            }
            break;
        default:
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "unknown SortingRuleType");
        } // end switch
    } // end for preference

    // Default to sort by national dex number
    return lhs->national_dex_number < rhs->national_dex_number;
}

std::ostream& operator<<(std::ostream& os, const std::optional<CollectedPokemonInfo>& pokemon)
{
    if (pokemon.has_value()){
        // NOTE edit when adding new struct members
        os << "(";
        os << "national_dex_number:" << pokemon->national_dex_number << " ";
        os << "shiny:" << (pokemon->shiny ? "true" : "false") << " ";
        os << "gmax:" << (pokemon->gmax ? "true" : "false") << " ";
        os << "alpha:" << (pokemon->alpha ? "true" : "false") << " ";
        os << "ball_slug:" << pokemon->ball_slug << " ";
        os << "gender:" << gender_to_string(pokemon->gender) << " ";
        os << "ot_id:" << pokemon->ot_id << " ";
        os << ")";
    }else{
        os << "(empty)";
    }
    return os;
}

std::string create_overlay_info(const CollectedPokemonInfo& pokemon){
    const std::string& species_slug = NATIONAL_DEX_SLUGS()[pokemon.national_dex_number-1];
    const std::string& display_name = get_pokemon_name(species_slug).display_name();
    std::string overlay_log = display_name;
    if(pokemon.gender == StatsHuntGenderFilter::Male){
        overlay_log += " " + UNICODE_MALE;
    } else if (pokemon.gender == StatsHuntGenderFilter::Female){
        overlay_log += " " + UNICODE_FEMALE;
    }
    if (pokemon.shiny){
        overlay_log += " *";
    }
    if (pokemon.gmax){
        overlay_log += " G";
    }
    if (pokemon.alpha){
        overlay_log += " " + UNICODE_ALPHA;
    }
    return overlay_log;
}


}
}

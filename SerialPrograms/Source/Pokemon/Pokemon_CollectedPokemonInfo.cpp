/*  Collected Pokemon Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_BoxCursor.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon_CollectedPokemonInfo.h"

namespace PokemonAutomation{
namespace Pokemon{


bool operator==(const CollectedPokemonInfo& lhs, const CollectedPokemonInfo& rhs){
    // NOTE edit when adding new struct members
    return lhs.dex_number == rhs.dex_number &&
           lhs.name_slug == rhs.name_slug &&
           lhs.shiny == rhs.shiny &&
           lhs.gmax == rhs.gmax &&
           lhs.alpha == rhs.alpha &&
           lhs.ball_slug == rhs.ball_slug &&
           lhs.gender == rhs.gender &&
           lhs.ot_id == rhs.ot_id;
}


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
        case SortingRuleType::DexNo:
            if (lhs->dex_number != rhs->dex_number){
                // we use (boolean != reverse) to apply the reverse effect
                return (lhs->dex_number < rhs->dex_number) != preference.reverse;
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

    // Default to sort by dex number
    return lhs->dex_number < rhs->dex_number;
}

std::ostream& operator<<(std::ostream& os, const std::optional<CollectedPokemonInfo>& pokemon)
{
    if (pokemon.has_value()){
        // NOTE edit when adding new struct members
        os << "(";
        os << "dex: " << pokemon->dex_number << " " << pokemon->name_slug << " ";
        os << "shiny:" << (pokemon->shiny ? "true" : "false") << " ";
        os << "gmax:" << (pokemon->gmax ? "true" : "false") << " ";
        os << "alpha:" << (pokemon->alpha ? "true" : "false") << " ";
        os << "ball:" << pokemon->ball_slug << " ";
        os << "gender:" << gender_to_string(pokemon->gender) << " ";
        os << "ot_id:" << pokemon->ot_id << " ";
        os << ")";
    }else{
        os << "(empty)";
    }
    return os;
}

std::string create_overlay_info(const CollectedPokemonInfo& pokemon){
    const std::string& display_name = get_pokemon_name(pokemon.name_slug).display_name();
    char dex_str[7]; // leaving enough space to convert dex number uint16_t to string with "0" to fill up at least 4 characters, e.g. "0005"
    snprintf(dex_str, sizeof(dex_str), "%04d", pokemon.dex_number);
    std::string overlay_log = std::string(dex_str) + " " + display_name;
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


void save_boxes_data_to_json(const std::vector<std::optional<CollectedPokemonInfo>>& boxes_data, const std::string& json_path){
    JsonArray pokemon_data;
    for (size_t slot_idx = 0; slot_idx < boxes_data.size(); slot_idx++){
        BoxCursor cursor(slot_idx);
        JsonObject pokemon;
        pokemon["index"] = slot_idx;
        pokemon["box"] = cursor.box;
        pokemon["row"] =  cursor.row;
        pokemon["column"] =  cursor.column;
        const auto& current_pokemon = boxes_data[slot_idx];
        if (current_pokemon != std::nullopt){
            // NOTE edit when adding new struct members
            pokemon["name"] = current_pokemon->name_slug;
            pokemon["dex"] = current_pokemon->dex_number;
            pokemon["shiny"] = current_pokemon->shiny;
            pokemon["gmax"] = current_pokemon->gmax;
            pokemon["alpha"] = current_pokemon->alpha;
            pokemon["ball"] = current_pokemon->ball_slug;
            pokemon["gender"] = gender_to_string(current_pokemon->gender);
            pokemon["ot_id"] = current_pokemon->ot_id;
        }
        pokemon_data.push_back(std::move(pokemon));
    }
    pokemon_data.dump(json_path);
}


}
}

/*  Blueberry Quests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BBQuests_H
#define PokemonAutomation_PokemonSV_BBQuests_H

#include <string>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

enum class BBQuests{
    auto_10, make_tm, pickup_10, sneak_up, photo_fly, photo_swim, photo_canyon, photo_coastal, photo_polar, photo_savanna, tera_self_defeat,
    travel_500, catch_any, catch_normal, catch_fighting, catch_flying, catch_poison, catch_ground, catch_rock, catch_bug, catch_ghost, catch_steel,
    catch_fire, catch_water, catch_grass, catch_electric, catch_psychic, catch_ice, catch_dragon, catch_dark, catch_fairy,
    wash_pokemon, wild_tera, auto_30, tera_raid, sandwich_three, bitter_sandwich, sweet_sandwich, salty_sandwich, sour_sandwich, spicy_sandwich, hatch_egg,
    photo_normal, photo_fighting, photo_flying, photo_poison, photo_ground, photo_rock, photo_bug, photo_ghost, photo_steel, photo_fire, photo_water,
    photo_grass, photo_electric, photo_psychic, photo_ice, photo_dragon, photo_dark, photo_fairy,
    ditto_central, ditto_canyon, ditto_coastal, ditto_polar, ditto_savanna, group_canyon, group_coastal, group_polar, group_savanna, group_eyewear, group_nonuniform,
    group_masks, sandwich_four, catch_hint, catch_hint2,
    UnableToDetect
};

// Map a quest slug (as returned by BlueberryQuestDetector) to its enum.
// Unrecognized slugs return UnableToDetect.
BBQuests BBQuests_string_to_enum(const std::string& token);

// Map a quest to its slug. UnableToDetect maps to the empty string.
const std::string& BBQuests_enum_to_string(BBQuests quest);


}
}
}
#endif

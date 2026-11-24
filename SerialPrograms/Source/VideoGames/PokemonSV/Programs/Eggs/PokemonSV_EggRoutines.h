/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Various functions to do egg related automation.
 *  This file is used to prevent main function like EggAutonomous from becoming too long.
 */

#ifndef PokemonAutomation_PokemonSV_EggRoutines_H
#define PokemonAutomation_PokemonSV_EggRoutines_H

#include <chrono>
#include <functional>
#include <string>
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "VideoGames/PokemonSV/Options/PokemonSV_SinglesAIOption.h"
#include "VideoGames/PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"

namespace PokemonAutomation{

    struct ProgramInfo;
    class ImageViewRGB32;

    namespace OCR{
        class LanguageOCROption;
    }
    namespace Pokemon{
    enum class StatsHuntAction;
    class StatsHuntIvJudgeFilterTable;
    }

namespace NintendoSwitch{
namespace PokemonSV{

// While entering Gastronome En Famille, order the dish Compote du Fils.
// This gives Egg Power Lv. 2 at price of 2800.
void order_compote_du_fils(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
);

// Start at Zero Gate flying spot, go off ramp to start a picnic.
void picnic_at_zero_gate(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
);

// Starting at the initial position of a picnic, go to picnic table to make a sandwich that gives egg power.
// Can choose:
// - Great Peanut Butter Sandwich (No. 17) or Great Marmalade Sandwich (No. 25) to gain egg power Lv 2, or Peanut Butter
//   Sandwich (No. 16) or Jam Sandwich (No. 12) to gain egg power Lv 1. Must have unlocked the recipe and have enough
//   ingredients to make all the sandwiches for all the unlocked recipes.
// - BEST_AVAILABLE_RECIPE tries those recipes from best to worst and makes the first one it can find.
// - Two-sweet-herbs, bitter-sweet-herbs or salty-sweet-herbs custom sandwich to gain egg power Lv 3, must have enough ingredinets and
//   provide game language for OCR ingredient lists.
// Return false if no needed sandwich ingredients or recipe.
bool eat_egg_sandwich_at_picnic(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    EggSandwichType sandwich_type,
    Language language
);

// Text for the stats bar: " - Avg Time/Egg: 1m 05s", or "" when nothing has hatched yet.
std::string avg_time_per_egg_text(uint64_t total_ms, uint64_t eggs);

// Add the time since `last_mark` to `total_ms` and move `last_mark` to now. Call once per hatched egg.
void add_hatch_time(std::chrono::steady_clock::time_point& last_mark, std::atomic<uint64_t>& total_ms);

// How long to wait between egg basket checks after eating a sandwich of `sandwich_type`.
size_t egg_basket_wait_seconds(EggSandwichType sandwich_type);

// After eating a sandwich, go around picnic table to wait at basket and collect eggs.
// `num_eggs_collected` will be updated to add newly collected eggs.
void collect_eggs_after_sandwich(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t basket_wait_seconds,
    size_t max_eggs,
    size_t& num_eggs_collected,
    std::function<void(size_t new_eggs)> basket_check_callback);

// Start at Zero Gate flying spot, go in circles in front of the lab to hatch eggs.
// `egg_hatched_callback` will be called after each egg hatched, with egg index (0-indexed) 
void hatch_eggs_at_zero_gate(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    EventNotificationOption* attacked_notification,
    const SinglesAIOption* battle_ai,
    std::atomic<uint64_t>* attacked,
    uint8_t num_eggs_in_party,
    std::function<void(uint8_t)> egg_hatched_callback = nullptr
);

// Start at North Province (Area Three) Lighthouse flying spot, go in circles in Ruchbah (fairy) Squad's base to hatch eggs.
// Switch 2 only, this is an alternate egg hatch spot as Zero Gate now has spawns in the way.
// Switch 1 is untested in this location but the area is extremely laggy.
// This is a minor modification to hatch_eggs_at_zero_gate
// `egg_hatched_callback` will be called after each egg hatched, with egg index (0-indexed)
void hatch_eggs_at_area_three_lighthouse(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    EventNotificationOption* attacked_notification,
    const SinglesAIOption* battle_ai,
    std::atomic<uint64_t>* attacked,
    uint8_t num_eggs_in_party,
    std::function<void(uint8_t)> egg_hatched_callback = nullptr
);

// Start at the Cortondo (West) Pokemon Center flying spot. Walk out of town to the grass by the tree and start a picnic.
// Picnics are not allowed inside Cortondo or in the Olive Roll field, but they are allowed right outside town.
void picnic_at_cortondo(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// From inside the Cortondo Olive Roll field, fly back to the Cortondo (West) Pokemon Center.
void fly_to_cortondo_from_olive_field(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// Start at the Cortondo (West) Pokemon Center flying spot. Ride through town, jump the wall and the fence into the
// Olive Roll field north of town, and go in circles there to hatch eggs. Wild Pokemon do not spawn inside the field.
// `egg_hatched_callback` will be called after each egg hatched, with egg index (0-indexed)
void hatch_eggs_at_cortondo_olive_field(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    EventNotificationOption* attacked_notification,
    const SinglesAIOption* battle_ai,
    std::atomic<uint64_t>* attacked,
    uint8_t num_eggs_in_party,
    std::function<void(uint8_t)> egg_hatched_callback = nullptr
);

// From overworld, go in circles to hatch eggs.
// `egg_hatched_callback` will be called after each egg hatched, with egg index (0-indexed)
// `already_on_ride`: whether the player character is on ride when starting the function.
void hatch_eggs_anywhere(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    bool already_on_ride,
    uint8_t num_eggs_in_party,
    std::function<void(uint8_t)> egg_hatched_callback = nullptr
);


// Standing in front of basket during picnic, check basket and update egg count.
void check_basket_to_collect_eggs(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t max_eggs, size_t& num_eggs_collected
);

// In box view, check the five slots in the party column, after party lead.
// return how many eggs in the five slots, and how many non-egg pokemon in the five slots.
// Note: make sure the current cursor does not float above the five slots, otherwise it may affect detection.
std::pair<uint8_t, uint8_t> check_egg_party_column(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
);

// In box view, check whether there is the right amount of non-egg pokemon in the five slots in the party column, after party lead.
// return how many eggs in the five slots.
// Throw OperationFailedException if found unexpected non-egg pokemon count in the slots.
// Note: make sure the current cursor does not float above the five slots, otherwise it may affect detection.
uint8_t check_non_eggs_count_in_party(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    uint8_t expected_non_eggs_count_in_party
);


// When hatching at Zero Gate, use this function to reset player character position back to Zero Gate flying spot
void reset_position_at_zero_gate(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// Assuming the current selected slot in box system is a hatched pokemon, not empty space or egg,
// check the hatched pokemon's info to determine what to do with it.
// Return if the pokemon is shiny. The action to handle the pokemon is returned in `action`.
// If any row in `FILTERS` checks an IV, the function switches to judge view to read IVs, and
// throws OperationFailedException if the judge view can't be reached (e.g. it's not unlocked in game).
// Otherwise it settles onto stats or judge view, whichever comes first, skips reading IVs and
// regards them as unknown. So players who don't filter on IVs don't need the judge unlocked.
bool check_baby_info(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    OCR::LanguageOCROption& LANGUAGE,
    Pokemon::StatsHuntIvJudgeFilterTable& FILTERS,
    Pokemon::StatsHuntAction& action
);

}
}
}
#endif

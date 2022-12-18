/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Various functions to do egg related automation.
 *  This file is used to prevent main function like EggAutonomous from becoming too long.
 */

#ifndef PokemonAutomation_PokemonSV_EggRoutines_H
#define PokemonAutomation_PokemonSV_EggRoutines_H

#include <functional>

namespace PokemonAutomation{

    struct ProgramInfo;
    class AsyncDispatcher;
    class ConsoleHandle;
    class BotBaseContext;
    class ImageViewRGB32;

    namespace OCR{
        class LanguageOCROption;
    }
    namespace Pokemon{
        enum class EggHatchAction;
        class EggHatchFilterTable;
    }

namespace NintendoSwitch{
namespace PokemonSV{

// While entering Gastronome En Famille, order the dish Compote du Fils.
// This gives Egg Power Lv. 2 at price of 2800.
void order_compote_du_fils(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

// Start at Zero Gate flying spot, go off ramp to start a picnic.
void picnic_at_zero_gate(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

// Starting at the initial position of a picnic, go to picnic table to make a
// Great Peanut Butter Sandwich to gain egg power Lv 2.
// Return false if no needed sandwich ingredients or recipe.
bool eat_egg_sandwich_at_picnic(const ProgramInfo& info, AsyncDispatcher& dispatcher, ConsoleHandle& console, BotBaseContext& context);

// After eating a sandwich, go around picnic table to wait at basket and collect eggs.
// `num_eggs_collected` will be updated to add newly collected eggs.
void collect_eggs_after_sandwich(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    size_t max_eggs, size_t& num_eggs_collected,
    std::function<void(size_t new_eggs)> basket_check_callback);

// Start at Zero Gate flying spot, go in circles in front of the lab to hatch eggs.
// `egg_hatched_callback` will be called after each egg hatched, with egg index (0-indexed) 
void hatch_eggs_at_zero_gate(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    uint8_t num_eggs_in_party, std::function<void(uint8_t)> egg_hatched_callback = nullptr);


// Standing in front of basket during picnic, check basket and update egg count.
void check_basket_to_collect_eggs(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, size_t max_eggs, size_t& num_eggs_collected);

// In box view, check the five slots in the party column, after party lead.
// return how many eggs in the five slots, and how many non-egg pokemon in the five slots.
std::pair<uint8_t, uint8_t> check_egg_party_column(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

// In box view, check whether there are only eggs in the five slots in the party column, after party lead.
// return how many eggs in the five slots.
// Throw OperationFailedException if found non-egg pokemon in the slots.
uint8_t check_only_eggs_in_party(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);


// When hatching at Zero Gate, use this function to reset player character position back to Zero Gate flying spot
void reset_position_at_zero_gate(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

// Check hatched pokemon info to determine what to do with it.
// Return if the pokemon is shiny. The action to handle the pokemon is returned in `action`.
bool check_baby_info(ConsoleHandle& console, BotBaseContext& context,
    OCR::LanguageOCROption& LANGUAGE, Pokemon::EggHatchFilterTable& FILTERS,
    Pokemon::EggHatchAction& action
);


}
}
}
#endif

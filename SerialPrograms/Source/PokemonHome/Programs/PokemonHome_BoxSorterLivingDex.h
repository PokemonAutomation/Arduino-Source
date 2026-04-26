/*  Home Box Sorter Living Dex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_BoxSorterLivingDex_H
#define PokemonAutomation_PokemonHome_BoxSorterLivingDex_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_CollectedPokemonInfo.h"
#include "Pokemon/Pokemon_Types.h"
#include "Pokemon/Pokemon_BoxCursor.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
using namespace Pokemon;

class BoxSorterLivingDex_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BoxSorterLivingDex_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class BoxSorterLivingDex : public SingleSwitchProgramInstance{
public:
    BoxSorterLivingDex();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    struct LivingDexEntry{
        std::string species_slug;
        std::string form_slug;
        uint16_t nat_id;
        PokemonType primary_type;
        PokemonType secondary_type;
        std::string slug;
        bool has_gender_difference;
    };

    // Checks if the given pokemon is viable for the living dex entry. 
    // Currently the checks include: 
    // National Dex Id, Types (for regional forms), Gender (if no gender difference either is fine)
    // Shiny (if the user has selected to only include shinies in the living dex).
    bool is_viable_for_dex(const LivingDexEntry& entry, const CollectedPokemonInfo& pokemonInfo);

    // Goes through the summary screen of each pokemon in the range of boxes. 
    // Populates the boxes_data vector with the pokemon info. Returns the final cursor position after reading the boxes.
    [[nodiscard]] BoxCursor populate_box_data(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        std::vector<SortingRule> sort_preferences,
        std::vector<std::optional<CollectedPokemonInfo>>& boxes_data,
        size_t box_count,
        BoxCursor& nav_cursor
    );

    SimpleIntegerOption<uint16_t> LIVING_DEX_START_BOX;
    SimpleIntegerOption<uint16_t> REJECT_BOX_START;
    SimpleIntegerOption<uint16_t> REJECT_BOX_END;
    BooleanCheckBoxOption SHINY_DEX;
    MillisecondsOption VIDEO_DELAY;
    MillisecondsOption GAME_DELAY;
    StringOption OUTPUT_FILE;
    BooleanCheckBoxOption DRY_RUN;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif // PokemonAutomation_PokemonHome_BoxSorterLivingDex_H

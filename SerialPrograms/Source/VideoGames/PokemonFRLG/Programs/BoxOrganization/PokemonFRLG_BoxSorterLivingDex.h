/*  Box Sorter Living Dex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BoxSorterLivingDex_H
#define PokemonAutomation_PokemonFRLG_BoxSorterLivingDex_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_CollectedPokemonInfo.h"

namespace PokemonAutomation{ class Logger; }

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

struct BoxOrganizationStats;

class BoxSorterLivingDex_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BoxSorterLivingDex_Descriptor();

    using Stats = BoxOrganizationStats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class BoxSorterLivingDex : public SingleSwitchProgramInstance{
public:
    using Descriptor = BoxSorterLivingDex_Descriptor;
    BoxSorterLivingDex();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
    virtual void start_program_border_check(VideoStream&, FeedbackType) override{}

private:
    enum class LivingDexTemplate{
        Condensed,
    };

    SimpleIntegerOption<uint16_t> NUM_BOXES;
    EnumDropdownOption<LivingDexTemplate> LIVING_DEX_TEMPLATE;
    BooleanCheckBoxOption SHINY_DEX;
    BooleanCheckBoxOption DRY_RUN;
    EventNotificationsOption NOTIFICATIONS;
};

struct LivingDexEntry{
    uint16_t nat_id;
    std::string slug;
    bool has_gender_difference;
};

std::vector<LivingDexEntry> load_living_dex_template(const std::string& path);

// List empty template slots in National Dex order, with one number and English name per line.
std::string make_missing_living_dex_report(
    const std::vector<std::optional<Pokemon::CollectedPokemonInfo>>& boxes_sorted,
    const std::vector<LivingDexEntry>& living_dex_order
);

// FRLG's condensed dex matches species, ignoring forms. No spaces for missing pokemon from RSE
bool is_viable_for_dex(
    const LivingDexEntry& entry, const Pokemon::CollectedPokemonInfo& pokemon,
    bool obey_gender_differences, bool shiny_dex = false
);

std::vector<std::optional<Pokemon::CollectedPokemonInfo>> make_living_dex_order(
    Logger& logger,
    const std::vector<std::optional<Pokemon::CollectedPokemonInfo>>& boxes_data,
    const std::vector<LivingDexEntry>& living_dex_order,
    bool obey_gender_differences, bool shiny_dex = false
);


}
}
}
#endif

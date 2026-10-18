/*  Box Sorter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BoxSorter_H
#define PokemonAutomation_PokemonFRLG_BoxSorter_H

#include <cstdint>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_BoxSortingTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

struct BoxOrganizationStats;

class BoxSorter_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BoxSorter_Descriptor();

    using Stats = BoxOrganizationStats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class BoxSorter : public SingleSwitchProgramInstance{
public:
    using Descriptor = BoxSorter_Descriptor;
    BoxSorter();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
    virtual void start_program_border_check(VideoStream&, FeedbackType) override{}


private:
    SimpleIntegerOption<uint16_t> NUM_BOXES;
    Pokemon::BoxSortingTable SORT_TABLE;
    StringOption OUTPUT_FILE;
    BooleanCheckBoxOption DRY_RUN;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif

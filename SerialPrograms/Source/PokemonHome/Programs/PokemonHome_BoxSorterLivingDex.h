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

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

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

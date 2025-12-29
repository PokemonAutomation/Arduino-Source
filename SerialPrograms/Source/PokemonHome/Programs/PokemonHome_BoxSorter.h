/*  Home Box Sorter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_BoxSorter_H
#define PokemonAutomation_PokemonHome_BoxSorter_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "Pokemon/Options/Pokemon_BoxSortingTable.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

class BoxSorter_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BoxSorter_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class BoxSorter : public SingleSwitchProgramInstance{
public:
    BoxSorter();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint16_t> BOX_NUMBER;
    MillisecondsOption VIDEO_DELAY;
    MillisecondsOption GAME_DELAY;
    Pokemon::BoxSortingTable SORT_TABLE;
    StringOption OUTPUT_FILE;
    BooleanCheckBoxOption DRY_RUN;
    EventNotificationsOption NOTIFICATIONS;

};

}
}
}
#endif // BOXSORTING_H


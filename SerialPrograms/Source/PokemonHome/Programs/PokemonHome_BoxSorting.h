/*  Box Reorder National Dex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef BOXSORTING_H
#define BOXSORTING_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "PokemonHome/Options/PokemonHome_BoxSortingTable.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

class BoxSorting_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BoxSorting_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class BoxSorting : public SingleSwitchProgramInstance{
public:
    BoxSorting();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint16_t> BOX_NUMBER;
    SimpleIntegerOption<uint16_t> VIDEO_DELAY;
    SimpleIntegerOption<uint16_t> GAME_DELAY;
    BoxSortingTable SORT_TABLE;
    StringOption OUTPUT_FILE;
    BooleanCheckBoxOption DRY_RUN;
    EventNotificationsOption NOTIFICATIONS;

};

}
}
}
#endif // BOXSORTING_H


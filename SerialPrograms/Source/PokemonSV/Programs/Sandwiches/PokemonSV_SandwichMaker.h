/*  Sandwich Maker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_SandwichMaker_H
#define PokemonAutomation_PokemonSV_SandwichMaker_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "PokemonSV/Options/PokemonSV_SandwichMakerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class SandwichMaker_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SandwichMaker_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class SandwichMaker : public SingleSwitchProgramInstance{
public:
    SandwichMaker();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SandwichMakerOption SANDWICH_OPTIONS;
    SimpleIntegerOption<uint16_t> NUM_SANDWICHES;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif




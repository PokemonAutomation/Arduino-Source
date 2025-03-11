/*  ShinyHuntUnattended-StrongSpawn
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedStrongSpawn_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedStrongSpawn_H

#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntUnattendedStrongSpawn_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntUnattendedStrongSpawn_Descriptor();
};



class ShinyHuntUnattendedStrongSpawn : public SingleSwitchProgramInstance{
public:
    ShinyHuntUnattendedStrongSpawn();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrClosedOption START_LOCATION;
    TimeRollbackHoursOption TIME_ROLLBACK_HOURS;

    MillisecondsOption ENTER_GAME_TO_RUN_DELAY0;
    MillisecondsOption START_GAME_WAIT_DELAY0;
};

}
}
}
#endif

/*  ShinyHuntUnattended-StrongSpawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedStrongSpawn_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedStrongSpawn_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    StartInGripOrClosedOption START_LOCATION;
    TimeRollbackHoursOption TIME_ROLLBACK_HOURS;

    TimeExpressionOption<uint16_t> ENTER_GAME_TO_RUN_DELAY;
    TimeExpressionOption<uint16_t> START_GAME_WAIT_DELAY;
};

}
}
}
#endif

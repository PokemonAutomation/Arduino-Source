/*  ShinyHuntUnattended-StrongSpawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedStrongSpawn_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedStrongSpawn_H

//#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntUnattendedStrongSpawn_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntUnattendedStrongSpawn_Descriptor();
};



class ShinyHuntUnattendedStrongSpawn : public SingleSwitchProgramInstance{
public:
    ShinyHuntUnattendedStrongSpawn(const ShinyHuntUnattendedStrongSpawn_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, const BotBaseContext& context) override;

private:
    StartInGripOrClosedOption START_IN_GRIP_MENU;
    TimeRollbackHoursOption TIME_ROLLBACK_HOURS;

    TimeExpressionOption<uint16_t> ENTER_GAME_TO_RUN_DELAY;
    TimeExpressionOption<uint16_t> START_GAME_WAIT_DELAY;
};

}
}
}
#endif

/*  ShinyHuntUnattended-StrongSpawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedStrongSpawn_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedStrongSpawn_H

//#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntUnattendedStrongSpawn : public SingleSwitchProgram{
public:
    ShinyHuntUnattendedStrongSpawn();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    TimeExpression<uint16_t> ENTER_GAME_TO_RUN_DELAY;
    TimeExpression<uint16_t> START_GAME_WAIT_DELAY;
    SimpleInteger<uint8_t> TIME_ROLLBACK_HOURS;
};

}
}
}
#endif

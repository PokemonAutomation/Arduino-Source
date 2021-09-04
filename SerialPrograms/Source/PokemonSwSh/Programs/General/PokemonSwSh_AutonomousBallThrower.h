/*  Autonomous Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutonomousStatsReset_H
#define PokemonAutomation_PokemonSwSh_AutonomousStatsReset_H

#include "CommonFramework/OCR/LanguageOptionOCR.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "NintendoSwitch/Options/GoHomeWhenDone.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "Pokemon/Options/Pokemon_BallSelect.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class AutonomousBallThrower_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    AutonomousBallThrower_Descriptor();
};



class AutonomousBallThrower : public SingleSwitchProgramInstance{
public:
    AutonomousBallThrower(const AutonomousBallThrower_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    struct Stats;

    StartInGripOrGame START_IN_GRIP_MENU;
    GoHomeWhenDone GO_HOME_WHEN_DONE;
    Pokemon::PokemonBallSelect BALL_SELECT;
    OCR::LanguageOCR LANGUAGE;
};


}
}
}
#endif

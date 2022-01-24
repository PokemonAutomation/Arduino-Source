/*  Autonomous Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutonomousStatsReset_H
#define PokemonAutomation_PokemonBDSP_AutonomousStatsReset_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "Pokemon/Options/Pokemon_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
using namespace Pokemon;


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

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    PokemonBallSelect BALL_SELECT;
    OCR::LanguageOCR LANGUAGE;

    EventNotificationOption NOTIFICATION_CATCH_SUCCESS;
    EventNotificationOption NOTIFICATION_CATCH_FAILED;
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif

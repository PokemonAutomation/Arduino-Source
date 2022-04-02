/*  Autonomous Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutonomousStatsReset_H
#define PokemonAutomation_PokemonSwSh_AutonomousStatsReset_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "Pokemon/Options/Pokemon_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;


class AutonomousBallThrower_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    AutonomousBallThrower_Descriptor();
};



class AutonomousBallThrower : public SingleSwitchProgramInstance{
public:
    AutonomousBallThrower(const AutonomousBallThrower_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    struct Stats;

    StartInGripOrGameOption START_IN_GRIP_MENU;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    PokemonBallSelect BALL_SELECT;
    OCR::LanguageOCR LANGUAGE;

    EventNotificationOption NOTIFICATION_CATCH_SUCCESS;
    EventNotificationOption NOTIFICATION_CATCH_FAILED;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif

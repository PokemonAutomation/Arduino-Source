/*  Autonomous Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutonomousBallThrower_H
#define PokemonAutomation_PokemonBDSP_AutonomousBallThrower_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class AutonomousBallThrower_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutonomousBallThrower_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class AutonomousBallThrower : public SingleSwitchProgramInstance{
public:
    AutonomousBallThrower();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    OCR::LanguageOCROption LANGUAGE;

    EventNotificationOption NOTIFICATION_CATCH_SUCCESS;
    EventNotificationOption NOTIFICATION_CATCH_FAILED;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif

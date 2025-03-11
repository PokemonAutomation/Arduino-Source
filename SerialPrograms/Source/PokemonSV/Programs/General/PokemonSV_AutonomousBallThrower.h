/*  Autonomous Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutonomousBallThrower_H
#define PokemonAutomation_PokemonSV_AutonomousBallThrower_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



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
//    void throw_ball(VideoStream& stream, ProControllerContext& context);

    OCR::LanguageOCROption LANGUAGE;
    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    BooleanCheckBoxOption USE_FIRST_MOVE_IF_CANNOT_THROW_BALL;

    EventNotificationOption NOTIFICATION_CATCH_SUCCESS;
    EventNotificationOption NOTIFICATION_CATCH_FAILED;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif

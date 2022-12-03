/*  Autonomous Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutonomousBallThrower_H
#define PokemonAutomation_PokemonSV_AutonomousBallThrower_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
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
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void throw_ball(ConsoleHandle& console, BotBaseContext& context);

    OCR::LanguageOCROption LANGUAGE;
    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif

/*  Ride Cloner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_RideCloner_H
#define PokemonAutomation_PokemonSV_RideCloner_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSV/Options/PokemonSV_TeraAIOption.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{



class RideCloner101_Descriptor : public SingleSwitchProgramDescriptor{
public:
    RideCloner101_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class RideCloner101 : public SingleSwitchProgramInstance{
public:
    RideCloner101();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void setup(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);
    bool run_post_win(
        ProgramEnvironment& env,
        ConsoleHandle& console,
        ProControllerContext& context
    );

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCROption LANGUAGE;

    enum class Mode{
        CLONE_ONLY,
        SHINY_HUNT,
    };
    EnumDropdownOption<Mode> MODE;

    SimpleIntegerOption<uint16_t> RIDES_TO_CLONE;

    SimpleIntegerOption<uint8_t> MAX_STARS;

    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    BooleanCheckBoxOption FIX_TIME_ON_CATCH;

    MillisecondsOption A_TO_B_DELAY0;
    TeraAIOption BATTLE_AI;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_NONSHINY;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif

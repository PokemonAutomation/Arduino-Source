/*  Clone Items
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_CloneItems_H
#define PokemonAutomation_PokemonSV_CloneItems_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class CloneItems101_Descriptor : public SingleSwitchProgramDescriptor{
public:
    CloneItems101_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class CloneItems101 : public SingleSwitchProgramInstance{
public:
    CloneItems101();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    size_t fetch_eggs(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCR LANGUAGE;

    SimpleIntegerOption<uint16_t> ITEMS_TO_CLONE;

    SimpleIntegerOption<uint8_t> MAX_STARS;

    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    BooleanCheckBoxOption FIX_TIME_ON_CATCH;

    TimeExpressionOption<uint32_t> A_TO_B_DELAY;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif

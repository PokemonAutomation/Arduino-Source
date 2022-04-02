/*  Rolling Auto-Host
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoHostRolling_H
#define PokemonAutomation_PokemonSwSh_AutoHostRolling_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Options/RandomCodeOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_FriendCodeListOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_Catchability.h"
#include "PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class AutoHostRolling_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    AutoHostRolling_Descriptor();
};



class AutoHostRolling : public SingleSwitchProgramInstance{
public:
    AutoHostRolling(const AutoHostRolling_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, const BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    RandomCodeOption RAID_CODE;
    SimpleIntegerOption<uint8_t> SKIPS;
    BooleanCheckBoxOption BACKUP_SAVE;
    BooleanCheckBoxOption HOST_ONLINE;
    TimeExpressionOption<uint16_t> LOBBY_WAIT_DELAY;
    CatchabilitySelectorOption CATCHABILITY;
    SimpleIntegerOption<uint8_t> FRIEND_ACCEPT_USER_SLOT;
    TimeExpressionOption<uint16_t> EXTRA_DELAY_BETWEEN_RAIDS;

    SimpleIntegerOption<uint8_t> MOVE_SLOT;
    BooleanCheckBoxOption DYNAMAX;
    SimpleIntegerOption<uint8_t> TROLL_HOSTING;

    BooleanCheckBoxOption ALTERNATE_GAMES;

    AutoHostNotificationOption HOSTING_NOTIFICATIONS;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_internet_settings;
    TimeExpressionOption<uint16_t> CONNECT_TO_INTERNET_DELAY;
    TimeExpressionOption<uint16_t> ENTER_ONLINE_DEN_DELAY;
    TimeExpressionOption<uint16_t> OPEN_ONLINE_DEN_LOBBY_DELAY;
    TimeExpressionOption<uint16_t> RAID_START_TO_EXIT_DELAY;
    TimeExpressionOption<uint16_t> DELAY_TO_SELECT_MOVE;
};



}
}
}
#endif


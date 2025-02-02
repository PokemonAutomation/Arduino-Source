/*  Rolling Auto-Host
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoHostRolling_H
#define PokemonAutomation_PokemonSwSh_AutoHostRolling_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/RandomCodeOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
//#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
//#include "NintendoSwitch/Options/NintendoSwitch_FriendCodeListOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_Catchability.h"
#include "PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class AutoHostRolling_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoHostRolling_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class AutoHostRolling : public SingleSwitchProgramInstance{
public:
    AutoHostRolling();
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    RandomCodeOption RAID_CODE;
    SimpleIntegerOption<uint8_t> SKIPS;
    BooleanCheckBoxOption BACKUP_SAVE;
    BooleanCheckBoxOption HOST_ONLINE;
    MillisecondsOption LOBBY_WAIT_DELAY0;
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
    MillisecondsOption CONNECT_TO_INTERNET_DELAY0;
    TimeExpressionOption<uint16_t> ENTER_ONLINE_DEN_DELAY;
    MillisecondsOption OPEN_ONLINE_DEN_LOBBY_DELAY0;
    TimeExpressionOption<uint16_t> RAID_START_TO_EXIT_DELAY;
    TimeExpressionOption<uint16_t> DELAY_TO_SELECT_MOVE;
};



}
}
}
#endif


/*  Multi-Game Auto-Host
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoHostMultiGame_H
#define PokemonAutomation_PokemonSwSh_AutoHostMultiGame_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/RandomCodeOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/PokemonSwSh/PokemonSwSh_MultiHostTable.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class AutoHostMultiGame_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoHostMultiGame_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class AutoHostMultiGame : public SingleSwitchProgramInstance{
public:
    AutoHostMultiGame();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrClosedOption START_LOCATION;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    RandomCodeOption RAID_CODE;
    BooleanCheckBoxOption HOST_ONLINE;
    MillisecondsOption LOBBY_WAIT_DELAY0;
    MultiHostTable GAME_LIST;
    SimpleIntegerOption<uint8_t> FR_FORWARD_ACCEPT;

    AutoHostNotificationOption HOSTING_NOTIFICATIONS;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_internet_settings;
    MillisecondsOption CONNECT_TO_INTERNET_DELAY0;
    MillisecondsOption ENTER_ONLINE_DEN_DELAY0;
    MillisecondsOption OPEN_ONLINE_DEN_LOBBY_DELAY0;
    MillisecondsOption RAID_START_TO_EXIT_DELAY0;
    MillisecondsOption DELAY_TO_SELECT_MOVE0;
};



}
}
}
#endif

/*  Auto Host
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoHost_H
#define PokemonAutomation_PokemonSV_AutoHost_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TextEditOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSV/Options/PokemonSV_PlayerList.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class AutoHost_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoHost_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class AutoHost : public SingleSwitchProgramInstance{
public:
    AutoHost();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    bool run_lobby(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
//    OCR::LanguageOCR LANGUAGE;

    enum class Mode{
        LOCAL,
        ONLINE_CODED,
        ONLINE_EVERYONE,
    };

    EnumDropdownOption<Mode> MODE;

    SimpleIntegerOption<uint16_t> LOBBY_WAIT_DELAY;
    IntegerEnumDropdownOption START_RAID_PLAYERS;
    BooleanCheckBoxOption ROLLOVER_PREVENTION;

    TextEditOption DESCRIPTION;

    SimpleIntegerOption<uint8_t> CONSECUTIVE_FAILURE_PAUSE;
    SimpleIntegerOption<uint16_t> FAILURE_PAUSE_MINUTES;
    BooleanCheckBoxOption TRY_TO_TERASTILIZE;

    PlayerListTable BAN_LIST;

    EventNotificationOption NOTIFICATION_RAID_POST;
    EventNotificationOption NOTIFICATION_RAID_START;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif

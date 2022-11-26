/*  Auto Host
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoHost_H
#define PokemonAutomation_PokemonSV_AutoHost_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
//#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "Common/Cpp/Options/TextEditOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
//#include "CommonFramework/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

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
        ONLINE_EVERYONE,
        ONLINE_CODED,
    };

    EnumDropdownOption<Mode> MODE;

    SimpleIntegerOption<uint16_t> LOBBY_WAIT_DELAY;
    IntegerEnumDropdownOption START_RAID_PLAYERS;
    BooleanCheckBoxOption ROLLOVER_PREVENTION;

    TextEditOption DESCRIPTION;

    EventNotificationOption NOTIFICATION;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif

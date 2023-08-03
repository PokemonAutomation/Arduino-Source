/*  Tera Roller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraRoller_H
#define PokemonAutomation_PokemonSV_TeraRoller_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
//#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSV/Options/PokemonSV_TeraAIOption.h"

namespace PokemonAutomation{
   struct VideoSnapshot;
namespace NintendoSwitch{
namespace PokemonSV{

class TeraRoller;


class TeraRoller_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TeraRoller_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class TeraRollerOpponentFilter : public GroupOption{
public:
    TeraRollerOpponentFilter();

    bool should_battle(size_t stars, const std::string& pokemon) const;

    BooleanCheckBoxOption SKIP_HERBA;
    SimpleIntegerOption<uint8_t> MIN_STARS;
    SimpleIntegerOption<uint8_t> MAX_STARS;

};



class TeraRoller : public SingleSwitchProgramInstance{
public:
    TeraRoller();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    bool run_raid(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    OCR::LanguageOCROption LANGUAGE;

    TeraRollerOpponentFilter FILTER;
    TeraAIOption BATTLE_AI;

    //  Notifications
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_NONSHINY;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;

    uint16_t m_number_caught;

    //  Per iteration flags.
//    bool m_battle_finished;
//    bool m_caught;
//    bool m_summary_read;
};




}
}
}
#endif

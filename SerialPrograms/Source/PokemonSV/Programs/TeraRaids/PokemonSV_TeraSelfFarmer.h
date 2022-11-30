/*  Tera Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraSelfFarmer_H
#define PokemonAutomation_PokemonSV_TeraSelfFarmer_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
//#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"

namespace PokemonAutomation{
    struct VideoSnapshot;
namespace NintendoSwitch{
namespace PokemonSV{



class TeraSelfFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TeraSelfFarmer_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class TeraSelfFarmer : public SingleSwitchProgramInstance{
public:
    TeraSelfFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    bool run_raid(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    OCR::LanguageOCR LANGUAGE;

    enum class Mode{
        FARM_ITEMS_ONLY,
        CATCH_ALL,
        SHINY_HUNT,
    };
    static const EnumDatabase<Mode>& database();
    EnumDropdownOption<Mode> MODE;

    SimpleIntegerOption<uint8_t> MAX_STARS;
    BooleanCheckBoxOption TRY_TO_TERASTILLIZE;

    SimpleIntegerOption<uint16_t> MAX_CATCHES;
    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    BooleanCheckBoxOption FIX_TIME_ON_CATCH;

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

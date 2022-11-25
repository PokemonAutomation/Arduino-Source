/*  Tera Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraSelfFarmer_H
#define PokemonAutomation_PokemonSV_TeraSelfFarmer_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
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
    void process_catch_prompt(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    void read_summary(
        SingleSwitchProgramEnvironment& env,
        BotBaseContext& context,
        const VideoSnapshot& battle_snapshot
    );
    bool run_raid(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    enum class Mode{
        FARM_ITEMS_ONLY,
        CATCH_ALL,
        SHINY_HUNT,
    };
    static const EnumDatabase<TeraSelfFarmer::Mode>& database();
    EnumDropdownOption<Mode> MODE;

    OCR::LanguageOCR LANGUAGE;
    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    SimpleIntegerOption<uint16_t> MAX_CATCHES;
    BooleanCheckBoxOption FIX_TIME_ON_CATCH;

    SimpleIntegerOption<uint8_t> MAX_STARS;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_NONSHINY;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> RAID_SPAWN_DELAY;

    uint16_t m_caught;
};




}
}
}
#endif

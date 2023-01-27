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
#include "PokemonSV/Options/PokemonSV_TeraAIOption.h"

namespace PokemonAutomation{
    struct VideoSnapshot;
namespace NintendoSwitch{
namespace PokemonSV{

class TeraSelfFarmer;


class TeraSelfFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TeraSelfFarmer_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class TeraFarmerOpponentFilter : public GroupOption{
public:
    TeraFarmerOpponentFilter();

    bool should_battle(size_t stars) const;

    SimpleIntegerOption<uint8_t> MIN_STARS;
    SimpleIntegerOption<uint8_t> MAX_STARS;

};
class TeraFarmerCatchOnWin : public GroupOption{
public:
    TeraFarmerCatchOnWin(TeraSelfFarmer& program);
    virtual void on_set_enabled(bool enabled) override;

private:
    TeraSelfFarmer& m_program;

public:
    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    BooleanCheckBoxOption FIX_TIME_ON_CATCH;
};
class TeraFarmerStopConditions : public GroupOption{
public:
    TeraFarmerStopConditions();

    SimpleIntegerOption<uint16_t> MAX_CATCHES;
    BooleanCheckBoxOption STOP_ON_SHINY;
    SimpleIntegerOption<uint8_t> STOP_ON_RARE_ITEMS;
};



class TeraSelfFarmer : public SingleSwitchProgramInstance{
public:
    TeraSelfFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    bool run_raid(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    friend class TeraFarmerCatchOnWin;

    OCR::LanguageOCROption LANGUAGE;

    TeraFarmerOpponentFilter FILTER;
    TeraAIOption BATTLE_AI;
    TeraFarmerCatchOnWin CATCH_ON_WIN;
    TeraFarmerStopConditions STOP_CONDITIONS;

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

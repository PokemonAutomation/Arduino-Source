/*  Tera Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraSelfFarmer_H
#define PokemonAutomation_PokemonSV_TeraSelfFarmer_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSV/Options/PokemonSV_TeraRollFilter.h"
#include "PokemonSV/Options/PokemonSV_TeraAIOption.h"
#include "PokemonSV/Options/PokemonSV_TeraCatchOnWinOption.h"

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




class TeraFarmerStopConditions : public GroupOption{
public:
    TeraFarmerStopConditions();

    SimpleIntegerOption<uint16_t> MAX_CATCHES;
    BooleanCheckBoxOption STOP_ON_SHINY;
    SimpleIntegerOption<uint8_t> STOP_ON_RARE_ITEMS;
};



class TeraSelfFarmer : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ~TeraSelfFarmer();
    TeraSelfFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    virtual void on_config_value_changed(void* object) override;
    bool run_raid(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    friend class TeraFarmerCatchOnWin;

    OCR::LanguageOCROption LANGUAGE;

//    TeraFarmerOpponentFilter FILTER;
    TeraRollFilter FILTER;
    TeraAIOption BATTLE_AI;
    SimpleIntegerOption<uint8_t> PERIODIC_RESET;
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

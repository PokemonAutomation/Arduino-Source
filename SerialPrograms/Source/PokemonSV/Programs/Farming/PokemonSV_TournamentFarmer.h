/*  Tournament Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TournamentFarmer_H
#define PokemonAutomation_PokemonSwSh_TournamentFarmer_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Options/PokemonSV_TournamentPrizeTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


void return_to_academy_after_loss(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);

// attempt to fly back to academy fly point from the West Mesogoza Pokecenter. Will attempt maxAttempts times.
void go_to_academy_fly_point(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);

class TournamentFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TournamentFarmer_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class TournamentFarmer : public SingleSwitchProgramInstance, public ButtonListener{
public:
    ~TournamentFarmer();
    TournamentFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;
    virtual void on_press() override;

private:
    class StopButton : public ButtonOption{
    public:
        StopButton();
        void set_idle();
        void set_ready();
        void set_pressed();
    };
    class ResetOnExit;

    StopButton STOP_AFTER_CURRENT;
    SimpleIntegerOption<uint32_t> NUM_ROUNDS;
    BooleanCheckBoxOption TRY_TO_TERASTILLIZE;
    SimpleIntegerOption<uint16_t> SAVE_NUM_ROUNDS;
    SimpleIntegerOption<uint32_t> MONEY_LIMIT;
    BooleanCheckBoxOption HHH_ZOROARK;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    OCR::LanguageOCROption LANGUAGE;
    TournamentPrizeTable TARGET_ITEMS;
    EventNotificationOption NOTIFICATION_PRIZE_MATCH;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    std::atomic<bool> m_stop_after_current;

    void check_money(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
    void run_battle(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
    void check_prize(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
    void handle_end_of_tournament(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
};

}
}
}
#endif




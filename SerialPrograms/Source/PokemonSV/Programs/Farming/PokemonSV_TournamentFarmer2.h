/*  Tournament Farmer 2
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TournamentFarmer2_H
#define PokemonAutomation_PokemonSwSh_TournamentFarmer2_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Options/PokemonSV_SinglesAIOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class TournamentFarmer2_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TournamentFarmer2_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class TournamentFarmer2 : public SingleSwitchProgramInstance, public ButtonListener{
public:
    ~TournamentFarmer2();
    TournamentFarmer2();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
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
    SimpleIntegerOption<uint16_t> SAVE_NUM_ROUNDS;
    SinglesAIOption BATTLE_AI;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    std::atomic<bool> m_stop_after_current;
};

}
}
}
#endif




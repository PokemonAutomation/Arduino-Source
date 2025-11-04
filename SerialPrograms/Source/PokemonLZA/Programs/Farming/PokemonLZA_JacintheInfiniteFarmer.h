/*  Jacinthe Infinite Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_JacintheInfiniteFarmer_H
#define PokemonAutomation_PokemonLZA_JacintheInfiniteFarmer_H

#include <atomic>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonLZA{


class JacintheInfiniteFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    JacintheInfiniteFarmer_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class JacintheInfiniteFarmer : public SingleSwitchProgramInstance, public ButtonListener{
public:
    ~JacintheInfiniteFarmer();
    JacintheInfiniteFarmer();

    virtual void on_press() override;
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    // Advance dialog windows and select dialog menuitems when talking to the Jacinthe in overworld.
    // This function can be called in two cases:
    // - When starting the proram, player is in overworld with Button A floating in front of Jacinthe.
    // - When a battle finishes, player is in dialog with Jacinthe, where the program can choose to continue
    //   battling or exit.
    // Return true when the user clicks the button STOP_AFTER_CURRENT to stop or the required amount of rounds is
    // reached. Return false when it finishes the dialog menuitem selection to confirm entering the battle.
    bool talk_to_jacinthe(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    // Handle all round logic. Return when it detects the blue dialog box meaning the player character is at
    // the receptionist receiving reward items.
    void run_round(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    class StopButton : public ButtonOption{
    public:
        StopButton();
        void set_idle();
        void set_ready();
        void set_pressed();
    };
    class ResetOnExit;

    std::atomic<bool> m_stop_after_current;
    StopButton STOP_AFTER_CURRENT;
    SimpleIntegerOption<uint32_t> NUM_ROUNDS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    BooleanCheckBoxOption MOVE_AI;
    BooleanCheckBoxOption USE_PLUS_MOVES;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif

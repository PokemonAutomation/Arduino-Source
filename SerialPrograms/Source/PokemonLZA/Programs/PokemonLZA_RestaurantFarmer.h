/*  Restaurant Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_RestaurantFarmer_H
#define PokemonAutomation_PokemonLZA_RestaurantFarmer_H

#include <atomic>
#include "Common/Cpp/Options/ButtonOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class RestaurantFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    RestaurantFarmer_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class RestaurantFarmer : public SingleSwitchProgramInstance, public ButtonListener{
public:
    ~RestaurantFarmer();
    RestaurantFarmer();

    virtual void on_press() override;
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

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

    // Handle all the logic of talking to the restaurant receptionist.
    // Return true when the user clicks the button STOP_AFTER_CURRENT and the player character stops talking to
    // the receptionist. Return false when it enters battle.
    bool run_lobby(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    // Handle all battle logic. Return when it detects the blue dialog box meaning the player character is at
    // the receptionist receiving reward items.
    void run_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
};





}
}
}
#endif

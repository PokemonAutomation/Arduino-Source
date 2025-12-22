/*  Donut Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DonutFarmer_H
#define PokemonAutomation_PokemonLZA_DonutFarmer_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Common/Cpp/Options/ButtonOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class DonutFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DonutFarmer_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class DonutFarmer : public SingleSwitchProgramInstance{
public:
    DonutFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:

    // Helper functions for cleaner navigation
    void move_joystick_steps(ProControllerContext& context, uint8_t x, uint8_t y, int steps);
    void press_button_steps(ProControllerContext& context, Button button, int steps);

    enum class DonutOption{
        Kasib,
        Tanga,
        Battle_Zone_1,
        Battle_Zone_2,
        Item_Power_1,
        Item_Power_2,
        Sparkling_Power_1,
        Sparkling_Power_2,
        Sparkling_Power_3,
        Cherry
    };
    EnumDropdownOption<DonutOption> DONUT_OPTION;

    ButtonOption SAVE_AND_RESUME;
    ButtonOption RESET_WITHOUT_SAVING;


    struct MyButtonListener : public ButtonListener {
        std::atomic<bool>& pressed;
        MyButtonListener(std::atomic<bool>& p) : pressed(p) {}
        virtual void on_press() override { pressed.store(true); }
    };

    std::atomic<bool> save_requested{false};
    std::atomic<bool> reset_requested{false};

};



}
}
}
#endif

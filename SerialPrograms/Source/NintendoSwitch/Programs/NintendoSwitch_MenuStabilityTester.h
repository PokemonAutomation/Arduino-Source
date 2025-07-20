/*  Menu Stability Tester
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_MenuStabilityTester_H
#define PokemonAutomation_NintendoSwitch_MenuStabilityTester_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class MenuStabilityTester_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MenuStabilityTester_Descriptor();
};



class MenuStabilityTester : public SingleSwitchProgramInstance, private ConfigOption::Listener{
public:
    ~MenuStabilityTester();
    MenuStabilityTester();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    virtual void on_config_value_changed(void* object) override;

private:
    enum class TestType{
        Vertical,
        Horizontal,
        SimultaneousScrollA,
    };

    EnumDropdownOption<TestType> TEST_TYPE;
    SimpleIntegerOption<uint8_t> VERTICAL_RANGE;
    SimpleIntegerOption<uint8_t> HORIZONTAL_RANGE;
    BooleanCheckBoxOption PAUSE_BEFORE_UTURN;

    MillisecondsOption DELAY_TO_NEXT;
    MillisecondsOption HOLD_DURATION;
    MillisecondsOption COOLDOWN;

};




}
}
#endif




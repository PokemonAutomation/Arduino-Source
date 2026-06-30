/*  Joycon Gyro Tester
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_JoyconGyroTester_H
#define PokemonAutomation_NintendoSwitch_JoyconGyroTester_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class JoyconGyroTester_Descriptor : public SingleSwitchProgramDescriptor{
public:
    JoyconGyroTester_Descriptor();
};


class JoyconGyroTester : public SingleSwitchProgramInstance, private ConfigOption::Listener{
public:
    ~JoyconGyroTester();
    JoyconGyroTester();

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

};


}
}
#endif

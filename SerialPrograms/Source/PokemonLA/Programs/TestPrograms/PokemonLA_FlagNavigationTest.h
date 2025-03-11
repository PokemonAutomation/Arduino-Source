/*  Flag Navigation Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_FlagNavigationTest_H
#define PokemonAutomation_PokemonLA_FlagNavigationTest_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
//#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



class FlagNavigationTest_Descriptor : public SingleSwitchProgramDescriptor{
public:
    FlagNavigationTest_Descriptor();
};


class FlagNavigationTest : public SingleSwitchProgramInstance{
public:
    FlagNavigationTest();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint16_t> STOP_DISTANCE;
    FloatingPointOption FLAG_REACHED_DELAY;

    SimpleIntegerOption<uint64_t> NAVIGATION_TIMEOUT;
//    ShinyDetectedActionOption SHINY_DETECTED;
};



}
}
}
#endif

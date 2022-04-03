/*  Flag Navigation Test
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_FlagNavigationTest_H
#define PokemonAutomation_PokemonLA_FlagNavigationTest_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/FloatingPointOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



class FlagNavigationTest_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    FlagNavigationTest_Descriptor();
};


class FlagNavigationTest : public SingleSwitchProgramInstance{
public:
    FlagNavigationTest(const FlagNavigationTest_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    SimpleIntegerOption<uint16_t> STOP_DISTANCE;
    FloatingPointOption FLAG_REACHED_DELAY;

    SimpleIntegerOption<uint64_t> NAVIGATION_TIMEOUT;
    ShinyDetectedActionOption SHINY_DETECTED;
};



}
}
}
#endif

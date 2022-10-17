/*  RNG Manipulation Seedfinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Seedfinder_H
#define PokemonAutomation_PokemonSwSh_Seedfinder_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_Xoroshiro128Plus.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSwSh {

class Seedfinder_Descriptor : public SingleSwitchProgramDescriptor {
public:
    Seedfinder_Descriptor();
};


class Seedfinder : public SingleSwitchProgramInstance {
public:
    Seedfinder();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    
    StringOption STATE_0;
    StringOption STATE_1;
    BooleanCheckBoxOption UPDATE_STATE;
    SimpleIntegerOption<uint64_t> MIN_ADVANCES;
    SimpleIntegerOption<uint64_t> MAX_ADVANCES;

    SectionDividerOption m_advanced_options;
    BooleanCheckBoxOption SAVE_SCREENSHOTS;
    BooleanCheckBoxOption LOG_VALUES;
};




}
}
}
#endif




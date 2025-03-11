/*  RNG Manipulation SeedFinder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SeedFinder_H
#define PokemonAutomation_PokemonSwSh_SeedFinder_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class SeedFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SeedFinder_Descriptor();
};


class SeedFinder : public SingleSwitchProgramInstance{
public:
    SeedFinder();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

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




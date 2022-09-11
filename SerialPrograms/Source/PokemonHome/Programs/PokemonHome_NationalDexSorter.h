/*  Pokemon Home National Dex Sorter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonHome_NationalDexSorter_H
#define PokemonAutomation_PokemonHome_NationalDexSorter_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

class NationalDexSorter_Descriptor: public SingleSwitchProgramDescriptor{
public:
    NationalDexSorter_Descriptor();
};

class NationalDexSorter : public SingleSwitchProgramInstance{
public:
    NationalDexSorter();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    SimpleIntegerOption<uint32_t> START_FROM_DEX_NO;
    SimpleIntegerOption<uint32_t> GET_TO_DEX_NO;
    SimpleIntegerOption<uint32_t> START_FROM_PAGE;
    BooleanCheckBoxOption CHOOSE_SHINY_FIRST;
};


}
}
}
#endif

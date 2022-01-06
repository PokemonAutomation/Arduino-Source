/*  PokeJobs Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_PokeJobsFarmer_H
#define PokemonAutomation_PokemonSwSh_PokeJobsFarmer_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation
{
namespace NintendoSwitch
{
namespace PokemonSwSh
{

class PokeJobsFarmer_Descriptor : public RunnableSwitchProgramDescriptor
{
public:
    PokeJobsFarmer_Descriptor();
};

class PokeJobsFarmer : public SingleSwitchProgramInstance
{
public:
    PokeJobsFarmer(const PokeJobsFarmer_Descriptor &descriptor);

    virtual void program(SingleSwitchProgramEnvironment &env) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;

    SimpleIntegerOption<uint32_t> SKIPS;
    SimpleIntegerOption<uint32_t> CONCURRENCY;
    SimpleIntegerOption<uint32_t> MENU_INDEX;

    SectionDividerOption m_advanced_options;

    TimeExpressionOption<uint16_t> MASH_B_DURATION;
};

}
}
}
#endif

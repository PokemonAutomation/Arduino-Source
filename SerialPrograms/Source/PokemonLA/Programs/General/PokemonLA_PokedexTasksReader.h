/*  Pokedex Tasks Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_PokedexTasksReader_H
#define PokemonAutomation_PokemonLA_PokedexTasksReader_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class PokedexTasksReader_Descriptor : public SingleSwitchProgramDescriptor{
public:
    PokedexTasksReader_Descriptor();
};


class PokedexTasksReader : public SingleSwitchProgramInstance{
public:
    PokedexTasksReader();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
};





}
}
}
#endif

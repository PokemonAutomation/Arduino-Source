/*  Pokedex Tasks Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_PokedexTasksReader_H
#define PokemonAutomation_PokemonLA_PokedexTasksReader_H

#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class PokedexTasksReader_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    PokedexTasksReader_Descriptor();
};


class PokedexTasksReader : public SingleSwitchProgramInstance{
public:
    PokedexTasksReader(const PokedexTasksReader_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;
};





}
}
}
#endif

/*  Pokedex Tasks Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;
};





}
}
}
#endif

/*  Overworld Watcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_OverworldWatcher_H
#define PokemonAutomation_PokemonLZA_OverworldWatcher_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class OverworldWatcher_Descriptor : public SingleSwitchProgramDescriptor{
public:
    OverworldWatcher_Descriptor();
};


class OverworldWatcher : public SingleSwitchProgramInstance{
public:
    OverworldWatcher();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
};





}
}
}
#endif

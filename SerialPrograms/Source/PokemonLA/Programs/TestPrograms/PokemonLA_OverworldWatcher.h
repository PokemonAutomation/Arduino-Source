/*  Overworld Watcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_OverworldWatcher_H
#define PokemonAutomation_PokemonLA_OverworldWatcher_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class OverworldWatcher_Descriptor : public SingleSwitchProgramDescriptor{
public:
    OverworldWatcher_Descriptor();
};


class OverworldWatcher : public SingleSwitchProgramInstance{
public:
    OverworldWatcher();

    virtual void start_program_controller_check(ControllerSession& session) override{}
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;
};





}
}
}
#endif

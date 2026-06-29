/*  Read Summary Screen
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_ReadSummaryScreen_H
#define PokemonAutomation_PokemonHome_ReadSummaryScreen_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

class ReadSummaryScreen_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ReadSummaryScreen_Descriptor();
};

class ReadSummaryScreen : public SingleSwitchProgramInstance{
public:
    ReadSummaryScreen();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
};

}
}
}
#endif

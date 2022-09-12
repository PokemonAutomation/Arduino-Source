/*  Box Reorder National Dex
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef BOXSORTING_H
#define BOXSORTING_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

class BoxSorting_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BoxSorting_Descriptor();
};

class BoxSorting : public SingleSwitchProgramInstance{
public:
    BoxSorting();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;
};

}
}
}
#endif // BOXSORTING_H


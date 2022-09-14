/*  Box Reorder National Dex
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef BOXSORTING_H
#define BOXSORTING_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"

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

private:
    SimpleIntegerOption<uint16_t> BOX_NUMBER;
    SimpleIntegerOption<uint16_t> VIDEO_DELAY;
    SimpleIntegerOption<uint16_t> GAME_DELAY;

};

}
}
}
#endif // BOXSORTING_H


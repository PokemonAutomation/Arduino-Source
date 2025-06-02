/*  Box Draw
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_BoxDraw_H
#define PokemonAutomation_NintendoSwitch_BoxDraw_H

#include "Common/Cpp/Options/FloatingPointOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class BoxDraw_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BoxDraw_Descriptor();
};


// Draw box on the video stream
class BoxDraw : public SingleSwitchProgramInstance{
public:
    BoxDraw();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    class DrawnBox;

private:
    FloatingPointOption X;
    FloatingPointOption Y;
    FloatingPointOption WIDTH;
    FloatingPointOption HEIGHT;
};





}
}
#endif

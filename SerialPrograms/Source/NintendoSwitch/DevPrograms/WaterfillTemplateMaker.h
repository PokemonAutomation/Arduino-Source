/*  Waterfill Template Maker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_WaterfillTemplateMaker_H
#define PokemonAutomation_NintendoSwitch_WaterfillTemplateMaker_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/ColorOption.h"
#include "CommonFramework/Options/NestedBoxDrawOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class WaterfillTemplateMaker_Descriptor : public SingleSwitchProgramDescriptor{
public:
    WaterfillTemplateMaker_Descriptor();
};


class WaterfillTemplateMaker : public SingleSwitchProgramInstance{
public:
    WaterfillTemplateMaker(ConsoleInfra::ConsoleSystemSession& system);

    virtual void start_program_controller_check(ControllerSession& session) override{}
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    SimpleIntegerOption<size_t> MIN_AREA;
    ColorOption FILTER_LOWER;
    ColorOption FILTER_UPPER;
    NestedBoxDrawOption BOX_DRAW;
};





}
}
#endif

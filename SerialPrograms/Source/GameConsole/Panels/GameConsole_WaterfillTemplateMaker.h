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
#include "GameConsole/ConsoleProgram.h"

namespace PokemonAutomation{
namespace GameConsole{




class WaterfillTemplateMaker_Descriptor : public ConsoleProgramDescriptor{
public:
    WaterfillTemplateMaker_Descriptor();
};


class WaterfillTemplateMaker : public ConsoleProgramInstance{
public:
    using Descriptor = WaterfillTemplateMaker_Descriptor;
    WaterfillTemplateMaker(GameConsole::ConsoleSystemSession& system);

    virtual void program(ConsoleProgramEnvironment& env, CancellableScope& scope) override;

private:
    SimpleIntegerOption<size_t> MIN_AREA;
    ColorOption FILTER_LOWER;
    ColorOption FILTER_UPPER;
    NestedBoxDrawOption BOX_DRAW;
};





}
}
#endif

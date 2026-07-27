/*  Waterfill Template Maker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_WaterfillTemplateMaker_H
#define PokemonAutomation_NintendoSwitch_WaterfillTemplateMaker_H

#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/ColorOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "CommonFramework/Options/NestedBoxDrawOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class WaterfillTemplateMaker_Descriptor : public SingleSwitchProgramDescriptor{
public:
    WaterfillTemplateMaker_Descriptor();
};


class WaterfillTemplateMaker : public SingleSwitchProgramInstance, public ButtonListener{
public:
    ~WaterfillTemplateMaker();
    WaterfillTemplateMaker();

    virtual void start_program_controller_check(ControllerSession& session) override{}
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

    virtual void on_press(ButtonCell& button) override;

private:
    ButtonOption BUTTON;
    SimpleIntegerOption<size_t> MIN_AREA;
    ColorOption FILTER_LOWER;
    ColorOption FILTER_UPPER;
    NestedBoxDrawOption BOX_DRAW;

    Mutex m_lock;
    VideoStream* m_stream;
};





}
}
#endif

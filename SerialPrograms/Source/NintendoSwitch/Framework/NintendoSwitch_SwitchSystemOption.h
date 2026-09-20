/*  Switch System Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class represents the serializable state of a Switch console.
 *  Specifially, holds the settings of:
 *      -   Serial Port
 *      -   Camera
 *      -   Audio
 *
 *  This class maintains no runtime state or UI and is not thread-safe.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSystemOption_H
#define PokemonAutomation_NintendoSwitch_SwitchSystemOption_H

#include "CommonFramework/Panels/ProgramDescriptor.h"
#include "GameConsole/Framework/ConsoleSystemOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


Color pick_color(ProgramControllerClass color_class);


//  Options to control and monitor a Switch. It includes
//  what micro-controller and what video source to use and
//  what video overlay display option to set.
class SwitchSystemOption : public GameConsole::ConsoleSystemOption{
public:
    SwitchSystemOption()
        : ConsoleSystemOption(1, std::make_unique<ConsoleModelCell>())
    {}
    SwitchSystemOption(const JsonValue& json)
        : ConsoleSystemOption(1, std::make_unique<ConsoleModelCell>(), json)
    {
        SwitchSystemOption::load_json(json);
    }
};





}
}
#endif

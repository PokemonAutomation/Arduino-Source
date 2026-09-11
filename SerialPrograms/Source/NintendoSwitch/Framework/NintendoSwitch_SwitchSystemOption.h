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
#include "GameConsole/ConsoleSystemOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


Color pick_color(ProgramControllerClass color_class);


//  Options to control and monitor a Switch. It includes
//  what micro-controller and what video source to use and
//  what video overlay display option to set.
class SwitchSystemOption : public GameConsole::ConsoleSystemOption{
    static const std::string JSON_CONSOLE_TYPE;

public:
    SwitchSystemOption();
    SwitchSystemOption(const JsonValue& json);

    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;

    void load_json_self(const JsonValue& json);


public:
    ConsoleModelCell m_console_type;
};





}
}
#endif

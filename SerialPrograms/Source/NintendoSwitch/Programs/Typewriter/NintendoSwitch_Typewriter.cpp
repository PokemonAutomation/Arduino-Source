/*  Typewriter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_Typewriter.h"
#include "NintendoSwitch_TypewriterTextMapping.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace Typewriter{




Typewriter_Descriptor::Typewriter_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:Typewriter",
        "Nintendo Switch", "Typewriter",
        "Programs/NintendoSwitch/Typewriter.html",
        "Enter text into the Switch using the HID keyboard controller.",
        ProgramControllerClass::SpecializedController,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}




Typewriter::Typewriter()
    : TEXT(
        "<b>Text to Type:</b><br>"
        "Plain ASCII text. Tab presses Tab and newlines press Enter. "
        "Windows (CRLF) line endings are accepted.",
        LockMode::LOCK_WHILE_RUNNING,
        "",
        "Text to type into the Switch"
    )
    , KEYBOARD_LAYOUT(
        "<b>Keyboard Layout:</b><br>"
        "The layout the Switch's physical keyboard is configured for "
        "(i.e. the Switch's language)."
    )
{
    PA_ADD_OPTION(TEXT);
    PA_ADD_OPTION(KEYBOARD_LAYOUT);
}




void Typewriter::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    StandardHid::KeyboardContext context(scope, env.console.controller<StandardHid::Keyboard>());

    std::string text = TEXT;
    if (text.empty()){
        throw UserSetupError(
            context->logger(),
            "No text to type. Please enter some text in the \"Text to Type\" option."
        );
    }

    //  Converts the whole text up front, so we don't type half the text and
    //  then fail on an unsupported character.
    std::vector<std::vector<KeyboardKey>> chords =
        text_to_key_chords(KEYBOARD_LAYOUT.get(), text);

    Milliseconds delay = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.TIME_UNIT;
    Milliseconds hold = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.HOLD;
    Milliseconds cool = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.COOLDOWN;

    for (const std::vector<KeyboardKey>& keys : chords){
        if (keys.size() == 1){
            context->issue_key(&context, delay, hold, cool, keys[0]);
        }else{
            context->issue_keys(&context, delay, hold, cool, keys);
        }
    }

    context.wait_for_all_requests();
}




}
}
}

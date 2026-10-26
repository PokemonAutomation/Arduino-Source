/*  Typewriter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include "Common/Cpp/Exceptions.h"
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
        PanelQualityStanding::OFFICIAL_BETA,
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
    , DELAY(
        "<b>Delay:</b><br>Delay between key presses.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "24 ms"
    )
    , HOLD(
        "<b>Hold:</b><br>Duration to hold each key press down.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "40 ms"
    )
    , COOLDOWN(
        "<b>Cooldown:</b><br>Do not reuse a key until this long after it is released.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "24 ms"
    )
{
    PA_ADD_OPTION(TEXT);
    PA_ADD_OPTION(KEYBOARD_LAYOUT);
    PA_ADD_OPTION(DELAY);
    PA_ADD_OPTION(HOLD);
    PA_ADD_OPTION(COOLDOWN);
}




void Typewriter::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    using namespace std::chrono_literals;

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

    Milliseconds delay = DELAY;
    Milliseconds hold = HOLD;
    Milliseconds cool = COOLDOWN;

    for (const std::vector<KeyboardKey>& keys : chords){
        if (keys.size() == 1){
            context->issue_key(&context, delay, hold, cool, keys[0]);
        }else{
            context->issue_keys(&context, delay, hold, cool, keys);
        }
        if (keys.size() > 1){
            context->issue_barrier(&context);
        }
    }

    context.wait_for_all_requests();
}




}
}
}

/*  Typewriter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstdio>
#include <string>
#include "CommonFramework/Exceptions/OperationFailedException.h"
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
        "Plain ASCII text. Newline characters press Enter.",
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




static std::string char_to_display(char ch){
    if (ch == '\n'){
        return "newline";
    }
    unsigned char uch = (unsigned char)ch;
    if (uch >= 0x20 && uch <= 0x7e){
        return std::string(1, ch);
    }
    char buffer[8];
    std::snprintf(buffer, sizeof(buffer), "0x%02x", uch);
    return std::string(buffer);
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

    KeyboardLayout layout = KEYBOARD_LAYOUT.get();
    const std::map<char, TextKeyMapping>& mapping = TEXT_TO_HID_KEY(layout);

    //  Check the whole text up front, so we don't type half the text and
    //  then fail on an unsupported character.
    std::string reason;
    if (layout == KeyboardLayout::AZERTY){
        reason = "On a French keyboard this character requires the AltGr "
                 "key, which this program does not support.";
    }else{
        reason = "Only printable ASCII characters are supported.";
    }
    for (char ch : text){
        if (mapping.find(ch) == mapping.end()){
            throw_and_log<OperationFailedException>(
                env.console,
                ErrorReportMode::NO_ERROR_REPORT,
                std::string("Unsupported character for the ") +
                (layout == KeyboardLayout::QWERTY ? "QWERTY" : "AZERTY") +
                " keyboard layout: \"" + char_to_display(ch) + "\". " +
                reason + " Please use plain ASCII text."
            );
        }
    }

    Milliseconds delay = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.TIME_UNIT;
    Milliseconds hold = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.HOLD;
    Milliseconds cool = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.COOLDOWN;

    for (char ch : text){
        TextKeyMapping m = mapping.at(ch);
        std::vector<KeyboardKey> keys;
        control_flags_to_keys(m.control_flags, keys);
        keys.push_back(m.key);
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

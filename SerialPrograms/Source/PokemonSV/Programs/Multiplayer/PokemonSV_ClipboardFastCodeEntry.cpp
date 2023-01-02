/*  Fast Code Entry (Clipboard)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QApplication>
#include <QClipboard>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CancellableScope.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_DigitEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_FastCodeEntry.h"
#include "PokemonSV_ClipboardFastCodeEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


ClipboardFastCodeEntry_Descriptor::ClipboardFastCodeEntry_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSV:ClipboardFastCodeEntry",
        STRING_POKEMON + " SV", "Clipboard Fast Code Entry (C-FCE)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ClipboardFastCodeEntry.md",
        "Automatically enter a 4, 6, or 8 digit link code from your clipboard.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}

ClipboardFastCodeEntry::ClipboardFastCodeEntry()
    : KEYBOARD_LAYOUT(
        "<b>Keyboard Layout:</b>",
        {
            {KeyboardLayout::QWERTY, "qwerty", "QWERTY"},
            {KeyboardLayout::AZERTY, "azerty", "AZERTY"},
        },
        LockWhileRunning::LOCKED,
        KeyboardLayout::QWERTY
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options: (developer only)</b></font>"
    )
    , SKIP_PLUS(
        "<b>Skip the Plus:</b>",
        LockWhileRunning::LOCKED,
        false
    )
    , DIGIT_REORDERING(
        "<b>Digit Reordering:</b><br>Allow digits to be entered out of order.",
        LockWhileRunning::LOCKED,
        PreloadSettings::instance().DEVELOPER_MODE,
        PreloadSettings::instance().DEVELOPER_MODE
    )
    , SCROLL_DELAY(
        "<b>Scroll Delay:</b><br>Delay to scroll between adjacent keys.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        3, 15,
        PreloadSettings::instance().DEVELOPER_MODE ? "4" : "10",
        PreloadSettings::instance().DEVELOPER_MODE ? "4" : "10"
    )
    , WRAP_DELAY(
        "<b>Wrap Delay:</b><br>Delay to wrap between left/right edges.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        3, 15,
        PreloadSettings::instance().DEVELOPER_MODE ? "6" : "10",
        PreloadSettings::instance().DEVELOPER_MODE ? "6" : "10"
    )
{
    PA_ADD_OPTION(KEYBOARD_LAYOUT);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(m_advanced_options);
        PA_ADD_OPTION(SKIP_PLUS);
        PA_ADD_OPTION(DIGIT_REORDERING);
        PA_ADD_OPTION(SCROLL_DELAY);
        PA_ADD_OPTION(WRAP_DELAY);
    }
}


void ClipboardFastCodeEntry::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    FastCodeEntrySettings settings{
        KEYBOARD_LAYOUT,
        !SKIP_PLUS,
        SCROLL_DELAY, WRAP_DELAY, DIGIT_REORDERING
    };

    QClipboard* clipboard = QApplication::clipboard();
#if 0
    std::atomic<bool> cleared(false);
    QMetaObject::invokeMethod(clipboard, [clipboard, &cleared, &env]{
        clipboard->clear();
        cleared.store(true, std::memory_order_release);
        env.log("Clipboard cleared.");
    }, Qt::QueuedConnection);

    env.log("Clearing clipboard...");
    while (!cleared.load(std::memory_order_acquire)){
        scope.throw_if_cancelled();
    }
#endif
    std::string start_text = clipboard->text().toStdString();

    while (true){
        std::string code = clipboard->text().toStdString();
        if (code != start_text && !code.empty()){
            const char* error = enter_code(env, scope, settings, code);
            if (error == nullptr){
                return;
            }
        }
        scope.wait_for(std::chrono::milliseconds(1));
    }

}








}
}
}

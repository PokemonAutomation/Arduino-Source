/*  Fast Code Entry (Clipboard)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QApplication>
#include <QClipboard>
//#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CancellableScope.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_ConsoleTypeDetector.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_CodeEntry.h"
#include "PokemonSV_ClipboardFastCodeEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

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
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        1, 4, 1
    )
{}

ClipboardFastCodeEntry::ClipboardFastCodeEntry(){
    PA_ADD_OPTION(SETTINGS);
}
void ClipboardFastCodeEntry::update_active_consoles(size_t switch_count){
    SETTINGS.set_active_consoles(switch_count);
}


void ClipboardFastCodeEntry::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    //  Connect the controller.
    env.run_in_parallel(scope, [&](CancellableScope& scope, ConsoleHandle& console){
        auto* procon = console.controller().cast<ProController>();
        if (procon == nullptr){
            return;
        }
        ProControllerContext context(scope, *procon);
        ssf_press_button_ptv(context, BUTTON_R | BUTTON_L);
        detect_console_type_from_in_game(console, context);
    });


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
//        cout << code << endl;
        if (code != start_text && !code.empty()){
            const char* error = enter_code(env, scope, SETTINGS, code, false, false);
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

/*  Fast Code Entry (Clipboard)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QApplication>
#include <QClipboard>
//#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CancellableScope.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_CodeEntry.h"
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
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {ControllerFeature::NintendoSwitch_ProController},
        FasterIfTickPrecise::MUCH_FASTER,
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
    env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
        pbf_press_button(context, BUTTON_R | BUTTON_L, 5, 3);
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

/*  Shiny Hunt - Overworld Watcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceSession.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_MoveEffectivenessSymbol.h"
#include "PokemonLZA_OverworldWatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;


OverworldWatcher_Descriptor::OverworldWatcher_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:OverworldWatcher",
        STRING_POKEMON + " LZA", "Overworld Watcher",
        "",
        "This is a test program that simply observes the game and labels things of interest. "
        "This program doesn't really do anything.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}


OverworldWatcher::OverworldWatcher(){}


void OverworldWatcher::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    VideoOverlay& overlay = env.console.overlay();

    FlatWhiteDialogWatcher dialog_white(COLOR_RED, &overlay);
    BlueDialogWatcher dialog_blue(COLOR_RED, &overlay);
    TealDialogWatcher dialog_teal(COLOR_RED, &overlay);
    SelectionArrowWatcher selection_arrow_right(COLOR_RED, &overlay, SelectionArrowType::RIGHT, {0, 0, 1, 1});
    SelectionArrowWatcher selection_arrow_down(COLOR_RED, &overlay, SelectionArrowType::DOWN, {0, 0, 1, 1});
    ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, {0, 0, 1, 1}, &overlay);
    MoveEffectivenessSymbolWatcher battle_menu(COLOR_RED, &overlay);

    CancellableHolder<CancellableScope> scope(*context.scope());
    InferenceSession session(
        scope, env.console,
        {
            dialog_white,
            dialog_blue,
            dialog_teal,
            selection_arrow_right,
            selection_arrow_down,
            buttonA,
            battle_menu,
        }
    );
    context.wait_until_cancel();
}




}
}
}

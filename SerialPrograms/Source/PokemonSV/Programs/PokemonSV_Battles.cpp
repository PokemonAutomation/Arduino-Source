/*  Battles
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV_Battles.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


int run_from_battle(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    console.log("Attempting to run away...");

    int attempts = 0;
    for (size_t c = 0; c < 10; c++){
        OverworldWatcher overworld(COLOR_RED);
        NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
        GradientArrowWatcher next_pokemon(COLOR_GREEN, GradientArrowType::RIGHT, {0.50, 0.51, 0.30, 0.10});
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context, std::chrono::seconds(60),
            {
                overworld,
                battle_menu,
                next_pokemon,
            }
        );

        switch (ret){
        case 0:
            console.log("Detected overworld...");
            return attempts;
        case 1:
            console.log("Detected battle menu...");
            pbf_press_dpad(context, DPAD_DOWN, 250, 0);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_B, 20, 1 * TICKS_PER_SECOND);
            attempts++;
            continue;
        case 2:
            console.log("Detected own " + STRING_POKEMON + " fainted...");
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "Your " + STRING_POKEMON + " fainted while attempting to run away.",
                true
            );
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "run_from_battle(): No state detected after 60 seconds.",
                true
            );
        }
    }

    throw OperationFailedException(
        ErrorReport::SEND_ERROR_REPORT, console,
        "Failed to run away after 10 attempts.",
        true
    );
}


}
}
}

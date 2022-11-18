/*  Tera Battler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV_TeraBattler.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



bool run_tera_battle(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    BotBaseContext& context,
    EventNotificationOption& error_notification
){

    while (true){
        context.wait_for_all_requests();

        BattleMenuFinder battle_menu(COLOR_RED);
        MoveSelectFinder move_select_menu(COLOR_YELLOW);
        GradientArrowFinder target_select_menu(console.overlay(), {0.45, 0.07, 0.10, 0.10}, COLOR_CYAN);
        TeraCatchFinder catch_menu(COLOR_BLUE);
        BlackScreenOverWatcher black_screen(COLOR_GREEN);
        int ret = wait_until(
            console, context,
            std::chrono::seconds(120),
            {
                battle_menu,
                move_select_menu,
                target_select_menu,
                catch_menu,
                black_screen,
            }
        );
        switch (ret){
        case 0:
            env.log("Detected battle menu.");
            pbf_press_button(context, BUTTON_A, 20, 10);
            break;
        case 1:
            env.log("Detected move select.");
            pbf_press_button(context, BUTTON_A, 20, 10);
            break;
        case 2:
            env.log("Detected target select.");
            pbf_press_button(context, BUTTON_A, 20, 10);
            break;
        case 3:
            env.log("Detected a win!", COLOR_BLUE);
            return true;
        case 4:
            env.log("Detected a loss!", COLOR_ORANGE);
            return false;
        default:
            dump_image_and_throw_recoverable_exception(
                env, console, error_notification,
                "NoStateFound",
                "No state detected after 2 minutes."
            );
        }
    }

    return false;
}



}
}
}

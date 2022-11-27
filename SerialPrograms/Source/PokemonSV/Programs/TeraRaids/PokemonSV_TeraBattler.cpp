/*  Tera Battler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
//#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV_TeraBattler.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



bool run_tera_battle(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    BotBaseContext& context,
    EventNotificationOption& error_notification,
    bool from_start,
    bool blindly_terastilize
){
    if (from_start){
        //  Wait for first battle menu.
        {
            BattleMenuWatcher battle_menu(COLOR_RED);
            int ret = wait_until(
                console, context,
                std::chrono::seconds(120),
                {battle_menu}
            );
            if (ret < 0){
                dump_image_and_throw_recoverable_exception(
                    env, console, error_notification,
                    "BattleMenuNotFound",
                    "Unable to detect Tera raid battle menu after 120 seconds."
                );
            }
            env.log("First battle menu found.");
        }
    }

    size_t consecutive_timeouts = 0;
    size_t consecutive_move_select = 0;
    while (true){
        BattleMenuWatcher battle_menu(COLOR_RED);
        MoveSelectWatcher move_select_menu(COLOR_YELLOW);
        GradientArrowWatcher target_select_menu(
            COLOR_CYAN,
            console.overlay(),
            GradientArrowType::DOWN,
            {0.45, 0.07, 0.10, 0.10}
        );
        TeraCatchWatcher catch_menu(COLOR_BLUE);
        OverworldWatcher overworld(COLOR_GREEN);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(120),
            {
                battle_menu,
                move_select_menu,
                target_select_menu,
                catch_menu,
                overworld,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            env.log("Detected battle menu.");
            pbf_press_button(context, BUTTON_A, 20, 10);
            break;
        case 1:
            env.log("Detected move select.");
            consecutive_move_select++;
            //  If we end up here consecutively too many times, the move is
            //  probably disabled. Select a different move.
            if (consecutive_move_select > 3){
                env.log("Failed to select a move 3 times. Choosing a different move.", COLOR_RED);
                pbf_press_dpad(context, DPAD_DOWN, 20, 40);
            }
            if (blindly_terastilize){
                // It might do nothing but display an error message for around 4 seconds
                pbf_press_button(context, BUTTON_R, 20, 4 * TICKS_PER_SECOND);
            }
            pbf_press_button(context, BUTTON_A, 20, 10);
            break;
        case 2:
            env.log("Detected target select.");
            consecutive_move_select = 0;
            pbf_press_button(context, BUTTON_A, 20, 10);
            break;
        case 3:
            env.log("Detected a win!", COLOR_BLUE);
            pbf_mash_button(context, BUTTON_B, 30);
            return true;
        case 4:
            env.log("Detected a loss!", COLOR_ORANGE);
            return false;
        default:
            consecutive_timeouts++;
            if (consecutive_timeouts == 3){
                dump_image_and_throw_recoverable_exception(
                    env, console, error_notification,
                    "NoStateFound",
                    "No state detected after 6 minutes."
                );
            }
            env.log("Unable to detect any state for 2 minutes. Mashing B...", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 250);
        }
    }

    return false;
}









}
}
}

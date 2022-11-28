/*  Tera Exit Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
//#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h"
#include "PokemonSV/Inference/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_BasicCatcher.h"
#include "PokemonSV_TeraRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



bool open_raid(ConsoleHandle& console, BotBaseContext& context){
    TeraCardWatcher card_detector(COLOR_RED);
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            pbf_press_button(context, BUTTON_A, 20, 355);
        },
        {card_detector}
    );
    if (ret < 0){
        console.log("No Tera raid found.", COLOR_ORANGE);
        return false;
    }

    console.log("Tera raid found!", COLOR_BLUE);
    return true;
}
void close_raid(ConsoleHandle& console, BotBaseContext& context){
    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(console, "Failed to return to overworld after 5 minutes.");
        }

        TeraCardWatcher card_detector(COLOR_RED);
        OverworldWatcher overworld(COLOR_CYAN);
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {card_detector, overworld}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 1:
            console.log("Detected overworld.");
            return;
        default:
            throw OperationFailedException(console.logger(), "close_raid(): No recognized state after 60 seconds.");
        }
    }
}



void exit_tera_win_without_catching(
    ConsoleHandle& console,
    BotBaseContext& context
){
    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(console, "Failed to return to overworld after 5 minutes.");
        }

        TeraCatchWatcher catch_menu(COLOR_BLUE);
        WhiteButtonWatcher next_button(
            COLOR_CYAN,
            WhiteButton::ButtonA, 20,
            console.overlay(),
            {0.8, 0.93, 0.2, 0.07}
        );
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        OverworldWatcher overworld(COLOR_RED);
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {catch_menu, next_button, dialog, overworld}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected catch prompt.");
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            pbf_press_button(context, BUTTON_A, 20, 10);
            pbf_mash_button(context, BUTTON_B, 125);
            continue;
        case 1:
            console.log("Detected possible (A) Next button.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 2:
            console.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            break;
        case 3:
            console.log("Detected overworld.");
            return;
        default:
            throw OperationFailedException(console.logger(), "exit_tera_win_without_catching(): No recognized state after 60 seconds.");
        }
    }
}



TeraResult exit_tera_win_by_catching(
    ConsoleHandle& console,
    BotBaseContext& context,
    Language language,
    const std::string& ball_slug,
    bool stop_on_shiny
){
    WallClock start = current_time();
    TeraResult result = TeraResult::NO_DETECTION;
    VideoSnapshot screenshot;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(console, "Failed to return to overworld after 5 minutes.");
        }

        TeraCatchWatcher catch_menu(COLOR_BLUE);
        WhiteButtonWatcher next_button(
            COLOR_CYAN,
            WhiteButton::ButtonA, 20,
            console.overlay(),
            {0.8, 0.93, 0.2, 0.07}
        );
        AdvanceDialogWatcher advance(COLOR_YELLOW);
        PromptDialogWatcher add_to_party(COLOR_GREEN);
        PromptDialogWatcher nickname(COLOR_PURPLE);
        PokemonSummaryWatcher summary(COLOR_MAGENTA);
        MainMenuWatcher main_menu(COLOR_BLUE);
        OverworldWatcher overworld(COLOR_RED);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {
                catch_menu,
                next_button,
                advance,
                add_to_party,
                nickname,
                summary,
                main_menu,
                overworld,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:{
            console.log("Detected catch prompt.");
            screenshot = console.video().snapshot();

            pbf_press_button(context, BUTTON_A, 20, 150);
            context.wait_for_all_requests();

            BattleBallReader reader(console, language);
            int quantity = move_to_ball(reader, console, context, ball_slug);
            if (quantity == 0){
                throw FatalProgramException(console.logger(), "Unable to find appropriate ball. Did you run out?");
            }
            if (quantity < 0){
                console.log("Unable to read ball quantity.", COLOR_RED);
            }
            pbf_mash_button(context, BUTTON_A, 125);

            continue;
        }
        case 2:
            console.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 3:
            console.log("Detected add-to-party prompt.");
            if (result == TeraResult::NO_DETECTION){
                pbf_press_dpad(context, DPAD_DOWN, 20, 40);
                pbf_press_button(context, BUTTON_A, 20, 105);
            }else{
                pbf_press_button(context, BUTTON_B, 20, 105);
            }
            continue;
        case 4:
            console.log("Detected nickname prompt.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 1:
            //  Next button detector is unreliable. Check if the summary is
            //  open. If so, fall-through to that.
            if (!summary.detect(console.video().snapshot())){
                console.log("Detected possible (A) Next button.");
                pbf_press_button(context, BUTTON_A, 20, 105);
                pbf_press_button(context, BUTTON_B, 20, 105);
                break;
            }
            console.log("Detected false positive (A) Next button.", COLOR_RED);
        case 5:
            console.log("Detected summary.");
            if (result == TeraResult::NO_DETECTION){
                context.wait_for(std::chrono::milliseconds(500));
                PokemonSummaryDetector reader;
                VideoSnapshot screen = console.video().snapshot();
                result = reader.is_shiny(screen)
                    ? TeraResult::SHINY
                    : TeraResult::NOT_SHINY;
                if (result == TeraResult::SHINY && stop_on_shiny){
                    return result;
                }
            }
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 6:
            console.log("Detected unexpected main menu.", COLOR_RED);
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 7:
            console.log("Detected overworld.");
            return result;
        default:
            throw OperationFailedException(console.logger(), "exit_tera_win_by_catching(): No recognized state after 60 seconds.");
        }
    }
}



}
}
}

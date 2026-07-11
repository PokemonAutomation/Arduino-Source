/*  Start Game
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/ErrorDumper.h"
//#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MainMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BoxHelpers.h"
#include "PokemonSwSh_MenuNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void navigate_to_menu_app(
    VideoStream& stream,
    ProControllerContext& context,
    size_t target_app_index
){
    context.wait_for_all_requests();
    RotomPhoneMenuArrowWatcher menu_arrow(stream.overlay());
    wait_until(
        stream, context,
        5000ms,
        {menu_arrow}
    );
    const int cur_app_index = menu_arrow.current_index();
    if (cur_app_index < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Cannot detect Rotom phone menu.",
            stream
        );
    }
    stream.log("Detect menu cursor at " + std::to_string(cur_app_index) + ".");

    const int cur_row = cur_app_index / 5;
    const int cur_col = cur_app_index % 5;

    const int target_row = (int)target_app_index / 5;
    const int target_col = (int)target_app_index % 5;

    const DpadPosition dir = (cur_col < target_col ? DPAD_RIGHT : DPAD_LEFT);
    const int steps = std::abs(cur_col - target_col);
    for (int i = 0; i < steps; i++){
        box_scroll(context, dir);
    }

    if (cur_row < target_row){
        box_scroll(context, DPAD_DOWN);
    }else if (cur_row > target_row){
        box_scroll(context, DPAD_UP);
    }

    context.wait_for_all_requests();
    return;
}



void menus_to_mainmenu(VideoStream& stream, ProControllerContext& context){
    WallClock deadline = current_time() + std::chrono::minutes(2);
    do{
        YCommIconWatcher overworld;
        MainMenuWatcher main_menu;
        PartyMenuWatcher party_menu;
        BoxMenuWatcher box_menu;
        context.wait_for_all_requests();

        int ret = wait_until(
            stream, context,
            std::chrono::seconds(30),
            {
                overworld,
                main_menu,
                party_menu,
                box_menu,
            }
        );
        switch (ret){
        case 0:
            stream.log("Detected Overworld...", COLOR_BLUE);
            pbf_press_button(context, BUTTON_X, 160ms, 40ms);
            continue;
        case 1:
            stream.log("Detected Main Menu...", COLOR_BLUE);
            return;
        case 2:
            stream.log("Detected Party Menu...", COLOR_BLUE);
            pbf_press_button(context, BUTTON_B, 160ms, 40ms);
            continue;
        case 3:
            stream.log("Detected Box System...", COLOR_BLUE);
            pbf_press_button(context, BUTTON_B, 160ms, 40ms);
            continue;
        default:
            stream.log("menus_to_mainmenu(): No recognized state after 30 seconds.", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 5000ms);
        }
    }while (current_time() < deadline);

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Unable to reach Main Menu after 2 minutes.",
        stream
    );
}
void menus_to_boxsystem(VideoStream& stream, ProControllerContext& context){
    WallClock deadline = current_time() + std::chrono::minutes(2);
    do{
        YCommIconWatcher overworld;
        MainMenuWatcher main_menu;
        PartyMenuWatcher party_menu;
        BoxMenuWatcher box_menu;
        context.wait_for_all_requests();

        int ret = wait_until(
            stream, context,
            std::chrono::seconds(30),
            {
                overworld,
                main_menu,
                party_menu,
                box_menu,
            }
        );
        context.wait_for(100ms);
        switch (ret){
        case 0:
            stream.log("Detected Overworld...", COLOR_BLUE);
            pbf_press_button(context, BUTTON_X, 160ms, 40ms);
            continue;
        case 1:
            stream.log("Detected Main Menu...", COLOR_BLUE);
            navigate_to_menu_app(stream, context, POKEMON_APP_INDEX);
            pbf_press_button(context, BUTTON_A, 160ms, 40ms);
            continue;
        case 2:
            stream.log("Detected Party Menu...", COLOR_BLUE);
            pbf_press_button(context, BUTTON_R, 160ms, 40ms);
            continue;
        case 3:
            stream.log("Detected Box System...", COLOR_BLUE);
            return;
        default:
            stream.log("menus_to_boxsystem(): No recognized state after 30 seconds.", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 10000ms);
        }
    }while (current_time() < deadline);

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Unable to reach Box System after 2 minutes.",
        stream
    );
}































}
}
}

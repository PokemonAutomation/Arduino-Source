/*  Start Game
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/ErrorDumper.h"
//#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BoxHelpers.h"
#include "PokemonSwSh_MenuNavigation.h"
namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void navigate_to_menu_app(
    ProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context,
    size_t target_app_index,
    EventNotificationOption& notification_option
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
    for(int i = 0; i < steps; i++){
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



}
}
}

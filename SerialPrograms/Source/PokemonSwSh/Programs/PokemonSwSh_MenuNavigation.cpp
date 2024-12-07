/*  Start Game
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/FixedInterval.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh_MenuNavigation.h"
namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void navigate_to_menu_app(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    BotBaseContext& context,
    size_t target_app_index,
    EventNotificationOption& notification_option
){
    context.wait_for_all_requests();
    RotomPhoneMenuArrowFinder menu_arrow_detector(console);
    auto snapshot = console.video().snapshot();
    const int cur_app_index = menu_arrow_detector.detect(snapshot);
    if (cur_app_index < 0){
        OperationFailedException::fire(
            console, ErrorReport::SEND_ERROR_REPORT,
            "Cannot detect Rotom phone menu.",
            std::move(snapshot)
        );
    }
    console.log("Detect menu cursor at " + std::to_string(cur_app_index) + ".");

    const int cur_row = cur_app_index / 5;
    const int cur_col = cur_app_index % 5;

    const int target_row = (int)target_app_index / 5;
    const int target_col = (int)target_app_index % 5;
    
    const uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY;

    const DpadPosition dir = (cur_col < target_col ? DPAD_RIGHT : DPAD_LEFT);
    const int steps = std::abs(cur_col - target_col);
    for(int i = 0; i < steps; i++){
        ssf_press_dpad2(context, dir, BOX_SCROLL_DELAY, 10);
    }

    if (cur_row < target_row){
        ssf_press_dpad2(context, DPAD_DOWN, BOX_SCROLL_DELAY, 10);
    }else if (cur_row > target_row){
        ssf_press_dpad2(context, DPAD_UP, BOX_SCROLL_DELAY, 10);
    }

    context.wait_for_all_requests();
    return;
}



}
}
}

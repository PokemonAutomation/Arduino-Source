/*  Start Game
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "ClientSource/Connection/BotBase.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/FixedInterval.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh_MenuNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void navigate_to_menu_app(ConsoleHandle& console, BotBaseContext& context, size_t target_app_index){
    context.wait_for_all_requests();
    RotomPhoneMenuArrowFinder menu_arrow_detector(console);
    const int cur_app_index = menu_arrow_detector.detect(console.video().snapshot());
    if (cur_app_index < 0){
        throw OperationFailedException(console, "Cannot detect Rotom phone menu.");
    }
    console.log("Detect menu cursor at " + std::to_string(cur_app_index) + ".");

    const int cur_row = cur_app_index / 5;
    const int cur_col = cur_app_index % 5;

    const int target_row = target_app_index / 5;
    const int target_col = target_app_index % 5;
    
    const uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY;

    Button dir = (cur_col < target_col ? DPAD_RIGHT : DPAD_LEFT);
    const int steps = std::abs(cur_col - target_col);
    for(int i = 0; i < steps; i++){
        ssf_press_dpad2(context, dir, BOX_SCROLL_DELAY, 10);
    }

    if (cur_row < target_row){
        ssf_press_dpad2(context, DPAD_DOWN, BOX_SCROLL_DELAY, 10);
    } else if (cur_row > target_row){
        ssf_press_dpad2(context, DPAD_UP, BOX_SCROLL_DELAY, 10);
    }

    context.wait_for_all_requests();
    return;
}



}
}
}

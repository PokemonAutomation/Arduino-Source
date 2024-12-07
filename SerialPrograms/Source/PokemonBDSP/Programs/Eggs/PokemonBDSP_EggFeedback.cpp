/*  Egg Feedback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "CommonFramework/Inference/FrozenImageDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP_EggRoutines.h"
#include "PokemonBDSP_EggFeedback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



void hatch_egg(ConsoleHandle& console, BotBaseContext& context){
    //  Spin until egg starts hatching.
    do{
        ShortDialogWatcher dialog;
        FrozenImageDetector frozen(COLOR_CYAN, {0, 0, 1, 0.5}, std::chrono::seconds(10), 20);
        if (dialog.detect(console.video().snapshot())){
            break;
        }

        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                egg_spin(context, 480 * TICKS_PER_SECOND);
            },
            {
                {dialog},
                {frozen},
            }
        );
        switch (ret){
        case 0:
            console.log("Egg is hatching!");
            break;
        case 1:
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Frozen screen detected!"
            );
        default:
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "No hatch detected after 8 minutes of spinning."
            );
        }
    }while (false);


    //  Hatch the egg.
    VideoSnapshot overworld = console.video().snapshot();
//    overworld.save("test-0.png");
    {
        pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
        context.wait_for_all_requests();

        ShortDialogWatcher dialog;
        int ret = wait_until(
            console, context, std::chrono::seconds(30),
            {{dialog}}
        );
        if (ret < 0){
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "End of hatch not detected after 30 seconds."
            );
        }
        console.log("Egg finished hatching.");
        pbf_mash_button(context, BUTTON_B, 1 * TICKS_PER_SECOND);
    }

    //  Return to overworld.
    while (true){
        context.wait_for_all_requests();

        //  Wait for steady state and read it again.
        context.wait_for(std::chrono::milliseconds(200));
        ImageMatchWatcher matcher(overworld.frame, {0.10, 0.10, 0.80, 0.60}, 100);
        SelectionArrowFinder arrow(console, {0.50, 0.60, 0.30, 0.20}, COLOR_GREEN);
        int ret = wait_until(
            console, context, std::chrono::seconds(30),
            {
                {matcher},
                {arrow},
            }
        );
        switch (ret){
        case 0:
            console.log("Returned to overworld.");
            return;
        case 1:
            throw UserSetupError(console, "Detected prompt. Please turn off nicknaming.");
        default:
            console.log("Failed to detect overworld after 30 seconds. Did day/night change?", COLOR_RED);
//            pbf_mash_button(context, BUTTON_ZL, 30 * TICKS_PER_SECOND);
            return;
        }
    }
}
void hatch_party(ConsoleHandle& console, BotBaseContext& context, size_t eggs){
    for (size_t c = 0; c < eggs; c++){
        hatch_egg(console, context);
    }
}

void withdraw_1st_column_from_overworld(ConsoleHandle& console, BotBaseContext& context){
    const uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY_0;
    const uint16_t BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY;
    overworld_to_box(console, context);
    pbf_press_button(context, BUTTON_Y, 20, 50);
    pbf_press_button(context, BUTTON_Y, 20, 50);
    pickup_column(context);
    pbf_move_right_joystick(context, 0, 128, 20, BOX_SCROLL_DELAY);
    pbf_move_right_joystick(context, 128, 255, 20, BOX_SCROLL_DELAY);
    pbf_press_button(context, BUTTON_ZL, 20, BOX_PICKUP_DROP_DELAY);
    box_to_overworld(console, context);
}



void release(ConsoleHandle& console, BotBaseContext& context){
    pbf_press_button(context, BUTTON_ZL, 20, 50);
    pbf_move_right_joystick(context, 128, 0, 20, 10);
    pbf_move_right_joystick(context, 128, 0, 20, 10);
    pbf_press_button(context, BUTTON_ZL, 20, 105);
    pbf_move_right_joystick(context, 128, 255, 20, 10);

    ShortDialogDetector detector;
    for (size_t c = 0; c < 3; c++){
        context.wait_for_all_requests();
        if (!detector.detect(console.video().snapshot())){
            return;
        }
        pbf_press_button(context, BUTTON_ZL, 20, 105);
    }
    OperationFailedException::fire(
        console, ErrorReport::SEND_ERROR_REPORT,
        "Unexpected dialogs when releasing."
    );
}





}
}
}

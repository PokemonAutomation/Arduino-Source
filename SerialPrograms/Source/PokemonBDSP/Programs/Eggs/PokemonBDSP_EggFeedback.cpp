/*  Egg Feedback
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/FrozenImageDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
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



void hatch_egg(VideoStream& stream, ProControllerContext& context){
    //  Spin until egg starts hatching.
    do{
        ShortDialogWatcher dialog;
        FrozenImageDetector frozen(COLOR_CYAN, {0, 0, 1, 0.5}, std::chrono::seconds(10), 20);
        if (dialog.detect(stream.video().snapshot())){
            break;
        }

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                egg_spin(context, 8min);
            },
            {
                {dialog},
                {frozen},
            }
        );
        switch (ret){
        case 0:
            stream.log("Egg is hatching!");
            break;
        case 1:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Frozen screen detected!",
                stream
            );
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "No hatch detected after 8 minutes of spinning.",
                stream
            );
        }
    }while (false);


    //  Hatch the egg.
    VideoSnapshot overworld = stream.video().snapshot();
//    overworld.save("test-0.png");
    {
        pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
        context.wait_for_all_requests();

        ShortDialogWatcher dialog;
        int ret = wait_until(
            stream, context, std::chrono::seconds(30),
            {{dialog}}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "End of hatch not detected after 30 seconds.",
                stream
            );
        }
        stream.log("Egg finished hatching.");
        pbf_mash_button(context, BUTTON_B, 1 * TICKS_PER_SECOND);
    }

    //  Return to overworld.
    while (true){
        context.wait_for_all_requests();

        //  Wait for steady state and read it again.
        context.wait_for(std::chrono::milliseconds(200));
        ImageMatchWatcher matcher(overworld.frame, {0.10, 0.10, 0.80, 0.60}, 100);
        SelectionArrowFinder arrow(stream.overlay(), {0.50, 0.60, 0.30, 0.20}, COLOR_GREEN);
        int ret = wait_until(
            stream, context, std::chrono::seconds(30),
            {
                {matcher},
                {arrow},
            }
        );
        switch (ret){
        case 0:
            stream.log("Returned to overworld.");
            return;
        case 1:
            throw UserSetupError(stream.logger(), "Detected prompt. Please turn off nicknaming.");
        default:
            stream.log("Failed to detect overworld after 30 seconds. Did day/night change?", COLOR_RED);
//            pbf_mash_button(context, BUTTON_ZL, 30 * TICKS_PER_SECOND);
            return;
        }
    }
}
void hatch_party(VideoStream& stream, ProControllerContext& context, size_t eggs){
    for (size_t c = 0; c < eggs; c++){
        hatch_egg(stream, context);
    }
}

void withdraw_1st_column_from_overworld(VideoStream& stream, ProControllerContext& context){
    const Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;
    const Milliseconds BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY0;
    overworld_to_box(stream, context);
    pbf_press_button(context, BUTTON_Y, 20, 50);
    pbf_press_button(context, BUTTON_Y, 20, 50);
    pickup_column(context);
    pbf_move_right_joystick(context, 0, 128, 160ms, BOX_SCROLL_DELAY);
    pbf_move_right_joystick(context, 128, 255, 160ms, BOX_SCROLL_DELAY);
    pbf_press_button(context, BUTTON_ZL, 160ms, BOX_PICKUP_DROP_DELAY);
    box_to_overworld(stream, context);
}



void release(VideoStream& stream, ProControllerContext& context){
    pbf_press_button(context, BUTTON_ZL, 20, 50);
    pbf_move_right_joystick(context, 128, 0, 20, 30);
    pbf_move_right_joystick(context, 128, 0, 20, 30);
    pbf_press_button(context, BUTTON_ZL, 20, 105);
    pbf_move_right_joystick(context, 128, 255, 20, 30);

    ShortDialogDetector detector;
    for (size_t c = 0; c < 3; c++){
        context.wait_for_all_requests();
        if (!detector.detect(stream.video().snapshot())){
            return;
        }
        pbf_press_button(context, BUTTON_ZL, 20, 105);
    }
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Unexpected dialogs when releasing.",
        stream
    );
}





}
}
}

/*  Box Release
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_BoxRoutines.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV_BoxRelease.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


void release_one_pokemon(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t& errors
){
    bool release_attempted = false;
    bool release_completed = false;
    int expected = 0;
    int consecutive_box_neutrals = 0;
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(60)){
            dump_image_and_throw_recoverable_exception(
                info, stream, "ReleaseFailed",
                "Failed to release " + Pokemon::STRING_POKEMON + " after 1 minute."
            );
        }

        //  The advance dialog is ambiguous against 2 other states. Therefore,
        //  if we see it, we do not decide that is the state unless it holds for
        //  1 second without any of the other states firing.
        SomethingInBoxSlotDetector exists(COLOR_BLUE);
//        GradientArrowDetector change_marks(COLOR_MAGENTA, GradientArrowType::DOWN, {0.28, 0.38, 0.30, 0.10});
        BoxWatcher box_detector(COLOR_RED);
        BoxSelectWatcher selected(COLOR_YELLOW, std::chrono::milliseconds(100));
        PromptDialogWatcher confirm(COLOR_CYAN, std::chrono::milliseconds(100));
        AdvanceDialogWatcher advance_dialog(COLOR_GREEN, DialogType::DIALOG_ALL, std::chrono::milliseconds(1000));

        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(10),
            {
                box_detector,
                selected,
                confirm,
                advance_dialog
            }
        );
//        cout << "ret0 = " << ret << endl;
        context.wait_for(std::chrono::milliseconds(100));
        if (ret == 3){
            //  Make sure we're not mistakening this for the other dialogs.
            auto screenshot = stream.video().snapshot();
            if (selected.detect(screenshot)){
                ret = 1;
            }else if (confirm.detect(screenshot)){
                ret = 2;
            }
        }
//        cout << "ret1 = " << ret << endl;
//        cout << "consecutive_box_neutrals = " << consecutive_box_neutrals << endl;

        switch (ret){
        case 0:{
            auto screenshot = stream.video().snapshot();
#if 0
            //  Disambiguate with mark change.
            if (change_marks.detect(screenshot)){
                stream.log("Detected mark change.", COLOR_RED);
                pbf_press_button(context, BUTTON_B, 20, 20);
                expected = 0;
                continue;
            }
#endif

            if (ret == expected){
                stream.log("Detected box neutral.");
                consecutive_box_neutrals = 0;
            }else{
                //  Disambiguate with mark change.
                stream.log("Detected box neutral. (unexpected)", COLOR_RED);
                errors++;
                consecutive_box_neutrals++;
//                cout << "consecutive_box_neutrals = " << consecutive_box_neutrals << endl;
                if (consecutive_box_neutrals >= 5){
                    pbf_press_button(context, BUTTON_B, 20, 20);
                    expected = 0;
                    continue;
                }
            }

            if (exists.detect(screenshot)){
                if (release_attempted && release_completed){
//                    cout << "return 0" << endl;
                    return;
                }
                release_attempted = false;
                release_completed = false;
                pbf_press_button(context, BUTTON_A, 20, 20);
                expected = 1;
                continue;
            }else{
                stream.log("Slot is empty.");
//                cout << "return 1" << endl;
                return;
            }
        }
        case 1:
            if (ret == expected){
                stream.log("Detected selection. Releasing...");
            }else{
                stream.log("Detected selection. Releasing... (unexpected)", COLOR_RED);
                errors++;
            }
            pbf_press_dpad(context, DPAD_UP, 10, 10);
            pbf_press_dpad(context, DPAD_UP, 10, 10);

            //  Double up this A press since dropping it currently isn't recoverable.
            pbf_press_button(context, BUTTON_A, 5, 3);
            pbf_press_button(context, BUTTON_A, 5, 27);

            expected = 2;
            continue;
        case 2:
            if (ret == expected){
                stream.log("Detected release confirmation.");
            }else{
                stream.log("Detected release confirmation. (unexpected)", COLOR_RED);
                errors++;
            }
            pbf_press_dpad(context, DPAD_UP, 10, 10);
            pbf_press_button(context, BUTTON_A, 20, 20);
            release_attempted = true;
            expected = 3;
            continue;
        case 3:
            if (ret == expected){
                stream.log("Detected advance dialog.");
            }else{
                stream.log("Detected advance dialog. (unexpected)", COLOR_RED);
                errors++;
            }
            pbf_press_dpad(context, DPAD_UP, 10, 10);
            pbf_press_button(context, BUTTON_A, 20, 20);
            release_completed = true;
            expected = 0;
            continue;
        default:
            dump_image_and_throw_recoverable_exception(
                info, stream,
                "NoStateReleasingPokemon",
                "No recognized state while releasing a pokemon after 10 seconds."
            );
        }
    }
}

void release_box(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t& errors, uint8_t start_row
){
    context.wait_for_all_requests();
    stream.log("Release one box.");
    stream.overlay().add_log("Release box", COLOR_WHITE);
    for (uint8_t row = start_row; row < 5; row++){
        for (uint8_t j_col = 0; j_col < 6; j_col++){
            // Go through slots in a Z-shape pattern
            uint8_t col = (row % 2 == 0 ? j_col : 5 - j_col);
            move_box_cursor(info, stream, context, BoxCursorLocation::SLOTS, row, col);
            release_one_pokemon(info, stream, context, errors);
        }
    }
}




}
}
}

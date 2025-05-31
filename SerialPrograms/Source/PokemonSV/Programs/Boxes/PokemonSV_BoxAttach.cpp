/*  Box Attach
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "Pokemon/Pokemon_Strings.h"
//#include "PokemonSV_BoxRoutines.h"
#include "PokemonSV/Inference/PokemonSV_BagDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV_BoxAttach.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



//  With the cursor over the item you want to attach, attach to the current
//  Pokemon, replacing if necessary.
void attach_item_from_bag(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t& errors
){
//    bool attach_attempted = false;
    bool attach_completed = false;

    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(60)){
            dump_image_and_throw_recoverable_exception(
                info, stream, "AttachFailed",
                "Failed to attach item after 1 minute."
            );
        }

        BagWatcher bag_detector(BagWatcher::FinderType::GONE, COLOR_GREEN);
        GradientArrowWatcher bag_neutral(COLOR_RED, GradientArrowType::RIGHT, {0.10, 0.15, 0.05, 0.77}, std::chrono::milliseconds(1000));
        GradientArrowWatcher selected(COLOR_RED, GradientArrowType::RIGHT, {0.20, 0.20, 0.30, 0.60});
        PromptDialogWatcher prompt(COLOR_CYAN, std::chrono::milliseconds(100));
        AdvanceDialogWatcher advance_dialog(COLOR_YELLOW, DialogType::DIALOG_ALL, std::chrono::milliseconds(100));

        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(10),
            {
                bag_detector,
                bag_neutral,
                selected,
                prompt,
                advance_dialog,
            }
        );
        context.wait_for(std::chrono::milliseconds(50));
        if (ret == 4){
            //  Make sure we're not mistakening this for the other dialogs.
            auto screenshot = stream.video().snapshot();
            if (prompt.detect(screenshot)){
                ret = 3;
            }
        }


        switch (ret){
        case 0:
            stream.log("No longer in bag...");
            if (!attach_completed){
                errors++;
            }
            return;
        case 1:
            stream.log("Detected bag neutral...");
            if (attach_completed){
                pbf_press_button(context, BUTTON_B, 20, 30);
            }else{
                pbf_press_button(context, BUTTON_A, 20, 30);
            }
            break;
        case 2:
            stream.log("Detected selection...");
            pbf_press_button(context, BUTTON_A, 20, 30);
//            attach_completed = true;
            break;
        case 3:
            stream.log("Detected prompt...");
            pbf_press_button(context, BUTTON_A, 20, 30);
            break;
        case 4:
            stream.log("Detected dialog...");
            pbf_press_button(context, BUTTON_B, 20, 30);
            attach_completed = true;
            break;
        }
    }
}



void attach_item_from_box(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t category_index,
    size_t& errors
){
    bool attach_attempted = false;
//    bool attach_completed = false;
    int expected = 0;
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(60)){
            dump_image_and_throw_recoverable_exception(
                info, stream, "AttachFailed",
                "Failed to attach item after 1 minute."
            );
        }

        SomethingInBoxSlotDetector exists(COLOR_BLUE);
        BoxWatcher box_detector(COLOR_RED);
        PromptDialogWatcher selected_empty(COLOR_CYAN, {0.60, 0.57, 0.30, 0.09}, std::chrono::milliseconds(100));
        PromptDialogWatcher selected_held(COLOR_CYAN, {0.60, 0.46, 0.30, 0.09}, std::chrono::milliseconds(100));
        BagWatcher bag_detector(BagWatcher::FinderType::PRESENT, COLOR_GREEN);
//        AdvanceDialogWatcher advance_dialog(COLOR_YELLOW, std::chrono::milliseconds(250));

        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(10),
            {
                box_detector,
                selected_empty,
                selected_held,
                bag_detector,
//                advance_dialog,
            }
        );
        context.wait_for(std::chrono::milliseconds(50));
#if 0
        if (ret == 4){
            //  Make sure we're not mistakening this for the other dialogs.
            auto screenshot = stream.video().snapshot();
            if (selected_empty.detect(screenshot)){
                ret = 1;
            }else if (selected_held.detect(screenshot)){
                ret = 2;
            }
        }
#endif

        switch (ret){
        case 0:{
            if (ret == expected){
                stream.log("Detected box neutral.");
            }else{
                stream.log("Detected box neutral. (unexpected)", COLOR_RED);
                errors++;
            }

            auto screenshot = stream.video().snapshot();
            if (exists.detect(screenshot)){
                if (attach_attempted){
                    return;
                }
                attach_attempted = false;
                pbf_press_button(context, BUTTON_A, 20, 20);
                expected = 1;
                continue;
            }else{
                stream.log("Slot is empty.");
                return;
            }
        }
        case 1:
            if (ret == expected){
                stream.log("Detected empty selection. Attaching...");
            }else{
                stream.log("Detected empty selection. Attaching... (unexpected)", COLOR_RED);
                errors++;
            }

            pbf_press_button(context, BUTTON_A, 20, 30);

            expected = 3;
            continue;

        case 2:
            if (1 == expected){
                stream.log("Detected held selection. Replacing...");
            }else{
                stream.log("Detected held selection. Replacing... (unexpected)", COLOR_RED);
                errors++;
            }

            pbf_press_dpad(context, DPAD_DOWN, 20, 20);
            pbf_press_button(context, BUTTON_A, 20, 30);

            expected = 3;
            continue;

        case 3:
            if (ret == expected){
                stream.log("Detected bag.");
            }else{
                stream.log("Detected bag.(unexpected)", COLOR_RED);
                errors++;
            }

            context.wait_for(std::chrono::milliseconds(200));
            if (attach_attempted){
                pbf_press_button(context, BUTTON_B, 20, 30);
                expected = 0;
                continue;
            }

            for (size_t c = 0; c < category_index; c++){
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
            }

            attach_item_from_bag(info, stream, context, errors);
            attach_attempted = true;

            expected = 0;
            continue;

        default:
            dump_image_and_throw_recoverable_exception(
                info, stream, "NoStateAttachingItem", "No recognized state while attaching item after 10 seconds."
            );
        }
    }
}




}
}
}

/*  Item Printer Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterJobsDetector.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterPrizeReader.h"
#include "PokemonSV_ItemPrinterTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


const EnumDropdownDatabase<ItemPrinterJobs>& ItemPrinterJobs_Database(){
    static const EnumDropdownDatabase<ItemPrinterJobs> database({
        {ItemPrinterJobs::Jobs_1, "1", "1 Job"},
        {ItemPrinterJobs::Jobs_5, "5", "5 Jobs"},
        {ItemPrinterJobs::Jobs_10, "10", "10 Jobs"},
    });
    return database;
}



void item_printer_start_print(
    VideoStream& stream, ProControllerContext& context,
    Language language, ItemPrinterJobs jobs
){
    stream.log("Starting print...");

    while (true){
        PromptDialogWatcher  prompt(COLOR_YELLOW);
        WhiteButtonWatcher   material(COLOR_GREEN, WhiteButton::ButtonX, {0.63, 0.93, 0.17, 0.06});
        WhiteButtonWatcher   handle(COLOR_BLUE,    WhiteButton::ButtonA, {0.40, 0.80, 0.20, 0.14});
        context.wait_for_all_requests();

        int ret_print_start = wait_until(
            stream, context,
            std::chrono::seconds(120),
            { handle, prompt, material }
        );
        context.wait_for_all_requests();

        switch (ret_print_start){
        case 0: // handle
            return;
        case 1: // prompt
            stream.log("Confirming material selection...");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 2:{    // material
            ItemPrinterJobsDetector detector(COLOR_RED);
            VideoOverlaySet overlays(stream.overlay());
            detector.make_overlays(overlays);
            detector.set_print_jobs(stream, context, (uint8_t)jobs);
            pbf_press_button(context, BUTTON_X, 20, 230);
            continue;
        }
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "start_print(): No recognized state after 120 seconds.",
                stream
            );
        }
    }
}
ItemPrinterPrizeResult item_printer_finish_print(
    VideoStream& stream, ProControllerContext& context,
    Language language
){
    stream.log("Finishing print...");
    bool print_finished = false;

    ItemPrinterPrizeResult prize_result;
    while (true){
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        WhiteButtonWatcher   material(COLOR_GREEN, WhiteButton::ButtonX, {0.63, 0.93, 0.17, 0.06});
        WhiteButtonWatcher   handle(COLOR_BLUE,    WhiteButton::ButtonA, {0.40, 0.80, 0.20, 0.14});
        WhiteButtonWatcher   result(COLOR_CYAN,    WhiteButton::ButtonA, {0.87, 0.93, 0.10, 0.06});
        context.wait_for_all_requests();

        int ret_print_end = wait_until(
            stream, context,
            std::chrono::seconds(120),
            { material, handle, dialog, result }
        );
        context.wait_for_all_requests();

        switch (ret_print_end){
        case 0: // material
            stream.log("Material selection screen detected.");
            return prize_result;
        case 1: // handle
        case 2: // dialog
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 3:{    // result
            stream.log("Result screen detected.");
            if (print_finished){
                continue;
            }

            if (language != Language::None){
                ItemPrinterPrizeReader reader(language);
                VideoOverlaySet overlays(stream.overlay());
                reader.make_overlays(overlays);
                auto snapshot = stream.video().snapshot();
                std::array<std::string, 10> prizes = reader.read_prizes(stream.logger(), snapshot);
                std::array<int16_t, 10> quantities = reader.read_quantity(stream.logger(), snapshot);
                prize_result = {prizes, quantities};
//                static int c = 0;
//                snapshot->save("test-" + std::to_string(c) + ".png");
            }

            pbf_mash_button(context, BUTTON_A, 1 * TICKS_PER_SECOND);
            print_finished = true;
            continue;
        }
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "finish_print(): No recognized state after 120 seconds.",
                stream
            );
        }
    }
}


}
}
}

/*  Item Printer Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterJobsDetector.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterPrizeReader.h"
#include "PokemonSV_ItemPrinterTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


const EnumDatabase<ItemPrinterJobs>& ItemPrinterJobs_Database(){
    static const EnumDatabase<ItemPrinterJobs> database({
        {ItemPrinterJobs::Jobs_1, "1", "1 Job"},
        {ItemPrinterJobs::Jobs_5, "5", "5 Jobs"},
        {ItemPrinterJobs::Jobs_10, "10", "10 Jobs"},
    });
    return database;
}



void item_printer_start_print(
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console, BotBaseContext& context,
    Language language, ItemPrinterJobs jobs
){
    console.log("Starting print...");

    while (true){
        PromptDialogWatcher  prompt(COLOR_YELLOW);
        WhiteButtonWatcher   material(COLOR_GREEN, WhiteButton::ButtonX, {0.63, 0.93, 0.17, 0.06});
        WhiteButtonWatcher   handle(COLOR_BLUE,    WhiteButton::ButtonA, {0.40, 0.80, 0.20, 0.14});
        context.wait_for_all_requests();

        int ret_print_start = wait_until(
            console, context,
            std::chrono::seconds(120),
            { handle, prompt, material }
        );
        context.wait_for_all_requests();

        switch (ret_print_start){
        case 0: // handle
            return;
        case 1: // prompt
            console.log("Confirming material selection...");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 2:{    // material
            ItemPrinterJobsDetector detector(COLOR_RED);
            VideoOverlaySet overlays(console.overlay());
            detector.make_overlays(overlays);
            detector.set_print_jobs(dispatcher, console, context, (uint8_t)jobs);
            pbf_press_button(context, BUTTON_X, 20, 230);
            continue;
        }
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "start_print(): No recognized state after 120 seconds.",
                true
            );
        }
    }
}
std::array<std::string, 10> item_printer_finish_print(
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console, BotBaseContext& context,
    Language language
){
    console.log("Finishing print...");
    bool print_finished = false;

    std::array<std::string, 10> ret;
    while (true){
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        WhiteButtonWatcher   material(COLOR_GREEN, WhiteButton::ButtonX, {0.63, 0.93, 0.17, 0.06});
        WhiteButtonWatcher   handle(COLOR_BLUE,    WhiteButton::ButtonA, {0.40, 0.80, 0.20, 0.14});
        WhiteButtonWatcher   result(COLOR_CYAN,    WhiteButton::ButtonA, {0.87, 0.93, 0.10, 0.06});
        context.wait_for_all_requests();

        int ret_print_end = wait_until(
            console, context,
            std::chrono::seconds(120),
            { material, handle, dialog, result }
        );
        context.wait_for_all_requests();

        switch (ret_print_end){
        case 0: // material
            console.log("Material selection screen detected.");
            return ret;
        case 1: // handle
        case 2: // dialog
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 3:{    // result
            console.log("Result screen detected.");
            if (print_finished){
                continue;
            }

            if (language != Language::None){
                ItemPrinterPrizeReader reader(language);
                VideoOverlaySet overlays(console.overlay());
                reader.make_overlays(overlays);
                auto snapshot = console.video().snapshot();
                ret = reader.read_prizes(console.logger(), dispatcher, snapshot);
//                static int c = 0;
//                snapshot->save("test-" + std::to_string(c) + ".png");
            }

            pbf_mash_button(context, BUTTON_A, 1 * TICKS_PER_SECOND);
            print_finished = true;
            continue;
        }
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "finish_print(): No recognized state after 120 seconds.",
                true
            );
        }
    }
}


}
}
}

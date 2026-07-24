/*  Box Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSwSh_BoxHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



bool change_view_to_stats_or_judge(
    VideoStream& stream, ProControllerContext& context,
    bool throw_exception
){
    ImageFloatBox name_bar(0.69, 0.03, 0.30, 0.05);
    OverlayBoxScope name_bar_overlay(stream.overlay(), name_bar);
    for (size_t attempts = 0;; attempts++){
        if (throw_exception){
            if (attempts == 10){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Unable to change Pokemon view after 10 tries.",
                    stream
                );
            }
        }else{
            if (attempts == 3){
                return false;
            }
        }

        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();
        ImageStats stats = image_stats(extract_box_reference(screen, name_bar));
//        cout << stats.stddev << endl;
        if (stats.stddev.sum() > 50){
            break;
        }

        stream.log("Unable to detect stats menu. Attempting to correct.", COLOR_RED);

//        //  Alternate one and two + presses. If IV checker is enabled, we should
//        //  land on the IV checker. Otherwise, it will land us back to nothing.
//        //  Then the next press will be a single which will put us on the stats
//        //  with no IV checker.
        pbf_press_button(context, BUTTON_PLUS, 160ms, 840ms);
//        if (attempts % 2 == 0){
//            pbf_press_button(context, BUTTON_PLUS, 160ms, 1840ms);
//        }
    }
    return true;
}


void change_view_to_judge(
    VideoStream& stream, ProControllerContext& context,
    Language language
){
    if (language == Language::None){
        throw InternalProgramError(
            &stream.logger(), PA_CURRENT_FUNCTION,
            "change_view_to_judge() called with no language."
        );
    }

    ImageFloatBox name_bar(0.69, 0.03, 0.30, 0.05);
    IvJudgeReaderScope iv_checker(stream.overlay(), language);
    OverlayBoxScope name_bar_overlay(stream.overlay(), name_bar);
    for (size_t attempts = 0;; attempts++){
        if (attempts == 10){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to change Pokemon view to judge after 10 tries. Have you unlocked it?",
                stream
            );
        }

        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();
        ImageStats stats = image_stats(extract_box_reference(screen, name_bar));
//        cout << stats.stddev << endl;

        //  Check if we're even on a stats screen.
        if (stats.stddev.sum() < 50){
            stream.log("Unable to detect stats menu. Attempting to correct.", COLOR_RED);
            pbf_press_button(context, BUTTON_PLUS, 160ms, 840ms);
            continue;
        }

        //  See if we're on the judge screen.
        IvJudgeReader::Results ivs = iv_checker.read(stream.logger(), screen);

        size_t detected = 0;
        if (ivs.hp      != IvJudgeValue::UnableToDetect) detected++;
        if (ivs.attack  != IvJudgeValue::UnableToDetect) detected++;
        if (ivs.defense != IvJudgeValue::UnableToDetect) detected++;
        if (ivs.spatk   != IvJudgeValue::UnableToDetect) detected++;
        if (ivs.spdef   != IvJudgeValue::UnableToDetect) detected++;
        if (ivs.speed   != IvJudgeValue::UnableToDetect) detected++;

        //  If less than 4 of the IVs are read, assume we're not on the judge screen.
        if (detected < 4){
            pbf_press_button(context, BUTTON_PLUS, 160ms, 1840ms);
        }else{
            break;
        }
    }
}





}
}
}

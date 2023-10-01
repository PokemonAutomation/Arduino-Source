/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
//#include "Pokemon/Options/Pokemon_StatsResetFilter.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxNatureDetector.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


bool check_stats_reset_info(
    ConsoleHandle& console, BotBaseContext& context,
    OCR::LanguageOCROption& LANGUAGE, Pokemon::StatsHuntFilterTable& FILTERS,
    Pokemon::StatsHuntAction& action
){
    context.wait_for_all_requests();

    Language language = LANGUAGE;

    change_view_to_judge(console, context, language);

    VideoOverlaySet overlay_set(console.overlay());
    IvJudgeReaderScope iv_reader_scope(console.overlay(), language);
    VideoSnapshot screen = console.video().snapshot();
    IvJudgeReader::Results IVs = iv_reader_scope.read(console.logger(), screen);
    BoxNatureDetector nature_detector(console.overlay(), LANGUAGE);
    NatureReader::Results nature = nature_detector.read(console.logger(), screen);
    bool shiny = false;

    console.log(IVs.to_string(), COLOR_GREEN);
    action = FILTERS.get_action(shiny, IVs, StatsHuntGenderFilter::Any, nature);

    return shiny;
}



}
}
}

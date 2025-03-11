/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxNatureDetector.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV_StatsResetChecker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


bool check_stats_reset_info(
    VideoStream& stream, ProControllerContext& context,
    OCR::LanguageOCROption& LANGUAGE, Pokemon::StatsHuntIvJudgeFilterTable& FILTERS,
    Pokemon::StatsHuntAction& action
){
    context.wait_for_all_requests();

    Language language = LANGUAGE;

    change_view_to_judge(stream, context, language);

    VideoOverlaySet overlay_set(stream.overlay());
    IvJudgeReaderScope iv_reader_scope(stream.overlay(), language);
    VideoSnapshot screen = stream.video().snapshot();
    IvJudgeReader::Results IVs = iv_reader_scope.read(stream.logger(), screen);
    BoxNatureDetector nature_detector(stream.overlay(), LANGUAGE);
    NatureReader::Results nature = nature_detector.read(stream.logger(), screen);
    bool shiny = false;

    stream.log(IVs.to_string(), COLOR_GREEN);
    action = FILTERS.get_action(shiny, StatsHuntGenderFilter::Any, nature.nature, IVs);

    return shiny;
}



}
}
}

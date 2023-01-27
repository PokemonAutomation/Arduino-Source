/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Options/Pokemon_EggHatchFilter.h"
#include "Pokemon/Options/Pokemon_StatsResetFilter.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IVCheckerReader.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV_StatsResetChecker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


bool check_stats_reset_info(
    ConsoleHandle& console, BotBaseContext& context,
    OCR::LanguageOCROption& LANGUAGE, Pokemon::StatsResetFilterTable& FILTERS,
    Pokemon::EggHatchAction& action
){
    context.wait_for_all_requests();

    Language language = LANGUAGE;

    change_view_to_judge(console, context, language);

    VideoOverlaySet overlay_set(console.overlay());
    IVCheckerReaderScope iv_reader_scope(console.overlay(), language);
    VideoSnapshot screen = console.video().snapshot();
    IVCheckerReader::Results IVs = iv_reader_scope.read(console.logger(), screen);
    const bool shiny = 0;

    console.log(IVs.to_string(), COLOR_GREEN);
    action = FILTERS.get_action(shiny, IVs);

    return shiny;
}



}
}
}

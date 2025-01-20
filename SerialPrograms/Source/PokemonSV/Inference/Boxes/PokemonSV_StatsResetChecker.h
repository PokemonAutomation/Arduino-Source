/*  Stats Reset Checker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_StatsResetChecker_H
#define PokemonAutomation_PokemonSV_StatsResetChecker_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    struct ProgramInfo;
    class ImageViewRGB32;

namespace OCR{
    class LanguageOCROption;
}
namespace Pokemon{
    enum class StatsHuntAction;
class StatsHuntIvJudgeFilterTable;
}

namespace NintendoSwitch{
namespace PokemonSV{



//This is a similar to check_baby_info, except for Stats Reset
bool check_stats_reset_info(
    VideoStream& stream, SwitchControllerContext& context,
    OCR::LanguageOCROption& LANGUAGE, Pokemon::StatsHuntIvJudgeFilterTable& FILTERS,
    Pokemon::StatsHuntAction& action
);




}
}
}
#endif

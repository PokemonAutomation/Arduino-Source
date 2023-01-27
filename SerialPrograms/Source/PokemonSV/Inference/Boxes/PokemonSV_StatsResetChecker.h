/*  Stats Reset Checker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_StatsResetChecker_H
#define PokemonAutomation_PokemonSV_StatsResetChecker_H

#include <functional>

namespace PokemonAutomation{
    struct ProgramInfo;
    class ConsoleHandle;
    class BotBaseContext;
    class ImageViewRGB32;

namespace OCR{
    class LanguageOCROption;
}
namespace Pokemon{
    enum class EggHatchAction;
    class EggHatchFilterTable;
    class StatsResetFilterTable;
}

namespace NintendoSwitch{
namespace PokemonSV{



//This is a similar to check_baby_info, except for Stats Reset
bool check_stats_reset_info(
    ConsoleHandle& console, BotBaseContext& context,
    OCR::LanguageOCROption& LANGUAGE, Pokemon::StatsResetFilterTable& FILTERS,
    Pokemon::EggHatchAction& action
);




}
}
}
#endif

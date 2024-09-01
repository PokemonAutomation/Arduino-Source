/*  Menu Option Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_MenuOptionReader_H
#define PokemonAutomation_PokemonSV_MenuOptionReader_H

#include <map>
#include <array>
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/OCR/OCR_SmallDictionaryMatcher.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_MenuOptionDatabase.h"

namespace PokemonAutomation{
    class AsyncDispatcher;
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSV{

class MenuOptionReader : public OCR::SmallDictionaryMatcher{
    static constexpr double MAX_LOG10P = -1.30;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    MenuOptionReader();

    static MenuOptionReader& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;
};    



}
}
}
#endif

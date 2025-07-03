/*  Pokemon Moves Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_PokemonMovesReader_H
#define PokemonAutomation_PokemonSV_PokemonMovesReader_H

#include <array>
#include "CommonFramework/Language.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class PokemonMovesOCR : public OCR::SmallDictionaryMatcher{
    static constexpr double MAX_LOG10P = -1.30;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    PokemonMovesOCR();

    static PokemonMovesOCR& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;

    
};    

class PokemonMovesReader{

public:
    PokemonMovesReader(Language language);

    void make_overlays(VideoOverlaySet& items) const;

    std::string read_move(Logger& logger, const ImageViewRGB32& screen, int8_t index) const;

private:
    Language m_language;
    std::array<ImageFloatBox, 4> m_boxes_rearrange;
};



}
}
}
#endif

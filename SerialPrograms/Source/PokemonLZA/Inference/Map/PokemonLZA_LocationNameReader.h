/*  Location Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_LocationNameReader_H
#define PokemonAutomation_Pokemon_LocationNameReader_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class LocationNameOCR : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.40;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    LocationNameOCR();

    static LocationNameOCR& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
        ) const;
};

class LocationNameReader{
public:
    LocationNameReader(Color color = COLOR_WHITE);
    static constexpr size_t PAGE_SIZE = 7;

    void make_overlays(VideoOverlaySet& items) const;

    // Read a single location display name via OCR given by the index
    OCR::StringMatchResult read_location_name(
        const ImageViewRGB32& screen, 
        Logger& logger, 
        Language language, 
        size_t index
    ) const;

private:
    Color m_color;
    // Store the boxes that contain each location name in the fast travel menu
    std::array<ImageFloatBox, PAGE_SIZE> m_box_location_name;
    // Get the location of all location boxes based on the initial box location
    std::array<ImageFloatBox, PAGE_SIZE> get_all_box_locations(ImageFloatBox initial_box);
};

}
}
}
#endif

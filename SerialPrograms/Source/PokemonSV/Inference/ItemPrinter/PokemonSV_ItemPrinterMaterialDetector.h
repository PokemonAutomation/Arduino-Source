/*  Item Printer Jobs Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterMaterialDetector_H
#define PokemonAutomation_PokemonSV_ItemPrinterMaterialDetector_H

#include <array>
#include "Common/Cpp/Color.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"
#include "CommonTools/VisualDetector.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{


class MaterialNameReader : public OCR::SmallDictionaryMatcher{
    static constexpr double MAX_LOG10P = -1.40;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    MaterialNameReader();

    static MaterialNameReader& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;
};    

class ItemPrinterMaterialDetector{
public:
    ItemPrinterMaterialDetector(Color color, Language language);

    void make_overlays(VideoOverlaySet& items) const;

    std::array<ImageFloatBox, 10> Material_Boxes(ImageFloatBox initial_box);

    int8_t find_happiny_dust_row_index(
        VideoStream& stream, ProControllerContext& context
    ) const;

    std::vector<int8_t> find_material_value_row_index(
        VideoStream& stream,
        ProControllerContext& context,
        int16_t material_value
    ) const;

    // detect the quantity of material at the given row number
    int16_t detect_material_quantity(
        VideoStream& stream,
        const ImageViewRGB32& screen,
        ProControllerContext& context,
        int8_t row_index
    ) const;

    // detects the material name at the given row_index, given the screen
    std::string detect_material_name(
        VideoStream& stream,
        const ImageViewRGB32& screen,
        ProControllerContext& context,
        int8_t row_index
    ) const;    

    int16_t read_number(
        Logger& logger,
        const ImageViewRGB32& screen, const ImageFloatBox& box,
        int8_t row_index
    ) const;


private:
    Color m_color;
    Language m_language;
    std::array<ImageFloatBox, 10> m_box_mat_value;    // {0.39,0.176758,0.025,0.05}, {0.39,0.250977,0.025,0.05}, {0.39,0.325196,0.025,0.05}, {0.39,0.399415,0.025,0.05}, {0.39,0.473634,0.025,0.05}, {0.39,0.547853,0.025,0.05}, {0.39,0.622072,0.025,0.05}, {0.39,0.696291,0.025,0.05}, {0.39,0.77051,0.025,0.05}, {0.39,0.844729,0.025,0.05}, 
    std::array<ImageFloatBox, 10> m_box_mat_quantity; // {0.485,0.176758,0.037,0.05}, {0.485,0.250977,0.037,0.05}, {0.485,0.325196,0.037,0.05}, {0.485,0.399415,0.037,0.05}, {0.485,0.473634,0.037,0.05}, {0.485,0.547853,0.037,0.05}, {0.485,0.622072,0.037,0.05}, {0.485,0.696291,0.037,0.05}, {0.485,0.77051,0.037,0.05}, {0.485,0.844729,0.037,0.05}, 
    std::array<ImageFloatBox, 10> m_box_mat_name;     // {0.09,0.176758,0.275,0.05}, {0.09,0.250977,0.275,0.05}, {0.09,0.325196,0.275,0.05}, {0.09,0.399415,0.275,0.05}, {0.09,0.473634,0.275,0.05}, {0.09,0.547853,0.275,0.05}, {0.09,0.622072,0.275,0.05}, {0.09,0.696291,0.275,0.05}, {0.09,0.77051,0.275,0.05}, {0.09,0.844729,0.275,0.05}, 

    
};



}
}
}
#endif

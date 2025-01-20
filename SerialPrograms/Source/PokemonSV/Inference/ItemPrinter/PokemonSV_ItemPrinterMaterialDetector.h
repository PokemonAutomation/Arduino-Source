/*  Item Printer Jobs Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    class Logger;
    class AsyncDispatcher;
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
        AsyncDispatcher& dispatcher,
        VideoStream& stream, SwitchControllerContext& context
    ) const;

    std::vector<int8_t> find_material_value_row_index(
        AsyncDispatcher& dispatcher,
        VideoStream& stream,
        SwitchControllerContext& context,
        int16_t material_value
    ) const;

    int16_t detect_material_quantity(
        AsyncDispatcher& dispatcher,
        VideoStream& stream,
        SwitchControllerContext& context,
        int8_t row_index
    ) const;

    std::string detect_material_name(
        VideoStream& stream,
        SwitchControllerContext& context,
        int8_t row_index
    ) const;    


private:
    int16_t read_number(
        Logger& logger, AsyncDispatcher& dispatcher,
        const ImageViewRGB32& screen, const ImageFloatBox& box
    ) const;


private:
    Color m_color;
    Language m_language;
    std::array<ImageFloatBox, 10> m_box_mat_value;
    std::array<ImageFloatBox, 10> m_box_mat_quantity;
    std::array<ImageFloatBox, 10> m_box_mat_name;
};



}
}
}
#endif

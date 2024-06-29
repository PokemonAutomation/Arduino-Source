/*  Item Printer Jobs Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterMaterialDetector_H
#define PokemonAutomation_PokemonSV_ItemPrinterMaterialDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/OCR/OCR_SmallDictionaryMatcher.h"

namespace PokemonAutomation{
    class Logger;
    class AsyncDispatcher;
    class ConsoleHandle;
    class BotBaseContext;
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

    int8_t find_happiny_dust_row_num(
        AsyncDispatcher& dispatcher,
        ConsoleHandle& console, BotBaseContext& context
    ) const;

    int8_t find_material_value_row_num(
        AsyncDispatcher& dispatcher,
        ConsoleHandle& console, 
        BotBaseContext& context,
        int16_t material_value
    ) const;

    int16_t detect_material_quantity(
        AsyncDispatcher& dispatcher,
        ConsoleHandle& console, 
        BotBaseContext& context,
        int8_t row_num
    ) const;

    std::string detect_material_name(
        ConsoleHandle& console, 
        BotBaseContext& context,
        int8_t row_num
    ) const;    


private:
    int16_t read_number(
        Logger& logger, AsyncDispatcher& dispatcher,
        const ImageViewRGB32& screen, const ImageFloatBox& box
    ) const;

    int16_t read_number_black_or_white_text(
        Logger& logger, AsyncDispatcher& dispatcher,
        const ImageViewRGB32& screen, const ImageFloatBox& box,
        bool is_white_text
    ) const;    

    ImageFloatBox get_material_quantity_box(int8_t row_num) const;

    ImageFloatBox get_material_value_box(int8_t row_num) const;

    ImageFloatBox get_material_name_box(int8_t row_num) const;

private:
    Color m_color;
    Language m_language;
    ImageFloatBox m_box_1_mat_value;
    ImageFloatBox m_box_1_mat_quantity;
    ImageFloatBox m_box_1_mat_name;
    ImageFloatBox m_box_2_mat_value;
    ImageFloatBox m_box_2_mat_quantity;
    ImageFloatBox m_box_2_mat_name;
    ImageFloatBox m_box_3_mat_value;
    ImageFloatBox m_box_3_mat_quantity;
    ImageFloatBox m_box_3_mat_name;
    ImageFloatBox m_box_4_mat_value;
    ImageFloatBox m_box_4_mat_quantity;
    ImageFloatBox m_box_4_mat_name;
    ImageFloatBox m_box_5_mat_value;
    ImageFloatBox m_box_5_mat_quantity;
    ImageFloatBox m_box_5_mat_name;
    ImageFloatBox m_box_6_mat_value;
    ImageFloatBox m_box_6_mat_quantity;
    ImageFloatBox m_box_6_mat_name;
    ImageFloatBox m_box_7_mat_value;
    ImageFloatBox m_box_7_mat_quantity;
    ImageFloatBox m_box_7_mat_name;
    ImageFloatBox m_box_8_mat_value;
    ImageFloatBox m_box_8_mat_quantity;
    ImageFloatBox m_box_8_mat_name;
    ImageFloatBox m_box_9_mat_value;
    ImageFloatBox m_box_9_mat_quantity;
    ImageFloatBox m_box_9_mat_name;
    ImageFloatBox m_box_10_mat_value;
    ImageFloatBox m_box_10_mat_quantity;
    ImageFloatBox m_box_10_mat_name;
};



}
}
}
#endif

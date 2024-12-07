/*  Tournament Prize Jobs Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSV_ItemPrinterMaterialDetector.h"
#include <iostream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


MaterialNameReader& MaterialNameReader::instance(){
    static MaterialNameReader reader;
    return reader;
}

// MaterialNameReader has a very limited dictionary,
// so it can only reliably read the material names with 68% value
// (i.e. Ditto Goo, Happiny Dust, Magby Hair, Beldum Claw)
MaterialNameReader::MaterialNameReader()
    : SmallDictionaryMatcher("PokemonSV/ItemPrinterMaterialOCR.json")
{}

OCR::StringMatchResult MaterialNameReader::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image, text_color_ranges,
        MAX_LOG10P, MAX_LOG10P_SPREAD, min_text_ratio, max_text_ratio
    );
}


ItemPrinterMaterialDetector::ItemPrinterMaterialDetector(Color color, Language language)
    : m_color(color)
    , m_language(language)
    , m_box_mat_value(Material_Boxes(ImageFloatBox(0.39, 0.176758, 0.025, 0.050)))
    , m_box_mat_quantity(Material_Boxes(ImageFloatBox(0.485, 0.176758, 0.037, 0.050)))
    , m_box_mat_name(Material_Boxes(ImageFloatBox(0.090, 0.176758, 0.275, 0.050)))
{}

std::array<ImageFloatBox, 10> ItemPrinterMaterialDetector::Material_Boxes(ImageFloatBox initial_box){
    std::array<ImageFloatBox, 10> material_boxes;
    double x = initial_box.x;
    double width = initial_box.width;
    double height = initial_box.height;
    double initial_y = initial_box.y;
    double y_spacing = 0.074219;
    for (size_t i = 0; i < 10; i++){
        double y = initial_y + i*y_spacing;
        material_boxes[i] = ImageFloatBox(x, y, width, height);
    }
    return material_boxes;
}

void ItemPrinterMaterialDetector::make_overlays(VideoOverlaySet& items) const{
    for (size_t i = 0; i < 10; i++){
        items.add(m_color, m_box_mat_value[i]);
        items.add(m_color, m_box_mat_quantity[i]);
        items.add(m_color, m_box_mat_name[i]);
    }
}

int16_t ItemPrinterMaterialDetector::read_number(
    Logger& logger, AsyncDispatcher& dispatcher,
    const ImageViewRGB32& screen, const ImageFloatBox& box
) const{

    ImageViewRGB32 cropped = extract_box_reference(screen, box);
    ImageRGB32 filtered = to_blackwhite_rgb32_range(
        cropped,
        0xff000000, 0xff808080,
        true
    );
    // filtered.save("DebugDumps/test-one-filter-1.png");
    bool is_dark_text_light_background = image_stats(filtered).average.sum() > 400;
    // std::cout << "Average sum of filtered: "<< std::to_string(image_stats(filtered).average.sum()) << std::endl;

    int16_t number;
    if (is_dark_text_light_background){
        number = (int16_t)OCR::read_number_waterfill(logger, cropped, 0xff000000, 0xff808080);
    }else{
        number = (int16_t)OCR::read_number_waterfill(logger, cropped, 0xff808080, 0xffffffff);
    }

    if (number < 1 || number > 999){
        number = -1;
    }
    return (int16_t)number;
}


// return row number where Happiny dust is located on screen
// keep pressing DPAD_RIGHT until Happiny dust is on screen
// check each row on the screen for Happiny Dust
int8_t ItemPrinterMaterialDetector::find_happiny_dust_row_index(
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console, BotBaseContext& context
) const{
    int8_t value_68_row_index;
    for (size_t c = 0; c < 30; c++){
        context.wait_for_all_requests();
        std::vector<int8_t> value_68_row_index_list = find_material_value_row_index(dispatcher, console, context, 68);
        for (size_t i = 0; i < value_68_row_index_list.size(); i++){
            value_68_row_index = value_68_row_index_list[i];
            if (detect_material_name(console, context, value_68_row_index) == "happiny-dust"){  
                // found screen and row number with Happiny dust.
                // std::cout << "Happiny dust found. Row number: " << std::to_string(value_68_row_index) << std::endl;
                return value_68_row_index;
            }
        }
        
        // keep searching for Happiny dust
        pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
    }

    OperationFailedException::fire(
        console, ErrorReport::SEND_ERROR_REPORT,
        "Failed to find Happiny dust after 10 tries."
    );

}

// detects the material name at the given row_index
// MaterialNameReader has a very limited dictionary,
// so it can only reliably read the material names with 68% value
// (i.e. Ditto Goo, Happiny Dust, Magby Hair, Beldum Claw)
std::string ItemPrinterMaterialDetector::detect_material_name(
    ConsoleHandle& console, 
    BotBaseContext& context,
    int8_t row_index
) const{
    VideoSnapshot snapshot = console.video().snapshot();
    ImageFloatBox material_name_box = m_box_mat_name[row_index];
    ImageViewRGB32 material_name_image = extract_box_reference(snapshot, material_name_box);
    const auto ocr_result = MaterialNameReader::instance().read_substring(
        console, m_language, 
        material_name_image, OCR::BLACK_OR_WHITE_TEXT_FILTERS()
    );

    std::multimap<double, OCR::StringMatchData> results;
    if (!ocr_result.results.empty()){
        for (const auto& result : ocr_result.results){
            results.emplace(result.first, result.second);
        }
    }

    if (results.empty()){
        return "";
    }

    if (results.size() > 1){
        OperationFailedException::fire(
            console, ErrorReport::SEND_ERROR_REPORT,
            "ItemPrinterMaterialDetector::detect_material_name(): Unable to read selected item. Ambiguous or multiple results."
        );
    }

    return results.begin()->second.token;
}

// return vector of row index(es) that matches given material_value.
std::vector<int8_t> ItemPrinterMaterialDetector::find_material_value_row_index(
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console, 
    BotBaseContext& context,
    int16_t material_value
) const{
    context.wait_for_all_requests();
    VideoSnapshot snapshot = console.video().snapshot();
    int8_t total_rows = 10;
    std::vector<int8_t> row_indexes;
    SpinLock lock;
    std::vector<std::unique_ptr<AsyncTask>> tasks(total_rows);
    for (int8_t row_index = 0; row_index < total_rows; row_index++){
        tasks[row_index] = dispatcher.dispatch([&, row_index]{
            int16_t value = read_number(console, dispatcher, snapshot, m_box_mat_value[row_index]);
            if (value == material_value){
                WriteSpinLock lg(lock);
                row_indexes.push_back(row_index);
            }
        });
    }

    return row_indexes;    

}

// detect the quantity of material at the given row number
int16_t ItemPrinterMaterialDetector::detect_material_quantity(
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console, 
    BotBaseContext& context,
    int8_t row_index
) const{
    context.wait_for_all_requests();
    VideoSnapshot snapshot = console.video().snapshot();
    int16_t value = read_number(console, dispatcher, snapshot, m_box_mat_quantity[row_index]);
    return value;
}




}
}
}

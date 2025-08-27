/*  Tournament Prize Jobs Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV_ItemPrinterMaterialDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

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
        // std::cout << "{" << x << "," << y << "," << width << "," << height << "}, ";
    }
    // std::cout << std::endl;
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
    Logger& logger,
    const ImageViewRGB32& screen, const ImageFloatBox& box,
    int8_t row_index
) const{

    ImageViewRGB32 cropped = extract_box_reference(screen, box);
    ImageRGB32 filtered = to_blackwhite_rgb32_range(
        cropped,
        true,
        0xff000000, 0xff808080
    );
    // filtered.save("DebugDumps/test-one-filter-1.png");
    bool is_dark_text_light_background = image_stats(filtered).average.sum() > 400;
    // std::cout << "Average sum of filtered: "<< std::to_string(image_stats(filtered).average.sum()) << std::endl;

    const std::vector<std::pair<uint32_t, uint32_t>> filters = 
        [&](){
            if (is_dark_text_light_background){
                return std::vector<std::pair<uint32_t, uint32_t>>{
                    // {0xff000000, 0xffb0b0b0},
                    {0xff000000, 0xffa0a0a0},
                    {0xff000000, 0xff959595},
                    {0xff000000, 0xff909090},
                    {0xff000000, 0xff858585},
                    {0xff000000, 0xff808080},
                    // {0xff000000, 0xff707070},
                    // {0xff000000, 0xff606060},
                    // {0xff000000, 0xff505050},
                    // {0xff000000, 0xff404040},
                    // {0xff000000, 0xff303030},
                    // {0xff000000, 0xff202020},
                    // {0xff000000, 0xff101010},
                };
            }else{
                return std::vector<std::pair<uint32_t, uint32_t>>{
                    {0xff808080, 0xffffffff},
                    {0xff858585, 0xffffffff},
                    {0xff909090, 0xffffffff},
                    {0xff959595, 0xffffffff},
                    {0xffa0a0a0, 0xffffffff},
                    // {0xffb0b0b0, 0xffffffff},
                    // {0xffc0c0c0, 0xffffffff},
                    // {0xffd0d0d0, 0xffffffff},
                    // {0xffe0e0e0, 0xffffffff},
                    // {0xfff0f0f0, 0xffffffff},
                };
            }
        }();
    
    size_t max_width = (size_t)((double)24 * screen.width() / 1080);

    int16_t number = (int16_t)OCR::read_number_waterfill_multifilter(
        logger,
        cropped, filters,
        max_width,
        true, true,
        row_index
    );

    if (number < 1 || number > 999){
        number = -1;
    }
    return (int16_t)number;
}


// return row number where Happiny dust is located on screen
// keep pressing DPAD_RIGHT until Happiny dust is on screen
// check each row on the screen for Happiny Dust
int8_t ItemPrinterMaterialDetector::find_happiny_dust_row_index(
    VideoStream& stream, ProControllerContext& context
) const{
    int8_t happiny_dust_row_index = -1;
    for (size_t c = 0; c < 30; c++){
        context.wait_for_all_requests();
        VideoSnapshot snapshot = stream.video().snapshot();
        int8_t total_rows = 10;
        
        SpinLock lock;
        GlobalThreadPools::normal_inference().run_in_parallel(
            [&](size_t index){
                std::string material_name = detect_material_name(stream, snapshot, context, (int8_t)index);
                if ("happiny-dust" == material_name){
                    WriteSpinLock lg(lock);
                    happiny_dust_row_index = (int8_t)index;
                }
            },
            0, total_rows, 1
        );

        if (happiny_dust_row_index != -1){
            // found screen and row number with Happiny dust.
            // std::cout << "Happiny dust found. Row number: " << std::to_string(value_68_row_index) << std::endl;
            return happiny_dust_row_index;
        }

        // keep searching for Happiny dust
        pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Failed to find Happiny dust after multiple attempts.",
        stream
    );

}

std::string ItemPrinterMaterialDetector::detect_material_name(
    VideoStream& stream,
    const ImageViewRGB32& screen,
    ProControllerContext& context,
    int8_t row_index
) const{
    
    ImageFloatBox material_name_box = m_box_mat_name[row_index];
    ImageViewRGB32 material_name_image = extract_box_reference(screen, material_name_box);
    const auto ocr_result = MaterialNameReader::instance().read_substring(
        stream.logger(), m_language,
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
            ErrorReport::SEND_ERROR_REPORT,
            "ItemPrinterMaterialDetector::detect_material_name(): Unable to read selected item. Ambiguous or multiple results.\n" + language_warning(m_language),
            stream
        );
    }

    return results.begin()->second.token;
}

// return vector of row index(es) that matches given material_value.
std::vector<int8_t> ItemPrinterMaterialDetector::find_material_value_row_index(
    VideoStream& stream,
    ProControllerContext& context,
    int16_t material_value
) const{
    context.wait_for_all_requests();
    VideoSnapshot snapshot = stream.video().snapshot();
    int8_t total_rows = 10;
    std::vector<int8_t> row_indexes;
    SpinLock lock;

    GlobalThreadPools::normal_inference().run_in_parallel(
        [&](size_t index){
            int16_t value = read_number(stream.logger(), snapshot, m_box_mat_value[index], (int8_t)index);
            if (value == material_value){
                WriteSpinLock lg(lock);
                row_indexes.push_back((int8_t)index);
            }
        },
        0, total_rows, 1
    );

    return row_indexes;    

}

int16_t ItemPrinterMaterialDetector::detect_material_quantity(
    VideoStream& stream,
    const ImageViewRGB32& screen,
    ProControllerContext& context,
    int8_t row_index
) const{
    int16_t value = read_number(stream.logger(), screen, m_box_mat_quantity[row_index], row_index);
    return value;
}




}
}
}

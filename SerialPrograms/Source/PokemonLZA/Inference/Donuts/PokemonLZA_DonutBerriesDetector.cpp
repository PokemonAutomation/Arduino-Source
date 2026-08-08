/*  Donut Berries Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Filesystem.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
//#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLZA/Resources/PokemonLZA_DonutBerries.h"
#include "PokemonLZA_DonutBerriesDetector.h"
#include "Tests/TestUtils.h"

#include <fstream>
//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


DonutBerriesSelectionDetector::DonutBerriesSelectionDetector(size_t menu_index)
    : m_symbol_box({0.001, menu_index*0.087 + 0.245, 0.006, 0.070}){}

void DonutBerriesSelectionDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_symbol_box);
}

bool DonutBerriesSelectionDetector::detect(const ImageViewRGB32& screen){
    // Match the pink bar:
    ImageViewRGB32 symbol = extract_box_reference(screen, m_symbol_box);
    return is_solid(symbol, { 0.4375, 0.268, 0.295 }, 0.05, 15);
}


const DonutBerriesMatcher& DONUT_BERRIES_MATCHER(){
    static DonutBerriesMatcher matcher;
    return matcher;
}


DonutBerriesMatcher::DonutBerriesMatcher(const std::vector<double>& min_euclidean_distance)
    : CroppedImageDictionaryMatcher({0, 1})
{
    for (double x : min_euclidean_distance){
        m_min_euclidean_distance_squared.emplace_back(x * x);
    }
    for (const auto& item : DONUT_BERRIES_DATABASE()){
        add(item.first, item.second.sprite);
    }
}
auto DonutBerriesMatcher::get_crop_candidates(const ImageViewRGB32& image) const -> std::vector<ImageViewRGB32>{
    ImageStats border = image_border_stats(image);
//    cout << "border = " << border.average << endl;
//    image.save("image.png");
    std::vector<ImageViewRGB32> ret;
    for (double min_euclidean_distance_squared : m_min_euclidean_distance_squared){
        ImagePixelBox box = ImageMatch::enclosing_rectangle_with_pixel_filter(
            image,
            [&](Color pixel){
                double r = (double)pixel.red() - border.average.r;
                double g = (double)pixel.green() - border.average.g;
                double b = (double)pixel.blue() - border.average.b;
                bool stop = r * r + g * g + b * b >= min_euclidean_distance_squared;
                return stop;
            }
        );
        ret.emplace_back(extract_box_reference(image, box));
    }
    return ret;
}


const DonutBerriesOCR& DonutBerriesOCR::instance(){
    static DonutBerriesOCR reader;
    return reader;
}
DonutBerriesOCR::DonutBerriesOCR()
    : SmallDictionaryMatcher("PokemonLZA/Donuts/donut_berry_ocr.json")
{}
OCR::StringMatchResult DonutBerriesOCR::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image,
        OCR::BLACK_OR_WHITE_TEXT_FILTERS(),
        MAX_LOG10P, MAX_LOG10P_SPREAD,
        0.01, 0.50,
        OCR::PageSegMode::SINGLE_LINE
    );
}


DonutBerriesReader::DonutBerriesReader(Color color)
    : m_color(color)
    , m_box_berry_text(berry_list_boxes(ImageFloatBox(0.065, 0.250, 0.173, 0.037)))
    , m_box_berry_icon(berry_list_boxes(ImageFloatBox(0.029, 0.254, 0.030, 0.047)))
{}

void DonutBerriesReader::make_overlays(VideoOverlaySet& items) const{
    for (size_t c = 0; c < BERRY_PAGE_LINES; c++){
        items.add(m_color, m_box_berry_text[c]);
        items.add(m_color, m_box_berry_icon[c]);
    }
}

std::array<ImageFloatBox, 8> DonutBerriesReader::berry_list_boxes(ImageFloatBox initial_box){
    std::array<ImageFloatBox, 8> material_boxes;
    double x = initial_box.x;
    double width = initial_box.width;
    double height = initial_box.height;
    double initial_y = initial_box.y;
    double y_spacing = 0.086;
    for (size_t i = 0; i < 8; i++){
        double y = initial_y + i*y_spacing;
        material_boxes[i] = ImageFloatBox(x, y, width, height);
    }
    return material_boxes;
}


ImageMatch::ImageMatchResult DonutBerriesReader::read_berry_page_with_icon_matcher(const ImageViewRGB32& screen, size_t index) const{
    return read_with_icon_matcher(screen, m_box_berry_icon[index]);
}

ImageMatch::ImageMatchResult DonutBerriesReader::read_with_icon_matcher(const ImageViewRGB32& screen, const ImageFloatBox icon_box) const{
    // Get a crop of the berry icon
    ImageViewRGB32 image = extract_box_reference(screen, icon_box);
    //ImageRGB32 filtered_image = filter_rgb32_range(image, 0xffdfaf00, 0xffffef20, Color(0x00000000), true);
    //image.save("image" + std::to_string(icon_box.x) + ".png");

    ImageMatch::ImageMatchResult results;
    results = DONUT_BERRIES_MATCHER().match(image, ALPHA_SPREAD);
    //results.clear_beyond_alpha(MAX_ALPHA);

    return results;
}

OCR::StringMatchResult DonutBerriesReader::read_berry_page_with_ocr(
    const ImageViewRGB32& screen, 
    Logger& logger, 
    Language language, 
    size_t index
) const{
    return read_with_ocr(screen, logger, language, m_box_berry_text[index]);
}

OCR::StringMatchResult DonutBerriesReader::read_with_ocr(
    const ImageViewRGB32& screen, 
    Logger& logger, 
    Language language, 
    const ImageFloatBox icon_box
) const{

    // Get a crop of the berry text
    ImageViewRGB32 image = extract_box_reference(screen, icon_box);
    //image.save("image.png");

    OCR::StringMatchResult results;
    results = DonutBerriesOCR::instance().read_substring(logger, language, image);

    return results;
}


class Test_DonutBerriesReader : public UnitTest{
public:
    Test_DonutBerriesReader(const std::string& image)
        : UnitTest("PokemonPLZA::DonutBerriesReader - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        Filesystem::Path file_path(m_image);
        Filesystem::Path parent_dir = file_path.parent_path();
        std::string base_name = file_path.stem().string();

        const std::vector<std::string> words = parse_words(base_name);
        if (words.size() < 2){
            return "Error: not enough number of words in the filename.";
        }

        std::string code = words[words.size() - 2];
        if (code == "chiSim"){
            code = "chi_sim";
        }else if (code == "chiTra"){
            code = "chi_tra";
        }
        Language language = language_code_to_enum(code);
        if (language == Language::None || language == Language::EndOfList){
            return "Error: invalid language word in filename.";
        }

        size_t selected_berry = 0;
        if (!parse_size_t(words.back(), selected_berry)){
            return "Error: selected berry word must be int of range [0, 7].";
        }
        if (selected_berry >= DonutBerriesReader::BERRY_PAGE_LINES){
            return "Error: selected_berry must be in range [0, 7].";
        }

        ImageRGB32 image(m_image);
        Filesystem::Path target_berries_path = parent_dir / ("_" + base_name + ".txt");

        if (STATIC_GLOBALS.GENERATE_TEST_GOLDEN_FILES){
            std::ofstream output_file(target_berries_path.stdpath());
            if (!output_file.is_open()){
                return "Error: cannot open output file " + target_berries_path.string() + " for writing";
            }

            DonutBerriesReader reader;
            for (size_t i = 0; i < DonutBerriesReader::BERRY_PAGE_LINES; ++i){
                OCR::StringMatchResult results = reader.read_berry_page_with_ocr(image, global_logger_command_line(), language, i);
                output_file << (results.results.empty() ? "unknown-berry" : results.results.begin()->second.token) << std::endl;
            }
            return true;
        }

        std::vector<std::string> target_berries;
        if (!load_slug_list(target_berries_path.string(), target_berries)){
            return "Cannot load slug list from " + target_berries_path.string();
        }
        if (target_berries.size() != DonutBerriesReader::BERRY_PAGE_LINES){
            return "Error: need to have exactly 8 berries in " + target_berries_path.string();
        }

        DonutBerriesReader reader;
        for (size_t i = 0; i < DonutBerriesReader::BERRY_PAGE_LINES; ++i){
            DonutBerriesSelectionDetector selection_detector(i);
            TEST_RESULT_COMPONENT_EQUAL_STR(selection_detector.detect(image), i == selected_berry, "selection detector : berry slot " + std::to_string(i));

            OCR::StringMatchResult results = reader.read_berry_page_with_ocr(image, global_logger_command_line(), language, i);
            if (results.results.empty()){
                return "No berry detected via OCR";
            }
            TEST_RESULT_COMPONENT_EQUAL_STR(results.results.begin()->second.token, target_berries[i], "ocr : berry slot " + std::to_string(i));
        }

        return true;
    }

private:
    std::string m_image;
};


void add_tests_DonutBerriesReader(UnitTestDatabase& database){
    database.add<Test_DonutBerriesReader>("PokemonLZA/DonutBerriesReader/20260102_01_chiSim_7.jpg");
    database.add<Test_DonutBerriesReader>("PokemonLZA/DonutBerriesReader/20260102_01_eng_6.jpg");
    database.add<Test_DonutBerriesReader>("PokemonLZA/DonutBerriesReader/20260102_02_chiSim_5.jpg");
    database.add<Test_DonutBerriesReader>("PokemonLZA/DonutBerriesReader/20260102_02_eng_6.jpg");
    database.add<Test_DonutBerriesReader>("PokemonLZA/DonutBerriesReader/20260102_03_eng_0.jpg");
    database.add<Test_DonutBerriesReader>("PokemonLZA/DonutBerriesReader/20260102_04_eng_7.jpg");
    database.add<Test_DonutBerriesReader>("PokemonLZA/DonutBerriesReader/20260102_05_eng_3.jpg");
    database.add<Test_DonutBerriesReader>("PokemonLZA/DonutBerriesReader/20260120_01_kor_0.jpg");
    database.add<Test_DonutBerriesReader>("PokemonLZA/DonutBerriesReader/20260120_02_kor_0.jpg");
    database.add<Test_DonutBerriesReader>("PokemonLZA/DonutBerriesReader/20260122_01_eng_0.jpg");
}

}
}
}

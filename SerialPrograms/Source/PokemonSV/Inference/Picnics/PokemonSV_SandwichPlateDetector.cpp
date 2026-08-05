/*  Sandwich Plate Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonSV_SandwichPlateDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichIngredientDetector.h"
#include "Tests/TestUtils.h"
// #include "CommonFramework/Tools/DebugDumper.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


SandwichPlateDetector::~SandwichPlateDetector() = default;

SandwichPlateDetector::SandwichPlateDetector(Logger& logger, Color color, Language language, Side side)
    : m_logger(logger), m_color(color), m_language(language), m_side(side)
{
    switch(side){
    case Side::LEFT:
        m_box = ImageFloatBox(0.099, 0.270, 0.205, 0.041);
        break;
    case Side::MIDDLE:
        m_box = ImageFloatBox(0.397, 0.268, 0.203, 0.044);
        break;
    case Side::RIGHT:
        m_box = ImageFloatBox(0.699, 0.269, 0.201, 0.044);
        break;
    default:
        throw InternalProgramError(&logger, PA_CURRENT_FUNCTION,
            "Invalid Side for SandwichPlateDetector()");
    }
}

void SandwichPlateDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool SandwichPlateDetector::detect(const ImageViewRGB32& screen){
    return !detect_filling_name(screen).empty();
}

std::string SandwichPlateDetector::detect_filling_name(const ImageViewRGB32& screen) const{
    std::multimap<double, OCR::StringMatchData> results;

    const uint32_t image_filter_low_bounds[2] = {
        combine_rgb(215, 215, 215),
        combine_rgb(200, 200, 200)
    };
    const uint32_t image_filter_high_bound = combine_rgb(255, 255, 255);

    for (uint32_t image_filter_low_bound : image_filter_low_bounds){
        ImageRGB32 plate_label = to_blackwhite_rgb32_range(
            extract_box_reference(screen, m_box),
            true,
            image_filter_low_bound, image_filter_high_bound
        );

        // dump_debug_image(m_logger, "PokemonSV/SandwichPlateDetector", "blackwhite_input", plate_label);
        
        OCR::StringMatchResult ocr_result = PokemonSV::SandwichFillingOCR::instance().read_substring(
            m_logger, m_language, plate_label,
            OCR::BLACK_TEXT_FILTERS()
        );
        ocr_result.clear_beyond_log10p(SandwichFillingOCR::MAX_LOG10P);
        ocr_result.clear_beyond_spread(SandwichFillingOCR::MAX_LOG10P_SPREAD);
        if (!ocr_result.results.empty()){
            for (const auto& result : ocr_result.results){
                results.emplace(result.first, result.second);
            }
            
        }
    }

    if (results.empty()){
        return "";
    }
    
    return results.begin()->second.token;
}


bool SandwichPlateDetector::is_label_yellow(const ImageViewRGB32& screen) const{
    ImageRGB32 yellow_region = filter_rgb32_range(
        extract_box_reference(screen, m_box),
        combine_rgb(180, 161, 0), combine_rgb(255, 255, 100), Color(0), false
    );
    ImageStats stats = image_stats(yellow_region);

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * 200);

    return stats.count >= min_size;
}



SandwichPlateWatcher::~SandwichPlateWatcher() = default;

SandwichPlateWatcher::SandwichPlateWatcher(Logger& logger, Color color, VideoOverlay& overlay, Language language, Side side)
    : VisualInferenceCallback("SandwichPlateWatcher")
    , m_overlay(overlay)
    , m_detector(logger, color, language, side)
{}

void SandwichPlateWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool SandwichPlateWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    m_filling_name = m_detector.detect_filling_name(screen);
    if (m_filling_name.empty()){
        return false;
    }

    return true;
}


class Test_SandwichPlateDetector : public UnitTest{
public:
    Test_SandwichPlateDetector(const std::string& image, std::vector<std::string> words)
        : UnitTest("PokemonSV::SandwichPlateDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_words(std::move(words))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (m_words.size() < 4){
            return "Error: not enough number of words in the filename.";
        }

        Language language = language_code_to_enum(m_words[m_words.size() - 4]);
        if (language == Language::None || language == Language::EndOfList){
            return "Error: invalid language word in filename.";
        }

        auto& unit_logger = global_logger_command_line();
        FixedLimitVector<SandwichPlateDetector> detectors(3);
        detectors.emplace_back(unit_logger, COLOR_RED, language, SandwichPlateDetector::Side::LEFT);
        detectors.emplace_back(unit_logger, COLOR_RED, language, SandwichPlateDetector::Side::MIDDLE);
        detectors.emplace_back(unit_logger, COLOR_RED, language, SandwichPlateDetector::Side::RIGHT);

        std::string sides[3] = {"left", "middle", "right"};
        ImageRGB32 image(m_image);
        for (int i = 0; i < 3; i++){
            bool is_yellow = detectors[i].is_label_yellow(image);
            std::string target = m_words[m_words.size() - 3 + i];
            if (target == "Yellow"){
                TEST_RESULT_COMPONENT_EQUAL(is_yellow, true, "yellow label detection at side: " + sides[i]);
            }else{
                std::string filling = detectors[i].detect_filling_name(image);
                if (target == "none"){
                    target.clear();
                }
                TEST_RESULT_COMPONENT_EQUAL(filling, target, "side: " + sides[i]);
            }
        }

        return true;
    }

private:
    std::string m_image;
    std::vector<std::string> m_words;
};


void add_tests_SandwichPlateDetector(UnitTestDatabase& database){
    database.add<Test_SandwichPlateDetector>("PokemonSV/SandwichPlateDetector/Violet_eng_cucumber_cherry-tomatoes_pickle.png", std::vector<std::string>{"Violet", "eng", "cucumber", "cherry-tomatoes", "pickle"});
    database.add<Test_SandwichPlateDetector>("PokemonSV/SandwichPlateDetector/Violet_eng_Yellow_lettuce_lettuce.png", std::vector<std::string>{"Violet", "eng", "Yellow", "lettuce", "lettuce"});
}








}
}
}

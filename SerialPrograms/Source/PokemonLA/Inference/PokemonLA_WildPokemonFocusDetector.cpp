/*  Wild Pokemon Focus Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/ImageGradient.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "Tests/TestUtils.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/Inference/PokemonLA_StatusInfoScreenDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ShinySymbolDetector.h"
#include "PokemonLA_WildPokemonFocusDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


WildPokemonFocusDetector::WildPokemonFocusDetector(Logger& logger, VideoOverlay& overlay)
    : VisualInferenceCallback("WildPokemonFocusDetector")
    , m_pokemon_tab_upper_bound(0.109, 0.857, 0.24, 0.012)
    , m_pokemon_tab_lower_bound(0.109, 0.949, 0.24, 0.012)
    , m_pokemon_tab_left_bound(0.102, 0.875, 0.007, 0.073)
    , m_pokemon_tab_right_bound(0.348, 0.873, 0.007, 0.073)
{}

void WildPokemonFocusDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_BLUE, m_pokemon_tab_upper_bound);
    items.add(COLOR_BLUE, m_pokemon_tab_lower_bound);
    items.add(COLOR_BLUE, m_pokemon_tab_left_bound);
    items.add(COLOR_BLUE, m_pokemon_tab_right_bound);
}

//  Return true if the inference session should stop.
bool WildPokemonFocusDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){

    // dump_debug_image(m_logger, "PokemonLA/WildPokemonFocusDetector", "Received", frame);

    // float threshold = 10.0;
    const Color threshold(10, 10, 10);
    const ImageViewRGB32& upper_border = extract_box_reference(frame, m_pokemon_tab_upper_bound);

    const size_t upper_border_length = count_horizontal_translucent_border_pixels(
        upper_border, threshold, false);
    
    if (upper_border_length / (float)upper_border.width() <= 0.9){
        return false;
    }

    const ImageViewRGB32& lower_border = extract_box_reference(frame, m_pokemon_tab_lower_bound);
    
    const size_t lower_border_length = count_horizontal_translucent_border_pixels(
        lower_border, threshold, true);
    
    if (lower_border_length / (float)lower_border.width() <= 0.9){
        return false;
    }

    const ImageViewRGB32& left_border = extract_box_reference(frame, m_pokemon_tab_left_bound);
    const size_t left_border_length = count_vertical_translucent_border_pixels(
        left_border, threshold, false);
    if (left_border_length / (float)left_border.height() <= 0.9){
        return false;
    }

    const ImageViewRGB32& right_border = extract_box_reference(frame, m_pokemon_tab_right_bound);
    const size_t right_border_length = count_vertical_translucent_border_pixels(
        right_border, threshold, true);
    if (right_border_length / (float)right_border.height() <= 0.9){
        return false;
    }

    // dump_debug_image(m_logger, "PokemonLA/WildPokemonFocusDetector", "Detected", frame);
    return true;
}


PokemonDetails read_focused_wild_pokemon_info(
    Logger& logger, VideoOverlay& overlay,
    const ImageViewRGB32& frame,
    Language language
){
    PokemonDetails ret;

    const OverlayBoxScope name_box(overlay, {0.108, 0.868, 0.135, 0.037}, COLOR_BLACK);
    const OverlayBoxScope gender_box(overlay, {0.307, 0.873, 0.016, 0.030}, COLOR_PURPLE);
    const OverlayBoxScope alpha_box(overlay, {0.307, 0.920, 0.016, 0.029}, COLOR_RED);

    const ImageViewRGB32 name_image = extract_box_reference(frame, name_box);

    const OCR::StringMatchResult name_result = Pokemon::PokemonNameReader::instance().read_substring(
        logger, language, name_image,
        OCR::WHITE_TEXT_FILTERS());
    for (const auto& item : name_result.results){
        ret.name_candidates.insert(std::move(item.second.token));
    }

    // Replacing white color of the gender symbol with zero-alpha color so that they won't be counted in
    // the following image_stats().
    // The white color is defined as the color between (160, 160, 160) and (255, 255, 255).
    bool replace_color_range = true;
    const ImageStats gender_stats = image_stats(filter_rgb32_range(
        extract_box_reference(frame, gender_box),
        combine_rgb(160, 160, 160), combine_rgb(255, 255, 255), Color(0), replace_color_range
    ));
    const FloatPixel gender_avg = gender_stats.average;
    if (gender_stats.count > 0 && gender_avg.r > gender_avg.b + 50.0 && gender_avg.r > 150.0){
        ret.gender = Gender::Female;
        logger.log("Gender Female, color " + gender_avg.to_string());
    }else if (gender_stats.count > 0 && gender_avg.b > gender_avg.r + 50.0 && gender_avg.b > 150.0){
        ret.gender = Gender::Male;
        logger.log("Gender Male, color " + gender_avg.to_string());
    }else{
        ret.gender = Gender::Genderless;
        logger.log("Gender Genderless, color " + gender_avg.to_string());
    }

    // Replace non-red color with zero-alpha color so that they won't be counted in
    // the following image_stats().
    // The red color is defined as ranging from (200, 0, 0) to (255, 100, 100).
    replace_color_range = false;
    const ImageStats alpha_stats = image_stats(filter_rgb32_range(
        extract_box_reference(frame, alpha_box),
        combine_rgb(200, 0, 0), combine_rgb(255, 100, 100), Color(0), replace_color_range
    ));
    const FloatPixel alpha_avg = alpha_stats.average;
    logger.log("Alpha color " + alpha_avg.to_string());
    if (alpha_stats.count > 0 && alpha_avg.r > alpha_avg.g + 100.0 && alpha_avg.r > alpha_avg.b + 100.0 && alpha_avg.r > 200.0){
        ret.is_alpha = true;
        logger.log("Is alpha, color " + alpha_avg.to_string());
    }
    
    const std::vector<ImagePixelBox> shiny_locations = find_shiny_symbols(frame);
    if (shiny_locations.size() > 0){
        ret.is_shiny = true;
        logger.log("Found shiny symbol.");
    }

    return ret;
}


bool detect_change_focus(Logger& logger, VideoOverlay& overlay, const ImageViewRGB32& frame){
    const bool stop_on_detect = true;
    ButtonDetector button(logger, overlay, ButtonType::ButtonA, {0.244, 0.815, 0.026, 0.047},
        std::chrono::milliseconds(0), stop_on_detect);
    
    return button.process_frame(frame, current_time());
}








class Test_WildPokemonFocusDetector : public UnitTest{
public:
    Test_WildPokemonFocusDetector(
        const std::string& image,
        std::vector<std::string> keywords
    )
        : UnitTest("PokemonLA::WildPokemonFocusDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_keywords(std::move(keywords))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        std::stringstream ss;

        // two keywords: <True/False> <True/False>
        if (m_keywords.size() < 2){
            ss << "Error: not enough number of keywords in the filename. Found only " << m_keywords.size() << "." << std::endl;
            return ss.str();
        }

        DummyVideoOverlay overlay;

        WildPokemonFocusDetector detector(logger, overlay);
        ImageRGB32 image(m_image);
        bool result_has_focus = detector.process_frame(image, current_time());

        auto check_can_change_focus = [&](const std::string& word) -> std::string {
            bool result_can_change = detect_change_focus(logger, overlay, image);
            bool target_can_change = false;
            if (parse_bool(word, target_can_change) == false){
                ss << "Error: True/False keyword " << word << " is wrong. Must be \"True\" or \"False\"." << std::endl;
                return ss.str();
            }
            TEST_RESULT_EQUAL_STR(result_can_change, target_can_change);
            return "";
        };

        bool target_has_focus = false;
        if (parse_bool(m_keywords[m_keywords.size()-2], target_has_focus) == true){
            TEST_RESULT_EQUAL(result_has_focus, target_has_focus);

            return check_can_change_focus(m_keywords[m_keywords.size()-1]);
        }

        // Or seven keywords:
        // the last seven keywords should be: <True/False> <True/False> <language> <pokemon name slug> <Shiny/NotShiny> <Alpha/NotAlpha> <Male/Female/Genderless>
        if (m_keywords.size() < 7){
            ss << "Error: not enough number of keywords in the filename. Found only " << m_keywords.size() << "." << std::endl;
            return ss.str();
        }

        if (parse_bool(m_keywords[m_keywords.size()-7], target_has_focus) == false){
            ss << "Error: True/False keyword " << m_keywords[m_keywords.size()-7] << " is wrong. Must be \"True\" or \"False\"." << std::endl;
            return ss.str();
        }

        TEST_RESULT_EQUAL(result_has_focus, target_has_focus);

        if (result_has_focus){
            std::string ret = check_can_change_focus(m_keywords[m_keywords.size()-6]);
            if (!ret.empty()){
                return ret;
            }

            Language language = Language::None;
            PokemonDetails target_info;
            ret = read_pokemon_info_from_words(m_keywords, language, target_info);
            if (!ret.empty()){
                return ret;
            }

            const auto info = read_focused_wild_pokemon_info(logger, overlay, image, language);

            return test_pokemon_details(info, target_info);
        }

        return true;
    };

private:
    std::string m_image;
    std::vector<std::string> m_keywords;
};





void add_tests_WildPokemonFocusDetector(UnitTestDatabase& database){

}




}
}
}

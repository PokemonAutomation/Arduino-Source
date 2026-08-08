/*  MMO Question Marks Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include <iostream>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "Tests/TestUtils.h"
#include "PokemonLA_MMOQuestionMarkDetector.h"
#include "PokemonLA/PokemonLA_Locations.h"

using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels::Waterfill;

namespace{

// Match the dark blue background of the question mark
class MMOQuestionMarkBackgroundMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MMOQuestionMarkBackgroundMatcher();
    static const MMOQuestionMarkBackgroundMatcher& instance();
};

MMOQuestionMarkBackgroundMatcher::MMOQuestionMarkBackgroundMatcher()
    : WaterfillTemplateMatcher(
        "PokemonLA/MMOQuestionMark-Template.png",
        Color(0, 20, 40), Color(60, 90, 130), 200
    )
{
    m_aspect_ratio_lower = 0.8;
    m_aspect_ratio_upper = 1.2;
    m_area_ratio_lower = 0.8;
    m_area_ratio_upper = 1.2;
}

const MMOQuestionMarkBackgroundMatcher& MMOQuestionMarkBackgroundMatcher::instance(){
    static MMOQuestionMarkBackgroundMatcher matcher;
    return matcher;
}


// Match the main curve of the question mark
class MMOQuestionMarkCurveMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MMOQuestionMarkCurveMatcher();
    static const MMOQuestionMarkCurveMatcher& instance();
};

MMOQuestionMarkCurveMatcher::MMOQuestionMarkCurveMatcher()
    : WaterfillTemplateMatcher(
        "PokemonLA/MMOQuestionMark-Template.png",
        Color(0xff808080), Color(0xffffffff), 200
    )
{
    m_aspect_ratio_lower = 0.8;
    m_aspect_ratio_upper = 1.2;
    m_area_ratio_lower = 0.8;
    m_area_ratio_upper = 1.2;
}

const MMOQuestionMarkCurveMatcher& MMOQuestionMarkCurveMatcher::instance(){
    static MMOQuestionMarkCurveMatcher matcher;
    return matcher;
}



// The boxes that cover the locations on the Hisui map that MMO question marks will appear.
const std::array<ImageFloatBox, 5> hisui_map_boxes{{
    {0.362, 0.670, 0.045, 0.075},
    {0.683, 0.555, 0.039, 0.076},
    {0.828, 0.372, 0.042, 0.082},
    {0.485, 0.440, 0.044, 0.080},
    {0.393, 0.144, 0.050, 0.084}
}};


bool detect_MMO_question_mark(const PokemonAutomation::ImageViewRGB32& frame, const ImageFloatBox& box){
    auto image = extract_box_reference(frame, box);

    const double screen_rel_size = (frame.height() / 1080.0);
    const double rel_scale = screen_rel_size * screen_rel_size;

    const size_t min_bg_size = 1300;
    const size_t max_bg_size = 1600;

    auto scale = [&](size_t size) -> size_t{
        return size_t(size * rel_scale);
    };

    bool detected = match_template_by_waterfill(
        frame.size(),
        image,
        MMOQuestionMarkBackgroundMatcher::instance(),
        {
            {combine_rgb(0, 0, 0), combine_rgb(127, 127, 127)},
            {combine_rgb(0, 10, 30), combine_rgb(60, 90, 130)},
        },
        {scale(min_bg_size), scale(max_bg_size)},
        90,
        [](WaterfillObject&){ return true; }
    );

//    cout << "detected = " << detected << endl;

    if (detected){
        const size_t min_curve_size = 250;
        const size_t max_curve_size = 450;
        detected = match_template_by_waterfill(
            frame.size(),
            image, MMOQuestionMarkCurveMatcher::instance(),
            {{combine_rgb(180, 180, 180), combine_rgb(255, 255, 255)}},
            {scale(min_curve_size), scale(max_curve_size)}, 100,
            [](WaterfillObject&){ return true; }
        );
    }

    return detected;
}

} // anonymous namespace


MMOQuestionMarkDetector::MMOQuestionMarkDetector(Logger& logger)
    : m_logger(logger)
{}


void MMOQuestionMarkDetector::make_overlays(VideoOverlaySet& items) const{
    for (size_t i = 0; i < hisui_map_boxes.size(); i++){
        items.add(COLOR_RED, hisui_map_boxes[i]);
    }
}

std::array<bool, 5> MMOQuestionMarkDetector::detect_MMO_on_hisui_map(const ImageViewRGB32& frame){
    std::array<bool, 5> detected{false};
    for (size_t i = 0; i < hisui_map_boxes.size(); i++){
        detected[i] = detect_MMO_question_mark(frame, hisui_map_boxes[i]);
    }
    
    if (std::find(detected.begin(), detected.end(), true) != detected.end()){
        std::ostringstream os;
        os << "Detected MMO question mark on region ";
        for (size_t i = 0; i < detected.size(); i++){
            if (detected[i]){
                os << WILD_REGION_SHORT_NAMES[i] << ", ";
            }
        }
        m_logger.log(os.str(), COLOR_PURPLE);
    }

    return detected;
}

std::vector<ImagePixelBox> MMOQuestionMarkDetector::detect_MMOs_on_region_map(const ImageViewRGB32& frame){
    ImageFloatBox map_view{0.261, 0.060, 0.481, 0.842};
    size_t map_min_x = (size_t)(frame.width() * map_view.x + 0.5);
    size_t map_min_y = (size_t)(frame.height() * map_view.y + 0.5);
    size_t map_width = (size_t)(frame.width() * map_view.width + 0.5);
    size_t map_height = (size_t)(frame.height() * map_view.height + 0.5);
    ImageViewRGB32 map_image(frame.sub_image(map_min_x, map_min_y, map_width, map_height));

    std::vector<ImagePixelBox> results;

    const double screen_rel_size = (frame.height() / 1080.0);
    const double rel_scale = screen_rel_size * screen_rel_size;

    const size_t min_bg_size = 1300;
    const size_t max_bg_size = 1800;

    auto scale = [&](size_t size) -> size_t{
        return size_t(size * rel_scale);
    };

    match_template_by_waterfill(
        frame.size(),
        map_image, MMOQuestionMarkBackgroundMatcher::instance(),
        {{combine_rgb(0, 5, 30), combine_rgb(100, 130, 130)}},
        {scale(min_bg_size), scale(max_bg_size)}, 110,
        [&](WaterfillObject& object){
            size_t min_x = object.min_x + map_min_x;
            size_t min_y = object.min_y + map_min_y;
            size_t max_x = object.max_x + map_min_x;
            size_t max_y = object.max_y + map_min_y;
            results.emplace_back(min_x, min_y, max_x, max_y);
            return false;
        }
    );

    return results;
}




void add_hisui_MMO_detection_to_overlay(const std::array<bool, 5>& detection_result, VideoOverlaySet& items){
    for (size_t i = 0; i < hisui_map_boxes.size(); i++){
        if (detection_result[i]){
            items.add(COLOR_CYAN, hisui_map_boxes[i]);
        }
    }
}






class Test_MMOQuestionMarkDetector : public UnitTest{
public:
    Test_MMOQuestionMarkDetector(
        const std::string& image,
        std::vector<std::string> keywords
    )
        : UnitTest("PokemonLA::MMOQuestionMarkDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_keywords(std::move(keywords))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        bool hisui_kw_found = false;
        std::array<bool, 5> target_hisui_region_has_MMO = {false};

        bool region_kw_found = false;
        int target_num_MMOs_on_region_map = -1;
        int target_region_index = -1;

        for (size_t keyword_index = 0; keyword_index < m_keywords.size(); keyword_index++){
            const std::string& word = m_keywords[keyword_index];
            if (hisui_kw_found == false && word == "Hisui"){
                hisui_kw_found = true;
                continue;
            }
            if (region_kw_found == false && word == "Region"){
                region_kw_found = true;
                continue;
            }

            if (hisui_kw_found){
                // We have found the "Hisui" keyword. So the next words will be the index of the region on the Hisui map
                // that has MMO.
                try{
                    int region = std::stoi(word);
                    if (region < 0 || region > 4){
                        cerr << "Error: wrong region number, must be [0, 4] but got " << region << endl;
                        return "Error: wrong region number, must be [0, 4] but got " + std::to_string(region);
                    }
                    target_hisui_region_has_MMO[region] = true;
                }catch (std::exception&){
                    cerr << "Error: keyword must be a region number, ranging in [0, 4], but got " << word << endl;
                    return "Error: keyword must be a region number, ranging in [0, 4], but got " + word;
                }
            }else if (region_kw_found){
                // Found "Region" keyword, read a number as how many MMOs on the region map, and a region name.
                if (target_region_index < 0){
                    for (size_t index = 0; index < 5; index++){
                        if (word == WILD_REGION_SHORT_NAMES[index]){
                            target_region_index = (int)index;
                            break;
                        }
                    }
                    if (target_region_index >= 0){
                        continue;
                    }
                }
                if (target_num_MMOs_on_region_map < 0){
                    try{
                        int num_MMOs = std::stoi(word);
                        if (num_MMOs < 0){
                            cerr << "Error: wrong number " << num_MMOs << ", must be non-negative" << endl;
                            return "Error: wrong number " + std::to_string(num_MMOs) + ", must be non-negative";
                        }
                        target_num_MMOs_on_region_map = num_MMOs;
                    }catch (std::exception&){}
                }
            }
        }

        if (hisui_kw_found == false && region_kw_found == false){
            cerr << "Error: need keyword \"Hisui\" or \"Region\" in filename/" << endl;
            return "Error: need keyword \"Hisui\" or \"Region\" in filename/";
        }

        MMOQuestionMarkDetector detector(logger);
        ImageRGB32 image(m_image);

        if (hisui_kw_found){
            const auto region_has_MMO = detector.detect_MMO_on_hisui_map(image);

            for (size_t i = 0; i < 5; i++){
                const bool result = region_has_MMO[i];
                const bool target = target_hisui_region_has_MMO[i];
                if (result != target){
                    cerr << "Error: " << __func__ << " result on region " << i << " is " << result << " but should be " << target << "." << endl;
                    return std::string("Error: ") + __func__ +
                    " result on region " + std::to_string(i) +
                    " is " + std::to_string(result) +
                    " but should be " + std::to_string(target) + ".";
                }
            }
        }else{ // Region keyword found
            if (target_region_index < 0 || target_num_MMOs_on_region_map < 0){
                cerr << "Error: need a region name and a number of MMOs in the filename (e.g. image-Fieldlands_5.png)." << endl;
            }
            const auto results = detector.detect_MMOs_on_region_map(image);
            TEST_RESULT_EQUAL((int)results.size(), target_num_MMOs_on_region_map);
        }

        return true;
    };

private:
    std::string m_image;
    std::vector<std::string> m_keywords;
};




void add_tests_MMOQuestionMarkDetector(UnitTestDatabase& database){
    database.add<Test_MMOQuestionMarkDetector>(
        "PokemonLA/MMOQuestionMarkDetector/macOS_bright/All_Hisui_0_1_2_3_4.png",
        std::vector<std::string>{"All", "Hisui", "0", "1", "2", "3", "4"}
    );
    database.add<Test_MMOQuestionMarkDetector>(
        "PokemonLA/MMOQuestionMarkDetector/macOS_bright/MMO_Whiscash_Hisui_0_3.png",
        std::vector<std::string>{"MMO", "Whiscash", "Hisui", "0", "3"}
    );
    database.add<Test_MMOQuestionMarkDetector>(
        "PokemonLA/MMOQuestionMarkDetector/WinElgato/MMO_Barboach_Hisui_0_3.png",
        std::vector<std::string>{"MMO", "Barboach", "Hisui", "0", "3"}
    );
    database.add<Test_MMOQuestionMarkDetector>(
        "PokemonLA/MMOQuestionMarkDetector/WinElgato/MMO_Hisui_3_4.png",
        std::vector<std::string>{"MMO", "Hisui", "3", "4"}
    );
    database.add<Test_MMOQuestionMarkDetector>(
        "PokemonLA/MMOQuestionMarkDetector/WinHD60S/MMO_Hisui_3_4.png",
        std::vector<std::string>{"MMO", "Hisui", "3", "4"}
    );
    database.add<Test_MMOQuestionMarkDetector>(
        "PokemonLA/MMOQuestionMarkDetector/WinHD60S/MMO_Yanmega_Hisui_3_4.png",
        std::vector<std::string>{"MMO", "Yanmega", "Hisui", "3", "4"}
    );
    database.add<Test_MMOQuestionMarkDetector>(
        "PokemonLA/MMOQuestionMarkDetector/WinMirabox/MMO_Hisui_0_1_3.png",
        std::vector<std::string>{"MMO", "Hisui", "0", "1", "3"}
    );
    database.add<Test_MMOQuestionMarkDetector>(
        "PokemonLA/MMOQuestionMarkDetector/WinMirabox/MMO_Hisui_3_4.png",
        std::vector<std::string>{"MMO", "Hisui", "3", "4"}
    );
}









}
}
}

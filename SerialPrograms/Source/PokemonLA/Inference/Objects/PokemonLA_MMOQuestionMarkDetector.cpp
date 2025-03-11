/*  MMO Question Marks Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonLA_MMOQuestionMarkDetector.h"
#include "PokemonLA/PokemonLA_Locations.h"

//#include <iostream>
//using std::cout;
//using std::endl;

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


bool detect_MMO_question_mark(const PokemonAutomation::ImageViewRGB32 &frame, const ImageFloatBox& box){
    auto image = extract_box_reference(frame, box);

    const double screen_rel_size = (frame.height() / 1080.0);
    const double rel_scale = screen_rel_size * screen_rel_size;

    const size_t min_bg_size = 1300;
    const size_t max_bg_size = 1600;

    auto scale = [&](size_t size) -> size_t{
        return size_t(size * rel_scale);
    };

    bool detected = match_template_by_waterfill(
        image,
        MMOQuestionMarkBackgroundMatcher::instance(),
        {
            {combine_rgb(0, 0, 0), combine_rgb(127, 127, 127)},
            {combine_rgb(0, 10, 30), combine_rgb(60, 90, 130)},
        },
        {scale(min_bg_size), scale(max_bg_size)},
        90,
        [](WaterfillObject&) { return true; }
    );

//    cout << "detected = " << detected << endl;

    if (detected){
        const size_t min_curve_size = 250;
        const size_t max_curve_size = 450;
        detected = match_template_by_waterfill(
            image, MMOQuestionMarkCurveMatcher::instance(),
            {{combine_rgb(180, 180, 180), combine_rgb(255, 255, 255)}},
            {scale(min_curve_size), scale(max_curve_size)}, 100,
            [](WaterfillObject&) { return true; }
        );
    }

    return detected;
}

} // anonymous namespace


MMOQuestionMarkDetector::MMOQuestionMarkDetector(Logger& logger)
    : m_logger(logger)
{}


void MMOQuestionMarkDetector::make_overlays(VideoOverlaySet& items) const{
    for(size_t i = 0; i < hisui_map_boxes.size(); i++){
        items.add(COLOR_RED, hisui_map_boxes[i]);
    }
}

std::array<bool, 5> MMOQuestionMarkDetector::detect_MMO_on_hisui_map(const ImageViewRGB32& frame){
    std::array<bool, 5> detected{false};
    for(size_t i = 0; i < hisui_map_boxes.size(); i++){
        detected[i] = detect_MMO_question_mark(frame, hisui_map_boxes[i]);
    }
    
    if (std::find(detected.begin(), detected.end(), true) != detected.end()){
        std::ostringstream os;
        os << "Detected MMO question mark on region ";
        for(size_t i = 0; i < detected.size(); i++){
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
    for(size_t i = 0; i < hisui_map_boxes.size(); i++){
        if (detection_result[i]){
            items.add(COLOR_CYAN, hisui_map_boxes[i]);
        }
    }
}



}
}
}

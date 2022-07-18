/*  MMO Question Marks Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <functional>
#include <QImage>
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageTools/WaterfillUtilities.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonLA_MMOQuestionMarkDetector.h"
#include "PokemonLA/PokemonLA_Locations.h"

#include <sstream>
#include <iostream>
#include <iomanip>
using std::cout;
using std::endl;

// #define DEBUG_MMO_QUESTION_MARK

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels::Waterfill;

namespace {

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

} // anonymous namespace


struct WaterfillTemplateDetectionDebugParams{
    std::string base_filename = "WaterfillTemplateDebug";
    uint32_t filter_color = (uint32_t)255 << 16; // default red color
    uint32_t match_color = (uint32_t)255 << 8; // default green color
};

bool detect_template_by_single_pass_waterfill(
    const ImageViewRGB32 &image,
    const ImageMatch::WaterfillTemplateMatcher &matcher,
    const std::vector<std::pair<uint32_t, uint32_t>> &filters,
    const std::pair<size_t, size_t> &area_thresholds,
    double rmsd_threshold,
    std::function<bool(WaterfillObject& object)> stop_search,
    const WaterfillTemplateDetectionDebugParams* debug_params)
{
    auto matrix = compress_rgb32_to_binary_multirange(image, filters);

    static int debug_count = 0;
    std::unique_ptr<QImage> debug_image;
    if (debug_params){
        debug_image = std::make_unique<QImage>(image.to_QImage_owning());
        draw_matrix_on_image(matrix, debug_params->filter_color, *debug_image, 0, 0);
    }

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
    WaterfillObject object;

    const size_t min_area = area_thresholds.first;
    bool detected = false;
    session->set_source(matrix);
    auto finder = session->make_iterator(min_area);
    const bool keep_object_matrix = (debug_params != nullptr);
    while (finder->find_next(object, keep_object_matrix)){
        if (object.area > area_thresholds.second){
            continue;
        }
        double rmsd = matcher.rmsd_original(image, object);
        if (debug_params){
            cout << debug_params->base_filename << ": rmsd " << rmsd << " area " << object.area << endl;
        }
        
        if (rmsd < rmsd_threshold){
            if (debug_params){
                draw_object_on_image(object, debug_params->match_color, *debug_image, 0, 0);
            }
            detected = true;
            
            if (stop_search(object)){
                break;
            }
        }
    }

    if (debug_params){
        std::ostringstream os;
        os << debug_params->base_filename << std::setfill('0') << std::setw(3) << debug_count << ".png";
        debug_image->save(QString::fromStdString(os.str()));
        debug_count++;
    }

    return detected;
}


bool detect_MMO_question_mark(const PokemonAutomation::ImageViewRGB32 &image){

    std::unique_ptr<WaterfillTemplateDetectionDebugParams> debug_params = nullptr;

#ifdef DEBUG_MMO_QUESTION_MARK
    debug_params = std::make_unique<WaterfillTemplateDetectionDebugParams>();
    debug_params->base_filename = "test_MMO_background_";
#endif
    bool detected = detect_template_by_single_pass_waterfill(
        image,
        MMOQuestionMarkBackgroundMatcher::instance(),
        {{combine_rgb(0, 10, 30), combine_rgb(60, 90, 130)}},
        // The dark blue background is about 1400-1500 pixels on 1080P.
        // Should be at least 550 pixels on 720P image.
        {550, 1500},
        90,
        [](WaterfillObject&) { return true; },
        debug_params.get()
    );

    if (detected){
#ifdef DEBUG_MMO_QUESTION_MARK
        debug_params->base_filename = "test_MMO_curve_";
#endif
        detected = detect_template_by_single_pass_waterfill(
            image,
            MMOQuestionMarkCurveMatcher::instance(),
            {{combine_rgb(180, 180, 180), combine_rgb(255, 255, 255)}},
            // The main curve of the question mark is about 310 pixels on 1080P.
            // Should be about 130 pixels on 720P image.
            {100, 450},
            100,
            [](WaterfillObject&) { return true; },
            debug_params.get()
        );
    }

    return detected;
}



MMOQuestionMarkDetector::MMOQuestionMarkDetector(LoggerQt& logger)
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
        auto cropped_frame = extract_box_reference(frame, hisui_map_boxes[i]);
        detected[i] = detect_MMO_question_mark(cropped_frame);
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

    std::unique_ptr<WaterfillTemplateDetectionDebugParams> debug_params = nullptr;

#ifdef DEBUG_MMO_QUESTION_MARK
    debug_params = std::make_unique<WaterfillTemplateDetectionDebugParams>();
    debug_params->base_filename = "test_region_MMO_";
#endif

    std::vector<ImagePixelBox> results;

    detect_template_by_single_pass_waterfill(
        map_image,
        MMOQuestionMarkBackgroundMatcher::instance(),
        // {{combine_rgb(0, 10, 30), combine_rgb(60, 90, 130)}},
        {{combine_rgb(0, 5, 30), combine_rgb(100, 130, 130)}},
        // The dark blue background is about 1400-1500 pixels on 1080P.
        // Should be at least 550 pixels on 720P image.
        {550, 1800},
        110,
        [&](WaterfillObject& object){
            size_t min_x = object.min_x + map_min_x;
            size_t min_y = object.min_y + map_min_y;
            size_t max_x = object.max_x + map_min_x;
            size_t max_y = object.max_y + map_min_y;
            results.emplace_back((pxint_t)min_x, (pxint_t)min_y, (pxint_t)max_x, (pxint_t)max_y);
            return false;
        },
        debug_params.get()
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

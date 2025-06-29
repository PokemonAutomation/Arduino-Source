/*  Object Name Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/WaterfillUtilities.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "GameName_ObjectNameDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace GameName{


class ObjectNameMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    ObjectNameMatcher() : WaterfillTemplateMatcher(
        "GameName/ObjectName-Template.png", Color(100,100,100), Color(255, 255, 255), 50
    ) {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance() {
        static ObjectNameMatcher matcher;
        return matcher;
    }
};


ObjectNameDetector::~ObjectNameDetector() = default;

ObjectNameDetector::ObjectNameDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_box(box)
{}

void ObjectNameDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool ObjectNameDetector::detect(const ImageViewRGB32& screen) const{
    std::vector<ImageFloatBox> hits = detect_all(screen);
    return !hits.empty();
}

std::vector<ImageFloatBox> ObjectNameDetector::detect_all(const ImageViewRGB32& screen) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(150, 150, 150), combine_rgb(255, 255, 255)}
    };

    const double min_object_size = 5000.0;
    const double rmsd_threshold = 60.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    std::vector<ImageFloatBox> found_locations;

    ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), m_box);    
    match_template_by_waterfill(
        extract_box_reference(screen, m_box), 
        ObjectNameMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            ImagePixelBox found_box(
                object.min_x + pixel_search_area.min_x, object.min_y + pixel_search_area.min_y,
                object.max_x + pixel_search_area.min_x, object.max_y + pixel_search_area.min_y);
            found_locations.emplace_back(pixelbox_to_floatbox(screen.width(), screen.height(), found_box));
            return false;
        }
    );

    return found_locations;
}



ObjectNameWatcher::~ObjectNameWatcher() = default;

ObjectNameWatcher::ObjectNameWatcher(Color color, VideoOverlay& overlay, const ImageFloatBox& box)
    : VisualInferenceCallback("ObjectNameWatcher")
    , m_overlay(overlay)
    , m_detector(color, box)
{}

void ObjectNameWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool ObjectNameWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    std::vector<ImageFloatBox> hits = m_detector.detect_all(screen);

    m_hits.reset(hits.size());
    for (const ImageFloatBox& hit : hits){
        m_hits.emplace_back(m_overlay, hit, COLOR_MAGENTA);
    }
    return !hits.empty();
}








}
}
}

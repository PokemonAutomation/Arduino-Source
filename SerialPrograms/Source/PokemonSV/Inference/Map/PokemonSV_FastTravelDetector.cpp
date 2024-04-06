/*  Fast Travel Detector
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
#include "PokemonSV_FastTravelDetector.h"

// #include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

ImageFloatBox MINIMAP_AREA(0.815, 0.680, 0.180, 0.310);


class FastTravelMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    FastTravelMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Map/FastTravelIcon-Template.png", Color(0,0,0), Color(255, 255, 255), 450
    ) {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.6;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance() {
        static FastTravelMatcher matcher;
        return matcher;
    }
};


FastTravelDetector::~FastTravelDetector() = default;

FastTravelDetector::FastTravelDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_box(box)
{}

void FastTravelDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool FastTravelDetector::detect(const ImageViewRGB32& screen) const{
    std::vector<ImageFloatBox> hits = detect_all(screen);
    return !hits.empty();
}

std::vector<ImageFloatBox> FastTravelDetector::detect_all(const ImageViewRGB32& screen) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(0, 0, 80), combine_rgb(140, 140, 255)},
        {combine_rgb(5, 100, 200), combine_rgb(60, 135, 255)}, // matching at blueberry academy 1080p

        // the below filters are for matching at blueberry academy at 720p
        // however, they increase the risk of false positives. 
        // as they select for images that are both wrong and have low RMSD (<85)
        // {combine_rgb(0, 90, 180), combine_rgb(55, 140, 255)}, 
        // {combine_rgb(0, 100, 180), combine_rgb(60, 135, 255)}, 
        // {combine_rgb(0, 100, 195), combine_rgb(50, 135, 255)}, 
        // {combine_rgb(0, 100, 180), combine_rgb(55, 130, 255)}, 
        // {combine_rgb(0, 90, 180), combine_rgb(50, 125, 255)}, 
    };

    const double min_object_size = 400.0;
    // need to keep RMSD threshold below 100, else will false positive the blue dots at blueberry academy
    const double rmsd_threshold = 80.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    std::vector<ImageFloatBox> found_locations;

    ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), m_box);    
    match_template_by_waterfill(
        extract_box_reference(screen, m_box), 
        FastTravelMatcher::instance(),
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



FastTravelWatcher::~FastTravelWatcher() = default;

FastTravelWatcher::FastTravelWatcher(Color color, VideoOverlay& overlay, const ImageFloatBox& box)
    : VisualInferenceCallback("FastTravelWatcher")
    , m_overlay(overlay)
    , m_detector(color, box)
{}

void FastTravelWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool FastTravelWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
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

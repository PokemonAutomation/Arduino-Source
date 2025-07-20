/*  Destination Marker Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonSV_DestinationMarkerDetector.h"

// #include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class DestinationMarkerMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    
    DestinationMarkerMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Map/DestinationMarkerIcon-Template.png", Color(180,80,0), Color(255, 130, 50), 50
    ){
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;

    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static DestinationMarkerMatcher matcher;
        return matcher;
    }
};

class DestinationMarkerYellowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    
    DestinationMarkerYellowMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Map/DestinationMarkerIcon-Yellow.png", Color(180,80,0), Color(255, 200, 50), 50
    ){
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;

    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static DestinationMarkerYellowMatcher matcher;
        return matcher;
    }
};


DestinationMarkerDetector::~DestinationMarkerDetector() = default;

DestinationMarkerDetector::DestinationMarkerDetector(Color color, const ImageFloatBox& box, bool check_yellow)
    : m_color(color)
    , m_box(box)
    , m_check_yellow(check_yellow)
{}

void DestinationMarkerDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool DestinationMarkerDetector::detect(const ImageViewRGB32& screen){
    std::vector<ImageFloatBox> hits = detect_all(screen);
    if (!m_check_yellow){
        return !hits.empty();
    }else{
        std::vector<ImageFloatBox> hits_yellow = detect_all_yellow(screen);
        return !hits.empty() || !hits_yellow.empty();
    }
}

std::vector<ImageFloatBox> DestinationMarkerDetector::detect_all(const ImageViewRGB32& screen) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(180, 80, 0), combine_rgb(255, 130, 50)},

    };


    const double rmsd_threshold = 65.0;  // from my testing: RMSD is 15 at 1080p, 60 at 720p
    const double min_object_size = 150.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    std::vector<ImageFloatBox> found_locations;

    ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), m_box);    
    match_template_by_waterfill(
        extract_box_reference(screen, m_box), 
        DestinationMarkerMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            ImagePixelBox found_box(
                object.min_x + pixel_search_area.min_x, object.min_y + pixel_search_area.min_y,
                object.max_x + pixel_search_area.min_x, object.max_y + pixel_search_area.min_y);
            found_locations.emplace_back(pixelbox_to_floatbox(screen.width(), screen.height(), found_box));
            return true;
        }
    );

    return found_locations;
}



std::vector<ImageFloatBox> DestinationMarkerDetector::detect_all_yellow(const ImageViewRGB32& screen) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(180, 100, 0), combine_rgb(255, 190, 50)},  // to detect the marker within the minimap, when the radar beam is covering the marker
    };


    const double rmsd_threshold = 60.0;  // from my testing, RMSD ranges from 15-50, even at 720p
    /* 
    - min object size restrictions also helps to filter out false positives
    at pokemon centers
    */
    const double min_object_size = 150.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    std::vector<ImageFloatBox> found_locations;

    ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), m_box);    
    match_template_by_waterfill(
        extract_box_reference(screen, m_box), 
        DestinationMarkerYellowMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            ImagePixelBox found_box(
                object.min_x + pixel_search_area.min_x, object.min_y + pixel_search_area.min_y,
                object.max_x + pixel_search_area.min_x, object.max_y + pixel_search_area.min_y);
            found_locations.emplace_back(pixelbox_to_floatbox(screen.width(), screen.height(), found_box));
            return true;
        }
    );

    return found_locations;
}






}
}
}

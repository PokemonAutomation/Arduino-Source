/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonSV_MapDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


namespace{

class MapOrangleFixedViewArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MapOrangleFixedViewArrowMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Map/OrangleFixedView-Template.png", Color(50,50,0), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;
    }

    static const MapOrangleFixedViewArrowMatcher& instance(){
        static MapOrangleFixedViewArrowMatcher matcher;
        return matcher;
    }
};


class MapOrangleRotatedViewArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MapOrangleRotatedViewArrowMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Map/OrangleRotatedView-Template.png", Color(50,50,0), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;
    }

    static const MapOrangleRotatedViewArrowMatcher& instance(){
        static MapOrangleRotatedViewArrowMatcher matcher;
        return matcher;
    }
};


} // end anonymous namespace


ImageFloatBox MAP_READABLE_AREA{0.197, 0.182, 0.678, 0.632};

MapFixedViewDetector::MapFixedViewDetector(Color color)
    : m_color(color)
    , m_arrow_box(0.165, 0.712, 0.014, 0.040)
{}
void MapFixedViewDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool MapFixedViewDetector::detect(const ImageViewRGB32& frame){
     const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(100, 100, 0), combine_rgb(255, 255, 200)}
    };

    const double screen_rel_size = (frame.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * 150.0);
    
    const bool detected = match_template_by_waterfill(
        extract_box_reference(frame, m_arrow_box), 
        MapOrangleFixedViewArrowMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        120,
        [](Kernels::Waterfill::WaterfillObject& object) -> bool { return true; }
    );
    
    return detected;
}


MapRotatedViewDetector::MapRotatedViewDetector(Color color)
    : m_color(color)
    , m_arrow_box(0.157, 0.720, 0.029, 0.039)
{}
void MapRotatedViewDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool MapRotatedViewDetector::detect(const ImageViewRGB32& frame){
     const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(100, 100, 0), combine_rgb(255, 255, 200)}
    };

    const double screen_rel_size = (frame.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * 450.0);
    
    const bool detected = match_template_by_waterfill(
        extract_box_reference(frame, m_arrow_box), 
        MapOrangleRotatedViewArrowMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        120,
        [](Kernels::Waterfill::WaterfillObject& object) -> bool { return true; }
    );
    
    return detected;
}


MapWatcher::MapWatcher(Color color)
: VisualInferenceCallback("MapWatcher")
, m_exit_watcher(COLOR_RED, WhiteButton::ButtonY, {0.800, 0.118, 0.030, 0.060}), m_fixed_view_watcher(color), m_rotated_view_watcher(color){}

void MapWatcher::make_overlays(VideoOverlaySet& items) const{
    m_exit_watcher.make_overlays(items);
    m_fixed_view_watcher.make_overlays(items);
    m_rotated_view_watcher.make_overlays(items);
}

bool MapWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    const bool exit_found = m_exit_watcher.process_frame(frame, timestamp);
    if (!exit_found){
        return false;
    }

    const bool fixed_found = m_fixed_view_watcher.process_frame(frame, timestamp);
    const bool rotated_found = m_rotated_view_watcher.process_frame(frame, timestamp);

    if (fixed_found && !rotated_found){
        m_in_fixed_view = true;
        return true;
    }else if (!fixed_found && rotated_found){
        m_in_fixed_view = false;
        return true;
    }

    return false;
}


}
}
}

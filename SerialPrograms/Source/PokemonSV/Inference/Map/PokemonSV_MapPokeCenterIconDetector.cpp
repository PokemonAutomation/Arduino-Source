/*  Map PokeCenter Icon Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonSV_MapPokeCenterIconDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class MapPokeCenterIconMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MapPokeCenterIconMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Map/PokeCenterIcon-Template.png", Color(150,0,0), Color(255, 100, 100), 50
    ){
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.6;
        m_area_ratio_upper = 1.2;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static MapPokeCenterIconMatcher matcher;
        return matcher;
    }
};



MapPokeCenterIconDetector::MapPokeCenterIconDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_box(box)
{}

void MapPokeCenterIconDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool MapPokeCenterIconDetector::detect(const ImageViewRGB32& screen){
    std::vector<ImageFloatBox> hits = detect_all(screen);
    return !hits.empty();
}

std::vector<ImageFloatBox> MapPokeCenterIconDetector::detect_all(const ImageViewRGB32& screen) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        // {combine_rgb(150, 0, 0), combine_rgb(255, 120, 120)},
        {combine_rgb(100, 0, 0), combine_rgb(255, 100, 100)}
    };

    const double min_object_size = 350.0;
    const double rmsd_threshold = 100.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    std::vector<ImageFloatBox> found_locations;

    ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), m_box);    
    match_template_by_waterfill(
        extract_box_reference(screen, m_box), 
        MapPokeCenterIconMatcher::instance(),
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



MapPokeCenterIconWatcher::~MapPokeCenterIconWatcher() = default;

MapPokeCenterIconWatcher::MapPokeCenterIconWatcher(Color color, VideoOverlay& overlay, const ImageFloatBox& box)
    : VisualInferenceCallback("MapPokeCenterIconWatcher")
    , m_overlay(overlay)
    , m_detector(color, box)
{}

void MapPokeCenterIconWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool MapPokeCenterIconWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    m_hits = m_detector.detect_all(screen);

    m_hit_boxes.reset(m_hits.size());
    for (const ImageFloatBox& hit : m_hits){
        m_hit_boxes.emplace_back(m_overlay, hit, COLOR_MAGENTA);
    }
    return !m_hits.empty();
}





}
}
}

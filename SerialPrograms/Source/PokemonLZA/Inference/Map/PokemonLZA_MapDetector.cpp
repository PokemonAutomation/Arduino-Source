/* Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonLZA_MapDetector.h"
#include "PokemonLZA_MapIconDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

namespace{

// Matches the cyan "Travel Spots" feather icon shown next to the Y button on the map screen.
class TravelSpotMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    TravelSpotMatcher()
        : WaterfillTemplateMatcher(
            "PokemonLZA/MapIcons/TravelSpotIcon.png",
            Color(0xff004080),
            Color(0xff80ffff),
            50
        )
    {}

    static const TravelSpotMatcher& instance(){
        static TravelSpotMatcher matcher;
        return matcher;
    }

    virtual bool check_image(Resolution input_resolution, const ImageViewRGB32& image) const override{
        size_t min_w = 8 * input_resolution.width  / 3840;
        size_t min_h = 8 * input_resolution.height / 2160;
        return image.width() >= min_w && image.height() >= min_h;
    }
};



} // anonymous namespace



MapDetector::MapDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_travel_icon(0.040000, 0.235000, 0.030000, 0.068500)
    , m_x_button(
        color,
        ButtonType::ButtonX,
        {0.005000, 0.150000, 0.025000, 0.110000},
        overlay
    )
    , m_y_button(
        color,
        ButtonType::ButtonY,
        {0.005000, 0.210000, 0.025000, 0.110000},
        overlay
    )
{}

void MapDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_travel_icon);
    m_x_button.make_overlays(items);
    m_y_button.make_overlays(items);
}

bool MapDetector::detect(const ImageViewRGB32& screen){
//    cout << "MapDetector::detect()" << endl;
    const bool detected = detect_travel_spot(screen)
        && m_x_button.detect(screen)
        && m_y_button.detect(screen);

    if (detected){
        for (MapIconDetector* detector : m_map_icon_detectors){
            detector->detect(screen);
        }
    }
    return detected;
}


bool MapDetector::detect_travel_spot(const ImageViewRGB32& screen){
    // Progressive cyan filters to handle capture-card color variance.
    static const std::vector<std::pair<uint32_t, uint32_t>> CYAN_FILTERS{
        // Brighter cyan/teal.
        {0xff004898, 0xff70ffff},
        {0xff003878, 0xff80ffff},
        // Darker/desaturated cyan seen in some captures (e.g. 1080p hyperspace_True).
        {0xff002860, 0xff90ffff},
        {0xff001c50, 0xffa0ffff},
    };

    double rel = screen.height() / 2160.0;
    size_t min_area = size_t(50.0 * rel * rel);

    return match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_travel_icon),
        TravelSpotMatcher::instance(),
        CYAN_FILTERS,
        {min_area, SIZE_MAX},
        100.0,
        [](Kernels::Waterfill::WaterfillObject&) -> bool{ return true; }
    );
}
std::vector<DetectedBox> MapDetector::detected_map_icons() const{
    std::vector<DetectedBox> ret;
    for (MapIconDetector* detector : m_map_icon_detectors){
        const auto& detected_boxes = detector->last_detected();
        for (const auto& box: detected_boxes){
            ret.emplace_back(box.name, box.box);
        }
    }
    return ret;
}

void MapDetector::reset_state(){
    m_x_button.reset_state();
    m_y_button.reset_state();
    for (MapIconDetector* detector : m_map_icon_detectors){
        detector->reset_state();
    }
}



}
}
}

/* Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonLZA_MapDetector.h"
#include "PokemonLZA_MapIconDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



MapDetector::MapDetector(Color color, VideoOverlay* overlay)
    : m_b_button(
        color,
        ButtonType::ButtonB,
        {0.760730, 0.937023, 0.241416, 0.064885},
        overlay
    )
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
    m_b_button.make_overlays(items);
    m_x_button.make_overlays(items);
    m_y_button.make_overlays(items);
}

bool MapDetector::detect(const ImageViewRGB32& screen){
    const bool detected = m_b_button.detect(screen)
        && m_x_button.detect(screen)
        && m_y_button.detect(screen);

    if (detected){
        for (MapIconDetector* detector : m_map_icon_detectors){
            detector->detect(screen);
        }
    }
    return detected;
}

std::vector<DetectedBox> MapDetector::detected_map_icons() const{
    std::vector<DetectedBox> ret;
    for (MapIconDetector* detector : m_map_icon_detectors){
        const auto& detected_boxes = detector->last_detected();
        for(const auto& box: detected_boxes){
            ret.emplace_back(box.name, box.box);
        }
    }
    return ret;
}




}
}
}

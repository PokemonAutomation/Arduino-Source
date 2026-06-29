/* Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_MapDetector_H
#define PokemonAutomation_PokemonLZA_MapDetector_H

#include "CommonTools/VisualDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "CommonTools/DetectedBoxes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class MapIconDetector;


// Detect whether the map view is opened or not
class MapDetector : public StaticScreenDetector{
public:
    MapDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    //  Attach MapIconDetector. Those will be called if a map is detected to detect map icons.
    //  After detection session is done, call `detected_map_icons()` to get last detected map icons.
    void attach_map_icon_detector(MapIconDetector& detector){
        m_map_icon_detectors.push_back(&detector);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

    //  Return last detected map icons from attached MapIconDetectors.
    std::vector<DetectedBox> detected_map_icons() const;

    void reset_state() override;

private:
    ButtonDetector m_b_button;
    ButtonDetector m_x_button;
    ButtonDetector m_y_button;
    std::vector<MapIconDetector*> m_map_icon_detectors;
};

class MapWatcher : public DetectorToFinder<MapDetector>{
public:
    MapWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("MapWatcher", hold_duration, color, overlay)
    {}
};
class MapOverWatcher : public DetectorToFinder<MapDetector>{
public:
    MapOverWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(200)
    )
         : DetectorToFinder("MapWatcher", FinderType::GONE, hold_duration, color, overlay)
    {}
};








}
}
}
#endif

/* Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_MapDetector_H
#define PokemonAutomation_PokemonLZA_MapDetector_H

#include "CommonTools/VisualDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



class MapDetector : public StaticScreenDetector{
public:
    MapDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ButtonDetector m_b_button;
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








}
}
}
#endif

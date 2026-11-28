/*  Tutorial Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_TutorialMapDetector_H
#define PokemonAutomation_PokemonLZA_TutorialMapDetector_H

#include "VideoGames/PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

// The early map has lacks Travel Spots so we need a separate detector
class TutorialMapDetector : public StaticScreenDetector{
public:
    TutorialMapDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ButtonDetector m_mission_list;
    ButtonDetector m_close;
};

class TutorialMapWatcher : public DetectorToFinder<TutorialMapDetector>{
public:
    TutorialMapWatcher(
        Color color = COLOR_RED, VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(100)
    )
        : DetectorToFinder("TutorialMapWatcher", hold_duration, color, overlay)
    {}
};

}
}
}
#endif

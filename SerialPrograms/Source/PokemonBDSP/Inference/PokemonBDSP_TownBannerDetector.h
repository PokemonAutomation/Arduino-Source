/*  Town Banner Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_TownBannerDetector_H
#define PokemonAutomation_PokemonBDSP_TownBannerDetector_H

#include <array>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

// Detects the fully visible white location banner at the top left.
// Presence only: this does not read the name or identify the destination.
// Use route context and a fresh appearance to recognize a transition.
// Uses brightness/contrast, not an exact white RGB value or colored border.
class TownBannerDetector : public StaticScreenDetector{
public:
    TownBannerDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    std::array<ImageFloatBox, 4> m_white;
    ImageFloatBox m_border;
};

class TownBannerWatcher : public DetectorToFinder<TownBannerDetector>{
public:
    TownBannerWatcher(
        Color color = COLOR_RED,
        FinderType finder_type = FinderType::PRESENT,
        std::chrono::milliseconds duration = std::chrono::milliseconds(100)
    )
        : DetectorToFinder("TownBannerWatcher", finder_type, duration, color)
    {}
};

}
}
}
#endif

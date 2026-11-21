/*  Booth Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BoothDetector_H
#define PokemonAutomation_PokemonLZA_BoothDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

// Locate the Wild Zone 1 booth by its cyan roof strip and dark wall below it.
// No template is used: the booth's perspective and position can change.
class BoothDetector : public StaticScreenDetector{
public:
    BoothDetector(Color color = COLOR_CYAN);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    // Valid only after a successful detection. Coordinates are screen-relative.
    const ImageFloatBox& roof_box() const { return m_roof_box; }

private:
    Color m_color;
    ImageFloatBox m_search_box;
    ImageFloatBox m_roof_box;
};

class BoothWatcher : public DetectorToFinder<BoothDetector>{
public:
    BoothWatcher(Color color = COLOR_CYAN)
        : DetectorToFinder("BoothWatcher", std::chrono::milliseconds(250), color)
    {}
};

}
}
}
#endif

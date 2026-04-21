/*  Pokedex Registered Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PokedexRegisteredDetector_H
#define PokemonAutomation_PokemonFRLG_PokedexRegisteredDetector_H

#include "optional"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


// Detects the white "A" button prompt that appears in the bottom right of the
// screen when a Pokémon is registered in the Pokédex.
class PokedexRegisteredDetector : public StaticScreenDetector{
public:
    PokedexRegisteredDetector(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box = ImageFloatBox(0.85, 0.923, 0.082, 0.070)
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    virtual void reset_state() override { m_last_detected_box.reset(); }

private:
    Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};

class PokedexRegisteredWatcher : public DetectorToFinder<PokedexRegisteredDetector>{
public:
    PokedexRegisteredWatcher(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box = ImageFloatBox(0.85, 0.923, 0.082, 0.070),
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("PokedexRegisteredWatcher", hold_duration, color, overlay, box)
    {}
};


}
}
}
#endif

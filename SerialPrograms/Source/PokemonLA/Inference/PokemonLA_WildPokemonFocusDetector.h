/*  Wild Pokemon Focus Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detect the lower left dark translucent tab when player focuses on one pokemon
 */

#ifndef PokemonAutomation_PokemonLA_WildPokemonFocusDetector_H
#define PokemonAutomation_PokemonLA_WildPokemonFocusDetector_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"

namespace PokemonAutomation{

class VideoOverlay;

namespace NintendoSwitch{


namespace PokemonLA{


class WildPokemonFocusDetector : public VisualInferenceCallback{
public:
    WildPokemonFocusDetector(Logger& logger, VideoOverlay& overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Return true if the inference session should stop.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    // The upper bound of the transparent dark pokemon tab shown in lower left of the screen when focusing on
    // one pokemon. It should the pokemon name, lv, gender, whether caught before, alpha status and shiny status.
    ImageFloatBox m_pokemon_tab_upper_bound;
    ImageFloatBox m_pokemon_tab_lower_bound;
    ImageFloatBox m_pokemon_tab_left_bound;
    ImageFloatBox m_pokemon_tab_right_bound;
};

// Read the pokemon details: name, alpha, shiny, gender from the focused tab.
PokemonDetails read_focused_wild_pokemon_info(
    Logger& logger, VideoOverlay& overlay,
    const ImageViewRGB32& frame,
    Language language
);

// Detect the A button to know whether you can press A to change focus.
bool detect_change_focus(Logger& logger, VideoOverlay& overlay, const ImageViewRGB32& frame);


}
}
}
#endif

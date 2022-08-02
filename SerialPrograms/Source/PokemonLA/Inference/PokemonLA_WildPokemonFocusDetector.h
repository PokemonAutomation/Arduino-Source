/*  Wild Pokemon Focus Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Detect the black out screen when player character blacks out.
 */

#ifndef PokemonAutomation_PokemonLA_WildPokemonFocusDetector_H
#define PokemonAutomation_PokemonLA_WildPokemonFocusDetector_H

#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class WildPokemonFocusDetector : public VisualInferenceCallback{
public:
    WildPokemonFocusDetector(LoggerQt& logger, VideoOverlay& overlay);

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


PokemonDetails read_focused_wild_pokemon_info(
    LoggerQt& logger, VideoOverlay& overlay,
    const ImageViewRGB32& frame,
    Language language
);


}
}
}
#endif

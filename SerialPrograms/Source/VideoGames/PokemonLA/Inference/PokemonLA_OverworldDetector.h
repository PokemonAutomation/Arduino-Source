/*  Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_OverworldDetector_H
#define PokemonAutomation_PokemonLA_OverworldDetector_H

#include "PokemonLA_MountDetector.h"
#include "Objects/PokemonLA_ArcPhoneDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class OverworldDetector : public VisualInferenceCallback{
public:
    OverworldDetector(Logger& logger, VideoOverlay& overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    ArcPhoneDetector m_arc_phone;
    MountDetector m_mount;
};


//  Only works at full health.
bool is_pokemon_selection(VideoOverlay& overlay, const ImageViewRGB32& frame);


}
}
}
#endif

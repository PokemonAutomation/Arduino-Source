/*  Battle Start Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  Detect the start of a battle when the upper and lower part of the screen
 *  is covered by black.
 */

#ifndef PokemonAutomation_PokemonLA_BattleStartDetector_H
#define PokemonAutomation_PokemonLA_BattleStartDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class BattleStartDetector : public VisualInferenceCallback{
public:
    BattleStartDetector(LoggerQt& logger, VideoOverlay& overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    // The upper and lower black area boundary
    ImageFloatBox m_upper_boundary;
    ImageFloatBox m_lower_boundary;
};


}
}
}
#endif

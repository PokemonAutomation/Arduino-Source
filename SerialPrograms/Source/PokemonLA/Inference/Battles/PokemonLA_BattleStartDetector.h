/*  Battle Start Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  Detect the start of a battle when the upper and lower part of the screen
 *  is covered by black.
 */

#ifndef PokemonAutomation_PokemonLA_BattleStartDetector_H
#define PokemonAutomation_PokemonLA_BattleStartDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class BattleStartDetector : public VisualInferenceCallback{
public:
    BattleStartDetector(Logger& logger, VideoOverlay& overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Logger& m_logger;
    // The upper and lower black area boundary
    ImageFloatBox m_upper_boundary;
    ImageFloatBox m_lower_boundary;
    // Record whether a battle start has been detected. This is used for logging.
    bool m_started = false;
};


}
}
}
#endif

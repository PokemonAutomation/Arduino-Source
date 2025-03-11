/*  Dialog Triangle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DialogTriangleDetector_H
#define PokemonAutomation_PokemonSwSh_DialogTriangleDetector_H

#include <atomic>
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


// Detect the black triangle arrow in the lower right portion of the white dialog box.
// The dialog box shows up when talking to npcs.
class DialogTriangleDetector : public VisualInferenceCallback{
public:
    DialogTriangleDetector(
        Logger& logger, VideoOverlay& overlay,
        bool stop_on_detected
    );

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Logger& m_logger;
    bool m_stop_on_detected;

    std::atomic<bool> m_detected;
};





}
}
}
#endif

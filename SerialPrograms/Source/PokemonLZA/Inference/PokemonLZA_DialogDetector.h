/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DialogDetector_H
#define PokemonAutomation_PokemonLZA_DialogDetector_H

#include <atomic>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
class Logger;
class VideoOverlay;

namespace NintendoSwitch{
namespace PokemonLZA{

// Detect normal dialogue that is used in cases like when you talk to npcs in most situations.
class NormalDialogDetector : public VisualInferenceCallback{
public:
    NormalDialogDetector(Logger& logger, VideoOverlay& overlay, bool stop_on_detected);

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    bool m_stop_on_detected;
    std::atomic<bool> m_detected;
    ImageFloatBox m_title_green_line_box;
    ImageFloatBox m_black_arrow_box;
};


}
}
}
#endif

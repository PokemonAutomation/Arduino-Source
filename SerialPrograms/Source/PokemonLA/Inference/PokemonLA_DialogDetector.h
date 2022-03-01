/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_DialogDetector_H
#define PokemonAutomation_PokemonLA_DialogDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class DialogDetector : public VisualInferenceCallback{
public:
    DialogDetector(LoggerQt& logger, VideoOverlay& overlay, bool stop_on_detected);

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

private:
    bool m_stop_on_detected;
    std::atomic<bool> m_detected;
    ImageFloatBox m_title_top;
    ImageFloatBox m_title_bottom;
    ImageFloatBox m_top_white;
    ImageFloatBox m_bottom_white;
    ImageFloatBox m_cursor;
    ArcPhoneDetector m_arc_phone;
};


}
}
}
#endif

/*  Receive Pokemon (Orange Background) Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Returns true after a orange background has been detected
 * and has ended.
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ReceivePokemonDetector_H
#define PokemonAutomation_PokemonSwSh_ReceivePokemonDetector_H

#include <atomic>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ReceivePokemonDetector : public VisualInferenceCallback{
public:
    ReceivePokemonDetector(bool stop_on_detected);

    bool triggered() const{ return m_triggered.load(std::memory_order_acquire); }
    bool receive_is_over(const ImageViewRGB32& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    bool m_stop_on_detected;
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_top_right;
    ImageFloatBox m_box_bot_left;
    bool m_has_been_orange;

    std::atomic<bool> m_triggered;
};


}
}
}
#endif

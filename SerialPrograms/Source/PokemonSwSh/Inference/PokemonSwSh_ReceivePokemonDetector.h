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

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ReceivePokemonDetector : public VisualInferenceCallbackWithCommandStop{
public:
    ReceivePokemonDetector(VideoOverlay& overlay);

    bool receive_is_over(const QImage& frame);
    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
   ) override;

private:
    InferenceBoxScope m_box_top;
    InferenceBoxScope m_box_top_right;
    InferenceBoxScope m_box_bot_left;
    bool m_has_been_orange;
};


}
}
}
#endif

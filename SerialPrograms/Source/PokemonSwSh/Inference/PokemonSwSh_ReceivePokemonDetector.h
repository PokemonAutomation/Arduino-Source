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

#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ReceivePokemonDetector : public VisualInferenceCallback{
public:
    ReceivePokemonDetector();

    bool receive_is_over(const QImage& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const QImage& frame, WallClock timestamp) override;

private:
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_top_right;
    ImageFloatBox m_box_bot_left;
    bool m_has_been_orange;
};


}
}
}
#endif

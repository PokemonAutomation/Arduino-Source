/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_DialogDetector_H
#define PokemonAutomation_PokemonLA_DialogDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class DialogDetector : public VisualInferenceCallback{
public:
    DialogDetector(LoggerQt& logger, VideoOverlay& overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

private:
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

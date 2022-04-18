/*  Status Info Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Detect shiny and alpha on the Status Info Screen.
 */

#ifndef PokemonAutomation_PokemonLA_StatusInfoScreenDetector_H
#define PokemonAutomation_PokemonLA_StatusInfoScreenDetector_H

#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class StatusInfoScreenDetector : public VisualInferenceCallback{
public:
    StatusInfoScreenDetector();

    int16_t detected() const{
        return m_detected;
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

private:
    int16_t m_detected;

    ImageFloatBox m_shiny_box;
    ImageFloatBox m_alpha_box;
};


}
}
}
#endif

/*  Transparent Dialogue Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Detect the transparent dialogue box from your opponent shown before and/or after the battle, like
 *  Ingo's battles or Fortune Sisters'.
 */

#ifndef PokemonAutomation_PokemonLA_TransparentDialogueDetector_H
#define PokemonAutomation_PokemonLA_TransparentDialogueDetector_H

#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/Inference/DetectionDebouncer.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueEllipseDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueYellowArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class TransparentDialogueDetector : public VisualInferenceCallback{
public:
    TransparentDialogueDetector(
        LoggerQt& logger, VideoOverlay& overlay,
        bool stop_on_detected
    );

    bool detected() const{
        return m_detected;
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const QImage& frame, WallClock timestamp) override;

private:
    DialogueYellowArrowDetector m_arrow_detector;
    DialogueEllipseDetector m_ellipse_detector;
    std::atomic<bool> m_detected;
};



}
}
}
#endif

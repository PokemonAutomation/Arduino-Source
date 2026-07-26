/*  Transparent Dialogue Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detect the transparent dialogue box from your opponent shown before and/or after the battle, like
 *  Ingo's battles or Fortune Sisters'.
 */

#ifndef PokemonAutomation_PokemonLA_TransparentDialogueDetector_H
#define PokemonAutomation_PokemonLA_TransparentDialogueDetector_H

#include "Common/Cpp/TestRunners/UnitTest.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogEllipseDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogYellowArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class TransparentDialogueDetector : public VisualInferenceCallback{
public:
    TransparentDialogueDetector(
        Logger& logger, VideoOverlay& overlay,
        bool stop_on_detected
    );

    bool detected() const{
        return m_detected;
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    DialogYellowArrowDetector m_arrow_detector;
    DialogueEllipseDetector m_ellipse_detector;
    std::atomic<bool> m_detected;
};



void add_tests_TransparentDialogueDetector(UnitTestDatabase& database);



}
}
}
#endif

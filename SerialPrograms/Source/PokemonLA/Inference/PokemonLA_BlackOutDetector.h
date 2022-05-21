/*  Black Out Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Detect the black out screen as well as the dropped items screen after player
 *  character blacks out.
 */

#ifndef PokemonAutomation_PokemonLA_BlackOutDetector_H
#define PokemonAutomation_PokemonLA_BlackOutDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueYellowArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class BlackOutDetector : public VisualInferenceCallback{
public:
    BlackOutDetector(LoggerQt& logger, VideoOverlay& overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Return true if the inference session should stop.
    virtual bool process_frame(const QImage& frame, WallClock timestamp) override;

private:
    // The black screen when the dialog of "Everything went black!" appears
    ImageFloatBox m_black_screen;
    // The yellow arrow pointing to the white button on the "Everything went black!" dialog box.
    DialogueYellowArrowDetector m_yellow_arrow_detector;
    // The bottom white space of the "Return to Base Camp" button.
    // ImageFloatBox m_return_camp_bottom;
};


}
}
}
#endif

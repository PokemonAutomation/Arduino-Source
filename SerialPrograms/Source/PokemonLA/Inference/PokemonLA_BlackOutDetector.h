/*  Black Out Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detect the black out screen when player character blacks out.
 */

#ifndef PokemonAutomation_PokemonLA_BlackOutDetector_H
#define PokemonAutomation_PokemonLA_BlackOutDetector_H

#include "Common/Cpp/TestRunners/UnitTest.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogYellowArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class BlackOutDetector : public VisualInferenceCallback{
public:
    BlackOutDetector(Logger& logger, VideoOverlay& overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Return true if the inference session should stop.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    // The black screen when the dialog of "Everything went black!" appears
    ImageFloatBox m_black_screen;
    // The yellow arrow pointing to the white button on the "Everything went black!" dialog box.
    DialogYellowArrowDetector m_yellow_arrow_detector;
    // The bottom white space of the "Return to Base Camp" button.
    // ImageFloatBox m_return_camp_bottom;

    // ButtonDetector m_button_Y_detector;
};



void add_tests_BlackOutDetector(UnitTestDatabase& database);




}
}
}
#endif

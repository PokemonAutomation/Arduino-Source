/*  Dialogue Yellow Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The yellow marker pointing to the button to advance dialogue boxes.
 *  For different types of dialogue boxes, the locations of the yellow arrow is different.
 *  This detector covers all possible locations of the yellow arrow.
 */

#ifndef PokemonAutomation_PokemonLA_DialogYellowArrowDetector_H
#define PokemonAutomation_PokemonLA_DialogYellowArrowDetector_H

#include <atomic>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class Logger;
    class VideoOverlay;
namespace NintendoSwitch{
namespace PokemonLA{



class DialogYellowArrowDetector : public VisualInferenceCallback{
public:
    DialogYellowArrowDetector(
        Logger& logger, VideoOverlay& overlay,
        bool stop_on_detected
    );

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Logger& m_logger;
    bool m_stop_on_detected;

    std::atomic<bool> m_detected;
};




void add_tests_DialogYellowArrowDetector(UnitTestDatabase& database);


}
}
}
#endif

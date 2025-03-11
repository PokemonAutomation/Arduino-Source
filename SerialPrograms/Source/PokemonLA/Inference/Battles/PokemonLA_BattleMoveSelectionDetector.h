/*  Battle Move Selection Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detect the move selection screen. Useful to detect whether a move has no PP so it can
 *  not be used.
 *  Note: this detector is very easy to get false positives. Can be deleted if it is not used in future.
 */

#ifndef PokemonAutomation_PokemonLA_BattleMoveSelectionDetector_H
#define PokemonAutomation_PokemonLA_BattleMoveSelectionDetector_H

#include <atomic>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{

class VideoOverlay;
class Logger;

namespace NintendoSwitch{
namespace PokemonLA{

class BattleMoveSelectionDetector : public VisualInferenceCallback{
public:
    BattleMoveSelectionDetector(Logger& logger, VideoOverlay& overlay, bool stop_on_detected);

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    bool m_stop_on_detected;
    std::atomic<bool> m_detected;

    ImageFloatBox m_move_1_highlight;
    ImageFloatBox m_move_2_highlight;
    ImageFloatBox m_move_3_highlight;
    ImageFloatBox m_move_4_highlight;
};


}
}
}
#endif

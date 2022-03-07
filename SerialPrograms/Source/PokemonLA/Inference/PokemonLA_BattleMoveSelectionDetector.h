/*  Battle Move Selection Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Detect the move selection screen. Useful to detect whether a move has no PP so it can
 *  not be used.
 */

#ifndef PokemonAutomation_PokemonLA_BattleMoveSelectionDetector_H
#define PokemonAutomation_PokemonLA_BattleMoveSelectionDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Logging/LoggerQt.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class BattleMoveSelectionDetector : public VisualInferenceCallback{
public:
    BattleMoveSelectionDetector(LoggerQt& logger, VideoOverlay& overlay, bool stop_on_detected);

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

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

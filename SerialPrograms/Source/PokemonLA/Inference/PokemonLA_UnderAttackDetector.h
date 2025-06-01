/*  Under Attack Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_UnderAttackDetector_H
#define PokemonAutomation_PokemonLA_UnderAttackDetector_H

#include <deque>
#include <map>
#include <atomic>
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


enum class UnderAttackState{
    UNKNOWN,
    SAFE,
    UNDER_ATTACK,
};
extern const char* UNDER_ATTACK_STRINGS[];


class UnderAttackWatcher : public VisualInferenceCallback{
public:

public:
    UnderAttackWatcher(Logger& logger);

    UnderAttackState state() const{
        return m_state.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    UnderAttackState detect(const ImageViewRGB32& frame);

private:
    struct Sample{
        WallClock timestamp;
        UnderAttackState state;
    };

    Logger& m_logger;
    ImageFloatBox m_box;

    std::atomic<UnderAttackState> m_state;

//    SpinLock m_lock;
    std::deque<Sample> m_history;
    std::map<UnderAttackState, size_t> m_counts;
};



}
}
}
#endif

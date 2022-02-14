/*  Mount Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_MountDetector_H
#define PokemonAutomation_PokemonLA_MountDetector_H

#include <deque>
#include <map>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class VideoOverlaySet;
namespace NintendoSwitch{
namespace PokemonLA{


enum class MountState{
    NOTHING,
    WYRDEER_OFF,
    WYRDEER_ON,
    URSALUNA_OFF,
    URSALUNA_ON,
    BASCULEGION_OFF,
    BASCULEGION_ON,
    SNEASLER_OFF,
    SNEASLER_ON,
    BRAVIARY_OFF,
    BRAVIARY_ON,
};
extern const char* MOUNT_STATE_STRINGS[];


class MountDetector{
public:
    MountDetector();

    void make_overlays(VideoOverlaySet& items) const;
    MountState detect(const QImage& screen) const;

private:
    ImageFloatBox m_box;
};



class MountTracker : public VisualInferenceCallback{
public:
    MountTracker(Logger& logger);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    MountState state() const;

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

private:
    struct Sample{
        std::chrono::system_clock::time_point timestamp;
        MountState state;
    };

private:
    Logger& m_logger;
    std::atomic<MountState> m_state;

    SpinLock m_lock;
    MountDetector m_detector;

    std::deque<Sample> m_history;
    std::map<MountState, size_t> m_counts;
};



}
}
}
#endif

/*  Flag Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_FlagTracker_H
#define PokemonAutomation_PokemonLA_FlagTracker_H

#include <deque>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "PokemonLA_WhiteObjectDetector.h"
#include "PokemonLA_FlagDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class FlagTracker : public VisualInferenceCallback{
public:
    FlagTracker(Logger& logger, VideoOverlay& overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    bool get(
        double& distance, double& x, double& y,
        WallClock timestamp = current_time()
    ) const;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    struct Sample{
        WallClock timestamp;
        int distance;
        double x;
        double y;
    };

private:
    // Logger& m_logger;

    mutable SpinLock m_lock;
    FlagDetector m_flags;
    WhiteObjectWatcher m_watcher;

    std::deque<Sample> m_history;
};



}
}
}
#endif

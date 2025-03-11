/*  Mount Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_MountDetector_H
#define PokemonAutomation_PokemonLA_MountDetector_H

#include <deque>
#include <map>
#include <atomic>
#include "Common/Cpp/AbstractLogger.h"
//#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
//#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
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


enum class MountDetectorLogging{
    NONE,
    LOG_ONLY,
    LOG_AND_DUMP_FAILURES,
};

class MountDetectorLoggingOption : public EnumDropdownOption<MountDetectorLogging>{
public:
    MountDetectorLoggingOption()
        : EnumDropdownOption<MountDetectorLogging>(
            "<b>Detection Failed Action</b>",
            {
                {MountDetectorLogging::NONE,                    "none",     "Do Nothing"},
                {MountDetectorLogging::LOG_ONLY,                "log",      "Log to output window."},
                {MountDetectorLogging::LOG_AND_DUMP_FAILURES,   "log+dump", "Log to output window and save to file."},
            },
            LockMode::LOCK_WHILE_RUNNING,
            MountDetectorLogging::LOG_AND_DUMP_FAILURES
        )
    {}
};





class MountDetector{
public:
    MountDetector(MountDetectorLogging logging = MountDetectorLogging::NONE);

    void make_overlays(VideoOverlaySet& items) const;
    MountState detect(const ImageViewRGB32& screen) const;

private:
    ImageFloatBox m_box;
    MountDetectorLogging m_logging;
};



class MountTracker : public VisualInferenceCallback{
public:
    MountTracker(Logger& logger, MountDetectorLogging logging = MountDetectorLogging::NONE);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    MountState state() const;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    struct Sample{
        WallClock timestamp;
        MountState state;
    };

private:
    Logger& m_logger;
    std::atomic<MountState> m_state;

//    SpinLock m_lock;
    MountDetector m_detector;

    std::deque<Sample> m_history;
    std::map<MountState, size_t> m_counts;
};



}
}
}
#endif

/*  Raid Catch Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RaidCatchDetector_H
#define PokemonAutomation_PokemonSwSh_RaidCatchDetector_H

#include <chrono>
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class RaidCatchDetector{
public:
    RaidCatchDetector(
        VideoFeed& feed,
        std::chrono::milliseconds timeout
    );

    bool has_timed_out() const;
    bool detect();
    bool wait(ProgramEnvironment& env);

private:
    VideoFeed& m_feed;
    std::chrono::milliseconds m_timeout;
    InferenceBoxScope m_box0;
    InferenceBoxScope m_box1;
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
};


}
}
}
#endif


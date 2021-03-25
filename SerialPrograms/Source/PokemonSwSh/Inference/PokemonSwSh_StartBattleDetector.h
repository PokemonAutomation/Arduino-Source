/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StartBattleDetector_H
#define PokemonAutomation_PokemonSwSh_StartBattleDetector_H

#include <chrono>
#include "CommonFramework/Tools/VideoFeed.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StartBattleDetector{
public:
    StartBattleDetector(
        VideoFeed& feed,
        std::chrono::milliseconds timeout
    );

    bool has_timed_out() const;
    bool detect();

private:
    VideoFeed& m_feed;
    std::chrono::milliseconds m_timeout;
    InferenceBoxScope m_box;
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
};


}
}
}
#endif


/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StartBattleDetector_H
#define PokemonAutomation_PokemonSwSh_StartBattleDetector_H

#include <chrono>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



bool is_dialog_grey(const QImage& image);


class StartBattleDetector{
public:
    StartBattleDetector(
        VideoFeed& feed,
        std::chrono::milliseconds timeout
    );

    bool has_timed_out() const;
    bool detect(const QImage& screen);
    bool wait(ProgramEnvironment& env, Logger& logger);

private:
    VideoFeed& m_feed;
    std::chrono::milliseconds m_timeout;
    InferenceBoxScope m_screen_box;
    InferenceBoxScope m_dialog_box;
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
};


}
}
}
#endif


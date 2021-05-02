/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StartBattleDetector_H
#define PokemonAutomation_PokemonSwSh_StartBattleDetector_H

#include <chrono>
#include <thread>
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
    bool wait(ProgramEnvironment& env);

protected:
    VideoFeed& m_feed;
    std::chrono::milliseconds m_timeout;
    InferenceBoxScope m_screen_box;
    InferenceBoxScope m_dialog_box;
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
};


class AsyncStartBattleDetector : public StartBattleDetector{
public:
    AsyncStartBattleDetector(ProgramEnvironment& env, VideoFeed& feed);
    ~AsyncStartBattleDetector();

    bool detected() const;

    void start();


private:
    void thread_loop(ProgramEnvironment& env);

private:
    std::atomic<bool> m_stopping;
    std::atomic<bool> m_detected;
    std::thread m_thread;
};



}
}
}
#endif


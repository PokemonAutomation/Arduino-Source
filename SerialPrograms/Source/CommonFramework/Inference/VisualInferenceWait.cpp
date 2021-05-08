/*  Visual Inference Wait
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "VisualInferenceWait.h"

namespace PokemonAutomation{



VisualInferenceWait::VisualInferenceWait(
    ProgramEnvironment& env,
    VideoFeed& feed,
    std::chrono::milliseconds timeout,
    std::chrono::milliseconds period
)
    : m_env(env)
    , m_feed(feed)
    , m_timeout(timeout)
    , m_period(period)
{}

void VisualInferenceWait::operator+=(std::function<bool(const QImage&)>&& callback){
    m_callbacks0.emplace_back(std::move(callback));
}
void VisualInferenceWait::operator+=(VisualInferenceCallback& callback){
    m_callbacks1.insert(&callback);
}

bool VisualInferenceWait::run(){
    auto start = std::chrono::system_clock::now();
    auto timeout = start + m_timeout;
    auto next = start + m_period;
    while (true){
        m_env.check_stopping();

        QImage screen = m_feed.snapshot();
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

        for (auto& callback : m_callbacks0){
            if (callback(screen)){
                return true;
            }
        }
        for (VisualInferenceCallback* callback : m_callbacks1){
            if (callback->process_frame(screen, timestamp)){
                return true;
            }
        }

        auto now = std::chrono::system_clock::now();

        if (m_timeout != std::chrono::milliseconds(0) && now >= timeout){
            return false;
        }

        if (now >= next){
            next = now + m_period;
        }else{
            m_env.wait(next - now);
            next += m_period;
        }
    }
}



}


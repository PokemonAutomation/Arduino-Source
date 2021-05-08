/*  Async Visual Inference
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "VisualInferenceSession.h"


namespace PokemonAutomation{



VisualInferenceSession::VisualInferenceSession(
    ProgramEnvironment& env,
    VideoFeed& feed,
    std::chrono::milliseconds period
)
    : m_env(env)
    , m_feed(feed)
    , m_period(period)
    , m_stop(false)
{}
VisualInferenceSession::~VisualInferenceSession(){
    stop();
}
void VisualInferenceSession::stop(){
    m_stop.store(true, std::memory_order_release);
    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.notify_all();
}

void VisualInferenceSession::operator+=(std::function<bool(const QImage&)>&& callback){
    std::unique_lock<std::mutex> lg(m_lock);
    m_callbacks0.emplace_back(std::move(callback));
}
void VisualInferenceSession::operator+=(VisualInferenceCallback& callback){
    std::unique_lock<std::mutex> lg(m_lock);
    m_callbacks1.insert(&callback);
}
void VisualInferenceSession::operator-=(VisualInferenceCallback& callback){
    std::unique_lock<std::mutex> lg(m_lock);
    m_callbacks1.erase(&callback);
}

void VisualInferenceSession::run(){
    auto wait_until = std::chrono::system_clock::now();
    wait_until += m_period;
    while (true){
        m_env.check_stopping();
        if (m_stop.load(std::memory_order_acquire)){
            return;
        }

        QImage screen = m_feed.snapshot();
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

        std::unique_lock<std::mutex> lg(m_lock);
        for (auto& callback : m_callbacks0){
            if (callback(screen)){
                return;
            }
        }
        for (VisualInferenceCallback* callback : m_callbacks1){
            if (callback->process_frame(screen, timestamp)){
                return;
            }
        }

        auto now = std::chrono::system_clock::now();
        auto wait = wait_until - now;
        if (wait <= std::chrono::milliseconds(0)){
            wait_until = now + m_period;
        }else{
            m_cv.wait_for(
                lg, wait,
                [=]{
                    return
                        std::chrono::system_clock::now() >= wait_until ||
                        m_env.is_stopping() ||
                        m_stop.load(std::memory_order_acquire);
                }
            );
            wait_until += m_period;
        }
    }
}




VisualInferenceScope::VisualInferenceScope(
    VisualInferenceSession& session,
    VisualInferenceCallback& callback
)
    : m_session(session)
    , m_callback(callback)
{
    session += callback;
}
VisualInferenceScope::~VisualInferenceScope(){
    m_session -= m_callback;
}





AsyncVisualInferenceSession::AsyncVisualInferenceSession(
    ProgramEnvironment& env,
    VideoFeed& feed,
    std::chrono::milliseconds period
)
    : VisualInferenceSession(env, feed, period)
    , m_task(env.dispatcher().dispatch([this]{ thread_body(); }))
{}
AsyncVisualInferenceSession::~AsyncVisualInferenceSession(){
    stop();
}
void AsyncVisualInferenceSession::stop(){
    VisualInferenceSession::stop();
    if (m_task){
        m_task->wait();
    }
}
void AsyncVisualInferenceSession::thread_body(){
    try{
        run();
    }catch (CancelledException&){}
}






}

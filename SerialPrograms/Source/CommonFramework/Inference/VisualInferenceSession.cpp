/*  Async Visual Inference
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "VisualInferenceSession.h"


namespace PokemonAutomation{



VisualInferenceSession::VisualInferenceSession(
    ProgramEnvironment& env,
    VideoFeed& feed, VideoOverlay& overlay,
    std::chrono::milliseconds period
)
    : m_env(env)
    , m_feed(feed)
    , m_overlay(overlay)
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
    std::deque<InferenceBoxScope>& boxes = m_callbacks1[&callback];
    boxes.clear();
    callback.make_overlays(boxes, m_overlay);
}
void VisualInferenceSession::operator-=(VisualInferenceCallback& callback){
    std::unique_lock<std::mutex> lg(m_lock);
    m_callbacks1.erase(&callback);
}

VisualInferenceCallback* VisualInferenceSession::run(std::chrono::milliseconds timeout){
    auto now = std::chrono::system_clock::now();
    auto wait_until = now + m_period;
    auto stop_time = timeout == std::chrono::milliseconds(0)
        ? std::chrono::system_clock::time_point::max()
        : wait_until + timeout;
    return run(stop_time);
}
VisualInferenceCallback* VisualInferenceSession::run(std::chrono::system_clock::time_point stop){
    auto now = std::chrono::system_clock::now();
    auto wait_until = now + m_period;
    while (true){
        m_env.check_stopping();
        if (m_stop.load(std::memory_order_acquire)){
            return nullptr;
        }

        QImage screen = m_feed.snapshot();
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

        std::unique_lock<std::mutex> lg(m_lock);
        for (auto& callback : m_callbacks0){
            if (callback(screen)){
                return nullptr;
            }
        }
        for (auto& callback : m_callbacks1){
            if (callback.first->process_frame(screen, timestamp)){
                return callback.first;
            }
        }

        now = std::chrono::system_clock::now();
        if (now >= stop){
            return nullptr;
        }
        auto wait = wait_until - now;
        if (wait <= std::chrono::milliseconds(0)){
            wait_until = now + m_period;
        }else{
            m_cv.wait_for(
                lg, wait,
                [=]{
                    auto now = std::chrono::system_clock::now();
                    return
                        now >= stop ||
                        now >= wait_until ||
                        m_env.is_stopping() ||
                        m_stop.load(std::memory_order_acquire);
                }
            );
            wait_until += m_period;
        }
    }
}



#if 0
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
#endif




AsyncVisualInferenceSession::AsyncVisualInferenceSession(
    ProgramEnvironment& env,
    VideoFeed& feed, VideoOverlay& overlay,
    std::chrono::milliseconds period
)
    : VisualInferenceSession(env, feed, overlay, period)
    , m_callback(nullptr)
    , m_task(env.dispatcher().dispatch([this]{ thread_body(); }))
{}
AsyncVisualInferenceSession::~AsyncVisualInferenceSession(){
    VisualInferenceSession::stop();
}
VisualInferenceCallback* AsyncVisualInferenceSession::stop(){
    VisualInferenceSession::stop();
    if (m_task){
        m_task->wait_and_rethrow_exceptions();
    }
    return m_callback;
}
void AsyncVisualInferenceSession::thread_body(){
    try{
        m_callback = run();
    }catch (CancelledException&){}
}






}

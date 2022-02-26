/*  Async Visual Inference
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "VisualInferenceSession.h"

namespace PokemonAutomation{



struct VisualInferenceSession::Callback{
    VisualInferenceCallback* callback;
    VideoOverlaySet overlays;
    StatAccumulatorI32 stats;

    Callback(VideoOverlay& overlay, VisualInferenceCallback* p_callback)
        : callback(p_callback)
        , overlays(overlay)
    {}
};



VisualInferenceSession::VisualInferenceSession(
    ProgramEnvironment& env, Logger& logger,
    VideoFeed& feed, VideoOverlay& overlay,
    std::chrono::milliseconds period
)
    : m_env(env)
    , m_logger(logger)
    , m_feed(feed)
    , m_overlay(overlay)
    , m_period(period)
    , m_stop(false)
{}
VisualInferenceSession::~VisualInferenceSession(){
    stop();
}
void VisualInferenceSession::stop(){
    bool expected = false;
    if (!m_stop.compare_exchange_strong(expected, true)){
        return;
    }
    {
        std::unique_lock<std::mutex> lg(m_lock);
        m_cv.notify_all();
    }

    const double DIVIDER = std::chrono::milliseconds(1) / std::chrono::microseconds(1);
    const char* UNITS = " ms";


    m_stats_snapshot.log(m_logger, "Screenshot", UNITS, DIVIDER);
    for (Callback* callback : m_callback_list){
        callback->stats.log(m_logger, callback->callback->label(), UNITS, DIVIDER);
    }
}

void VisualInferenceSession::operator+=(VisualInferenceCallback& callback){
    std::unique_lock<std::mutex> lg(m_lock);

    auto iter = m_callback_map.find(&callback);
    if (iter != m_callback_map.end()){
        return;
    }

    Callback& entry = m_callback_map.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(&callback),
        std::forward_as_tuple(m_overlay, &callback)
    ).first->second;

    try{
        m_callback_list.emplace_back(&entry);
        entry.overlays.clear();
        callback.make_overlays(entry.overlays);
    }catch (...){
        m_callback_map.erase(&callback);
        if (!m_callback_list.empty() && m_callback_list.back() == &entry){
            m_callback_list.pop_back();
        }
        throw;
    }
}
void VisualInferenceSession::operator-=(VisualInferenceCallback& callback){
    std::unique_lock<std::mutex> lg(m_lock);
    auto iter0 = m_callback_map.find(&callback);
    if (iter0 == m_callback_map.end()){
        return;
    }
    auto iter1 = std::find(m_callback_list.begin(), m_callback_list.end(), &iter0->second);
    m_callback_list.erase(iter1);
    m_callback_map.erase(iter0);
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
    using WallClock = std::chrono::system_clock::time_point;

    WallClock now = std::chrono::system_clock::now();
    auto next_tick = now + m_period;

    while (true){
        m_env.check_stopping();
        if (m_stop.load(std::memory_order_acquire)){
            return nullptr;
        }

        WallClock time0_snapshot = std::chrono::system_clock::now();
        QImage screen = m_feed.snapshot();
        WallClock time1_snapshot = std::chrono::system_clock::now();
        m_stats_snapshot += std::chrono::duration_cast<std::chrono::microseconds>(time1_snapshot - time0_snapshot).count();

        std::unique_lock<std::mutex> lg(m_lock);
        for (Callback* callback : m_callback_list){
            WallClock time0 = std::chrono::system_clock::now();
            bool done = callback->callback->process_frame(screen, time1_snapshot);
            WallClock time1 = std::chrono::system_clock::now();
            callback->stats += std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
            if (done){
                return callback->callback;
            }
        }

        now = std::chrono::system_clock::now();
        if (now >= stop){
            return nullptr;
        }
        auto wait = next_tick - now;
        if (wait <= std::chrono::milliseconds(0)){
            next_tick = now + m_period;
        }else{
            WallClock stop_wait = std::min(next_tick, stop);
            m_cv.wait_until(
                lg, stop_wait,
                [=]{
                    return m_env.is_stopping() || m_stop.load(std::memory_order_acquire);
                }
            );
            next_tick += m_period;
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
    ProgramEnvironment& env, Logger& logger,
    VideoFeed& feed, VideoOverlay& overlay,
    std::chrono::milliseconds period
)
    : VisualInferenceSession(env, logger, feed, overlay, period)
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

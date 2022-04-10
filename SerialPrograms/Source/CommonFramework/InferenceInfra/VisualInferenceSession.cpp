/*  Async Visual Inference
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
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
    Logger& logger, CancellableScope& scope,
    VideoFeed& feed, VideoOverlay& overlay,
    std::chrono::milliseconds period
)
    : m_logger(logger)
    , m_feed(feed)
    , m_overlay(overlay)
    , m_period(period)
{
    attach(scope);
}
VisualInferenceSession::~VisualInferenceSession(){
    detach();
    VisualInferenceSession::cancel(nullptr);
}
bool VisualInferenceSession::cancel(std::exception_ptr exception) noexcept{
    if (Cancellable::cancel(std::move(exception))){
        return true;
    }
    {
        std::unique_lock<std::mutex> lg(m_lock);
        m_cv.notify_all();
    }

    const double DIVIDER = std::chrono::milliseconds(1) / std::chrono::microseconds(1);
    const char* UNITS = " ms";
    try{
        m_stats_snapshot.log(m_logger, "Screenshot", UNITS, DIVIDER);
        for (Callback* callback : m_callback_list){
            callback->stats.log(m_logger, callback->callback->label(), UNITS, DIVIDER);
        }
    }catch (...){}
    return false;
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
    auto now = current_time();
    auto wait_until = now + m_period;
    auto stop_time = timeout == std::chrono::milliseconds(0)
        ? WallClock::max()
        : wait_until + timeout;
    return run(stop_time);
}
VisualInferenceCallback* VisualInferenceSession::run(WallClock stop){
    WallClock now = current_time();
    auto next_tick = now + m_period;

    while (true){
        if (throw_if_cancelled_with_exception()){
            return nullptr;
        }

        WallClock time0_snapshot = current_time();
        WallClock timestamp;
        QImage screen = m_feed.snapshot(&timestamp);
        WallClock time1_snapshot = current_time();
        m_stats_snapshot += std::chrono::duration_cast<std::chrono::microseconds>(time1_snapshot - time0_snapshot).count();

        std::unique_lock<std::mutex> lg(m_lock);
        for (Callback* callback : m_callback_list){
            WallClock time0 = current_time();
            bool done = callback->callback->process_frame(screen, timestamp);
            WallClock time1 = current_time();
            callback->stats += std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
            if (done){
                return callback->callback;
            }
        }

        now = current_time();
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
                [=]{ return cancelled(); }
            );
            next_tick += m_period;
        }
    }
}



AsyncVisualInferenceSession::AsyncVisualInferenceSession(
    ProgramEnvironment& env, Logger& logger, CancellableScope& scope,
    VideoFeed& feed, VideoOverlay& overlay,
    std::chrono::milliseconds period
)
    : m_session(logger, scope, feed, overlay, period)
    , m_triggering_callback(nullptr)
    , m_task(env.inference_dispatcher().dispatch([this]{ thread_body(); }))
{}
AsyncVisualInferenceSession::AsyncVisualInferenceSession(
    ProgramEnvironment& env, Logger& logger, CancellableScope& scope,
    VideoFeed& feed, VideoOverlay& overlay,
    std::function<void()> on_finish_callback,
    std::chrono::milliseconds period
)
    : m_session(logger, scope, feed, overlay, period)
    , m_on_finish_callback(std::move(on_finish_callback))
    , m_triggering_callback(nullptr)
    , m_task(env.inference_dispatcher().dispatch([this]{ thread_body(); }))
{}
AsyncVisualInferenceSession::~AsyncVisualInferenceSession(){
    m_session.cancel(nullptr);
}
void AsyncVisualInferenceSession::operator+=(VisualInferenceCallback& callback){
    m_session += callback;
}
void AsyncVisualInferenceSession::operator-=(VisualInferenceCallback& callback){
    m_session -= callback;
}
void AsyncVisualInferenceSession::rethrow_exceptions(){
    if (m_task){
        m_task->rethrow_exceptions();
    }
}
VisualInferenceCallback* AsyncVisualInferenceSession::stop_and_rethrow(){
    m_session.cancel(nullptr);
    if (m_task){
        m_task->wait_and_rethrow_exceptions();
    }
    return m_triggering_callback;
}
void AsyncVisualInferenceSession::thread_body(){
    try{
        m_triggering_callback = m_session.run();
        if (m_on_finish_callback){
            m_on_finish_callback();
        }
//        m_session.cancel(std::current_exception());
    }catch (...){
        if (m_on_finish_callback){
            m_on_finish_callback();
        }
//        m_session.cancel(std::current_exception());
        throw;
    }
}






}

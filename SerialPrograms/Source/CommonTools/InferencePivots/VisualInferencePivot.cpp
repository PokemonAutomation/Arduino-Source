/*  Visual Inference Pivot
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "VisualInferencePivot.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



struct VisualInferencePivot::PeriodicCallback{
    Cancellable& scope;
    std::atomic<InferenceCallback*>* set_when_triggered;
    VisualInferenceCallback& callback;
    std::chrono::milliseconds period;
    StatAccumulatorI32 stats;
    WallClock last_timestamp;

    PeriodicCallback(
        Cancellable& p_scope,
        std::atomic<InferenceCallback*>* p_set_when_triggered,
        VisualInferenceCallback& p_callback,
        std::chrono::milliseconds p_period
    )
        : scope(p_scope)
        , set_when_triggered(p_set_when_triggered)
        , callback(p_callback)
        , period(p_period)
        , last_timestamp(WallClock::min())
    {}
};



VisualInferencePivot::VisualInferencePivot(CancellableScope& scope, VideoFeed& feed, AsyncDispatcher& dispatcher)
    : PeriodicRunner(dispatcher)
    , m_feed(feed)
{
    attach(scope);
}
VisualInferencePivot::~VisualInferencePivot(){
    detach();
    stop_thread();
}
void VisualInferencePivot::add_callback(
    Cancellable& scope,
    std::atomic<InferenceCallback*>* set_when_triggered,
    VisualInferenceCallback& callback,
    std::chrono::milliseconds period
){
    WriteSpinLock lg(m_lock);
    auto iter = m_map.find(&callback);
    if (iter != m_map.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to add the same callback twice.");
    }
    iter = m_map.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(&callback),
        std::forward_as_tuple(scope, set_when_triggered, callback, period)
    ).first;
    try{
        PeriodicRunner::add_event(&iter->second, period);
    }catch (...){
        m_map.erase(iter);
        throw;
    }
}
StatAccumulatorI32 VisualInferencePivot::remove_callback(VisualInferenceCallback& callback){
    WriteSpinLock lg(m_lock);
    auto iter = m_map.find(&callback);
    if (iter == m_map.end()){
        return StatAccumulatorI32();
    }
    StatAccumulatorI32 stats = iter->second.stats;
    PeriodicRunner::remove_event(&iter->second);
    m_map.erase(iter);
    return stats;
}
void VisualInferencePivot::run(void* event, bool is_back_to_back) noexcept{
    PeriodicCallback& callback = *(PeriodicCallback*)event;
    try{
        //  Reuse the cached screenshot.
        if (!is_back_to_back || callback.last_timestamp == m_last.timestamp){
//            cout << "back-to-back" << endl;
//            m_last = m_feed.snapshot();

            WallClock min_time = callback.last_timestamp;
            if (min_time == WallClock::min()){
                min_time = current_time() - 2 * callback.period;
            }

            m_last = m_feed.snapshot_recent_nonblocking(min_time);
        }

        if (!m_last){
            return;
        }

        WallClock time0 = current_time();
        bool stop = callback.callback.process_frame(m_last);
        WallClock time1 = current_time();
        callback.stats += (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
        callback.last_timestamp = m_last.timestamp;

        if (stop){
            if (callback.set_when_triggered){
                InferenceCallback* expected = nullptr;
                callback.set_when_triggered->compare_exchange_strong(expected, &callback.callback);
            }
            callback.scope.cancel(nullptr);
        }
    }catch (...){
        callback.scope.cancel(std::current_exception());
    }
}


OverlayStatSnapshot VisualInferencePivot::get_current(){
    return m_printer.get_snapshot("Video Pivot Utilization:", this->current_utilization());
}




}

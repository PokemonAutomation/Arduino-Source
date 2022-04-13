/*  Visual Inference Pivot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "VisualInferencePivot.h"

namespace PokemonAutomation{



struct VisualInferencePivot::PeriodicCallback{
    Cancellable& scope;
    VisualInferenceCallback& callback;
    std::chrono::milliseconds period;
    StatAccumulatorI32 stats;

    PeriodicCallback(
        Cancellable& p_scope,
        VisualInferenceCallback& p_callback,
        std::chrono::milliseconds p_period
    )
        : scope(p_scope)
        , callback(p_callback)
        , period(p_period)
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
void VisualInferencePivot::add_callback(Cancellable& scope, VisualInferenceCallback& callback, std::chrono::milliseconds period){
    SpinLockGuard lg(m_lock);
    auto iter = m_map.find(&callback);
    if (iter != m_map.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to add the same callback twice.");
    }
    iter = m_map.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(&callback),
        std::forward_as_tuple(scope, callback, period)
    ).first;
    try{
        PeriodicRunner::add_event(&iter->second, period);
    }catch (...){
        m_map.erase(iter);
        throw;
    }
}
StatAccumulatorI32 VisualInferencePivot::remove_callback(VisualInferenceCallback& callback){
    SpinLockGuard lg(m_lock);
    auto iter = m_map.find(&callback);
    if (iter == m_map.end()){
        return StatAccumulatorI32();
    }
    StatAccumulatorI32 stats = iter->second.stats;
    PeriodicRunner::remove_event(&iter->second);
    m_map.erase(iter);
    return stats;
}
void VisualInferencePivot::run(void* event) noexcept{
    PeriodicCallback& callback = *(PeriodicCallback*)event;
    try{
        WallClock timestamp;
        QImage frame = m_feed.snapshot(&timestamp);
        WallClock time0 = current_time();
        bool stop = callback.callback.process_frame(frame, timestamp);
        WallClock time1 = current_time();
        callback.stats += std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
        if (stop){
            callback.scope.cancel(nullptr);
        }
    }catch (...){
        callback.scope.cancel(std::current_exception());
    }
}




}

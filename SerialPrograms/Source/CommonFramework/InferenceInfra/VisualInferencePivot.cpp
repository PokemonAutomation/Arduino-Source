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
    iter = m_map.emplace(&callback, PeriodicVisualCallback{callback, period, scope}).first;
    try{
        PeriodicRunner::add_event(&iter->second, period);
    }catch (...){
        m_map.erase(iter);
        throw;
    }
}
void VisualInferencePivot::remove_callback(VisualInferenceCallback& callback){
    SpinLockGuard lg(m_lock);
    auto iter = m_map.find(&callback);
    if (iter == m_map.end()){
        return;
    }
    PeriodicRunner::remove_event(&iter->second);
    m_map.erase(iter);
}
void VisualInferencePivot::run(void* event) noexcept{
    PeriodicVisualCallback& callback = *(PeriodicVisualCallback*)event;
    try{
        WallClock timestamp;
        QImage frame = m_feed.snapshot(&timestamp);
        if (callback.callback.process_frame(frame, timestamp)){
            callback.scope.cancel(nullptr);
        }
    }catch (...){
        callback.scope.cancel(std::current_exception());
    }
}




}

/*  Audio Inference Pivot
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "AudioInferencePivot.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



struct AudioInferencePivot::PeriodicCallback{
    Cancellable& scope;
    std::atomic<InferenceCallback*>* set_when_triggered;
    AudioInferenceCallback& callback;
    std::chrono::milliseconds period;

    uint64_t last_seqnum = ~(uint64_t)0;

    StatAccumulatorI32 stats;

    PeriodicCallback(
        Cancellable& p_scope,
        std::atomic<InferenceCallback*>* p_set_when_triggered,
        AudioInferenceCallback& p_callback,
        std::chrono::milliseconds p_period
    )
        : scope(p_scope)
        , set_when_triggered(p_set_when_triggered)
        , callback(p_callback)
        , period(p_period)
    {}
};


AudioInferencePivot::AudioInferencePivot(CancellableScope& scope, AudioFeed& feed, AsyncDispatcher& dispatcher)
    : PeriodicRunner(dispatcher)
    , m_feed(feed)
{
    attach(scope);
}
AudioInferencePivot::~AudioInferencePivot(){
    detach();
    stop_thread();
}
void AudioInferencePivot::add_callback(
    Cancellable& scope,
    std::atomic<InferenceCallback*>* set_when_triggered,
    AudioInferenceCallback& callback,
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
StatAccumulatorI32 AudioInferencePivot::remove_callback(AudioInferenceCallback& callback){
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
void AudioInferencePivot::run(void* event, bool is_back_to_back) noexcept{
    PeriodicCallback& callback = *(PeriodicCallback*)event;
    try{
        std::vector<AudioSpectrum> spectrums;

        if (callback.last_seqnum == ~(uint64_t)0){
//            cout << "m_last_timestamp == SIZE_MAX" << endl;
            spectrums = m_feed.spectrums_latest(1);
        }else{
//            cout << "(m_last_timestamp != SIZE_MAX" << endl;
            //  Note: in this file we never consider the case that stamp may overflow.
            //  It requires on the order of 1e10 years to overflow if we have about 25ms per stamp.
            spectrums = m_feed.spectrums_since(callback.last_seqnum + 1);
        }
        if (spectrums.size() > 0){
            //  spectrums[0] has the newest spectrum with the largest stamp:
            callback.last_seqnum = spectrums[0].stamp;
        }

        WallClock time0 = current_time();
        bool stop = callback.callback.process_spectrums(spectrums, m_feed);
        WallClock time1 = current_time();
        callback.stats += (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
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


OverlayStatSnapshot AudioInferencePivot::get_current(){
    return m_printer.get_snapshot("Audio Pivot Utilization:", this->current_utilization());
}




}

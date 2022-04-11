/*  Audio Inference Pivot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/AudioFeed.h"
#include "AudioInferencePivot.h"

namespace PokemonAutomation{



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
void AudioInferencePivot::add_callback(Cancellable& scope, AudioInferenceCallback& callback, std::chrono::milliseconds period){
    SpinLockGuard lg(m_lock);
    auto iter = m_map.find(&callback);
    if (iter != m_map.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to add the same callback twice.");
    }
    iter = m_map.emplace(&callback, PeriodicAudioCallback{callback, period, scope}).first;
    try{
        PeriodicRunner::add_event(&iter->second, period);
    }catch (...){
        m_map.erase(iter);
        throw;
    }
}
void AudioInferencePivot::remove_callback(AudioInferenceCallback& callback){
    SpinLockGuard lg(m_lock);
    auto iter = m_map.find(&callback);
    if (iter == m_map.end()){
        return;
    }
    PeriodicRunner::remove_event(&iter->second);
    m_map.erase(iter);
}
void AudioInferencePivot::run(void* event) noexcept{
    PeriodicAudioCallback& callback = *(PeriodicAudioCallback*)event;
    try{
        std::vector<AudioSpectrum> spectrums;

        if (m_last_timestamp == SIZE_MAX){
            spectrums = m_feed.spectrums_latest(1);
        } else{
            //  Note: in this file we never consider the case that stamp may overflow.
            //  It requires on the order of 1e10 years to overflow if we have about 25ms per stamp.
            spectrums = m_feed.spectrums_since(m_last_timestamp + 1);
        }
        if (spectrums.size() > 0){
            //  spectrums[0] has the newest spectrum with the largest stamp:
            m_last_timestamp = spectrums[0].stamp;
        }

        if (callback.callback.process_spectrums(spectrums, m_feed)){
            callback.scope.cancel(nullptr);
        }
    }catch (...){
        callback.scope.cancel(std::current_exception());
    }
}




}

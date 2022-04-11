/*  Audio Inference Pivot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_AudioInferencePivot_H
#define PokemonAutomation_CommonFramework_AudioInferencePivot_H

#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/PeriodicScheduler.h"
#include "AudioInferenceCallback.h"

namespace PokemonAutomation{

class AudioFeed;


struct PeriodicAudioCallback{
    AudioInferenceCallback& callback;
    std::chrono::milliseconds period;
    Cancellable& scope;
};



class AudioInferencePivot final : public PeriodicRunner{
public:
    AudioInferencePivot(CancellableScope& scope, AudioFeed& feed, AsyncDispatcher& dispatcher);
    virtual ~AudioInferencePivot();

    //  Returns true if event was successfully added.
    void add_callback(Cancellable& scope, AudioInferenceCallback& callback, std::chrono::milliseconds period);
    void remove_callback(AudioInferenceCallback& callback);

private:
    virtual void run(void* event) noexcept override;

private:
    AudioFeed& m_feed;
    SpinLock m_lock;
    std::map<AudioInferenceCallback*, PeriodicAudioCallback> m_map;

    uint64_t m_last_timestamp = ~(uint64_t)0;
};



}
#endif

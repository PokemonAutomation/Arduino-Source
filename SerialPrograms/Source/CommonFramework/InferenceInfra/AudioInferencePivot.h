/*  Audio Inference Pivot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_AudioInferencePivot_H
#define PokemonAutomation_CommonFramework_AudioInferencePivot_H

#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/PeriodicScheduler.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "AudioInferenceCallback.h"

namespace PokemonAutomation{

class AudioFeed;



class AudioInferencePivot final : public PeriodicRunner{
public:
    AudioInferencePivot(CancellableScope& scope, AudioFeed& feed, AsyncDispatcher& dispatcher);
    virtual ~AudioInferencePivot();

    //  Returns true if event was successfully added.
    void add_callback(Cancellable& scope, AudioInferenceCallback& callback, std::chrono::milliseconds period);

    //  Returns the latency stats for the callback. Units are milliseconds.
    StatAccumulatorI32 remove_callback(AudioInferenceCallback& callback);

private:
    virtual void run(void* event) noexcept override;

private:
    struct PeriodicCallback;

    AudioFeed& m_feed;
    SpinLock m_lock;
    std::map<AudioInferenceCallback*, PeriodicCallback> m_map;

    uint64_t m_last_timestamp = ~(uint64_t)0;
};



}
#endif

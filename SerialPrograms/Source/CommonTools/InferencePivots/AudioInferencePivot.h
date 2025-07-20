/*  Audio Inference Pivot
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_AudioInferencePivot_H
#define PokemonAutomation_CommonTools_AudioInferencePivot_H

#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/PeriodicScheduler.h"
#include "CommonFramework/Tools/StatAccumulator.h"
#include "CommonFramework/VideoPipeline/VideoOverlayTypes.h"
#include "CommonTools/InferenceCallbacks/AudioInferenceCallback.h"

namespace PokemonAutomation{

class AudioFeed;



class AudioInferencePivot final : public PeriodicRunner, public OverlayStat{
public:
    AudioInferencePivot(CancellableScope& scope, AudioFeed& feed, AsyncDispatcher& dispatcher);
    virtual ~AudioInferencePivot();

    //  If this callback returns true:
    //      1.  Cancel "scope".
    //      2.  Set "set_when_triggered" to the callback.
    //  If the callback throws an exception, "scope" will be cancelled with that exception.
    void add_callback(
        Cancellable& scope,
        std::atomic<InferenceCallback*>* set_when_triggered,
        AudioInferenceCallback& callback,
        std::chrono::milliseconds period
    );

    //  Returns the latency stats for the callback. Units are microseconds.
    StatAccumulatorI32 remove_callback(AudioInferenceCallback& callback);

private:
    virtual void run(void* event, bool is_back_to_back) noexcept override;
    virtual OverlayStatSnapshot get_current() override;

private:
    struct PeriodicCallback;

    AudioFeed& m_feed;
    SpinLock m_lock;
    std::map<AudioInferenceCallback*, PeriodicCallback> m_map;

//    uint64_t m_last_seqnum = ~(uint64_t)0;

    OverlayStatUtilizationPrinter m_printer;
};



}
#endif

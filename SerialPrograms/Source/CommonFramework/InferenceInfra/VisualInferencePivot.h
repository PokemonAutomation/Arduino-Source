/*  Visual Inference Pivot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualInferencePivot_H
#define PokemonAutomation_CommonFramework_VisualInferencePivot_H

#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/PeriodicScheduler.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayTypes.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "VisualInferenceCallback.h"

namespace PokemonAutomation{

class VideoFeed;



class VisualInferencePivot final : public PeriodicRunner, public OverlayStat{
public:
    VisualInferencePivot(CancellableScope& scope, VideoFeed& feed, AsyncDispatcher& dispatcher);
    virtual ~VisualInferencePivot();

    //  If this callback returns true:
    //      1.  Cancel "scope".
    //      2.  Set "set_when_triggered" to the callback.
    //  If the callback throws an exception, "scope" will be cancelled with that exception.
    void add_callback(
        Cancellable& scope,
        std::atomic<InferenceCallback*>* set_when_triggered,
        VisualInferenceCallback& callback,
        std::chrono::milliseconds period
    );

    //  Returns the latency stats for the callback. Units are microseconds.
    StatAccumulatorI32 remove_callback(VisualInferenceCallback& callback);

private:
    virtual void run(void* event, bool is_back_to_back) noexcept override;
    virtual OverlayStatSnapshot get_current() override;

private:
    struct PeriodicCallback;

    VideoFeed& m_feed;
    SpinLock m_lock;
    std::map<VisualInferenceCallback*, PeriodicCallback> m_map;
    VideoSnapshot m_last;
    uint64_t m_seqnum = 0;

    OverlayStatUtilizationPrinter m_printer;
};



}
#endif

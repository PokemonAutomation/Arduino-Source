/*  Visual Inference Pivot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualInferencePivot_H
#define PokemonAutomation_CommonFramework_VisualInferencePivot_H

#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/PeriodicScheduler.h"
#include "VisualInferenceCallback.h"

namespace PokemonAutomation{

class VideoFeed;


struct PeriodicVisualCallback{
    VisualInferenceCallback& callback;
    std::chrono::milliseconds period;
    Cancellable& scope;
};



class VisualInferencePivot final : public PeriodicRunner{
public:
    VisualInferencePivot(CancellableScope& scope, VideoFeed& feed, AsyncDispatcher& dispatcher);
    virtual ~VisualInferencePivot();

    //  Returns true if event was successfully added.
    void add_callback(Cancellable& scope, VisualInferenceCallback& callback, std::chrono::milliseconds period);
    void remove_callback(VisualInferenceCallback& callback);

private:
    virtual void run(void* event) noexcept override;

private:
    VideoFeed& m_feed;
    SpinLock m_lock;
    std::map<VisualInferenceCallback*, PeriodicVisualCallback> m_map;
};



}
#endif

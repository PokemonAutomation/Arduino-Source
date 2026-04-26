/*  Inference Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_InferenceSession_H
#define PokemonAutomation_CommonTools_InferenceSession_H

#include <chrono>
#include <vector>
#include <map>
#include <atomic>
#include "Common/Cpp/CancellableScope.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/InferenceCallback.h"

namespace PokemonAutomation{

class VideoStream;



//
//  On construction, all the callbacks are attached to inference threads and
//  begin running at their specified period.
//
//  Upon destruction, all callbacks are removed from the inference threads and
//  stop running.
//
//  When a callback returns true or throws an exception, "scope.cancel()" will
//  be called.
//
//  "triggered_ptr()/triggered_index()" gives the first callback that
//  returned true. If nothing has returned true, it is null or -1 respectively.
//
//  Note that the callbacks will continue running even if one of them has
//  returned true. So it is expected that the object will be destructed soon
//  after "scope.cancel()" is called.
//
class InferenceSession : public Cancellable::CancelListener{
public:
    InferenceSession(
        Cancellable& scope, VideoStream& stream,
        const std::vector<PeriodicInferenceCallback>& callbacks,
        std::chrono::milliseconds default_video_period = std::chrono::milliseconds(50),
        std::chrono::milliseconds default_audio_period = std::chrono::milliseconds(20)
    );
    ~InferenceSession();

    InferenceCallback* triggered_ptr() const;
    int triggered_index() const;


private:
    void clear() noexcept;

    virtual void on_cancellable_cancel(
        Cancellable& cancellable,
        std::exception_ptr reason
    ) override;


private:
    Cancellable& m_scope;
    VideoStream& m_stream;
    VideoOverlaySet m_overlays;
    std::map<InferenceCallback*, size_t> m_map;
    std::atomic<InferenceCallback*> m_triggered;
};





}
#endif

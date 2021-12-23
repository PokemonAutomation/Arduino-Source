/*  Visual Inference Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualInferenceSession_H
#define PokemonAutomation_CommonFramework_VisualInferenceSession_H

#include <map>
#include "Common/Cpp/AsyncDispatcher.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "VisualInferenceCallback.h"

namespace PokemonAutomation{



class VisualInferenceSession{
public:
    VisualInferenceSession(
        ProgramEnvironment& env,
        VideoFeed& feed, VideoOverlay& overlay,
        std::chrono::milliseconds period = std::chrono::milliseconds(50)
    );
    ~VisualInferenceSession();

    void operator+=(std::function<bool(const QImage&)>&& callback);
    void operator+=(VisualInferenceCallback& callback);
    void operator-=(VisualInferenceCallback& callback);

    //  Run the session. This will not return until the session is stopped.
    VisualInferenceCallback* run(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    VisualInferenceCallback* run(std::chrono::system_clock::time_point stop);

    //  Call this from a different thread to asynchronously stop the session.
    void stop();

private:
    ProgramEnvironment& m_env;
    VideoFeed& m_feed;
    VideoOverlay& m_overlay;
    std::chrono::milliseconds m_period;
    std::atomic<bool> m_stop;

    std::vector<std::function<bool(const QImage&)>> m_callbacks0;
    std::map<VisualInferenceCallback*, OverlaySet> m_callbacks1;

    std::mutex m_lock;
    std::condition_variable m_cv;
};



#if 0
//  RAII wrapper for adding/removing infererence callbacks.
class VisualInferenceScope{
public:
    VisualInferenceScope(
        VisualInferenceSession& session,
        VisualInferenceCallback& callback
    );
    ~VisualInferenceScope();
private:
    VisualInferenceSession& m_session;
    VisualInferenceCallback& m_callback;
};
#endif




class AsyncVisualInferenceSession : public VisualInferenceSession{
public:
    AsyncVisualInferenceSession(
        ProgramEnvironment& env,
        VideoFeed& feed, VideoOverlay& overlay,
        std::chrono::milliseconds period = std::chrono::milliseconds(50)
    );

    //  This will not rethrow exceptions in the inference thread.
    ~AsyncVisualInferenceSession();

    //  This will rethrow any exceptions in the inference thread.
    //  You should call this at all natural destruction points.
    VisualInferenceCallback* stop();

private:
    void thread_body();

private:
    VisualInferenceCallback* m_callback;
    std::unique_ptr<AsyncTask> m_task;
};




}
#endif

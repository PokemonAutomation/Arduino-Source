/*  Visual Inference Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualInferenceSession_H
#define PokemonAutomation_CommonFramework_VisualInferenceSession_H

#include <set>
#include "Common/Clientside/AsyncDispatcher.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "VisualInferenceCallback.h"

namespace PokemonAutomation{



class VisualInferenceSession{
public:
    VisualInferenceSession(
        ProgramEnvironment& env,
        VideoFeed& feed,
        std::chrono::milliseconds period = std::chrono::milliseconds(50)
    );
    ~VisualInferenceSession();

    void operator+=(std::function<bool(const QImage&)>&& callback);
    void operator+=(VisualInferenceCallback& callback);
    void operator-=(VisualInferenceCallback& callback);

    //  Run the session. This will not return until the session is stopped.
    void run();

    //  Call this from a different thread to asynchronously stop the session.
    void stop();

private:
    ProgramEnvironment& m_env;
    VideoFeed& m_feed;
    std::chrono::milliseconds m_period;
    std::atomic<bool> m_stop;
    std::vector<std::function<bool(const QImage&)>> m_callbacks0;
    std::set<VisualInferenceCallback*> m_callbacks1;
    std::mutex m_lock;
    std::condition_variable m_cv;
};



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




class AsyncVisualInferenceSession : public VisualInferenceSession{
public:
    AsyncVisualInferenceSession(
        ProgramEnvironment& env,
        VideoFeed& feed,
        std::chrono::milliseconds period = std::chrono::milliseconds(50)
    );
    ~AsyncVisualInferenceSession();

    void stop();

private:
    void thread_body();

private:
    std::unique_ptr<AsyncTask> m_task;
};




}
#endif

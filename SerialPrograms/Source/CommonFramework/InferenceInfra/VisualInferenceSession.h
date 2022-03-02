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
#include "CommonFramework/Inference/StatAccumulator.h"
#include "VisualInferenceCallback.h"

namespace PokemonAutomation{



class VisualInferenceSession{
public:
    VisualInferenceSession(
        ProgramEnvironment& env, LoggerQt& logger,
        VideoFeed& feed, VideoOverlay& overlay,
        std::chrono::milliseconds period = std::chrono::milliseconds(50)
    );
    ~VisualInferenceSession();

    void operator+=(VisualInferenceCallback& callback);
    void operator-=(VisualInferenceCallback& callback);

    //  Run the session. This will not return until the session is stopped.
    VisualInferenceCallback* run(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    VisualInferenceCallback* run(std::chrono::system_clock::time_point stop);

    //  Call this from a different thread to asynchronously stop the session.
    void stop();

private:
    struct Callback;

    ProgramEnvironment& m_env;
    LoggerQt& m_logger;
    VideoFeed& m_feed;
    VideoOverlay& m_overlay;
    std::chrono::milliseconds m_period;
    std::atomic<bool> m_stop;

    std::vector<Callback*> m_callback_list;
    std::map<VisualInferenceCallback*, Callback> m_callback_map;

    std::mutex m_lock;
    std::condition_variable m_cv;

    StatAccumulatorI32 m_stats_snapshot;
};



class AsyncVisualInferenceSession : private VisualInferenceSession{
public:
    AsyncVisualInferenceSession(
        ProgramEnvironment& env, LoggerQt& logger,
        VideoFeed& feed, VideoOverlay& overlay,
        std::chrono::milliseconds period = std::chrono::milliseconds(50)
    );
    AsyncVisualInferenceSession(
        ProgramEnvironment& env, LoggerQt& logger,
        VideoFeed& feed, VideoOverlay& overlay,
        std::function<void()> on_finish_callback,
        std::chrono::milliseconds period = std::chrono::milliseconds(50)
    );

    //  This will not rethrow exceptions in the inference thread.
    ~AsyncVisualInferenceSession();

    using VisualInferenceSession::operator+=;
    using VisualInferenceSession::operator-=;

    //  Check if the thread died from an exception. If so, rethrow it.
    void rethrow_exceptions();

    //  This will rethrow any exceptions in the inference thread.
    //  You should call this at all natural destruction points.
    VisualInferenceCallback* stop();

private:
    void thread_body();

private:
    std::function<void()> m_on_finish_callback;
    VisualInferenceCallback* m_triggering_callback;
    std::unique_ptr<AsyncTask> m_task;
};




}
#endif

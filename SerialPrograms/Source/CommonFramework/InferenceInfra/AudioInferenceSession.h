/*  Audio Inference Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_AudioInferenceSession_H
#define PokemonAutomation_CommonFramework_AudioInferenceSession_H

#include <map>
#include "Common/Cpp/AsyncDispatcher.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/AudioFeed.h"
#include "AudioInferenceCallback.h"

namespace PokemonAutomation{



class AudioInferenceSession{
public:
    AudioInferenceSession(
        ProgramEnvironment& env, LoggerQt& logger,
        AudioFeed& feed,
        std::chrono::milliseconds period = std::chrono::milliseconds(20)
    );
    ~AudioInferenceSession();

    void operator+=(AudioInferenceCallback& callback);
    void operator-=(AudioInferenceCallback& callback);

    //  Run the session. This will not return until the session is stopped.
    AudioInferenceCallback* run(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    AudioInferenceCallback* run(std::chrono::system_clock::time_point stop);

    //  Call this from a different thread to asynchronously stop the session.
    void stop();

private:
    struct Callback;

    ProgramEnvironment& m_env;
    LoggerQt& m_logger;
    AudioFeed& m_feed;
    std::chrono::milliseconds m_period;
    std::atomic<bool> m_stop;

    std::vector<Callback*> m_callback_list;
    std::map<AudioInferenceCallback*, Callback> m_callback_map;

    std::mutex m_lock;
    std::condition_variable m_cv;
};


class AsyncAudioInferenceSession : private AudioInferenceSession{
public:
    AsyncAudioInferenceSession(
        ProgramEnvironment& env, LoggerQt& logger,
        AudioFeed& feed,
        std::chrono::milliseconds period = std::chrono::milliseconds(50)
    );
    AsyncAudioInferenceSession(
        ProgramEnvironment& env, LoggerQt& logger,
        AudioFeed& feed,
        std::function<void()> on_finish_callback,
        std::chrono::milliseconds period = std::chrono::milliseconds(50)
    );

    //  This will not rethrow exceptions in the inference thread.
    ~AsyncAudioInferenceSession();

    using AudioInferenceSession::operator+=;
    using AudioInferenceSession::operator-=;

    //  Check if the thread died from an exception. If so, rethrow it.
    void rethrow_exceptions();

    //  This will rethrow any exceptions in the inference thread.
    //  You should call this at all natural destruction points.
    AudioInferenceCallback* stop();

private:
    void thread_body();

private:
    std::function<void()> m_on_finish_callback;
    AudioInferenceCallback* m_triggering_callback;
    std::unique_ptr<AsyncTask> m_task;
};




}
#endif

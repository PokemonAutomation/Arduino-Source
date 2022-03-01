/*  Visual Inference Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "VisualInferenceSession.h"
#include "AudioInferenceSession.h"
#include "InferenceRoutines.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


int wait_until(
    ProgramEnvironment& env, ConsoleHandle& console,
    std::chrono::milliseconds timeout,
    std::vector<InferenceCallback*>&& callbacks,
    std::chrono::milliseconds period
){
    return wait_until(
        env, console,
        std::chrono::system_clock::now() + timeout,
        std::move(callbacks),
        period
    );
}
int wait_until(
    ProgramEnvironment& env, ConsoleHandle& console,
    std::chrono::system_clock::time_point deadline,
    std::vector<InferenceCallback*>&& callbacks,
    std::chrono::milliseconds period
){
    std::map<InferenceCallback*, size_t> map;
    InferenceCallback* trigger = nullptr;
    {
        //  Define the threads first so they get destructed *after* the sessions
        //  are destructed.

        //  Destroying the sessions will automatically stop the threads. If you
        //  try to destroy the threads first without destroying the sessions,
        //  they will deadlock waiting for the session to finish.

        std::unique_ptr<AsyncTask> visual_thread;
        std::unique_ptr<AsyncTask> audio_thread;

        std::unique_ptr<VisualInferenceSession> visual_session;
        std::unique_ptr<AudioInferenceSession> audio_session;

        //  Add all the callbacks. Lazy init the sessions only when needed.
        for (size_t c = 0; c < callbacks.size(); c++){
            InferenceCallback* callback = callbacks[c];
            if (callback == nullptr){
                continue;
            }
            map[callback] = c;
            switch (callback->type()){
            case InferenceType::VISUAL:
                if (visual_session == nullptr){
                    visual_session.reset(new VisualInferenceSession(env, console, console, console, period));
                }
                *visual_session += static_cast<VisualInferenceCallback&>(*callback);
                break;
            case InferenceType::AUDIO:
                if (audio_session == nullptr){
                    audio_session.reset(new AudioInferenceSession(env, console, console, period));
                }
                *audio_session += static_cast<AudioInferenceCallback&>(*callback);
            }
        }

        std::mutex lock;
        std::condition_variable cv;
        ProgramStopNotificationScope stopper(env, lock, cv);
        bool stopped = false;

        //  Start the inference threads.
        if (visual_session){
            visual_thread = env.dispatcher().dispatch([&]{
                try{
                    trigger = visual_session->run();
                    std::lock_guard<std::mutex> lg(lock);
                    stopped = true;
                    cv.notify_all();
                }catch (...){
                    std::lock_guard<std::mutex> lg(lock);
                    stopped = true;
                    cv.notify_all();
                    throw;
                }
            });
        }
        if (audio_session){
            audio_thread = env.dispatcher().dispatch([&]{
                try{
                    trigger = audio_session->run();
                    std::lock_guard<std::mutex> lg(lock);
                    stopped = true;
                    cv.notify_all();
                }catch (...){
                    std::lock_guard<std::mutex> lg(lock);
                    stopped = true;
                    cv.notify_all();
                    throw;
                }
            });
        }


        //  Wait
        {
            std::unique_lock<std::mutex> lg(lock);
            cv.wait_until(lg, deadline, [&](){ return stopped; });
        }

        //  Join the inference threads.
        if (visual_session){
            visual_session->stop();
            visual_thread->wait_and_rethrow_exceptions();
        }
        if (audio_session){
            audio_session->stop();
            audio_thread->wait_and_rethrow_exceptions();
        }
    }

    //  Lookup which callback triggered.
    auto iter = map.find(trigger);
    return iter == map.end() ? -1 : (int)iter->second;
}




int run_until(
    ProgramEnvironment& env, ConsoleHandle& console,
    std::function<void(const BotBaseContext& context)>&& command,
    std::vector<InferenceCallback*>&& callbacks,
    std::chrono::milliseconds period
){
    std::map<InferenceCallback*, size_t> map;
    InferenceCallback* trigger = nullptr;
    {
        //  Define the threads first so they get destructed *after* the sessions
        //  are destructed.

        //  Destroying the sessions will automatically stop the threads. If you
        //  try to destroy the threads first without destroying the sessions,
        //  they will deadlock waiting for the session to finish.

        std::unique_ptr<AsyncTask> visual_thread;
        std::unique_ptr<AsyncTask> audio_thread;

        std::unique_ptr<VisualInferenceSession> visual_session;
        std::unique_ptr<AudioInferenceSession> audio_session;

        //  Add all the callbacks. Lazy init the sessions only when needed.
        for (size_t c = 0; c < callbacks.size(); c++){
            InferenceCallback* callback = callbacks[c];
            if (callback == nullptr){
                continue;
            }
            map[callback] = c;
            switch (callback->type()){
            case InferenceType::VISUAL:
                if (visual_session == nullptr){
                    visual_session.reset(new VisualInferenceSession(env, console, console, console, period));
                }
                *visual_session += static_cast<VisualInferenceCallback&>(*callback);
                break;
            case InferenceType::AUDIO:
                if (audio_session == nullptr){
                    audio_session.reset(new AudioInferenceSession(env, console, console, period));
                }
                *audio_session += static_cast<AudioInferenceCallback&>(*callback);
            }
        }

        BotBaseContext context(console.botbase());

        //  Start the inference threads.
        if (visual_session){
            visual_thread = env.dispatcher().dispatch([&]{
                try{
                    trigger = visual_session->run();
                    context.cancel();
                }catch (...){
                    context.cancel();
                    throw;
                }
            });
        }
        if (audio_session){
            audio_thread = env.dispatcher().dispatch([&]{
                try{
                    trigger = audio_session->run();
                    context.cancel();
                }catch (...){
                    context.cancel();
                    throw;
                }
            });
        }


        //  Run commands.
        try{
            command(context);
            context.wait_for_all_requests();
        }catch (ProgramCancelledException&){
            throw;
        }catch (InvalidConnectionStateException&){
            throw;
        }catch (OperationCancelledException&){
        }

        //  Join the inference threads.
        if (visual_session){
            visual_session->stop();
            visual_thread->wait_and_rethrow_exceptions();
        }
        if (audio_session){
            audio_session->stop();
            audio_thread->wait_and_rethrow_exceptions();
        }
    }

    //  Lookup which callback triggered.
    auto iter = map.find(trigger);
    return iter == map.end() ? -1 : (int)iter->second;
}



}




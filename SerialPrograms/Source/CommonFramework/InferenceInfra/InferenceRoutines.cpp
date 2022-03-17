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
    InferenceCallback* visual_trigger = nullptr;
    InferenceCallback* audio_trigger = nullptr;
    {
        std::unique_ptr<AsyncVisualInferenceSession> visual_session;
        std::unique_ptr<AsyncAudioInferenceSession> audio_session;

        std::mutex lock;
        std::condition_variable cv;
        ProgramStopNotificationScope stopper(env, lock, cv);
        bool stopped = false;

        std::function<void()> stop_callback = [&](){
            std::lock_guard<std::mutex> lg(lock);
            stopped = true;
            cv.notify_all();
        };

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
                    visual_session.reset(new AsyncVisualInferenceSession(
                        env, console, console, console,
                        stop_callback, period
                    ));
                }
                *visual_session += static_cast<VisualInferenceCallback&>(*callback);
                break;
            case InferenceType::AUDIO:
                if (audio_session == nullptr){
                    audio_session.reset(new AsyncAudioInferenceSession(
                        env, console, console,
                        stop_callback, period
                    ));
                }
                *audio_session += static_cast<AudioInferenceCallback&>(*callback);
                break;
            }
        }


        //  Wait
        {
            std::unique_lock<std::mutex> lg(lock);
            cv.wait_until(lg, deadline, [&](){ return stopped || env.is_stopping(); });
        }

        //  Stop the inference threads (and rethrow exceptions).
        if (visual_session){
            visual_trigger = visual_session->stop();
        }
        if (audio_session){
            audio_trigger = audio_session->stop();
        }
    }

    //  Lookup which callback triggered.
    if (visual_trigger){
        auto iter = map.find(visual_trigger);
        if (iter != map.end()){
            return (int)iter->second;
        }
    }
    if (audio_trigger){
        auto iter = map.find(audio_trigger);
        if (iter != map.end()){
            return (int)iter->second;
        }
    }
    return -1;
}




int run_until(
    ProgramEnvironment& env, ConsoleHandle& console,
    std::function<void(const BotBaseContext& context)>&& command,
    std::vector<InferenceCallback*>&& callbacks,
    std::chrono::milliseconds period
){
    std::map<InferenceCallback*, size_t> map;
    InferenceCallback* visual_trigger = nullptr;
    InferenceCallback* audio_trigger = nullptr;
    {
        std::unique_ptr<AsyncVisualInferenceSession> visual_session;
        std::unique_ptr<AsyncAudioInferenceSession> audio_session;

        BotBaseContext context(console.botbase());

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
                    visual_session.reset(new AsyncVisualInferenceSession(
                        env, console, console, console,
                        [&](){ context.cancel_now(); },
                        period
                    ));
                }
                *visual_session += static_cast<VisualInferenceCallback&>(*callback);
                break;
            case InferenceType::AUDIO:
                if (audio_session == nullptr){
                    audio_session.reset(new AsyncAudioInferenceSession(
                        env, console, console,
                        [&](){ context.cancel_now(); },
                        period
                    ));
                }
                *audio_session += static_cast<AudioInferenceCallback&>(*callback);
                break;
            }
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

        //  Stop the inference threads (and rethrow exceptions).
        if (visual_session){
            visual_trigger = visual_session->stop();
        }
        if (audio_session){
            audio_trigger = audio_session->stop();
        }
    }

    //  Lookup which callback triggered.
    if (visual_trigger){
        auto iter = map.find(visual_trigger);
        if (iter != map.end()){
            return (int)iter->second;
        }
    }
    if (audio_trigger){
        auto iter = map.find(audio_trigger);
        if (iter != map.end()){
            return (int)iter->second;
        }
    }
    return -1;
}



}




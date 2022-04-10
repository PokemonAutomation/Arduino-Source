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
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    WallClock deadline,
    std::vector<InferenceCallback*>&& callbacks,
    std::chrono::milliseconds period
){
    std::map<InferenceCallback*, size_t> map;
    InferenceCallback* visual_trigger = nullptr;
    InferenceCallback* audio_trigger = nullptr;
    {
        //  The order here is important. BotBaseContext needs to outlive both inference sessions.
        BotBaseContext subcontext(context, console.botbase());

        std::unique_ptr<AsyncVisualInferenceSession> visual_session;
        std::unique_ptr<AsyncAudioInferenceSession> audio_session;

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
                        env, console, subcontext,
                        console, console,
                        [&](){ subcontext.cancel_now(); },
                        period
                    ));
                }
                *visual_session += static_cast<VisualInferenceCallback&>(*callback);
                break;
            case InferenceType::AUDIO:
                if (audio_session == nullptr){
                    audio_session.reset(new AsyncAudioInferenceSession(
                        env, console, subcontext,
                        console,
                        [&](){ subcontext.cancel_now(); },
                        period
                    ));
                }
                *audio_session += static_cast<AudioInferenceCallback&>(*callback);
                break;
            }
        }


        //  Wait
        try{
            subcontext.wait_until(deadline);
        }catch (OperationCancelledException&){}

        context.scope()->throw_if_cancelled();

        //  Stop the inference threads (and rethrow exceptions).
        if (visual_session){
            visual_trigger = visual_session->stop_and_rethrow();
        }
        if (audio_session){
            audio_trigger = audio_session->stop_and_rethrow();
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
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    std::function<void(BotBaseContext& context)>&& command,
    std::vector<InferenceCallback*>&& callbacks,
    std::chrono::milliseconds period
){
    std::map<InferenceCallback*, size_t> map;
    InferenceCallback* visual_trigger = nullptr;
    InferenceCallback* audio_trigger = nullptr;
    {
        //  The order here is important. BotBaseContext needs to outlive both inference sessions.
        BotBaseContext subcontext(context, console.botbase());

        std::unique_ptr<AsyncVisualInferenceSession> visual_session;
        std::unique_ptr<AsyncAudioInferenceSession> audio_session;

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
                        env, console, subcontext,
                        console, console,
                        [&](){ subcontext.cancel_now(); },
                        period
                    ));
                }
                *visual_session += static_cast<VisualInferenceCallback&>(*callback);
                break;
            case InferenceType::AUDIO:
                if (audio_session == nullptr){
                    audio_session.reset(new AsyncAudioInferenceSession(
                        env, console, subcontext,
                        console,
                        [&](){ subcontext.cancel_now(); },
                        period
                    ));
                }
                *audio_session += static_cast<AudioInferenceCallback&>(*callback);
                break;
            }
        }


        //  Run commands.
        try{
            command(subcontext);
            subcontext.wait_for_all_requests();
        }catch (OperationCancelledException&){}

        context.scope()->throw_if_cancelled();

        //  Stop the inference threads (and rethrow exceptions).
        if (visual_session){
            visual_trigger = visual_session->stop_and_rethrow();
        }
        if (audio_session){
            audio_trigger = audio_session->stop_and_rethrow();
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




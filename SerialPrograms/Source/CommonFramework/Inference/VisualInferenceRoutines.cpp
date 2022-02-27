/*  Visual Inference Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "VisualInferenceSession.h"
#include "VisualInferenceRoutines.h"

namespace PokemonAutomation{



int wait_until(
    ProgramEnvironment& env, LoggerQt& logger,
    VideoFeed& feed, VideoOverlay& overlay,
    std::chrono::milliseconds timeout,
    std::vector<VisualInferenceCallback*>&& callbacks,
    std::chrono::milliseconds period
){
    return wait_until(
        env, logger, feed, overlay,
        std::chrono::system_clock::now() + timeout,
        std::move(callbacks),
        period
    );
}
int wait_until(
    ProgramEnvironment& env, LoggerQt& logger,
    VideoFeed& feed, VideoOverlay& overlay,
    std::chrono::system_clock::time_point time_limit,
    std::vector<VisualInferenceCallback*>&& callbacks,
    std::chrono::milliseconds period
){
    std::map<VisualInferenceCallback*, size_t> map;
    VisualInferenceCallback* trigger = nullptr;
    {
        VisualInferenceSession session(env, logger, feed, overlay, period);

        for (size_t c = 0; c < callbacks.size(); c++){
            VisualInferenceCallback* callback = callbacks[c];
            if (callback != nullptr){
                session += *callback;
                map[callback] = c;
            }
        }

        trigger = session.run(time_limit);
        session.stop();
    }

    //  Lookup which callback triggered.
    auto iter = map.find(trigger);
    return iter == map.end() ? -1 : (int)iter->second;
}
int wait_until(
    ProgramEnvironment& env, ConsoleHandle& console,
    std::chrono::milliseconds timeout,
    std::vector<VisualInferenceCallback*>&& callbacks,
    std::chrono::milliseconds period
){
    return wait_until(env, console, console, console, timeout, std::move(callbacks), period);
}
int wait_until(
    ProgramEnvironment& env, ConsoleHandle& console,
    std::chrono::system_clock::time_point time_limit,
    std::vector<VisualInferenceCallback*>&& callbacks,
    std::chrono::milliseconds period
){
    return wait_until(env, console, console, console, time_limit, std::move(callbacks), period);
}




int run_until(
    ProgramEnvironment& env, ConsoleHandle& console,
    std::function<void(const BotBaseContext& context)>&& command,
    std::vector<VisualInferenceCallback*>&& callbacks,
    std::chrono::milliseconds period
){
    std::map<VisualInferenceCallback*, size_t> map;
    VisualInferenceCallback* trigger = nullptr;
    {
        VisualInferenceSession session(env, console, console, console, period);

        for (size_t c = 0; c < callbacks.size(); c++){
            VisualInferenceCallback* callback = callbacks[c];
            if (callback != nullptr){
                session += *callback;
                map[callback] = c;
            }
        }

        BotBaseContext context(console.botbase());

        std::unique_ptr<AsyncTask> task = env.dispatcher().dispatch([&]{
            try{
                trigger = session.run();
                context.cancel();
            }catch (...){
                context.cancel();
                throw;
            }
        });

        try{
            command(context);
            context.wait_for_all_requests();
        }catch (ProgramCancelledException&){
            throw;
        }catch (InvalidConnectionStateException&){
            throw;
        }catch (OperationCancelledException&){
        }

        session.stop();
        task->wait_and_rethrow_exceptions();
    }

    //  Lookup which callback triggered.
    auto iter = map.find(trigger);
    return iter == map.end() ? -1 : (int)iter->second;
}



}




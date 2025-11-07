/*  Visual Inference Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "InferenceSession.h"
#include "InferenceRoutines.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



int wait_until(
    VideoStream& stream, CancellableScope& scope,
    WallClock deadline,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_video_period,
    std::chrono::milliseconds default_audio_period
){
    CancellableHolder<CancellableScope> subscope(scope);
    InferenceSession session(
        subscope, stream,
        callbacks,
        default_video_period, default_audio_period
    );

    try{
        subscope.wait_until(deadline);
    }catch (OperationCancelledException&){}

    subscope.throw_if_cancelled_with_exception();
    scope.throw_if_cancelled();

    return session.triggered_index();
}




int run_until(
    VideoStream& stream, CancellableScope& scope,
    std::function<void(CancellableScope& scope)>&& command,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_video_period,
    std::chrono::milliseconds default_audio_period
){
    CancellableHolder<CancellableScope> subscope(scope);
    InferenceSession session(
        subscope, stream,
        callbacks,
        default_video_period, default_audio_period
    );

    try{
        command(subscope);
    }catch (OperationCancelledException&){}

    subscope.throw_if_cancelled_with_exception();
    scope.throw_if_cancelled();

    return session.triggered_index();
}



#if 1
int run_until_with_time_limit(
    ProgramEnvironment& env, VideoStream& stream, CancellableScope& scope,
    WallClock deadline,
    std::function<void(CancellableScope& scope)>&& command,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_video_period,
    std::chrono::milliseconds default_audio_period
){
    CancellableHolder<CancellableScope> subscope(scope);
    InferenceSession session(
        subscope, stream,
        callbacks,
        default_video_period, default_audio_period
    );

    bool timed_out = false;
    std::unique_ptr<AsyncTask> timer = env.realtime_dispatcher().dispatch([&]{
        subscope.wait_until(deadline);
        timed_out = true;
        subscope.cancel(nullptr);
    });

    try{
        if (command){
            command(subscope);
        }
//        subscope.wait_for_all_requests();
    }catch (OperationCancelledException&){}

    timer->wait_and_rethrow_exceptions();
    subscope.throw_if_cancelled_with_exception();
    scope.throw_if_cancelled();

    return timed_out ? -2 : session.triggered_index();
}
#endif






}




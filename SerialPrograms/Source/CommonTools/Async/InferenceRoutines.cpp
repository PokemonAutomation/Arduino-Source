/*  Visual Inference Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "InferenceSession.h"
#include "InferenceRoutines.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



class CancellableSetSession : public Cancellable::CancelListener{
public:
    CancellableSetSession(CancellableScope& scope, std::vector<Cancellable*> stoppers)
        : m_scope(scope)
        , m_subscope(scope)
        , m_stoppers(std::move(stoppers))
    {
        for (Cancellable* stopper : m_stoppers){
            stopper->add_cancel_listener(*this);
        }
    }
    ~CancellableSetSession(){
        detach();
    }

    virtual void on_cancellable_cancel(
        Cancellable& cancellable,
        std::exception_ptr reason
    ) override{
        m_subscope.cancel(std::move(reason));
    }

    int wait_until(WallClock deadline){
        try{
            m_subscope.wait_until(deadline);
        }catch (OperationCancelledException&){}

        m_subscope.throw_if_cancelled_with_exception();
        m_scope.throw_if_cancelled();

        for (size_t c = 0; c < m_stoppers.size(); c++){
            if (m_stoppers[c]->cancelled()){
                return (int)c;
            }
        }
        return -1;
    }
    int run_until(std::function<void(CancellableScope& scope)>&& command){
        try{
            command(m_subscope);
        }catch (OperationCancelledException&){}

        m_subscope.throw_if_cancelled_with_exception();
        m_scope.throw_if_cancelled();

        for (size_t c = 0; c < m_stoppers.size(); c++){
            if (m_stoppers[c]->cancelled()){
                return (int)c;
            }
        }
        return -1;
    }

private:
    void detach() noexcept{
        for (Cancellable* stopper : m_stoppers){
            stopper->remove_cancel_listener(*this);
        }
    }

private:
    CancellableScope& m_scope;
    CancellableHolder<CancellableScope> m_subscope;
    std::vector<Cancellable*> m_stoppers;
    std::atomic<size_t> m_triggered_index;
};



int wait_until(
    CancellableScope& scope,
    WallClock deadline,
    std::vector<Cancellable*> stoppers
){
    CancellableSetSession session(scope, std::move(stoppers));
    return session.wait_until(deadline);
}
int run_until(
    CancellableScope& scope,
    std::function<void(CancellableScope& scope)>&& command,
    std::vector<Cancellable*> stoppers
){
    CancellableSetSession session(scope, std::move(stoppers));
    return session.run_until(std::move(command));
}








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
    AsyncTask timer = GlobalThreadPools::unlimited_realtime().dispatch_now_blocking([&]{
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

    timer.wait_and_rethrow_exceptions();
    subscope.throw_if_cancelled_with_exception();
    scope.throw_if_cancelled();

    return timed_out ? -2 : session.triggered_index();
}
#endif






}




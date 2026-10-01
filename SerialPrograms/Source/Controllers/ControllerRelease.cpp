/*  Controller Release
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "Controller.h"
#include "ControllerRelease.h"

namespace PokemonAutomation{



bool release_all_and_confirm(AbstractController& controller, Milliseconds timeout){
    if (!controller.is_ready()){
        return false;
    }
    controller.cancel_all_commands();

    //  Bound the confirmation: this timer cancels `scope` when the timeout passes,
    //  which makes `issue_nop()` / `wait_for_all()` below throw
    //  OperationCancelledException.
    CancellableHolder<CancellableScope> scope;
    Mutex lock;
    ConditionVariable cv;
    bool done = false;
    Thread timer([&]{
        std::unique_lock<Mutex> lg(lock);
        if (!cv.wait_for(lg, timeout, [&]{ return done; })){
            scope.cancel(nullptr);
        }
    });

    bool confirmed = false;
    try{
        //  Queued after the cancel, so the device reports this no-op finished only
        //  after it has dropped everything before it and held neutral for 10 ms.
        controller.issue_nop(&scope, Milliseconds(10));
        controller.wait_for_all(&scope);
        confirmed = true;
    }catch (OperationCancelledException&){}

    {
        std::lock_guard<Mutex> lg(lock);
        done = true;
    }
    cv.notify_all();
    timer.join();
    return confirmed;
}



}

/*  Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/RecursiveThrottler.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "Controller.h"

namespace PokemonAutomation{




struct AbstractController::Data{
    RecursiveThrottler recursive_throttler;
    ListenerSet<InputSniffer> input_sniffers;
};
void AbstractController::add_input_sniffer(InputSniffer& listener){
    m_data->input_sniffers.add(listener);
}
void AbstractController::remove_input_sniffer(InputSniffer& listener){
    m_data->input_sniffers.remove(listener);
}
void AbstractController::on_command_input(WallClock timestamp, const ControllerState& state){
    m_data->input_sniffers.run_method(&InputSniffer::on_command_input, timestamp, state);
}


AbstractController::AbstractController()
    : m_data(CONSTRUCT_TOKEN)
{}
AbstractController::~AbstractController() = default;


RecursiveThrottler& AbstractController::logging_throttler(){
    return m_data->recursive_throttler;
}


bool AbstractController::cancel_all_commands_blocking(Milliseconds timeout){
    if (!is_ready()){
        return false;
    }
    cancel_all_commands();

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
        issue_nop(&scope, Milliseconds(10));
        wait_for_all(&scope);
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


void AbstractController::throw_bad_cast(const char* desired_typename){
    throw UserSetupError(
        logger(),
        std::string("Incompatible Controller:\n\n") +
        "Required:\n    " + desired_typename + "\n"
        "Actual:\n    " + name()
    );
}



}



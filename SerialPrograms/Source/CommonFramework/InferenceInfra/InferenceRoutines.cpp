/*  Visual Inference Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "ClientSource/Connection/BotBase.h"
//#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "InferenceSession.h"
#include "InferenceRoutines.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



int wait_until(
    ConsoleHandle& console, BotBaseContext& context,
    WallClock deadline,
    const std::vector<InferenceCallback*>& callbacks,
    std::chrono::milliseconds period
){
    BotBaseContext subcontext(context, console.botbase());
    InferenceSession session(subcontext, console, callbacks, period);

    try{
        subcontext.wait_until(deadline);
    }catch (OperationCancelledException&){}

    context.scope()->throw_if_cancelled();

    return session.triggered_index();
}
int wait_until(
    ConsoleHandle& console, BotBaseContext& context,
    WallClock deadline,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_period
){
    BotBaseContext subcontext(context, console.botbase());
    InferenceSession session(subcontext, console, callbacks, default_period);

    try{
        subcontext.wait_until(deadline);
    }catch (OperationCancelledException&){}

    context.scope()->throw_if_cancelled();

    return session.triggered_index();
}




int run_until(
    ConsoleHandle& console, BotBaseContext& context,
    std::function<void(BotBaseContext& context)>&& command,
    const std::vector<InferenceCallback*>& callbacks,
    std::chrono::milliseconds period
){
    BotBaseContext subcontext(context, console.botbase());
    InferenceSession session(subcontext, console, callbacks, period);

    //  Run commands.
    try{
        command(subcontext);
        subcontext.wait_for_all_requests();
    }catch (OperationCancelledException&){}

    context.scope()->throw_if_cancelled();

    return session.triggered_index();
}
int run_until(
    ConsoleHandle& console, BotBaseContext& context,
    std::function<void(BotBaseContext& context)>&& command,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_period
){
    BotBaseContext subcontext(context, console.botbase());
    InferenceSession session(subcontext, console, callbacks, default_period);

    try{
        command(subcontext);
        subcontext.wait_for_all_requests();
    }catch (OperationCancelledException&){}

    context.scope()->throw_if_cancelled();

    return session.triggered_index();
}



}




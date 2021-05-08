/*  Visual Inference Callback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/InterruptableCommands.h"
#include "VisualInferenceCallback.h"

namespace PokemonAutomation{


VisualInferenceCallbackWithCommandStop::VisualInferenceCallbackWithCommandStop()
    : m_triggered(false)
{}

void VisualInferenceCallbackWithCommandStop::register_command_stop(InterruptableCommandSession& session){
    m_command_stops.emplace_back(&session);
}


bool VisualInferenceCallbackWithCommandStop::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    if (!on_frame(frame, timestamp)){
        return false;
    }
    m_triggered.store(true, std::memory_order_release);
    for (InterruptableCommandSession* command : m_command_stops){
        command->stop();
    }
    return true;
}



}

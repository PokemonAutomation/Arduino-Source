/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "RunnablePanel.h"

namespace PokemonAutomation{



std::unique_ptr<StatsTracker> RunnablePanelDescriptor::make_stats() const{
    return nullptr;
}




RunnablePanelInstance::RunnablePanelInstance(const PanelDescriptor& descriptor)
    : PanelInstance(descriptor)
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATION_ERROR_RECOVERABLE(
        "Program Error (Recoverable)",
        true, false,
        ImageAttachmentMode::PNG,
        {"Notifs"}
    )
    , NOTIFICATION_ERROR_FATAL(
        "Program Error (Fatal)",
        true, true,
//        ImageAttachmentMode::PNG,
        {"Notifs", "LiveHost"}
    )
{}

void RunnablePanelInstance::from_json(const JsonValue& json){
    m_options.load_json(json);
}
JsonValue RunnablePanelInstance::to_json() const{
    return m_options.to_json();
}

std::string RunnablePanelInstance::check_validity() const{
    return m_options.check_validity();
}
void RunnablePanelInstance::restore_defaults(){
    m_options.restore_defaults();
}
void RunnablePanelInstance::reset_state(){
    m_options.reset_state();
}










}

/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "RunnablePanel.h"

namespace PokemonAutomation{



RunnablePanelInstance::RunnablePanelInstance(const PanelDescriptor& descriptor)
    : PanelInstance(descriptor)
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

std::unique_ptr<StatsTracker> RunnablePanelInstance::make_stats() const{
    return nullptr;
}

void RunnablePanelInstance::from_json(const QJsonValue& json){
    m_options.load_json(json);
}
QJsonValue RunnablePanelInstance::to_json() const{
    return m_options.to_json();
}

QString RunnablePanelInstance::check_validity() const{
    return m_options.check_validity();
}
void RunnablePanelInstance::restore_defaults(){
    m_options.restore_defaults();
}
void RunnablePanelInstance::reset_state(){
    m_options.reset_state();
}










}

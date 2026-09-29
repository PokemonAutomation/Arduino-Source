/*  Program Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "ProgramDescriptor.h"

namespace PokemonAutomation{



ProgramDescriptor::ProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    Color color,
    PanelDeprecation deprecation,
    bool restore_defaults_button,
    std::vector<std::string> required_resources
)
    : PanelDescriptor(
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        color,
        deprecation,
        restore_defaults_button
    )
    , m_required_resources(std::move(required_resources))
{}
std::unique_ptr<StatsTracker> ProgramDescriptor::make_stats() const{
    return nullptr;
}



ProgramInstance::ProgramInstance(
    const std::vector<std::string>& error_notification_tags,
    LockMode options_lock_mode
)
    : m_options(options_lock_mode)
    , NOTIFICATION_PROGRAM_FINISH(
        "Program Finished",
        true, true,
        ImageAttachmentMode::JPG,
        {"Notifs"}
    )
    , NOTIFICATION_ERROR_RECOVERABLE(
        "Program Error (Recoverable)",
        true, false,
        ImageAttachmentMode::PNG,
        error_notification_tags
    )
    , NOTIFICATION_ERROR_FATAL(
        "Program Error (Fatal)",
        true, true,
        ImageAttachmentMode::PNG,
        error_notification_tags
    )
{}
void ProgramInstance::add_option(ConfigOption& option, std::string serialization_string){
    m_options.add_option(option, std::move(serialization_string));
}

std::string ProgramInstance::check_validity() const{
    return m_options.check_validity();
}
void ProgramInstance::restore_defaults(){
    return m_options.restore_defaults();
}
JsonValue ProgramInstance::to_json() const{
    return m_options.to_json();
}
void ProgramInstance::load_json(const JsonValue& json){
    m_options.load_json(json);
}









}

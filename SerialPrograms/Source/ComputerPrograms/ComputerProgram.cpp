/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "ComputerProgram.h"
#include "ComputerPrograms/Framework/ComputerProgramSession.h"

namespace PokemonAutomation{


ComputerProgramDescriptor::ComputerProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    PanelDeprecation deprecation
)
    : ProgramDescriptor(
        COLOR_DARKCYAN,
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        deprecation
    )
{}
std::unique_ptr<PanelSession> ComputerProgramDescriptor::make_panel() const{
    return std::unique_ptr<PanelSession>(new ComputerProgramSession(*this));
}





ComputerProgramInstance::ComputerProgramInstance()
    : m_options(LockMode::LOCK_WHILE_RUNNING)
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
        {"Notifs"}
    )
    , NOTIFICATION_ERROR_FATAL(
        "Program Error (Fatal)",
        true, true,
        ImageAttachmentMode::PNG,
        {"Notifs"}
    )
{}
void ComputerProgramInstance::add_option(ConfigOption& option, std::string serialization_string){
    m_options.add_option(option, std::move(serialization_string));
}

std::string ComputerProgramInstance::check_validity() const{
    return m_options.check_validity();
}
void ComputerProgramInstance::restore_defaults(){
    return m_options.restore_defaults();
}
JsonValue ComputerProgramInstance::to_json() const{
    return m_options.to_json();
}
void ComputerProgramInstance::load_json(const JsonValue& json){
    m_options.load_json(json);
}






}

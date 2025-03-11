/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "ComputerProgram.h"
#include "ComputerPrograms/Framework/ComputerProgramOption.h"

namespace PokemonAutomation{


ComputerProgramDescriptor::ComputerProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description
)
    : ProgramDescriptor(
        COLOR_DARKCYAN,
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description)
    )
{}
std::unique_ptr<PanelInstance> ComputerProgramDescriptor::make_panel() const{
    return std::unique_ptr<PanelInstance>(new ComputerProgramOption(*this));
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
void ComputerProgramInstance::from_json(const JsonValue& json){
    m_options.load_json(json);
}
JsonValue ComputerProgramInstance::to_json() const{
    return m_options.to_json();
}
std::string ComputerProgramInstance::check_validity() const{
    return m_options.check_validity();
}
void ComputerProgramInstance::restore_defaults(){
    return m_options.restore_defaults();
}






}

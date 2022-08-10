/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch_SingleSwitchProgramWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{





SingleSwitchProgramInstance::SingleSwitchProgramInstance(const RunnableSwitchProgramDescriptor& descriptor)
    : RunnableSwitchProgramInstance(descriptor)
    , m_switch(
        0,
        descriptor.min_pabotbase_level(),
        descriptor.feedback(),
        descriptor.allow_commands_while_running()
    )
{
    m_setup = &m_switch;
}
QWidget* SingleSwitchProgramInstance::make_widget(QWidget& parent, PanelHolder& holder){
    return SingleSwitchProgramWidget::make(parent, *this, holder);
}




#if 1

SingleSwitchProgramInstance2::SingleSwitchProgramInstance2()
    : NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
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
        {"Notifs"}
    )
{}
void SingleSwitchProgramInstance2::add_option(ConfigOption& option, std::string serialization_string){
    m_options.add_option(option, std::move(serialization_string));
}
void SingleSwitchProgramInstance2::from_json(const JsonValue& json){
    m_options.load_json(json);
}
JsonValue SingleSwitchProgramInstance2::to_json() const{
    return m_options.to_json();
}
std::string SingleSwitchProgramInstance2::check_validity() const{
    return m_options.check_validity();
}
void SingleSwitchProgramInstance2::restore_defaults(){
    return m_options.restore_defaults();
}

#endif




}
}

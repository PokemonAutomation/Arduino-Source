/*  Computer Program Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "ComputerProgramOption.h"
#include "ComputerProgramWidget.h"

namespace PokemonAutomation{




ComputerProgramOption::ComputerProgramOption(const ComputerProgramDescriptor& descriptor)
    : PanelSession(descriptor)
    , m_descriptor(descriptor)
    , m_instance(descriptor.make_instance())
{}

JsonValue ComputerProgramOption::to_json() const{
    return m_instance->to_json();
}
void ComputerProgramOption::load_json(const JsonValue& json){
    m_instance->load_json(json);
}

ConfigOption& ComputerProgramOption::options(){
    return m_instance->m_options;
}

std::string ComputerProgramOption::check_validity() const{
    return m_instance->check_validity();
}
void ComputerProgramOption::restore_defaults(){
    m_instance->restore_defaults();
}


QWidget* ComputerProgramOption::make_widget(QWidget& parent){
    return new ComputerProgramWidget(parent, *this);
}



}

/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "Panel.h"
#include "PanelWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



PanelDescriptor::PanelDescriptor(
    Color color,
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description
)
    : m_color(color)
    , m_identifier(std::move(identifier))
    , m_category(std::move(category))
    , m_display_name(std::move(display_name))
    , m_doc_link(std::move(doc_link))
    , m_description(std::move(description))
{}
std::unique_ptr<PanelInstance> PanelDescriptor::make_panel() const{
    return std::unique_ptr<PanelInstance>(new PanelInstance(*this));
}



PanelInstance::PanelInstance(const PanelDescriptor& descriptor)
    : m_descriptor(descriptor)
{}

void PanelInstance::from_json(){
    JsonValue* node = PERSISTENT_SETTINGS().panels.get_value(m_descriptor.identifier());
    if (node == nullptr){
        return;
    }
    from_json(*node);
}
JsonValue PanelInstance::to_json() const{
    return JsonValue();
}
void PanelInstance::save_settings() const{
    const std::string& identifier = m_descriptor.identifier();
    if (!identifier.empty()){
        PERSISTENT_SETTINGS().panels[m_descriptor.identifier()] = to_json();
    }
    global_logger_tagged().log("Saving panel settings...");
    PERSISTENT_SETTINGS().write();
}
QWidget* PanelInstance::make_widget(QWidget& parent, PanelHolder& holder){
    return new PanelWidget(parent, *this, holder);
}





}

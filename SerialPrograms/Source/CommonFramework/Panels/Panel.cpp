/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include "CommonFramework/PersistentSettings.h"
#include "Panel.h"
#include "PanelWidget.h"

namespace PokemonAutomation{



PanelDescriptor::PanelDescriptor(
    QColor color,
    std::string identifier,
    QString category, QString display_name,
    QString doc_link,
    QString description
)
    : m_color(std::move(color))
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
    from_json(PERSISTENT_SETTINGS().panels[QString::fromStdString(m_descriptor.identifier())]);
}
QJsonValue PanelInstance::to_json() const{
    return QJsonValue();
}
void PanelInstance::save_settings() const{
    const std::string& identifier = m_descriptor.identifier();
    if (!identifier.empty()){
        PERSISTENT_SETTINGS().panels[QString::fromStdString(identifier)] = to_json();
    }
    PERSISTENT_SETTINGS().write();
}
QWidget* PanelInstance::make_widget(QWidget& parent, PanelListener& listener){
    return new PanelWidget(parent, *this, listener);
}





}

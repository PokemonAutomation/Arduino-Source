/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include "CommonFramework/Globals.h"
#include "CommonFramework/PersistentSettings.h"
#include "Panel.h"

namespace PokemonAutomation{



PanelDescriptor::PanelDescriptor(
    QColor color,
    std::string identifier,
    QString display_name,
    QString doc_link,
    QString description
)
    : m_color(std::move(color))
    , m_identifier(std::move(identifier))
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
    from_json(PERSISTENT_SETTINGS().panels[m_descriptor.identifier().c_str()]);
}
QJsonValue PanelInstance::to_json() const{
    return QJsonValue();
}
QWidget* PanelInstance::make_widget(QWidget& parent, PanelListener& listener){
    return new PanelWidget(parent, *this, listener);
}



PanelWidget::PanelWidget(
    QWidget& parent,
    PanelInstance& instance,
    PanelListener& listener
)
    : QWidget(&parent)
    , m_instance(instance)
    , m_listener(listener)
{}

QWidget* PanelWidget::make_header(QWidget& parent){
    QGroupBox* description_box = new QGroupBox("Current Program", &parent);
    QVBoxLayout* vbox = new QVBoxLayout(description_box);

//    description_box->ev

    QString name_text = "<b>Name:</b> " + m_instance.descriptor().display_name();
    if (m_instance.descriptor().doc_link().size() > 0){
        QString path = ONLINE_DOC_URL + "/blob/master/Documentation/" + m_instance.descriptor().doc_link();
        name_text += " (<a href=\"" + path + "\">online documentation</a>)</font>";
    }
    QLabel* name = new QLabel(name_text, description_box);
    name->setTextFormat(Qt::RichText);
    name->setTextInteractionFlags(Qt::TextBrowserInteraction);
    name->setOpenExternalLinks(true);
    vbox->addWidget(name);

    QString description = "<b>Description:</b> ";
    description += m_instance.descriptor().description();
    QLabel* text = new QLabel(description, description_box);
    vbox->addWidget(text);
    text->setWordWrap(true);

    return description_box;
}



}

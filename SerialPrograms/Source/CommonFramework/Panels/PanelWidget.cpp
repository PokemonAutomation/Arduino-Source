/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/Globals.h"
#include "PanelWidget.h"

namespace PokemonAutomation{


PanelWidget::PanelWidget(
    QWidget& parent,
    PanelInstance& instance,
    PanelListener& listener
)
    : QWidget(&parent)
    , m_instance(instance)
    , m_listener(listener)
{}

CollapsibleGroupBox* PanelWidget::make_header(QWidget& parent){
    CollapsibleGroupBox* description_box = new CollapsibleGroupBox(parent, "Current Program");

    QWidget* body = new QWidget(description_box);
    QVBoxLayout* vbox = new QVBoxLayout(body);
    vbox->setContentsMargins(0, 0, 0, 0);

    QString name_text = "<b>Name:</b> " + m_instance.descriptor().display_name();
    if (m_instance.descriptor().doc_link().size() > 0){
        QString path = ONLINE_DOC_URL + m_instance.descriptor().doc_link();
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

//    description_box->setContentLayout(*vbox);
    description_box->set_widget(body);

    return description_box;
}



}

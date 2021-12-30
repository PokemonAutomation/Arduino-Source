/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
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

QWidget* PanelWidget::make_header(QWidget& parent){
    QGroupBox* description_box = new QGroupBox("Current Program", &parent);
    QVBoxLayout* vbox = new QVBoxLayout(description_box);

//    description_box->ev

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

    return description_box;
}



}

/*  Right-Side Panel Options
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include <QLabel>
#include <QGroupBox>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "RightPanel.h"

namespace PokemonAutomation{


RightPanel::RightPanel(
    QColor color,
    QString name,
    QString doc_link,
    QString description
)
    : m_color(std::move(color))
    , m_name(std::move(name))
    , m_doc_link(std::move(doc_link))
    , m_description(std::move(description))
{}
QJsonValue RightPanel::to_json() const{
    return QJsonValue();
}

QWidget* RightPanel::make_ui(MainWindow& window){
    RightPanelUI* widget = new RightPanelUI(*this);
    widget->construct();
    return widget;
}

RightPanelUI::RightPanelUI(RightPanel& factory)
    : m_factory(factory)
{}
void RightPanelUI::construct(){
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);

    QGroupBox* description_box = new QGroupBox("Current Program", this);
    layout->addWidget(description_box);

    QVBoxLayout* vbox = new QVBoxLayout(description_box);
//    layout->setAlignment(Qt::AlignTop);

    QString name_text = "<b>Name:</b> " + m_factory.m_name;
    if (m_factory.m_doc_link.size() > 0){
        QString path = GITHUB_REPO + "/blob/master/Documentation/" + m_factory.m_doc_link;
        name_text += " (<a href=\"" + path + "\">online documentation</a>)</font>";
    }
    QLabel* name = new QLabel(name_text, description_box);
    name->setTextFormat(Qt::RichText);
    name->setTextInteractionFlags(Qt::TextBrowserInteraction);
    name->setOpenExternalLinks(true);
    vbox->addWidget(name);

#if 0
    {
        QString path = GITHUB_REPO + "/blob/master/Documentation/Programs/" + factory.name() + ".md";
        QLabel* text = new QLabel("<font size=4><a href=\"" + path + "\">Online Documentation for " + factory.name() + "</a></font>");
        layout.addWidget(text);
        text->setTextFormat(Qt::RichText);
        text->setTextInteractionFlags(Qt::TextBrowserInteraction);
        text->setOpenExternalLinks(true);
    }
#endif

    QString description = "<b>Description:</b> ";
    description += m_factory.m_description;
    QLabel* text = new QLabel(description, description_box);
    vbox->addWidget(text);
    text->setWordWrap(true);

    append_description(*description_box, *vbox);

    make_body(*this, *layout);
}





}

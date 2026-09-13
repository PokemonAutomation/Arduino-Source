/*  Multi-Video Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QScrollArea>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/UiStateQtWidget.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "NintendoSwitch_SwitchViewer.h"

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<NintendoSwitch::SwitchViewer_Widget>;

namespace NintendoSwitch{


SwitchViewer_Descriptor::SwitchViewer_Descriptor()
    : PanelDescriptor(
        Color(),
        "NintendoSwitch:SwitchViewer",
        "Nintendo Switch", "Switch Viewer",
        "Programs/NintendoSwitch/SwitchViewer.html",
        "View status information from one or more running programs."
    )
{}



SwitchViewer::SwitchViewer(const SwitchViewer_Descriptor& descriptor)
    : UiState<SwitchViewer, PanelSession>(descriptor)
    , m_option(1, 4, 1)
    , m_session(m_option, true)
{}
JsonValue SwitchViewer::to_json() const{
    return m_session.to_json();
}
void SwitchViewer::load_json(const JsonValue& json){
    m_session.load_json(json);
}



SwitchViewer_Widget::~SwitchViewer_Widget(){
}
SwitchViewer_Widget::SwitchViewer_Widget(
    QWidget& parent,
    SwitchViewer& session
)
    : PanelWidget(parent, session)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(make_header());

    QScrollArea* scroll_outer = new QScrollArea(this);
    layout->addWidget(scroll_outer);
    scroll_outer->setWidgetResizable(true);

    QWidget* scroll_inner = new QWidget(scroll_outer);
    scroll_outer->setWidget(scroll_inner);
    QVBoxLayout* scroll_layout = new QVBoxLayout(scroll_inner);
    scroll_layout->setAlignment(Qt::AlignTop);

    UiWrapper wrapper = session.m_session.make_ui_component(this);
    scroll_layout->addWidget(dynamic_cast<QWidget*>(wrapper.release()));
    scroll_layout->addStretch(1);
}








}
}

/*  Multi-Video Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QScrollArea>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "NintendoSwitch/Framework/UI/NintendoSwitch_MultiSwitchSystemWidget.h"
#include "NintendoSwitch_SwitchViewer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


SwitchViewer_Descriptor::SwitchViewer_Descriptor()
    : PanelDescriptor(
        Color(),
        "NintendoSwitch:SwitchViewer",
        "Nintendo Switch", "Switch Viewer",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/SwitchViewer.md",
        "View status information from one or more running programs."
    )
{}



SwitchViewer::SwitchViewer(const SwitchViewer_Descriptor& descriptor)
    : PanelInstance(descriptor)
    , m_switches(
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        1, 4, 1
    )
{}
void SwitchViewer::from_json(const JsonValue& json){
    m_switches.load_json(json);
}
JsonValue SwitchViewer::to_json() const{
    return m_switches.to_json();
}
QWidget* SwitchViewer::make_widget(QWidget& parent, PanelHolder& holder){
    return SwitchViewer_Widget::make(parent, *this, holder);
}



SwitchViewer_Widget* SwitchViewer_Widget::make(
    QWidget& parent,
    SwitchViewer& instance,
    PanelHolder& holder
){
    SwitchViewer_Widget* widget = new SwitchViewer_Widget(parent, instance, holder);
    widget->construct();
    return widget;
}
SwitchViewer_Widget::~SwitchViewer_Widget(){
    delete m_switches;
}
SwitchViewer_Widget::SwitchViewer_Widget(
    QWidget& parent,
    SwitchViewer& instance,
    PanelHolder& holder
)
    : PanelWidget(parent, instance, holder)
    , m_session(instance.m_switches, 0)
{}
void SwitchViewer_Widget::construct(){
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(make_header(*this));

    QScrollArea* scroll_outer = new QScrollArea(this);
    layout->addWidget(scroll_outer);
    scroll_outer->setWidgetResizable(true);

    QWidget* scroll_inner = new QWidget(scroll_outer);
    scroll_outer->setWidget(scroll_inner);
    QVBoxLayout* scroll_layout = new QVBoxLayout(scroll_inner);
    scroll_layout->setAlignment(Qt::AlignTop);

    m_switches = new MultiSwitchSystemWidget(*this, m_session, 0);
    scroll_layout->addWidget(m_switches);
    scroll_layout->addStretch(1);
}








}
}

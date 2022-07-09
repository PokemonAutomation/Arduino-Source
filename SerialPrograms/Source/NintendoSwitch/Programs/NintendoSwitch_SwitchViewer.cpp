/*  Multi-Video Test
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemWidget.h"
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
        PABotBaseLevel::NOT_PABOTBASE, FeedbackType::NONE, false,
        1, 4, 1
    )
{}
void SwitchViewer::from_json(const JsonValue2& json){
    m_switches.load_json(json);
}
JsonValue2 SwitchViewer::to_json() const{
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
SwitchViewer_Widget::SwitchViewer_Widget(
    QWidget& parent,
    SwitchViewer& instance,
    PanelHolder& holder
)
    : PanelWidget(parent, instance, holder)
{}
void SwitchViewer_Widget::construct(){
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(make_header(*this));

    SwitchViewer& instance = static_cast<SwitchViewer&>(m_instance);
    m_switches = (MultiSwitchSystemWidget*)instance.m_switches.make_ui(*this, m_holder.raw_logger(), 0);
    layout->addWidget(m_switches);
}








}
}

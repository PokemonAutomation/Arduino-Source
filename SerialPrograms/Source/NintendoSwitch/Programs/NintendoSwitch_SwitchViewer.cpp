/*  Multi-Video Test
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include "NintendoSwitch_SwitchViewer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


SwitchViewer_Descriptor::SwitchViewer_Descriptor()
    : PanelDescriptor(
        QColor(),
        "NintendoSwitch:SwitchViewer",
        "Nintendo Switch", "Switch Viewer",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/SwitchViewer.md",
        "View status information from one or more running programs."
    )
{}



SwitchViewer::SwitchViewer(const SwitchViewer_Descriptor& descriptor)
    : PanelInstance(descriptor)
    , m_switches(
        PABotBaseLevel::NOT_PABOTBASE, FeedbackType::NONE,
        1, 4, 1
    )
{}
void SwitchViewer::from_json(const QJsonValue& json){
    m_switches.load_json(json.toObject());
}
QJsonValue SwitchViewer::to_json() const{
    return m_switches.to_json();
}
QWidget* SwitchViewer::make_widget(QWidget& parent, PanelListener& listener){
    return SwitchViewer_Widget::make(parent, *this, listener);
}



SwitchViewer_Widget* SwitchViewer_Widget::make(
    QWidget& parent,
    SwitchViewer& instance,
    PanelListener& listener
){
    SwitchViewer_Widget* widget = new SwitchViewer_Widget(parent, instance, listener);
    widget->construct();
    return widget;
}
SwitchViewer_Widget::SwitchViewer_Widget(
    QWidget& parent,
    SwitchViewer& instance,
    PanelListener& listener
)
    : PanelWidget(parent, instance, listener)
{}
void SwitchViewer_Widget::construct(){
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(make_header(*this));

    SwitchViewer& instance = static_cast<SwitchViewer&>(m_instance);
    m_switches = (MultiSwitchSystem*)instance.m_switches.make_ui(*this, m_listener.raw_logger(), 0);
    layout->addWidget(m_switches);
}








}
}

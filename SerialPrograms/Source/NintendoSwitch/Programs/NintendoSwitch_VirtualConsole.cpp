/*  Virtual Game Console
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QScrollArea>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "NintendoSwitch/Framework/UI/NintendoSwitch_SwitchSystemWidget.h"
#include "NintendoSwitch_VirtualConsole.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

VirtualConsole_Descriptor::VirtualConsole_Descriptor()
    : PanelDescriptor(
        Color(),
        "NintendoSwitch:VirtualConsole",
        "Nintendo Switch", "Virtual Console",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/VirtualConsole.md",
        "Play your Switch from your computer. Device logging is logged to the output window."
    )
{}



VirtualConsole::VirtualConsole(const VirtualConsole_Descriptor& descriptor)
    : PanelInstance(descriptor)
    , m_switch_control_option({}, false)
{}
void VirtualConsole::from_json(const JsonValue& json){
    m_switch_control_option.load_json(json);
}
JsonValue VirtualConsole::to_json() const{
    return m_switch_control_option.to_json();
}
QWidget* VirtualConsole::make_widget(QWidget& parent, PanelHolder& holder){
    return VirtualConsole_Widget::make(parent, *this, holder);
}



VirtualConsole_Widget* VirtualConsole_Widget::make(
    QWidget& parent,
    VirtualConsole& instance,
    PanelHolder& holder
){
    VirtualConsole_Widget* widget = new VirtualConsole_Widget(parent, instance, holder);
    widget->construct();
    return widget;
}
VirtualConsole_Widget::~VirtualConsole_Widget(){
    delete m_switch_widget;
}
VirtualConsole_Widget::VirtualConsole_Widget(
    QWidget& parent,
    VirtualConsole& instance,
    PanelHolder& holder
)
    : PanelWidget(parent, instance, holder)
    , m_session(instance.m_switch_control_option, 0, 0)
{}
void VirtualConsole_Widget::construct(){
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

    m_switch_widget = new SwitchSystemWidget(*this, m_session, 0);
    scroll_layout->addWidget(m_switch_widget);
}






}
}


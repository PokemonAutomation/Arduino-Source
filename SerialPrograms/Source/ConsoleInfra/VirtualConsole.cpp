/*  Virtual Game Console
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QScrollArea>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "VirtualConsole.h"

namespace PokemonAutomation{
namespace ConsoleInfra{

VirtualConsole_Descriptor::VirtualConsole_Descriptor(size_t controllers)
    : PanelDescriptor(
        Color(),
        controllers == 1
            ? "CC:VirtualConsole"
            : "CC:MultiControllerTester",
        "CC",
        controllers == 1
            ? "Virtual Console"
            : "Multi-Controller Tester",
        "Programs/NintendoSwitch/VirtualConsole.html",
        controllers == 1
            ? "Play your console from your computer."
            : "Test multiple controllers at once."
    )
    , m_controllers(controllers)
{}



VirtualConsole::VirtualConsole(const VirtualConsole_Descriptor& descriptor)
    : PanelInstance(descriptor)
    , m_console_options(descriptor.m_controllers, true)
{}
void VirtualConsole::from_json(const JsonValue& json){
    m_console_options.load_json(json);
}
JsonValue VirtualConsole::to_json() const{
    return m_console_options.to_json();
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
    delete m_console_widget;
}
VirtualConsole_Widget::VirtualConsole_Widget(
    QWidget& parent,
    VirtualConsole& instance,
    PanelHolder& holder
)
    : PanelWidget(parent, instance, holder)
    , m_session(global_logger_raw(), instance.m_console_options, 0)
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

    m_console_widget = new ConsoleSystemWidget(*this, m_session);
    scroll_layout->addWidget(m_console_widget);
}






}
}


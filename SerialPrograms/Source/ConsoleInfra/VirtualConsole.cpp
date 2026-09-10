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

template class RegisterUiStateQtWidget<ConsoleInfra::VirtualConsole_Widget>;


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
    : UiState<VirtualConsole, PanelSession>(descriptor)
    , m_console_options(descriptor.m_controllers, true)
{}
JsonValue VirtualConsole::to_json() const{
    return m_console_options.to_json();
}
void VirtualConsole::load_json(const JsonValue& json){
    m_console_options.load_json(json);
}



VirtualConsole_Widget::~VirtualConsole_Widget(){
    delete m_console_widget;
}
VirtualConsole_Widget::VirtualConsole_Widget(
    QWidget& parent,
    VirtualConsole& session
)
    : PanelWidget(parent, session)
    , m_session(global_logger_raw(), session.m_console_options, 0)
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

    m_console_widget = new ConsoleSystemWidget(*this, m_session);
    scroll_layout->addWidget(m_console_widget);
}






}
}


/*  Console Panel Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Qt/ShutdownWithEvents.h"
#include "CommonFramework/Panels/UI/PanelElements.h"
#include "ConsolePanelWidget.h"

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<GameConsole::ConsolePanelWidget>;

namespace GameConsole{



ConsolePanelWidget::~ConsolePanelWidget(){
    shutdown_with_events(
        m_session.logger(),
        "ConsolePanelWidget",
        [this]{ return m_session.try_shutdown(); }
    );
}
ConsolePanelWidget::ConsolePanelWidget(
    QWidget& parent,
    ConsolePanelSession& session
)
    : QWidget(&parent)
    , m_session(session)
{
    UiWrapper wrapper = m_session.system().make_ui_component(this);
    QWidget* system = dynamic_cast<QWidget*>(wrapper.release());

    populate_panel_widget(
        *this,
        session.descriptor(),
        system,
        session.options(),
        make_actions_bar(*this, session)
    );
}




}
}

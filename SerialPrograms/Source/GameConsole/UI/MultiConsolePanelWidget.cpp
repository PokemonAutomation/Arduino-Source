/*  Multi-Console Panel Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Qt/ShutdownWithEvents.h"
#include "CommonFramework/Panels/UI/PanelElements.h"
#include "MultiConsolePanelWidget.h"

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<GameConsole::MultiConsolePanelWidget>;

namespace GameConsole{



MultiConsolePanelWidget::~MultiConsolePanelWidget(){
    shutdown_with_events(
        m_session.logger(),
        "MultiConsolePanelWidget",
        [this]{ return m_session.try_shutdown(); }
    );
}
MultiConsolePanelWidget::MultiConsolePanelWidget(
    QWidget& parent,
    MultiConsolePanelSession& session
)
    : QWidget(&parent)
    , m_session(session)
{
    populate_panel_widget(
        *this,
        session.descriptor(),
        &m_session.system(),
        session.options(),
        {make_actions_bar(*this, session)}
    );
}




}
}

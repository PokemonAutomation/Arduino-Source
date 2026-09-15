/*  Options Panel Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Panels/UI/PanelElements.h"
#include "OptionsPanelWidget.h"

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<OptionsPanelWidget>;




OptionsPanelWidget::OptionsPanelWidget(
    QWidget& parent,
    OptionsPanelSession& session
)
    : QWidget(&parent)
    , m_session(session)
{
    populate_panel_widget(
        *this,
        session.descriptor(),
        nullptr,
        session.options(),
        make_actions_bar(*this, session)
    );
}




}

/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/Panels/PanelDescriptor.h"
#include "CommonFramework/Panels/PanelSession.h"
#include "PanelElements.h"
#include "PanelWidget.h"

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<PanelWidget>;




PanelWidget::PanelWidget(
    QWidget& parent,
    PanelSession& session
)
    : QWidget(&parent)
    , m_session(session)
{}

CollapsibleGroupBox* PanelWidget::make_header(){
    return make_panel_header(
        *this,
        m_session.descriptor().display_name(),
        m_session.descriptor().doc_link(),
        m_session.descriptor().description()
    );
}



}

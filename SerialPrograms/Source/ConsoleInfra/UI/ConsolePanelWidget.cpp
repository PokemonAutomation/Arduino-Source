/*  Console Panel Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QScrollArea>
#include "Common/Qt/ShutdownWithEvents.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/Panels/UI/PanelElements.h"
#include "ConsolePanelWidget.h"

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<ConsoleInfra::ConsolePanelWidget>;

namespace ConsoleInfra{



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
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    const ConsolePanelDescriptor& descriptor = session.descriptor();

    CollapsibleGroupBox* header = make_panel_header(
        *this,
        descriptor.display_name(),
        descriptor.doc_link(),
        descriptor.description()
    );
    m_layout->addWidget(header);


    {
        QScrollArea* scroll_outer = new QScrollArea(this);
        m_layout->addWidget(scroll_outer);
        scroll_outer->setWidgetResizable(true);

        QWidget* scroll_inner = new QWidget(scroll_outer);
        scroll_outer->setWidget(scroll_inner);
        QVBoxLayout* scroll_layout = new QVBoxLayout(scroll_inner);
        scroll_layout->setAlignment(Qt::AlignTop);

        UiWrapper wrapper = m_session.system().make_ui_component(this);
        m_system = dynamic_cast<QWidget*>(wrapper.release());
        scroll_layout->addWidget(m_system);

        m_options = ConfigWidget::make_from_option(session.options(), this);
        scroll_layout->addWidget(&m_options->widget());

        scroll_layout->addStretch(1);
    }
}




}
}

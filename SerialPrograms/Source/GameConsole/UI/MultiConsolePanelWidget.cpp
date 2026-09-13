/*  Multi-Console Panel Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include "Common/Qt/ShutdownWithEvents.h"
#include "Common/Qt/Options/ConfigWidget.h"
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
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    const MultiConsolePanelDescriptor& descriptor = session.descriptor();

    CollapsibleGroupBox* header = make_panel_header(
        *this,
        descriptor.display_name(),
        descriptor.doc_link(),
        descriptor.description()
    );
    layout->addWidget(header);

    {
        QScrollArea* scroll_outer = new QScrollArea(this);
        layout->addWidget(scroll_outer);
        scroll_outer->setWidgetResizable(true);

        QWidget* scroll_inner = new QWidget(scroll_outer);
        scroll_outer->setWidget(scroll_inner);
        QVBoxLayout* scroll_layout = new QVBoxLayout(scroll_inner);
        scroll_layout->setAlignment(Qt::AlignTop);

        UiWrapper wrapper = m_session.system().make_ui_component(this);
        QWidget* system = dynamic_cast<QWidget*>(wrapper.release());
        scroll_layout->addWidget(system);

        ConfigWidget* options = ConfigWidget::make_from_option(session.options(), this);
        scroll_layout->addWidget(&options->widget());

        scroll_layout->addStretch(1);
    }

    if (session.descriptor().restore_defaults_button()){
        QGroupBox* actions_widget = new QGroupBox("Actions", &parent);

        QHBoxLayout* action_layout = new QHBoxLayout(actions_widget);
//        action_layout->setContentsMargins(0, 0, 0, 0);
        QPushButton* default_button = new QPushButton("Restore Defaults", actions_widget);
        {
            action_layout->addWidget(default_button, 1);
            QFont font = default_button->font();
            font.setPointSize(16);
            default_button->setFont(font);
        }

        connect(
            default_button, &QPushButton::clicked,
            this, [this](bool){
                QMessageBox::StandardButton button = QMessageBox::question(
                    nullptr,
                    "Restore Defaults",
                    "Are you sure you wish to restore settings back to defaults? This will wipe the current settings.",
                    QMessageBox::Ok | QMessageBox::Cancel
                );
                if (button == QMessageBox::Ok){
                    m_session.restore_defaults();
                }
            }
        );
        layout->addWidget(actions_widget);
    }
}




}
}

/*  Computer Program Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include "Common/Qt/CollapsibleGroupBox.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "CommonFramework/Panels/UI/PanelElements.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "ComputerPrograms/ComputerProgram.h"
#include "ComputerPrograms/Framework/ComputerProgramOption.h"
#include "ComputerProgramWidget.h"

namespace PokemonAutomation{


ComputerProgramWidget::~ComputerProgramWidget(){
    m_session.remove_listener(*this);
    delete m_actions_bar;
    delete m_stats_bar;
    delete m_options;
}
ComputerProgramWidget::ComputerProgramWidget(
    QWidget& parent,
    ComputerProgramOption& option,
    PanelHolder& holder
)
    : QWidget(&parent)
    , m_holder(holder)
    , m_session(option)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    const ComputerProgramDescriptor& descriptor = option.descriptor();

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

        m_options = option.options().make_QtWidget(*this);
        scroll_layout->addWidget(&m_options->widget());

        scroll_layout->addStretch(1);
    }

    m_stats_bar = new StatsBar(*this);
    m_stats_bar->set_stats("", m_session.historical_stats());
    layout->addWidget(m_stats_bar);

    m_actions_bar = new RunnablePanelActionBar(*this, m_session.current_state());
    layout->addWidget(m_actions_bar);

    connect(
        m_actions_bar, &RunnablePanelActionBar::start_clicked,
        this, [&](ProgramState state){
            std::string error;
            switch (state){
            case ProgramState::STOPPED:
                error = m_session.start_program();
                break;
            case ProgramState::RUNNING:
                error = m_session.stop_program();
                break;
            default:;
            }
            if (!error.empty()){
                this->error(error);
            }
        }
    );
    connect(
        m_actions_bar, &RunnablePanelActionBar::defaults_clicked,
        this, [&]{
            std::lock_guard<std::mutex> lg(m_session.program_lock());
            option.restore_defaults();
            m_options->update_all(false);
        }
    );

    m_session.add_listener(*this);
}

void ComputerProgramWidget::state_change(ProgramState state){
    QMetaObject::invokeMethod(this, [this, state]{
        m_options->widget().setEnabled(state == ProgramState::STOPPED);
        m_actions_bar->set_state(state);
        if (state == ProgramState::STOPPED){
            m_holder.on_idle();
        }else{
            m_holder.on_busy();
        }
    });
}
void ComputerProgramWidget::stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats){
    QMetaObject::invokeMethod(this, [this, current_stats, historical_stats]{
        m_stats_bar->set_stats(
            current_stats == nullptr ? "" : current_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN),
            historical_stats == nullptr ? "" : historical_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN)
        );
    });
}
void ComputerProgramWidget::error(const std::string& message){
    QMetaObject::invokeMethod(this, [message]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    });
}






}

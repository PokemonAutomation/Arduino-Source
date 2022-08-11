/*  Multi-Switch Program Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include "Common/Cpp/FixedLimitVector.tpp"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/Options/BatchOption/BatchWidget.h"
#include "CommonFramework/Panels/PanelElements.h"
#include "CommonFramework/Tools/BlackBorderCheck.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch_SwitchSystemWidget.h"
#include "NintendoSwitch_MultiSwitchProgramOption.h"
#include "NintendoSwitch_MultiSwitchProgramSession.h"
#include "NintendoSwitch_MultiSwitchProgramWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




MultiSwitchProgramWidget::~MultiSwitchProgramWidget(){
    RunnableSwitchProgramWidget::request_program_stop();
    join_program_thread();
}
MultiSwitchProgramWidget* MultiSwitchProgramWidget::make(
    QWidget& parent,
    MultiSwitchProgramInstance& instance,
    PanelHolder& holder
){
    instance.update_active_consoles();
    MultiSwitchProgramWidget* widget = new MultiSwitchProgramWidget(parent, instance, holder);
    widget->construct();
    connect(
        widget->m_setup, &SwitchSetupWidget::on_setup_changed,
        widget, [=, &instance]{
            instance.update_active_consoles();
            widget->m_options->update_visibility();
//            widget->redraw_options();
        }
    );
    return widget;
}
void MultiSwitchProgramWidget::run_switch_program(const ProgramInfo& info){
    MultiSwitchProgramInstance& instance = static_cast<MultiSwitchProgramInstance&>(m_instance);

    FixedLimitVector<ConsoleHandle> switches(instance.system_count());
    for (size_t c = 0; c < instance.system_count(); c++){
        SwitchSystemWidget& system = this->system(c);
        switches.emplace_back(
            c,
            system.logger(),
            sanitize_botbase(system.botbase()),
            system.camera(),
            system.overlay(),
            system.audio()
        );
    }

    CancellableHolder<CancellableScope> scope;
    MultiSwitchProgramEnvironment env(
        info,
        scope,
        m_logger,
        m_current_stats.get(), m_historical_stats.get(),
        std::move(switches)
    );
    connect(
        &env, &ProgramEnvironment::set_status,
        this, [=](std::string status){
            this->status_update(std::move(status));
        }
    );

    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_scope = &scope;
    }
    try{
        start_program_video_check(env.consoles, instance.descriptor().feedback());
        instance.program(env, scope);
        std::lock_guard<std::mutex> lg(m_lock);
        m_scope = nullptr;
    }catch (...){
        env.update_stats();
        std::lock_guard<std::mutex> lg(m_lock);
        m_scope = nullptr;
        throw;
    }
}



MultiSwitchProgramWidget2::~MultiSwitchProgramWidget2(){
    m_session.remove_listener(*this);
    delete m_actions_bar;
    delete m_stats_bar;
    delete m_options;
    delete m_system;
}


MultiSwitchProgramWidget2::MultiSwitchProgramWidget2(
    QWidget& parent,
    MultiSwitchProgramOption& option,
    PanelHolder& holder
)
    : QWidget(&parent)
    , m_holder(holder)
    , m_session(option)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    const MultiSwitchProgramDescriptor& descriptor = option.descriptor();

    CollapsibleGroupBox* header = make_panel_header(
        *this,
        descriptor.display_name(),
        descriptor.doc_link(),
        descriptor.description(),
        descriptor.feedback(),
        descriptor.min_pabotbase_level()
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

        m_system = new MultiSwitchSystemWidget(*this, m_session.system(), m_session.instance_id());
        scroll_layout->addWidget(m_system);

        m_options = option.options().make_ui(*this);
        scroll_layout->addWidget(&m_options->widget());

        scroll_layout->addStretch(1);
    }

    m_stats_bar = new StatsBar(*this);
    layout->addWidget(m_stats_bar);

    m_actions_bar = new RunnablePanelActionBar(*this, m_session.current_state());
    layout->addWidget(m_actions_bar);

    connect(
        m_actions_bar, &RunnablePanelActionBar::start_clicked,
        this, [&](ProgramState state){
            switch (state){
            case ProgramState::STOPPED:
                m_session.start_program();
                break;
            case ProgramState::RUNNING:
                m_session.stop_program();
                break;
            default:;
            }
        }
    );
    connect(
        m_actions_bar, &RunnablePanelActionBar::defaults_clicked,
        this, [&]{
            std::lock_guard<std::mutex> lg(m_session.program_lock());
            option.restore_defaults();
            m_options->update_ui();
        }
    );

    m_session.add_listener(*this);
}




void MultiSwitchProgramWidget2::state_change(ProgramState state){
    QMetaObject::invokeMethod(this, [=]{
        m_system->update_ui(state);
        m_options->widget().setEnabled(state == ProgramState::STOPPED);
        m_actions_bar->set_state(state);
        if (state == ProgramState::STOPPED){
            m_holder.on_idle();
        }else{
            m_holder.on_busy();
        }
    });
}
void MultiSwitchProgramWidget2::stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats){
    QMetaObject::invokeMethod(this, [=]{
        m_stats_bar->set_stats(
            current_stats == nullptr ? "" : current_stats->to_str(),
            historical_stats == nullptr ? "" : historical_stats->to_str()
        );
    });
}
void MultiSwitchProgramWidget2::error(const std::string& message){
    QMetaObject::invokeMethod(this, [=]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    });
}




}
}

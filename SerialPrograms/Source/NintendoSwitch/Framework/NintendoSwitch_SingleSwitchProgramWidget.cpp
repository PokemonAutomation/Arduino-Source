/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Panels/PanelElements.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/BlackBorderCheck.h"
#include "NintendoSwitch_SingleSwitchProgramSession.h"
#include "NintendoSwitch_SingleSwitchProgramWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


SingleSwitchProgramWidget::~SingleSwitchProgramWidget(){
    RunnableSwitchProgramWidget::request_program_stop();
    join_program_thread();
}
SingleSwitchProgramWidget* SingleSwitchProgramWidget::make(
    QWidget& parent,
    SingleSwitchProgramInstance& instance,
    PanelHolder& holder
){
    SingleSwitchProgramWidget* widget = new SingleSwitchProgramWidget(parent, instance, holder);
    widget->construct();
    return widget;
}
void SingleSwitchProgramWidget::run_switch_program(const ProgramInfo& info){
    SingleSwitchProgramInstance& instance = static_cast<SingleSwitchProgramInstance&>(m_instance);

    CancellableHolder<CancellableScope> scope;
    SingleSwitchProgramEnvironment env(
        info,
        scope,
        m_logger,
        m_current_stats.get(), m_historical_stats.get(),
        system().logger(),
        sanitize_botbase(system().botbase()),
        system().camera(),
        system().overlay(),
        system().audio()
    );
    connect(
        &env, &ProgramEnvironment::set_status,
        this, &SingleSwitchProgramWidget::status_update
    );

    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_scope = &scope;
    }
    try{
        start_program_video_check(env.console, instance.descriptor().feedback());
        BotBaseContext context(scope, env.console.botbase());
        instance.program(env, context);
        std::lock_guard<std::mutex> lg(m_lock);
        m_scope = nullptr;
    }catch (...){
        env.update_stats();
        std::lock_guard<std::mutex> lg(m_lock);
        m_scope = nullptr;
        throw;
    }
}




SingleSwitchProgramWidget2::~SingleSwitchProgramWidget2(){
    m_session.remove_listener(*this);
    delete m_actions_bar;
    delete m_stats_bar;
    delete m_options;
    delete m_system;
}
SingleSwitchProgramWidget2::SingleSwitchProgramWidget2(
    QWidget& parent,
    SingleSwitchProgramOption& option,
    PanelHolder& holder
)
    : QWidget(&parent)
    , m_holder(holder)
    , m_session(option)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    const SingleSwitchProgramDescriptor& descriptor = option.descriptor();

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

        m_system = new SwitchSystemWidget(*this, m_session.system(), m_session.instance_id());
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

void SingleSwitchProgramWidget2::state_change(ProgramState state){
    QMetaObject::invokeMethod(this, [=]{
        m_system->update_ui(state);
        m_options->widget().setEnabled(state == ProgramState::STOPPED);
        m_actions_bar->set_state(state);
    });
}
void SingleSwitchProgramWidget2::stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats){
    QMetaObject::invokeMethod(this, [=]{
        m_stats_bar->set_stats(
            current_stats == nullptr ? "" : current_stats->to_str(),
            historical_stats == nullptr ? "" : historical_stats->to_str()
        );
    });
}
void SingleSwitchProgramWidget2::error(const std::string& message){
    QMetaObject::invokeMethod(this, [=]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    });
}








}
}

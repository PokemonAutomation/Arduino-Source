/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "SwitchProgramTracker.h"
#include "SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



SingleSwitchProgramInstance::SingleSwitchProgramInstance(const RunnableSwitchProgramDescriptor& descriptor)
    : RunnableSwitchProgramInstance(descriptor)
    , m_switch(
        "Switch Settings", "Switch 0",
        descriptor.min_pabotbase_level(),
        descriptor.feedback()
    )
{
    m_setup = &m_switch;
}
QWidget* SingleSwitchProgramInstance::make_widget(QWidget& parent, PanelListener& listener){
    return SingleSwitchProgramWidget::make(parent, *this, listener);
}



SingleSwitchProgramWidget::~SingleSwitchProgramWidget(){
    SwitchProgramTracker::instance().remove_program(*this);
    on_destruct_stop();
}
SingleSwitchProgramWidget* SingleSwitchProgramWidget::make(
    QWidget& parent,
    SingleSwitchProgramInstance& instance,
    PanelListener& listener
){
    SingleSwitchProgramWidget* widget = new SingleSwitchProgramWidget(parent, instance, listener);
    widget->construct();
    SwitchProgramTracker::instance().add_program(*widget);
    return widget;
}
void SingleSwitchProgramWidget::run_program(
    StatsTracker* current_stats,
    const StatsTracker* historical_stats
){
    SingleSwitchProgramInstance& instance = static_cast<SingleSwitchProgramInstance&>(m_instance);
    SingleSwitchProgramEnvironment env(
        m_logger,
        current_stats, historical_stats,
        m_logger.base_logger(),
        sanitize_botbase(system().botbase()),
        system().camera(),
        system().overlay()
    );
    connect(
        this, &RunnableSwitchProgramWidget::signal_cancel,
        &env, [&]{
            m_state.store(ProgramState::STOPPING, std::memory_order_release);
            env.signal_stop();
        },
        Qt::DirectConnection
    );
    connect(
        &env, &ProgramEnvironment::set_status,
        this, &SingleSwitchProgramWidget::set_status
    );

    instance.program(env);
}





}
}

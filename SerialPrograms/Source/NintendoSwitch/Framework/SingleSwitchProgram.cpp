/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
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
    on_destruct_stop();
}
SingleSwitchProgramWidget* SingleSwitchProgramWidget::make(
    QWidget& parent,
    SingleSwitchProgramInstance& instance,
    PanelListener& listener
){
    SingleSwitchProgramWidget* widget = new SingleSwitchProgramWidget(parent, instance, listener);
    widget->construct();
    return widget;
}
void SingleSwitchProgramWidget::run_program(
    StatsTracker* current_stats,
    const StatsTracker* historical_stats
){
    SingleSwitchProgramInstance& instance = static_cast<SingleSwitchProgramInstance&>(m_instance);
    SingleSwitchProgramEnvironment env(
        ProgramInfo(
            instance.descriptor().identifier(),
            instance.descriptor().category(),
            instance.descriptor().display_name(),
            timestamp()
        ),
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

    try{
        instance.program(env);
    }catch (...){
        env.update_stats();
        throw;
    }
}





}
}

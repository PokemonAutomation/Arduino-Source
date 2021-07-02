/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
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
    if (!m_destructing){
        stop();
        m_destructing = true;
    }
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
    SwitchSystem* system = static_cast<SwitchSystem*>(m_setup);
    SingleSwitchProgramEnvironment env(
        m_logger,
        current_stats, historical_stats,
        sanitize_botbase(system->botbase()),
        system->camera()
    );
    connect(
        this, &RunnableSwitchProgramWidget::signal_cancel,
        &env, [&]{
            env.signal_stop();
        },
        Qt::DirectConnection
    );
    connect(
        &env, &ProgramEnvironment::set_status,
        this, &SingleSwitchProgramWidget::set_status
    );
    SingleSwitchProgramInstance& instance = static_cast<SingleSwitchProgramInstance&>(m_instance);
    instance.program(env);
}





}
}

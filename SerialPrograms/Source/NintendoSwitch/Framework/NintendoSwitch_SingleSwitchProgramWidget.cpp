/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/BlackBorderCheck.h"
#include "NintendoSwitch_SingleSwitchProgramWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


SingleSwitchProgramWidget::~SingleSwitchProgramWidget(){
    RunnableSwitchProgramWidget::request_program_stop();
    join_program_thread();
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
void SingleSwitchProgramWidget::run_switch_program(const ProgramInfo& info){
    SingleSwitchProgramInstance& instance = static_cast<SingleSwitchProgramInstance&>(m_instance);
    SingleSwitchProgramEnvironment env(
        info,
        m_logger,
        m_current_stats.get(), m_historical_stats.get(),
        system().logger(),
        sanitize_botbase(system().botbase()),
        system().camera(),
        system().overlay(),
        system().audio()
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
        this, &SingleSwitchProgramWidget::status_update
    );

    try{
        start_program_video_check(env.console, instance.descriptor().feedback());
        instance.program(env, env.scope());
    }catch (...){
        env.update_stats();
        throw;
    }
}





}
}

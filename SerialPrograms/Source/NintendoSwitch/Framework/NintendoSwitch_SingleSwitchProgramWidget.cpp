/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/BlackBorderCheck.h"
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
#if 0
    connect(
        this, &RunnableSwitchProgramWidget::signal_cancel,
        &env, [&]{
            m_state.store(ProgramState::STOPPING, std::memory_order_release);
            env.signal_stop();
        },
        Qt::DirectConnection
    );
#endif
    connect(
        &env, &ProgramEnvironment::set_status,
        this, &SingleSwitchProgramWidget::status_update
    );

    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_env = &env;
    }
    try{
        start_program_video_check(env.console, instance.descriptor().feedback());
        BotBaseContext context(env.scope(), env.console);
        instance.program(env, context);
        std::lock_guard<std::mutex> lg(m_lock);
        cout << "clearing()" << endl;
        m_env = nullptr;
    }catch (...){
        env.update_stats();
        std::lock_guard<std::mutex> lg(m_lock);
        cout << "clearing()" << endl;
        m_env = nullptr;
        throw;
    }
}





}
}

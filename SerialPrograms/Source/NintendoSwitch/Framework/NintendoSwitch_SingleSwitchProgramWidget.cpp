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





}
}

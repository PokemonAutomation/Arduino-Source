/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/FixedLimitVector.tpp"
#include "CommonFramework/Options/BatchOption/BatchWidget.h"
#include "CommonFramework/Tools/BlackBorderCheck.h"
#include "NintendoSwitch_SwitchSystemWidget.h"
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
    PanelListener& listener
){
    instance.update_active_consoles();
    MultiSwitchProgramWidget* widget = new MultiSwitchProgramWidget(parent, instance, listener);
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
    MultiSwitchProgramEnvironment env(
        info,
        m_logger,
        m_current_stats.get(), m_historical_stats.get(),
        std::move(switches)
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
        this, [=](QString status){
            this->status_update(std::move(status));
        }
    );

    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_env = &env;
    }
    try{
        start_program_video_check(env.consoles, instance.descriptor().feedback());
        instance.program(env, env.scope());
        std::lock_guard<std::mutex> lg(m_lock);
        m_env = nullptr;
    }catch (...){
        env.update_stats();
        std::lock_guard<std::mutex> lg(m_lock);
        m_env = nullptr;
        throw;
    }
}




}
}

/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cstddef>
#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
#include "CommonFramework/Options/BatchOption/BatchWidget.h"
#include "MultiSwitchProgram.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


MultiSwitchProgramEnvironment::MultiSwitchProgramEnvironment(
    ProgramInfo program_info,
    Logger& logger,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats,
    FixedLimitVector<ConsoleHandle> p_switches
)
    : ProgramEnvironment(std::move(program_info), logger, current_stats, historical_stats)
    , consoles(std::move(p_switches))
{}

void MultiSwitchProgramEnvironment::run_in_parallel(
    const std::function<void(ConsoleHandle& console)>& func
){
    run_in_parallel(0, consoles.size(), func);
}
void MultiSwitchProgramEnvironment::run_in_parallel(
    size_t s, size_t e,
    const std::function<void(ConsoleHandle& console)>& func
){
    dispatcher().run_in_parallel(
        s, e,
        [&](size_t index){
            func(consoles[index]);
            consoles[index].botbase().wait_for_all_requests();
        }
    );
}



MultiSwitchProgramDescriptor::MultiSwitchProgramDescriptor(
    std::string identifier,
    QString category, QString display_name,
    QString doc_link,
    QString description,
    FeedbackType feedback,
    PABotBaseLevel min_pabotbase_level,
    size_t min_switches,
    size_t max_switches,
    size_t default_switches
)
    : RunnableSwitchProgramDescriptor(
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        feedback,
        min_pabotbase_level
    )
    , m_min_switches(min_switches)
    , m_max_switches(max_switches)
    , m_default_switches(default_switches)
{}



MultiSwitchProgramInstance::MultiSwitchProgramInstance(const MultiSwitchProgramDescriptor& descriptor)
    : RunnableSwitchProgramInstance(descriptor)
    , m_switches(
        descriptor.min_pabotbase_level(),
        descriptor.feedback(),
        descriptor.min_switches(),
        descriptor.max_switches(),
        descriptor.default_switches()
    )
{
    m_setup = &m_switches;
}
QWidget* MultiSwitchProgramInstance::make_widget(QWidget& parent, PanelListener& listener){
    return MultiSwitchProgramWidget::make(parent, *this, listener);
}




MultiSwitchProgramWidget::~MultiSwitchProgramWidget(){
    on_destruct_stop();
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
        widget->m_setup, &SwitchSetup::on_setup_changed,
        widget, [=, &instance]{
            instance.update_active_consoles();
            widget->m_options->update_visibility();
//            widget->redraw_options();
        }
    );
    return widget;
}
void MultiSwitchProgramWidget::run_program(
    StatsTracker* current_stats,
    const StatsTracker* historical_stats
){
    MultiSwitchProgramInstance& instance = static_cast<MultiSwitchProgramInstance&>(m_instance);
    FixedLimitVector<ConsoleHandle> switches(instance.system_count());
    for (size_t c = 0; c < instance.system_count(); c++){
        SwitchSystem& system = this->system(c);
        switches.emplace_back(
            c,
            m_logger.base_logger(),
            sanitize_botbase(system.botbase()),
            system.camera(),
            system.overlay()
        );
    }
    MultiSwitchProgramEnvironment env(
        ProgramInfo(
            instance.descriptor().identifier(),
            instance.descriptor().category(),
            instance.descriptor().display_name(),
            timestamp()
        ),
        m_logger,
        current_stats, historical_stats,
        std::move(switches)
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
        this, [=](QString status){
            this->set_status(std::move(status));
        }
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

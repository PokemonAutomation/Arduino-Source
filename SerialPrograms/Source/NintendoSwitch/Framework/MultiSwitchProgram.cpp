/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




MultiSwitchProgramEnvironment::MultiSwitchProgramEnvironment(
    Logger& logger,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats,
    FixedLimitVector<ConsoleHandle> p_switches
)
    : ProgramEnvironment(logger, current_stats, historical_stats)
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
    QString display_name,
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
        std::move(display_name),
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
    if (!m_destructing){
        stop();
        m_destructing = true;
    }
}
MultiSwitchProgramWidget* MultiSwitchProgramWidget::make(
    QWidget& parent,
    MultiSwitchProgramInstance& instance,
    PanelListener& listener
){
    MultiSwitchProgramWidget* widget = new MultiSwitchProgramWidget(parent, instance, listener);
    widget->construct();
    return widget;
}
void MultiSwitchProgramWidget::run_program(
    StatsTracker* current_stats,
    const StatsTracker* historical_stats
){
    MultiSwitchProgramInstance& instance = static_cast<MultiSwitchProgramInstance&>(m_instance);
    FixedLimitVector<ConsoleHandle> switches(instance.count());
    for (size_t c = 0; c < instance.count(); c++){
        SwitchSystem& system = static_cast<MultiSwitchSystem&>(*m_setup)[c];
        switches.emplace_back(
            c,
            sanitize_botbase(system.botbase()),
            system.camera()
        );
    }
    MultiSwitchProgramEnvironment env(
        m_logger,
        current_stats, historical_stats,
        std::move(switches)
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
        this, [=](QString status){
            this->set_status(std::move(status));
        }
    );
    instance.program(env);
}





}
}

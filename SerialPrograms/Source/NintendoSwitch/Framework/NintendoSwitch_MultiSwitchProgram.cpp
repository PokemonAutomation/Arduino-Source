/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/AsyncDispatcher.h"
#include "NintendoSwitch_MultiSwitchProgram.h"
#include "NintendoSwitch_MultiSwitchProgramWidget.h"

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








}
}

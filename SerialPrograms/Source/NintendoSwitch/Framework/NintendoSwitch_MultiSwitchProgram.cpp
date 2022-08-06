/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/FixedLimitVector.tpp"
#include "Common/Cpp/AsyncDispatcher.h"
#include "NintendoSwitch_MultiSwitchProgram.h"
#include "NintendoSwitch_MultiSwitchProgramWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

MultiSwitchProgramEnvironment::~MultiSwitchProgramEnvironment(){}

MultiSwitchProgramEnvironment::MultiSwitchProgramEnvironment(
    const ProgramInfo& program_info,
    CancellableScope& scope,
    Logger& logger,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats,
    FixedLimitVector<ConsoleHandle> p_switches
)
    : ProgramEnvironment(program_info, logger, current_stats, historical_stats)
    , consoles(std::move(p_switches))
{
    for (ConsoleHandle& console : consoles){
        console.initialize_inference_threads(scope, inference_dispatcher());
    }
}

void MultiSwitchProgramEnvironment::run_in_parallel(
    CancellableScope& scope,
    const std::function<void(ConsoleHandle& console, BotBaseContext& context)>& func
){
    run_in_parallel(scope, 0, consoles.size(), func);
}
void MultiSwitchProgramEnvironment::run_in_parallel(
    CancellableScope& scope, size_t s, size_t e,
    const std::function<void(ConsoleHandle& console, BotBaseContext& context)>& func
){
    realtime_dispatcher().run_in_parallel(
        s, e,
        [&](size_t index){
            BotBaseContext context(scope, consoles[index].botbase());
            func(consoles[index], context);
            context.wait_for_all_requests();
        }
    );
}



MultiSwitchProgramDescriptor::MultiSwitchProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    FeedbackType feedback, bool allow_commands_while_running,
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
        feedback, allow_commands_while_running,
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
        descriptor.allow_commands_while_running(),
        descriptor.min_switches(),
        descriptor.max_switches(),
        descriptor.default_switches()
    )
{
    m_setup = &m_switches;
}
QWidget* MultiSwitchProgramInstance::make_widget(QWidget& parent, PanelHolder& holder){
    return MultiSwitchProgramWidget::make(parent, *this, holder);
}








}
}

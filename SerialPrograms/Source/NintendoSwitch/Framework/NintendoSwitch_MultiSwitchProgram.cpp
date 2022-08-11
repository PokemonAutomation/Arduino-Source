/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/FixedLimitVector.tpp"
#include "Common/Cpp/AsyncDispatcher.h"
#include "ClientSource/Connection/BotBase.h"
#include "NintendoSwitch_MultiSwitchProgram.h"
#include "NintendoSwitch_MultiSwitchProgramOption.h"

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
    : RunnablePanelDescriptor(
        pick_color(feedback, min_pabotbase_level),
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description)
    )
    , m_feedback(feedback)
    , m_min_pabotbase_level(min_pabotbase_level)
    , m_allow_commands_while_running(allow_commands_while_running)
    , m_min_switches(min_switches)
    , m_max_switches(max_switches)
    , m_default_switches(default_switches)
{}
std::unique_ptr<PanelInstance> MultiSwitchProgramDescriptor::make_panel() const{
    return std::unique_ptr<PanelInstance>(new MultiSwitchProgramOption(*this));
}





MultiSwitchProgramInstance2::MultiSwitchProgramInstance2()
    : NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATION_ERROR_RECOVERABLE(
        "Program Error (Recoverable)",
        true, false,
        ImageAttachmentMode::PNG,
        {"Notifs"}
    )
    , NOTIFICATION_ERROR_FATAL(
        "Program Error (Fatal)",
        true, true,
//        ImageAttachmentMode::PNG,
        {"Notifs"}
    )
{}
void MultiSwitchProgramInstance2::add_option(ConfigOption& option, std::string serialization_string){
    m_options.add_option(option, std::move(serialization_string));
}
void MultiSwitchProgramInstance2::from_json(const JsonValue& json){
    m_options.load_json(json);
}
JsonValue MultiSwitchProgramInstance2::to_json() const{
    return m_options.to_json();
}
std::string MultiSwitchProgramInstance2::check_validity() const{
    return m_options.check_validity();
}
void MultiSwitchProgramInstance2::restore_defaults(){
    return m_options.restore_defaults();
}










}
}

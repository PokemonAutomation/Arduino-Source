/*  Console Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  ConsolePanel is a panel with:
 *    - Has options.
 *    - Has a console.
 *    - Can be run.
 *
 *  This is the main header for game console programs.
 *  This file is intentionally thin and has only the bare minimum dependencies
 *  for programs.
 *
 */

#ifndef PokemonAutomation_GameConsole_ConsoleProgram_H
#define PokemonAutomation_GameConsole_ConsoleProgram_H

#include <memory>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "ConsoleHandle.h"

namespace PokemonAutomation{
namespace GameConsole{

class ConsoleSystemSession;
class ConsoleProgramInstance;



class ConsoleProgramEnvironment : public ProgramEnvironment{
public:
    ConsoleProgramEnvironment(
        const ProgramInfo& program_info,
        CancellableScope& scope,
        ProgramSession& session,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats,
        ConsoleSystemSession& system
    );
    ConsoleProgramEnvironment(
        const ProgramInfo& program_info,
        CancellableScope& scope,
        ProgramSession& session,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats,
        std::unique_ptr<ConsoleHandle> console
    );


public:
    ConsoleHandle& console(){ return *m_console; }
    virtual void log_to_ui(const std::string& msg, Color color = Color()) override;


private:
    std::unique_ptr<ConsoleHandle> m_console;
};



class ConsoleProgramDescriptor : public ProgramDescriptor{
public:
    ConsoleProgramDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        ProgramControllerClass controller_class,
        Color color,
        FeedbackType feedback,
        AllowCommandsWhenRunning allow_commands_while_running,
        PanelDeprecation deprecation = PanelDeprecation::NOT_DEPRECATED,
        std::vector<std::string> required_resources = {}
    );

    ProgramControllerClass controller_class() const{ return m_controller_class; }
    FeedbackType feedback() const{ return m_feedback; }
    bool allow_commands_while_running() const{ return m_allow_commands_while_running; }

    virtual std::unique_ptr<PanelSession> make_panel() const override;
    virtual std::unique_ptr<ConsoleProgramInstance> make_instance(
        ConsoleSystemSession& system
    ) const = 0;


private:
    const ProgramControllerClass m_controller_class;
    const FeedbackType m_feedback;
    const bool m_allow_commands_while_running;
};



class ConsoleProgramInstance : public ProgramInstance{
public:
    using ProgramInstance::ProgramInstance;

    virtual void program(ConsoleProgramEnvironment& env, CancellableScope& scope) = 0;


public:
    //  Startup Checks: Feel free to override to change behavior.

    virtual void run_start_program_checks(
        const ProgramDescriptor& descriptor,
        ProgramEnvironment& env
    ) override;
    virtual void start_program_feedback_check(
        VideoStream& stream,
        FeedbackType feedback_type
    );
};




template <typename Instance>
class ConsoleProgramWrapper : public Instance::Descriptor{
public:
    virtual std::unique_ptr<ConsoleProgramInstance> make_instance(
        ConsoleSystemSession& system
    ) const override{
        if constexpr (std::is_constructible_v<Instance, ConsoleSystemSession&>){
            return std::make_unique<Instance>(system);
        }else{
            return std::make_unique<Instance>();
        }
    }
};

template <typename Instance>
std::unique_ptr<PanelDescriptor> make_ConsoleProgram(){
    return std::make_unique<ConsoleProgramWrapper<Instance>>();
}




}
}
#endif

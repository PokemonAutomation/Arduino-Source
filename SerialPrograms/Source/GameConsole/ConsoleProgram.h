/*  Console Program
 *
 *  From: https://github.com/PokemonAutomation/
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
    using SystemSession = ConsoleSystemSession;

public:
    ConsoleProgramDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        Color color,
        FeedbackType feedback,
        AllowCommandsWhenRunning allow_commands_while_running,
        PanelDeprecation deprecation = PanelDeprecation::NOT_DEPRECATED,
        std::vector<std::string> required_resources = {}
    );

    FeedbackType feedback() const{ return m_feedback; }
    bool allow_commands_while_running() const{ return m_allow_commands_while_running; }

    virtual std::unique_ptr<PanelSession> make_panel() const override;
    virtual std::unique_ptr<ConsoleProgramInstance> make_instance(
        ConsoleSystemSession& system
    ) const = 0;


private:
    const FeedbackType m_feedback;
    const bool m_allow_commands_while_running;
};



class ConsoleProgramInstance : public ProgramInstance{
public:
    using ProgramInstance::ProgramInstance;

    virtual void program(ConsoleProgramEnvironment& env, CancellableScope& scope);


public:
    //  Startup Checks: Feel free to override to change behavior.

    virtual void start_program_controller_check(
        ConsoleSystemSession& session
    );
    virtual void start_program_feedback_check(
        VideoStream& stream,
        FeedbackType feedback_type
    );
};




template <typename Descriptor, typename Instance>
class ConsoleProgramWrapper : public Descriptor{
public:
    using SystemSession = typename Descriptor::SystemSession;

    virtual std::unique_ptr<ConsoleProgramInstance> make_instance(
        SystemSession& system
    ) const override{
        if constexpr (std::is_constructible_v<Instance, SystemSession&>){
            return std::make_unique<Instance>(system);
        }else{
            return std::make_unique<Instance>();
        }
    }
};

template <typename Descriptor, typename Instance>
std::unique_ptr<PanelDescriptor> make_ConsoleProgram(){
    return std::make_unique<ConsoleProgramWrapper<Descriptor, Instance>>();
}




}
}
#endif

/*  Multi-Console Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_MultiConsoleProgram_H
#define PokemonAutomation_GameConsole_MultiConsoleProgram_H

#include <functional>
#include <memory>
#include <vector>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"
#include "ConsoleHandle.h"

namespace PokemonAutomation{
namespace GameConsole{

class MultiConsoleSystemSession;
class MultiConsoleProgramInstance;



class MultiConsoleProgramEnvironment : public ProgramEnvironment{
public:
    MultiConsoleProgramEnvironment(
        const ProgramInfo& program_info,
        CancellableScope& scope,
        ProgramSession& session,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats,
        std::vector<std::unique_ptr<ConsoleHandle>> consoles
    );


public:
    size_t consoles() const{ return m_consoles.size(); }
    ConsoleHandle& console(size_t index){ return *m_consoles[index]; }
    virtual void log_to_ui(const std::string& msg, Color color = Color()) override;


public:
    //  Run the specified lambda for all consoles in parallel.
    void run_in_parallel(
        CancellableScope& scope,
        const std::function<void(CancellableScope& scope, ConsoleHandle& console)>& func
    ){
        run_in_parallel(scope, 0, m_consoles.size(), func);
    }

    //  Run the specified lambda for switch indices [s, e) in parallel.
    void run_in_parallel(
        CancellableScope& scope, size_t s, size_t e,
        const std::function<void(CancellableScope& scope, ConsoleHandle& console)>& func
    );


protected:
    template <typename ConsoleHandleType>
    void run_in_parallel(
        CancellableScope& scope,
        const std::function<void(CancellableScope& scope, ConsoleHandleType& console)>& func
    ){
        run_in_parallel<ConsoleHandleType>(scope, 0, m_consoles.size(), func);
    }
    template <typename ConsoleHandleType>
    void run_in_parallel(
        CancellableScope& scope, size_t s, size_t e,
        const std::function<void(CancellableScope& scope, ConsoleHandleType& console)>& func
    ){
        GameConsole::MultiConsoleProgramEnvironment::run_in_parallel(
            scope, s, e,
            [&](CancellableScope& scope, ConsoleHandle& console){
                func(scope, dynamic_cast<ConsoleHandleType&>(console));
            }
        );
    }

    template <typename ConsoleHandleType, typename ControllerType>
    void run_in_parallel(
        CancellableScope& scope,
        const std::function<void(ConsoleHandleType& console, ControllerContext<ControllerType>& context)>& func
    ){
        run_in_parallel<ConsoleHandleType, ControllerType>(scope, 0, m_consoles.size(), func);
    }
    template <typename ConsoleHandleType, typename ControllerType>
    void run_in_parallel(
        CancellableScope& scope, size_t s, size_t e,
        const std::function<void(ConsoleHandleType& console, ControllerContext<ControllerType>& context)>& func
    ){
        GameConsole::MultiConsoleProgramEnvironment::run_in_parallel(
            scope, s, e,
            [&](CancellableScope& scope, ConsoleHandle& console){
                ControllerContext<ControllerType> context(scope, console.controller<ControllerType>());
                func(dynamic_cast<ConsoleHandleType&>(console), context);
            }
        );
    }


private:
    std::vector<std::unique_ptr<ConsoleHandle>> m_consoles;
};



class MultiConsoleProgramDescriptor : public ProgramDescriptor{
public:
    MultiConsoleProgramDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        ProgramControllerClass controller_class,
        Color color,
        FeedbackType feedback,
        AllowCommandsWhenRunning allow_commands_while_running,
        size_t min_consoles,
        size_t max_consoles,
        size_t default_consoles,
        PanelDeprecation deprecation = PanelDeprecation::NOT_DEPRECATED,
        std::vector<std::string> required_resources = {}
    );

    ProgramControllerClass controller_class() const{ return m_controller_class; }
    FeedbackType feedback() const{ return m_feedback; }
    bool allow_commands_while_running() const{ return m_allow_commands_while_running; }

    size_t min_consoles() const{ return m_min_consoles; }
    size_t max_consoles() const{ return m_max_consoles; }
    size_t default_consoles() const{ return m_default_consoles; }

    virtual std::unique_ptr<PanelSession> make_panel() const override;
    virtual std::unique_ptr<MultiConsoleProgramInstance> make_instance(
        MultiConsoleSystemSession& system
    ) const = 0;

private:
    const ProgramControllerClass m_controller_class;
    const FeedbackType m_feedback;
    const bool m_allow_commands_while_running;

    const size_t m_min_consoles;
    const size_t m_max_consoles;
    const size_t m_default_consoles;
};



class MultiConsoleProgramInstance : public ProgramInstance{
public:
    //  Options are locked individually while the program runs, so options
    //  constructed with UNLOCK_WHILE_RUNNING (and buttons) stay usable.
    MultiConsoleProgramInstance(
        const std::vector<std::string>& error_notification_tags = {"Notifs"}
    )
        : ProgramInstance(error_notification_tags, LockMode::UNLOCK_WHILE_RUNNING)
    {}

    //  Called when the # of consoles changes.
    virtual void update_active_consoles(size_t console_count){}

    virtual void program(MultiConsoleProgramEnvironment& env, CancellableScope& scope) = 0;


public:
    //  Startup Checks: Feel free to override to change behavior.

    virtual void run_start_program_checks(
        const ProgramDescriptor& descriptor,
        ProgramEnvironment& env
    ) override;
    virtual void start_program_feedback_check(
        const MultiConsoleProgramDescriptor& descriptor,
        size_t console_index, VideoStream& stream
    );
};





template <typename Instance>
class MultiConsoleProgramWrapper : public Instance::Descriptor{
public:
    virtual std::unique_ptr<MultiConsoleProgramInstance> make_instance(
        MultiConsoleSystemSession& system
    ) const override{
        if constexpr (std::is_constructible_v<Instance, MultiConsoleSystemSession&>){
            return std::make_unique<Instance>(system);
        }else{
            return std::make_unique<Instance>();
        }
    }
};

template <typename Instance>
std::unique_ptr<PanelDescriptor> make_MultiConsoleProgram(){
    return std::make_unique<MultiConsoleProgramWrapper<Instance>>();
}












}
}
#endif

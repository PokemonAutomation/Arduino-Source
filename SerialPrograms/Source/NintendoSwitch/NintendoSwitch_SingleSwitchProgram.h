/*  Single Switch Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SingleSwitchProgram_H
#define PokemonAutomation_NintendoSwitch_SingleSwitchProgram_H

#include <type_traits>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "GameConsole/ConsoleProgram.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
    class ControllerSession;
namespace NintendoSwitch{


class SwitchSystemSession;
class SingleSwitchProgramInstance;


class SingleSwitchProgramEnvironment : public GameConsole::ConsoleProgramEnvironment{
public:
    ConsoleHandle& console;

public:
    friend class SingleSwitchProgramSession;
    friend class SingleSwitchProgramWidget;

    SingleSwitchProgramEnvironment(
        const ProgramInfo& program_info,
        CancellableScope& scope,
        ProgramSession& session,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats,
        GameConsole::ConsoleSystemSession& system
    );
};



class SingleSwitchProgramDescriptor : public GameConsole::ConsoleProgramDescriptor{
public:
    SingleSwitchProgramDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        ProgramControllerClass controller_class,
        FeedbackType feedback,
        AllowCommandsWhenRunning allow_commands_while_running,
        PanelDeprecation deprecation = PanelDeprecation::NOT_DEPRECATED,
        std::vector<std::string> required_resources = {}
    );

    ProgramControllerClass controller_class() const{ return m_controller_class; }

    virtual std::unique_ptr<PanelSession> make_panel() const override;

private:
    const ProgramControllerClass m_controller_class;
};



class SingleSwitchProgramInstance : public GameConsole::ConsoleProgramInstance{
public:
    using GameConsole::ConsoleProgramInstance::ConsoleProgramInstance;

    //  Called by SingleSwitchProgramSession::run_program_instance() to start an automation program.
    //  Child classes should override one of the overloaded functions.
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope);
    //  Called by SingleSwitchProgramSession::run_program_instance() to start an automation program.
    //  Child classes should override one of the overloaded functions.
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context);


public:
    //  Startup Checks: Feel free to override to change behavior.

    virtual void run_start_program_checks(
        const ProgramDescriptor& descriptor,
        ProgramEnvironment& env
    ) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    );

private:
    virtual void program(GameConsole::ConsoleProgramEnvironment& env, CancellableScope& scope) override;
};




#if 0
template <typename Instance>
class SingleSwitchProgramWrapper : public Instance::Descriptor{
public:
    virtual std::unique_ptr<SingleSwitchProgramInstance> make_instance(
        GameConsole::ConsoleSystemSession& system
    ) const override{
        if constexpr (std::is_constructible_v<Instance, GameConsole::ConsoleSystemSession&>){
            return std::make_unique<Instance>(system);
        }else{
            return std::make_unique<Instance>();
        }
    }
};
#endif

template <typename Instance>
using SingleSwitchProgramWrapper = GameConsole::ConsoleProgramWrapper<Instance>;


// Create a program PanelDescriptor
template <typename Instance>
std::unique_ptr<PanelDescriptor> make_SingleSwitchProgram(){
    return std::make_unique<SingleSwitchProgramWrapper<Instance>>();
}








}
}
#endif


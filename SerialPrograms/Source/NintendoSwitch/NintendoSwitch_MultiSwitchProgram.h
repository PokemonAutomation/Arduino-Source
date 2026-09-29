/*  Multi-Switch Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_MultiSwitchProgram_H
#define PokemonAutomation_NintendoSwitch_MultiSwitchProgram_H

//#include "Common/Compiler.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"
#include "GameConsole/MultiConsoleProgram.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
    class ControllerSession;
namespace NintendoSwitch{


class SwitchSystemSession;
class MultiSwitchProgramInstance;


class MultiSwitchProgramEnvironment : public GameConsole::MultiConsoleProgramEnvironment{
public:
    MultiSwitchProgramEnvironment(
        const ProgramInfo& program_info,
        CancellableScope& scope,
        ProgramSession& session,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats,
        std::vector<std::unique_ptr<GameConsole::ConsoleHandle>> consoles
    );

    ConsoleHandle& console(size_t index){
        return static_cast<ConsoleHandle&>(GameConsole::MultiConsoleProgramEnvironment::console(index));
    }


public:
    void run_in_parallel(
        CancellableScope& scope,
        const std::function<void(CancellableScope& scope, ConsoleHandle& console)>& func
    ){
        GameConsole::MultiConsoleProgramEnvironment::run_in_parallel<ConsoleHandle>(
            scope, func
        );
    }
    void run_in_parallel(
        CancellableScope& scope, size_t s, size_t e,
        const std::function<void(CancellableScope& scope, ConsoleHandle& console)>& func
    ){
        GameConsole::MultiConsoleProgramEnvironment::run_in_parallel<ConsoleHandle>(
            scope, s, e, func
        );
    }

    template <typename ControllerType>
    void run_in_parallel(
        CancellableScope& scope,
        const std::function<void(ConsoleHandle& console, ControllerContext<ControllerType>& context)>& func
    ){
        GameConsole::MultiConsoleProgramEnvironment::run_in_parallel<ConsoleHandle, ControllerType>(
            scope, func
        );
    }
    template <typename ControllerType>
    void run_in_parallel(
        CancellableScope& scope, size_t s, size_t e,
        const std::function<void(ConsoleHandle& console, ControllerContext<ControllerType>& context)>& func
    ){
        GameConsole::MultiConsoleProgramEnvironment::run_in_parallel<ConsoleHandle, ControllerType>(
            scope, s, e, func
        );
    }
};



class MultiSwitchProgramDescriptor : public GameConsole::MultiConsoleProgramDescriptor{
public:
    MultiSwitchProgramDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        ProgramControllerClass controller_class,
        FeedbackType feedback,
        AllowCommandsWhenRunning allow_commands_while_running,
        size_t min_switches,
        size_t max_switches,
        size_t default_switches,
        PanelDeprecation deprecation = PanelDeprecation::NOT_DEPRECATED,
        std::vector<std::string> required_resources = {}
    );

    virtual std::unique_ptr<PanelSession> make_panel() const override;
};



class MultiSwitchProgramInstance : public GameConsole::MultiConsoleProgramInstance{
public:
    using GameConsole::MultiConsoleProgramInstance::MultiConsoleProgramInstance;

    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) = 0;


public:
    //  Startup Checks: Feel free to override to change behavior.

    virtual void run_start_program_checks(
        const ProgramDescriptor& descriptor,
        ProgramEnvironment& env
    ) override;
    virtual void start_program_border_check(
        VideoStream& stream, size_t console_index,
        FeedbackType feedback_type
    );

private:
    virtual void program(GameConsole::MultiConsoleProgramEnvironment& env, CancellableScope& scope) override;
};



#if 0
template <typename Instance>
class MultiSwitchProgramWrapper : public Instance::Descriptor{
public:
    virtual std::unique_ptr<MultiSwitchProgramInstance> make_instance() const override{
        return std::unique_ptr<MultiSwitchProgramInstance>(new Instance());
    }
};
#endif


template <typename Instance>
using MultiSwitchProgramWrapper = GameConsole::MultiConsoleProgramWrapper<Instance>;

template <typename Instance>
std::unique_ptr<PanelDescriptor> make_MultiSwitchProgram(){
    return std::make_unique<MultiSwitchProgramWrapper<Instance>>();
}









}
}
#endif


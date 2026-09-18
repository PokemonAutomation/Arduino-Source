/*  Single Switch Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SingleSwitchProgram_H
#define PokemonAutomation_NintendoSwitch_SingleSwitchProgram_H

#include <type_traits>
#include "Common/Cpp/Options/BatchOption.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
    class ControllerSession;
namespace NintendoSwitch{


class SwitchSystemSession;
class SingleSwitchProgramInstance;


class SingleSwitchProgramEnvironment : public ProgramEnvironment{
public:
    ConsoleHandle console;

    // Call console.overlay().add_log(msg, color) to add a log message to overlay display
    virtual void log_to_ui(const std::string& msg, Color color = Color()) override;

public:
    friend class SingleSwitchProgramSession;
    friend class SingleSwitchProgramWidget;
    template <class... Args>
    SingleSwitchProgramEnvironment(
        const ProgramInfo& program_info,
        CancellableScope& scope,
        ProgramSession& session,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats,
        Args&&... args
    )
        : ProgramEnvironment(program_info, session, current_stats, historical_stats)
        , console(std::forward<Args>(args)...)
    {
        console.initialize_inference_threads(scope);
    }
};



class SingleSwitchProgramDescriptor : public ProgramDescriptor{
public:
    SingleSwitchProgramDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        ProgramControllerClass color_class,
        FeedbackType feedback,
        AllowCommandsWhenRunning allow_commands_while_running,
        PanelDeprecation deprecation = PanelDeprecation::NOT_DEPRECATED,
        std::vector<std::string> required_resources = {}
    );

    ProgramControllerClass color_class() const{ return m_color_class; }
    FeedbackType feedback() const{ return m_feedback; }
    bool allow_commands_while_running() const{ return m_allow_commands_while_running; }

    virtual std::unique_ptr<PanelSession> make_panel() const override;
    virtual std::unique_ptr<SingleSwitchProgramInstance> make_instance(
        SwitchSystemSession& system
    ) const = 0;

private:
    const ProgramControllerClass m_color_class;
    const FeedbackType m_feedback;
    const bool m_allow_commands_while_running;
    const std::vector<std::string> m_required_resources;
};



class SingleSwitchProgramInstance : public ProgramInstance{
public:
    using ProgramInstance::ProgramInstance;

    //  Called by SingleSwitchProgramSession::run_program_instance() to start an automation program.
    //  Child classes should override one of the overloaded functions.
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope);
    //  Called by SingleSwitchProgramSession::run_program_instance() to start an automation program.
    //  Child classes should override one of the overloaded functions.
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context);


public:
    //  Startup Checks: Feel free to override to change behavior.

    virtual void start_program_controller_check(
        ControllerSession& session
    );
    virtual void start_program_feedback_check(
        VideoStream& stream,
        FeedbackType feedback_type
    );
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    );
};




template <typename Descriptor, typename Instance>
class SingleSwitchProgramWrapper : public Descriptor{
public:
    virtual std::unique_ptr<SingleSwitchProgramInstance> make_instance(
        SwitchSystemSession& system
    ) const override{
        if constexpr (std::is_constructible_v<Instance, SwitchSystemSession&>){
            return std::make_unique<Instance>(system);
        }else{
            return std::make_unique<Instance>();
        }
    }
};

// Create a program PanelDescriptor
template <typename Descriptor, typename Instance>
std::unique_ptr<PanelDescriptor> make_single_switch_program(){
    return std::make_unique<SingleSwitchProgramWrapper<Descriptor, Instance>>();
}








}
}
#endif


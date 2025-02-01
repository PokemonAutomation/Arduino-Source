/*  Single Switch Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SingleSwitchProgram_H
#define PokemonAutomation_NintendoSwitch_SingleSwitchProgram_H

#include "Common/Cpp/Options/BatchOption.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"
#include "Controllers/ControllerCapability.h"
#include "Controllers/SerialPABotBase/SerialPABotBase.h"    //  REMOVE
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
    class ControllerSession;
namespace NintendoSwitch{


class SingleSwitchProgramInstance;


class SingleSwitchProgramEnvironment : public ProgramEnvironment{
public:
    ConsoleHandle console;

private:
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
        , console(0, std::forward<Args>(args)...)
    {
        console.initialize_inference_threads(scope, inference_dispatcher());
    }
};


class SingleSwitchProgramDescriptor : public ProgramDescriptor{
public:
    SingleSwitchProgramDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        FeedbackType feedback,
        AllowCommandsWhenRunning allow_commands_while_running,
        ControllerRequirements requirements
    );

    FeedbackType feedback() const{ return m_feedback; }
    const ControllerRequirements& requirements() const{ return m_requirements; }
    bool allow_commands_while_running() const{ return m_allow_commands_while_running; }

    virtual std::unique_ptr<PanelInstance> make_panel() const override;
    virtual std::unique_ptr<SingleSwitchProgramInstance> make_instance() const = 0;

private:
    const FeedbackType m_feedback;
    const ControllerRequirements m_requirements;
    const bool m_allow_commands_while_running;
};






//
//  As of this writing, this class will never be called in a manner where
//  thread-safety is of concern with one exception: config options
//
//  Here is the curent status:
//
//  Called from UI thread:
//    - Construction/destruction
//    - from/to_json()
//    - restore_defaults()
//
//  Called from program thread:
//    - program()
//
//  Called from both UI and program threads:
//    - check_validity()
//    - All config options.
//
//  With the exception of the configs, nothing will be called concurrently from
//  different threads.
//
class SingleSwitchProgramInstance{
public:
    virtual ~SingleSwitchProgramInstance();
    SingleSwitchProgramInstance(const SingleSwitchProgramInstance&) = delete;
    void operator=(const SingleSwitchProgramInstance&) = delete;

    SingleSwitchProgramInstance(
        const std::vector<std::string>& error_notification_tags = {"Notifs"}
    );

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) = 0;


public:
    //  Startup Checks: Feel free to override to change behavior.

    virtual void start_program_controller_check(
        CancellableScope& scope,
        ControllerSession& session
    );
    virtual void start_program_feedback_check(
        CancellableScope& scope,
        VideoStream& stream,
        FeedbackType feedback_type
    );
    virtual void start_program_border_check(
        CancellableScope& scope,
        VideoStream& stream
    );


public:
    //  Settings

    virtual void from_json(const JsonValue& json);
    virtual JsonValue to_json() const;

    virtual std::string check_validity() const;
    virtual void restore_defaults();


protected:
    friend class SingleSwitchProgramOption;

    BatchOption m_options;
    void add_option(ConfigOption& option, std::string serialization_string);


public:
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationOption NOTIFICATION_ERROR_RECOVERABLE;
    EventNotificationOption NOTIFICATION_ERROR_FATAL;
};




template <typename Descriptor, typename Instance>
class SingleSwitchProgramWrapper : public Descriptor{
public:
    virtual std::unique_ptr<SingleSwitchProgramInstance> make_instance() const override{
        return std::unique_ptr<SingleSwitchProgramInstance>(new Instance());
    }
};

template <typename Descriptor, typename Instance>
std::unique_ptr<PanelDescriptor> make_single_switch_program(){
    return std::make_unique<SingleSwitchProgramWrapper<Descriptor, Instance>>();
}








}
}
#endif


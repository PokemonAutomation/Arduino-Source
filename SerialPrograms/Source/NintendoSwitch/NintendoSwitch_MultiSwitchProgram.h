/*  Multi-Switch Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_MultiSwitchProgram_H
#define PokemonAutomation_NintendoSwitch_MultiSwitchProgram_H

#include <functional>
//#include "Common/Compiler.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "Common/Cpp/Options/BatchOption.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/ControllerDevices/SerialPortGlobals.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"

namespace PokemonAutomation{
    class BotBaseContext;
namespace NintendoSwitch{


class MultiSwitchProgramInstance;


class MultiSwitchProgramEnvironment : public ProgramEnvironment{
public:
    ~MultiSwitchProgramEnvironment();
    MultiSwitchProgramEnvironment(
        const ProgramInfo& program_info,
        CancellableScope& scope,
        ProgramSession& session,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats,
        FixedLimitVector<ConsoleHandle> p_switches
    );

    FixedLimitVector<ConsoleHandle> consoles;

    //  Run the specified lambda for all switches in parallel.
    void run_in_parallel(
        CancellableScope& scope,
        const std::function<void(ConsoleHandle& console, BotBaseContext& context)>& func
    );

    //  Run the specified lambda for switch indices [s, e) in parallel.
    void run_in_parallel(
        CancellableScope& scope, size_t s, size_t e,
        const std::function<void(ConsoleHandle& console, BotBaseContext& context)>& func
    );

};



class MultiSwitchProgramDescriptor : public ProgramDescriptor{
public:
    MultiSwitchProgramDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        FeedbackType feedback,
        AllowCommandsWhenRunning allow_commands_while_running,
        PABotBaseLevel min_pabotbase_level,
        size_t min_switches,
        size_t max_switches,
        size_t default_switches
    );

    FeedbackType feedback() const{ return m_feedback; }
    PABotBaseLevel min_pabotbase_level() const{ return m_min_pabotbase_level; }
    bool allow_commands_while_running() const{ return m_allow_commands_while_running; }

    size_t min_switches() const{ return m_min_switches; }
    size_t max_switches() const{ return m_max_switches; }
    size_t default_switches() const{ return m_default_switches; }

    virtual std::unique_ptr<PanelInstance> make_panel() const override;
    virtual std::unique_ptr<MultiSwitchProgramInstance> make_instance() const{ return nullptr; }

private:
    const FeedbackType m_feedback;
    const PABotBaseLevel m_min_pabotbase_level;
    const bool m_allow_commands_while_running;

    const size_t m_min_switches;
    const size_t m_max_switches;
    const size_t m_default_switches;
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
class MultiSwitchProgramInstance{
public:
    virtual ~MultiSwitchProgramInstance();
    MultiSwitchProgramInstance(const MultiSwitchProgramInstance&) = delete;
    void operator=(const MultiSwitchProgramInstance&) = delete;

    MultiSwitchProgramInstance(
        const std::vector<std::string>& error_notification_tags = {"Notifs"}
    );

    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) = 0;


public:
    //  Settings

    virtual void from_json(const JsonValue& json);
    virtual JsonValue to_json() const;

    virtual std::string check_validity() const;
    virtual void restore_defaults();

    //  Called when the # of Switches changes.
    virtual void update_active_consoles(size_t switch_count){}


protected:
    friend class MultiSwitchProgramOption;

    BatchOption m_options;
    void add_option(ConfigOption& option, std::string serialization_string);


public:
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationOption NOTIFICATION_ERROR_RECOVERABLE;
    EventNotificationOption NOTIFICATION_ERROR_FATAL;
};




template <typename Descriptor, typename Instance>
class MultiSwitchProgramWrapper : public Descriptor{
public:
    virtual std::unique_ptr<MultiSwitchProgramInstance> make_instance() const override{
        return std::unique_ptr<MultiSwitchProgramInstance>(new Instance());
    }
};

template <typename Descriptor, typename Instance>
std::unique_ptr<PanelDescriptor> make_multi_switch_program(){
    return std::make_unique<MultiSwitchProgramWrapper<Descriptor, Instance>>();
}









}
}
#endif


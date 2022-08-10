/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SingleSwitchProgram_H
#define PokemonAutomation_NintendoSwitch_SingleSwitchProgram_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "NintendoSwitch_SwitchSystemOption.h"
#include "NintendoSwitch_RunnableProgram.h"
#include "NintendoSwitch_SingleSwitchProgramOption.h"

namespace PokemonAutomation{
    class BotBaseContext;
namespace NintendoSwitch{


class SingleSwitchProgramInstance2;


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
        Logger& logger,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats,
        Args&&... args
    )
        : ProgramEnvironment(program_info, logger, current_stats, historical_stats)
        , console(0, std::forward<Args>(args)...)
    {
        console.initialize_inference_threads(scope, inference_dispatcher());
    }
};


class SingleSwitchProgramDescriptor : public RunnableSwitchProgramDescriptor{
public:
    using RunnableSwitchProgramDescriptor::RunnableSwitchProgramDescriptor;

    virtual std::unique_ptr<SingleSwitchProgramInstance2> make_instance() const{ return nullptr; }
};







class SingleSwitchProgramInstance2{
public:
    virtual ~SingleSwitchProgramInstance2() = default;
    SingleSwitchProgramInstance2(const SingleSwitchProgramInstance2&) = delete;
    void operator=(const SingleSwitchProgramInstance2&) = delete;

    SingleSwitchProgramInstance2();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) = 0;


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
    virtual std::unique_ptr<PanelInstance> make_panel() const override{
        return std::unique_ptr<PanelInstance>(new SingleSwitchProgramOption(*this));
    }
    virtual std::unique_ptr<SingleSwitchProgramInstance2> make_instance() const override{
        return std::unique_ptr<SingleSwitchProgramInstance2>(new Instance());
    }
};

template <typename Descriptor, typename Instance>
std::unique_ptr<PanelDescriptor> make_single_switch_program(){
    return std::make_unique<SingleSwitchProgramWrapper<Descriptor, Instance>>();
}








}
}
#endif


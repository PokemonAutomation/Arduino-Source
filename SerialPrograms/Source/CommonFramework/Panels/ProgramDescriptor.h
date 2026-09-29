/*  Program Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_ProgramDescriptor_H
#define PokemonAutomation_CommonFramework_ProgramDescriptor_H

#include "Common/Cpp/Options/BatchOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "PanelDescriptor.h"

namespace PokemonAutomation{

class JsonValue;
class StatsTracker;
class ProgramEnvironment;


enum class ProgramControllerClass{
    StandardController_NoRestrictions,              //  Blue
    StandardController_PerformanceClassSensitive,   //  Green
    StandardController_RequiresPrecision,           //  Purple
    StandardController_WithRestrictions,            //  Red
    SpecializedController,                          //  Pink
};
enum class AllowCommandsWhenRunning{
    DISABLE_COMMANDS,
    ENABLE_COMMANDS,
};



class ProgramDescriptor : public PanelDescriptor{
public:
    ProgramDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        Color color,
        PanelDeprecation deprecation,
        bool restore_defaults_button,
        std::vector<std::string> required_resources
    );

public:
    const std::vector<std::string>& required_resources() const{ return m_required_resources; }

    virtual std::unique_ptr<StatsTracker> make_stats() const;

private:
    const std::vector<std::string> m_required_resources;
};




class ProgramInstance{
public:
    virtual ~ProgramInstance() = default;
    ProgramInstance(const ProgramInstance&) = delete;
    void operator=(const ProgramInstance&) = delete;

public:
    //  `options_lock_mode` is the lock mode of the program's options container
    //  (`m_options`) while the program runs:
    //  - LOCK_WHILE_RUNNING: the whole options panel is disabled while running.
    //  - UNLOCK_WHILE_RUNNING: each option follows its own lock mode, so options
    //    constructed with UNLOCK_WHILE_RUNNING (and buttons) stay usable while the
    //    program runs. This is the default for all program instances.
    ProgramInstance(
        const std::vector<std::string>& error_notification_tags = {"Notifs"},
        LockMode options_lock_mode = LockMode::UNLOCK_WHILE_RUNNING
    );
    ConfigOption& options(){ return m_options; }

public:
    virtual std::string check_validity() const;
    virtual void restore_defaults();
    virtual JsonValue to_json() const;
    virtual void load_json(const JsonValue& json);

public:
    virtual void run_start_program_checks(
        const ProgramDescriptor& descriptor,
        ProgramEnvironment& env
    ){}

protected:
    BatchOption m_options;
    void add_option(ConfigOption& option, std::string serialization_string);

public:
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationOption NOTIFICATION_ERROR_RECOVERABLE;
    EventNotificationOption NOTIFICATION_ERROR_FATAL;
};




}
#endif

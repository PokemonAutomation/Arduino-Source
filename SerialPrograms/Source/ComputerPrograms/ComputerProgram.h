/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_RunnableComputerProgram_H
#define PokemonAutomation_RunnableComputerProgram_H

#include "Common/Cpp/Options/BatchOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"

namespace PokemonAutomation{

class CancellableScope;
class ComputerProgramInstance;


class ComputerProgramDescriptor : public ProgramDescriptor{
public:
    ComputerProgramDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description
    );

    virtual std::unique_ptr<PanelInstance> make_panel() const override;
    virtual std::unique_ptr<ComputerProgramInstance> make_instance() const = 0;
};





//
//  As of this writing, this class will never be called in a manner where
//  thread-safety is of concern. However, this may change in the future.
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
//
//  Calls to this class will never be concurrent from different threads.
//
class ComputerProgramInstance{
public:
    virtual ~ComputerProgramInstance() = default;
    ComputerProgramInstance(const ComputerProgramInstance&) = delete;
    void operator=(const ComputerProgramInstance&) = delete;

    ComputerProgramInstance();

    virtual void program(ProgramEnvironment& env, CancellableScope& scope) = 0;


public:
    //  Settings

    virtual void from_json(const JsonValue& json);
    virtual JsonValue to_json() const;

    virtual std::string check_validity() const;
    virtual void restore_defaults();


protected:
    friend class ComputerProgramOption;

    BatchOption m_options;
    void add_option(ConfigOption& option, std::string serialization_string);


public:
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationOption NOTIFICATION_ERROR_RECOVERABLE;
    EventNotificationOption NOTIFICATION_ERROR_FATAL;
};



template <typename Descriptor, typename Instance>
class ComputerProgramWrapper : public Descriptor{
public:
    virtual std::unique_ptr<ComputerProgramInstance> make_instance() const override{
        return std::unique_ptr<ComputerProgramInstance>(new Instance());
    }
};

template <typename Descriptor, typename Instance>
std::unique_ptr<PanelDescriptor> make_computer_program(){
    return std::make_unique<ComputerProgramWrapper<Descriptor, Instance>>();
}




}
#endif

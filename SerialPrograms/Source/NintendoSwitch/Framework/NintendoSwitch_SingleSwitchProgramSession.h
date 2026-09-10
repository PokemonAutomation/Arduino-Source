/*  Single Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class holds the run-time state of a Switch program.
 *
 *  This class is fully thread-safe. You can call any functions from anywhere at
 *  anytime.
 *
 *  Warning: Constructing this class requires an "option" parameter. It is not
 *  safe to modify this "option" parameter during the lifetime of this class.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SingleSwitchProgramSession_H
#define PokemonAutomation_NintendoSwitch_SingleSwitchProgramSession_H

#include "CommonFramework/Panels/PanelSession.h"
#include "CommonFramework/ProgramSession.h"
#include "NintendoSwitch_SwitchSystemSession.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class SingleSwitchProgramOption;


class SingleSwitchProgramSession final
    : public UiState<SingleSwitchProgramSession, PanelSession>
    , public ProgramSession
{
public:
    bool try_shutdown();
    ~SingleSwitchProgramSession();
    SingleSwitchProgramSession(const SingleSwitchProgramDescriptor& descriptor);

    void restore_defaults();


public:
    const SingleSwitchProgramDescriptor& descriptor() const{ return m_descriptor; }
    SwitchSystemSession& system(){ return m_system; }
    ConfigOption& options();


private:
    virtual std::string check_validity() const override;

    virtual void internal_run_program() override;
    virtual void internal_stop_program() override;


private:
    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;


private:
    void run_program_instance(SingleSwitchProgramEnvironment& env, CancellableScope& scope);


private:
    const SingleSwitchProgramDescriptor& m_descriptor;

    SwitchSystemOption m_system_option;
    SwitchSystemSession m_system;

    std::unique_ptr<SingleSwitchProgramInstance> m_instance;

    std::atomic<CancellableScope*> m_scope;
};




}
}
#endif

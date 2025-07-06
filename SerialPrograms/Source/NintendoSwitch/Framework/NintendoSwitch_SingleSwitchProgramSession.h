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

#include "CommonFramework/ProgramSession.h"
#include "NintendoSwitch_SwitchSystemSession.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class SingleSwitchProgramOption;


class SingleSwitchProgramSession final : public ProgramSession{
public:
    virtual ~SingleSwitchProgramSession();
    SingleSwitchProgramSession(SingleSwitchProgramOption& option, size_t console_number);

    void restore_defaults();

public:
    SwitchSystemSession& system(){ return m_system; }

private:
    virtual std::string check_validity() const override;

    virtual void internal_run_program() override;
    virtual void internal_stop_program() override;


private:
    void run_program_instance(SingleSwitchProgramEnvironment& env, CancellableScope& scope);

private:
    SingleSwitchProgramOption& m_option;
    SwitchSystemSession m_system;

    std::atomic<CancellableScope*> m_scope;
};




}
}
#endif

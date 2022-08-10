/*  Single Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SingleSwitchProgramSession_H
#define PokemonAutomation_NintendoSwitch_SingleSwitchProgramSession_H

#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/ProgramSession.h"
#include "CommonFramework/Panels/Panel.h"
#include "NintendoSwitch_SwitchSystemSession.h"
#include "NintendoSwitch_SingleSwitchProgramOption.h"
#include "NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SingleSwitchProgramSession final : public ProgramSession{
public:
    virtual ~SingleSwitchProgramSession();
    SingleSwitchProgramSession(SingleSwitchProgramOption& option);

    void restore_defaults();

public:
    SwitchSystemSession& system(){ return m_system; }

private:
    virtual std::string check_validity() const override;

    virtual void internal_run_program() override;
    virtual void internal_stop_program() override final;


private:
    void run_program_instance(const ProgramInfo& info);

private:
    SingleSwitchProgramOption& m_option;
    SwitchSystemSession m_system;

    SpinLock m_lock;
    CancellableScope* m_scope = nullptr;
};




}
}
#endif

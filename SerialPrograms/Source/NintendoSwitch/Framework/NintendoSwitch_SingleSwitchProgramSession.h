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

#include "GameConsole/Framework/ConsoleProgramSession.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class SingleSwitchProgramSession final : public GameConsole::ConsoleProgramSession{
public:
    ~SingleSwitchProgramSession();
    SingleSwitchProgramSession(const SingleSwitchProgramDescriptor& descriptor);

    const SingleSwitchProgramDescriptor& descriptor() const{ return m_descriptor; }

private:
    virtual std::unique_ptr<ProgramEnvironment> make_env(const ProgramInfo& program_info) override;

private:
    const SingleSwitchProgramDescriptor& m_descriptor;
};




}
}
#endif

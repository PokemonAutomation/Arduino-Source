/*  Multi-Switch Program Session
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

#ifndef PokemonAutomation_NintendoSwitch_MultiSwitchProgramSession_H
#define PokemonAutomation_NintendoSwitch_MultiSwitchProgramSession_H

#include "GameConsole/Framework/MultiConsoleProgramSession.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class MultiSwitchProgramOption;


class MultiSwitchProgramSession final : public GameConsole::MultiConsoleProgramSession{
public:
    ~MultiSwitchProgramSession();
    MultiSwitchProgramSession(const MultiSwitchProgramDescriptor& descriptor);

private:
    virtual std::unique_ptr<ProgramEnvironment> make_env(const ProgramInfo& program_info) override;

private:
    const MultiSwitchProgramDescriptor& m_descriptor;
};





}
}
#endif

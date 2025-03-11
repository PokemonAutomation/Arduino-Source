/*  Computer Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class holds the run-time state of a computer program.
 *
 *  This class is fully thread-safe. You can call any functions from anywhere at
 *  anytime.
 *
 *  Warning: Constructing this class requires an "option" parameter. It is not
 *  safe to modify this "option" parameter during the lifetime of this class.
 *
 */

#ifndef PokemonAutomation_ComputerPrograms_ComputerProgramSession_H
#define PokemonAutomation_ComputerPrograms_ComputerProgramSession_H

#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/ProgramSession.h"
#include "ComputerPrograms/ComputerProgram.h"

namespace PokemonAutomation{

struct ProgramInfo;
class ComputerProgramOption;
class ProgramEnvironment;


class ComputerProgramSession final : public ProgramSession{
public:
    virtual ~ComputerProgramSession();
    ComputerProgramSession(ComputerProgramOption& option);

    void restore_defaults();

private:
    virtual std::string check_validity() const override;

    virtual void internal_run_program() override;
    virtual void internal_stop_program() override;


private:
    void run_program_instance(ProgramEnvironment& env, CancellableScope& scope);

private:
    ComputerProgramOption& m_option;

    SpinLock m_lock;
    CancellableScope* m_scope = nullptr;
};





}
#endif

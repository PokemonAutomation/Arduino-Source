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
#include "CommonFramework/Panels/PanelSession.h"
#include "CommonFramework/ProgramSession.h"
#include "ComputerPrograms/ComputerProgram.h"

namespace PokemonAutomation{

struct ProgramInfo;
class ComputerProgramOption;
class ProgramEnvironment;


class ComputerProgramSession final : public UiState<ComputerProgramSession, PanelSession>, public ProgramSession{
public:
    virtual ~ComputerProgramSession();
    ComputerProgramSession(const ComputerProgramDescriptor& descriptor);

    void restore_defaults();


public:
    const ComputerProgramDescriptor& descriptor() const{ return m_descriptor; }
    ConfigOption& options();


private:
    virtual std::string check_validity() const override;

    virtual void internal_run_program() override;
    virtual void internal_stop_program() override;


private:
    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;


private:
    void run_program_instance(ProgramEnvironment& env, CancellableScope& scope);


private:
    const ComputerProgramDescriptor& m_descriptor;
    std::unique_ptr<ComputerProgramInstance> m_instance;

    SpinLock m_lock;
    CancellableScope* m_scope = nullptr;
};





}
#endif

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

#include "CommonFramework/Panels/PanelSession.h"
#include "CommonFramework/ProgramSession.h"
#include "ComputerPrograms/ComputerProgram.h"

namespace PokemonAutomation{

struct ProgramInfo;
class ComputerProgramOption;
class ProgramEnvironment;


class ComputerProgramSession final
    : public UiState<ComputerProgramSession, PanelSession>
    , public ProgramSession
{
public:
    virtual ~ComputerProgramSession();
    ComputerProgramSession(const ComputerProgramDescriptor& descriptor);


public:
    const ComputerProgramDescriptor& descriptor() const{ return m_descriptor; }
    ConfigOption& options();


public:
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;
    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;


private:
    virtual std::unique_ptr<ProgramEnvironment> make_env(const ProgramInfo& program_info) override;
    virtual void internal_run_program(ProgramEnvironment& env) override;


private:
    const ComputerProgramDescriptor& m_descriptor;
};





}
#endif

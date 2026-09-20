/*  Console Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_ConsoleProgramSession_H
#define PokemonAutomation_GameConsole_ConsoleProgramSession_H

#include "CommonFramework/Panels/PanelSession.h"
#include "CommonFramework/ProgramSession.h"
#include "GameConsole/ConsoleProgram.h"
#include "ConsoleSystemSession.h"

namespace PokemonAutomation{
namespace GameConsole{


class ConsoleProgramSession
    : public UiState<ConsoleProgramSession, PanelSession>
    , public ProgramSession
{
public:
    bool try_shutdown();
    ~ConsoleProgramSession();
    ConsoleProgramSession(
        const ConsoleProgramDescriptor& descriptor,
        std::unique_ptr<ConfigOption> extra_option = nullptr
    );


public:
    const ConsoleProgramDescriptor& descriptor() const{ return m_descriptor; }
    ConsoleSystemSession& system(){ return m_system; }
    ConfigOption& options();


public:
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;
    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;


protected:
    virtual std::unique_ptr<ProgramEnvironment> make_env(const ProgramInfo& program_info) override;
    virtual void internal_run_program(ProgramEnvironment& env) override;


private:
    const ConsoleProgramDescriptor& m_descriptor;

    ConsoleSystemOption m_system_option;
    ConsoleSystemSession m_system;
};



}
}
#endif

/*  Multi-Console Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_MultiConsoleProgramSession_H
#define PokemonAutomation_GameConsole_MultiConsoleProgramSession_H

#include "CommonFramework/Panels/PanelSession.h"
#include "CommonFramework/ProgramSession.h"
#include "GameConsole/MultiConsoleProgram.h"
#include "MultiConsoleSystemSession.h"

namespace PokemonAutomation{
namespace GameConsole{


class MultiConsoleProgramSession
    : public UiState<MultiConsoleProgramSession, PanelSession>
    , public ProgramSession
    , private GameConsole::MultiConsoleSystemSession::Listener
{
public:
    bool try_shutdown();
    ~MultiConsoleProgramSession();
    MultiConsoleProgramSession(
        const MultiConsoleProgramDescriptor& descriptor,
        std::unique_ptr<MultiConsoleSystemOption> option = nullptr
    );


public:
    const MultiConsoleProgramDescriptor& descriptor() const{ return m_descriptor; }
    MultiConsoleSystemSession& system(){ return m_system; }
    ConfigOption& options();


public:
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;
    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;


protected:
    virtual std::unique_ptr<ProgramEnvironment> make_env(const ProgramInfo& program_info) override;
    virtual void internal_run_program(ProgramEnvironment& env) override;

    virtual void on_console_count_lock(bool locked) override{}
    virtual void shutdown() override;
    virtual void startup(size_t switch_count) override;


private:
    const MultiConsoleProgramDescriptor& m_descriptor;

    std::unique_ptr<MultiConsoleSystemOption> m_system_option;
    MultiConsoleSystemSession m_system;
};





}
}
#endif

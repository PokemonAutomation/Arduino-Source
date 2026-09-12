/*  Console Panel Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_ConsolePanelSession_H
#define PokemonAutomation_GameConsole_ConsolePanelSession_H

#include "CommonFramework/Panels/PanelSession.h"
#include "ConsoleSystemSession.h"
#include "ConsolePanel.h"

namespace PokemonAutomation{
namespace GameConsole{



class ConsolePanelSession final : public UiState<ConsolePanelSession, PanelSession>{
public:
    bool try_shutdown();
    ~ConsolePanelSession();
    ConsolePanelSession(const ConsolePanelDescriptor& descriptor);


public:
    const ConsolePanelDescriptor& descriptor() const{ return m_descriptor; }

    Logger& logger(){ return m_system.logger(); }
    ConsoleSystemSession& system(){ return m_system; }
    ConfigOption& options();


public:
    //  Serialization

    void restore_defaults();
    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;


private:
    const ConsolePanelDescriptor& m_descriptor;

    ConsoleSystemOption m_system_option;
    ConsoleSystemSession m_system;

    std::unique_ptr<ConsolePanelInstance> m_instance;
};





}
}
#endif

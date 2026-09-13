/*  Multi-Console Panel Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_MultiConsolePanelSession_H
#define PokemonAutomation_GameConsole_MultiConsolePanelSession_H

#include "CommonFramework/Panels/PanelSession.h"
#include "MultiConsoleSystemSession.h"
#include "GameConsole/MultiConsolePanel.h"

namespace PokemonAutomation{
namespace GameConsole{



class MultiConsolePanelSession final : public UiState<MultiConsolePanelSession, PanelSession>{
public:
    bool try_shutdown();
    ~MultiConsolePanelSession();
    MultiConsolePanelSession(const MultiConsolePanelDescriptor& descriptor);


public:
    const MultiConsolePanelDescriptor& descriptor() const{ return m_descriptor; }

    Logger& logger(){ return m_system.logger(); }
    MultiConsoleSystemSession& system(){ return m_system; }
    ConfigOption& options();


public:
    //  Serialization

    void restore_defaults();
    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;


private:
    const MultiConsolePanelDescriptor& m_descriptor;

    MultiConsoleSystemOption m_system_option;
    MultiConsoleSystemSession m_system;

    std::unique_ptr<MultiConsolePanelInstance> m_instance;
};





}
}
#endif

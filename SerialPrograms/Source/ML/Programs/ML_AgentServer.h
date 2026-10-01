/*  ML AI Agent Server Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Lets AI agents control the Switch through SerialPrograms. While running, this
 *  program hosts an MCP (Model Context Protocol) server on a local port. Any MCP
 *  client (Claude Code, Claude Desktop, Codex, ...) can connect to it and press
 *  buttons, move sticks, take screenshots and read on-screen text, using this
 *  console's controller and video. The tools are defined in the shared
 *  Integrations/AgentServer/AgentTools.json, which the Python MCP server
 *  (pokemon_automation.mcp_server) serves too.
 *
 *  The user watches the agent in this program's video panel and can take over at any
 *  time with keyboard control: the agent's inputs are refused until the user clicks
 *  "Return control to agent" (or, optionally, after some idle seconds).
 *  Stopping the program stops the server and releases the controller.
 */

#ifndef PokemonAutomation_ML_AgentServer_H
#define PokemonAutomation_ML_AgentServer_H

#include <atomic>
#include <mutex>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace ML{


class AgentServer_Descriptor : public NintendoSwitch::SingleSwitchProgramDescriptor{
public:
    AgentServer_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class AgentSession;

class AgentServerProgram : public NintendoSwitch::SingleSwitchProgramInstance,
                           public ButtonListener,
                           public ConfigOption::Listener{
public:
    using Descriptor = AgentServer_Descriptor;

    ~AgentServerProgram();
    AgentServerProgram();

    virtual void program(
        NintendoSwitch::SingleSwitchProgramEnvironment& env,
        NintendoSwitch::ProControllerContext& context
    ) override;

    //  "Return control to agent" and "New access token" buttons.
    virtual void on_press(ButtonCell& button) override;
    //  Keeps the connection instructions in sync with the port and token options.
    virtual void on_config_value_changed(void* object) override;

private:
    //  Refresh CONNECTION_INFO from the current options.
    void update_connection_info();

public:
    SimpleIntegerOption<uint16_t> PORT;
    BooleanCheckBoxOption ALLOW_NETWORK;
    BooleanCheckBoxOption REQUIRE_TOKEN;
    StringOption ACCESS_TOKEN;
    ButtonOption NEW_TOKEN;
    StaticTextOption CONNECTION_INFO;

    SimpleIntegerOption<uint32_t> SETTLE_MS;
    SimpleIntegerOption<uint32_t> MAX_HOLD_MS;
    SimpleIntegerOption<uint32_t> MAX_SEQUENCE_MS;
    SimpleIntegerOption<uint32_t> SCREENSHOT_WIDTH;
    SimpleIntegerOption<uint8_t> JPEG_QUALITY;
    SimpleIntegerOption<uint32_t> AUTO_RETURN_SECONDS;
    ButtonOption RETURN_CONTROL;

private:
    //  The running session, if any. Buttons are pressed on the UI thread.
    std::mutex m_session_lock;
    AgentSession* m_session = nullptr;
};




}
}
#endif

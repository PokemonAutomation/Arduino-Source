# AI Agent Server (MCP)

Lets AI agents (Claude Code, Claude Desktop, Codex, and other
[MCP](https://modelcontextprotocol.io) clients) see and control the Switch through
SerialPrograms, while you watch in the app and can take over with the keyboard.

## Using it

1. Enable developer mode, open **ML → AI Agent Server**, pick the controller and the
   capture card like any program, and press **Start**.
2. Connect the agent to the URL shown in the program (default
   `http://127.0.0.1:8765/mcp`, transport "Streamable HTTP") with the header
   `Authorization: Bearer <token>`. The program shows a ready-to-paste command, e.g.
   for Claude Code:

   ```bash
   claude mcp add --transport http switch http://127.0.0.1:8765/mcp --header "Authorization: Bearer <token>"
   ```
3. Watch the agent in the video panel; each action is shown on the overlay and in
   the log. **Press any mapped key** to take over: the agent's inputs are refused
   (and told why) until you click **Return control to agent**, or after the optional
   idle time. **Stop** shuts the server down and releases the controller.

Only this computer can connect by default. The access token keeps other local
programs and web pages from pressing buttons; see the options for turning it off or
allowing other computers (not recommended).

## Tools

The tools, their argument schemas and the instructions sent to agents are defined in
[`AgentTools.json`](AgentTools.json), which is shared with the Python MCP server
(`Source/PythonBindings`, `python -m pokemon_automation.mcp_server`). Each tool lists
the hosts that implement it (`app`, `python`), so both servers expose the same
interface. The input vocabulary (button names, sticks, step fields) is tested against
[`AgentInputTestCases.json`](AgentInputTestCases.json) in both languages.

App tools: `switch_status`, `screenshot`, `wait_and_observe`, `read_text` (the app's
Tesseract OCR), `press_buttons`, `move_stick`, `run_inputs`, `release_all`,
`get_logs`.

## Code

| File | Purpose |
|---|---|
| `AgentServer_HttpServer.*` | Minimal HTTP/1.1 server on Qt's `QTcpServer` (Qt's `QHttpServer` is GPL-only, so it's not used). |
| `AgentServer_McpServer.*` | MCP over Streamable HTTP: JSON-RPC, initialize handshake (protocol 2024-11-05 … 2025-11-25), sessions, token/Origin/Host checks. |
| `AgentServer_ToolDefinitions.*` | Loads `AgentTools.json` (compiled in via `cmake/EmbedTextFile.cmake`), validates tool arguments. |
| `AgentServer_InputSteps.*` | Button/stick/step parsing; the C++ twin of `pokemon_automation/buttons.py`. |
| `ML/Programs/ML_AgentServer.*` | The program: runs input tools on the program thread, screenshots, OCR, user takeover. |

When changing a tool, edit `AgentTools.json` and both implementations; the Python
tests (`tests/test_shared_interface.py`) check that the Python server matches the file.

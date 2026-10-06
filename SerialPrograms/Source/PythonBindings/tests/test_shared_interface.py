"""The Python server must match the shared interface in AgentTools.json, and the
input vocabulary must pass the shared AgentInputTestCases.json (also used by C++)."""

import json
import math

import anyio
import pytest

from pokemon_automation import agent_tools
from pokemon_automation import buttons as btn
from pokemon_automation.controller import InputStep, validate_steps


def test_schemas_are_self_contained():
    text = json.dumps([t["inputSchema"] for t in agent_tools.load()["tools"]])
    assert "$ref" not in text


def test_every_tool_names_known_hosts():
    for tool in agent_tools.load()["tools"]:
        assert tool["hosts"] and set(tool["hosts"]) <= {"app", "python"}, tool["name"]


def test_python_server_serves_the_shared_definitions():
    pytest.importorskip("mcp")
    from mcp import Client
    from pokemon_automation.mcp_server import ServerConfig, create_server

    server, _ = create_server(ServerConfig(fake=True))

    async def go():
        async with Client(server) as client:
            return (await client.list_tools()).tools, client.instructions

    tools, instructions = anyio.run(go)
    shared = agent_tools.tools_for("python")
    assert {t.name for t in tools} == set(shared)
    for t in tools:
        assert t.description == shared[t.name]["description"]
        assert t.input_schema == shared[t.name]["inputSchema"]
    assert instructions == agent_tools.instructions()


def test_python_signatures_accept_the_shared_arguments():
    """Each tool function takes exactly the shared schema's properties, with the same
    required ones. (The function signature is what validates arguments in Python.)"""
    pytest.importorskip("mcp")
    from pokemon_automation.mcp_server import ServerConfig, create_server

    server, _ = create_server(ServerConfig(fake=True))
    for name, definition in agent_tools.tools_for("python").items():
        derived = server._tool_manager.get_tool(name).fn_metadata.arg_model.model_json_schema()
        schema = definition["inputSchema"]
        assert set(derived.get("properties", {})) == set(schema.get("properties", {})), name
        assert set(derived.get("required", [])) == set(schema.get("required", [])), name


CASES = agent_tools.load_test_cases()


def expect_error(fn, arg, substring):
    """Shared cases name an expected substring of the error message (not a regex)."""
    with pytest.raises(ValueError) as e:
        fn(arg)
    assert substring in str(e.value)


@pytest.mark.parametrize("case", CASES["buttons"], ids=lambda c: repr(c["input"]))
def test_shared_button_cases(case):
    if "error" in case:
        expect_error(btn.parse_buttons, case["input"], case["error"])
        return
    parsed = btn.parse_buttons(case["input"])
    assert (parsed.bitfield, parsed.dpad) == (case["bitfield"], case["dpad"])


@pytest.mark.parametrize("case", CASES["sticks"], ids=lambda c: repr(c["input"]))
def test_shared_stick_cases(case):
    if "error" in case:
        expect_error(btn.parse_stick, case["input"], case["error"])
        return
    x, y = btn.parse_stick(case["input"])
    assert math.isclose(x, case["x"], abs_tol=1e-9) and math.isclose(y, case["y"], abs_tol=1e-9)


@pytest.mark.parametrize("case", CASES["steps"], ids=lambda c: json.dumps(c["input"]))
def test_shared_step_cases(case):
    if "error" in case:
        expect_error(validate_steps, [case["input"]], case["error"])
        return
    (step,) = validate_steps([case["input"]])
    assert isinstance(step, InputStep)
    assert step.duration_ms() == case["duration_ms"]

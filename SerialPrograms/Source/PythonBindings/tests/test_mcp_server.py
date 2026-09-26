"""End-to-end tests of the MCP server with fake devices, through a real MCP client."""

import anyio
import pytest

pytest.importorskip("mcp")
from mcp import Client  # noqa: E402

from pokemon_automation import buttons as btn  # noqa: E402
from pokemon_automation.mcp_server import ServerConfig, create_server  # noqa: E402


def run(coro_fn):
    return anyio.run(coro_fn)


def make(**overrides):
    config = ServerConfig(fake=True, default_settle_ms=0, **overrides)
    server, manager = create_server(config)
    return server, manager


def texts(result):
    return [c.text for c in result.content if c.type == "text"]


def images(result):
    return [c for c in result.content if c.type == "image"]


def test_lists_expected_tools():
    server, _ = make()

    async def go():
        async with Client(server) as client:
            return {t.name for t in (await client.list_tools()).tools}

    names = run(go)
    assert {"screenshot", "press_buttons", "move_stick", "run_inputs", "read_text",
            "release_all", "switch_status", "wait_and_observe", "connect"} <= names


def test_press_returns_screenshot_and_sends_input():
    server, manager = make()

    async def go():
        async with Client(server) as client:
            return await client.call_tool("press_buttons", {"buttons": "A", "repeat": 2})

    result = run(go)
    assert not result.is_error, texts(result)
    assert "press A x2" in texts(result)[0]
    assert len(images(result)) == 1
    fake = manager.current().controller.backend
    assert [c for c in fake.log if c[0] == "press_buttons"][0][1][3] == btn.BUTTON_BITS["A"]


def test_run_inputs_and_limits():
    server, manager = make(max_sequence_ms=5000)

    async def go():
        async with Client(server) as client:
            ok = await client.call_tool("run_inputs", {
                "steps": [
                    {"buttons": "DOWN", "repeat": 3},
                    {"buttons": "A"},
                    {"left_stick": "up", "buttons": "B", "hold_ms": 1000, "release_ms": 0},
                ],
                "observe": False,
            })
            too_long = await client.call_tool("run_inputs", {
                "steps": [{"left_stick": "up", "hold_ms": 6000}], "observe": False})
            bad_button = await client.call_tool("press_buttons", {"buttons": "Q"})
            return ok, too_long, bad_button

    ok, too_long, bad_button = run(go)
    assert not ok.is_error, texts(ok)
    assert images(ok) == []
    fake = manager.current().controller.backend
    assert fake.commands() == ["press_dpad"] * 3 + ["press_buttons", "set_state"]
    assert too_long.is_error and "limit" in texts(too_long)[0]
    assert bad_button.is_error and "Unknown button" in texts(bad_button)[0]


def test_read_only_mode_blocks_inputs():
    server, _ = make(read_only=True)

    async def go():
        async with Client(server) as client:
            return await client.call_tool("press_buttons", {"buttons": "A"})

    result = run(go)
    assert result.is_error and "read-only" in texts(result)[0]


def test_status_screenshot_and_release():
    server, manager = make()

    async def go():
        async with Client(server) as client:
            status = await client.call_tool("switch_status", {})
            shot = await client.call_tool("screenshot", {"box": [0, 0, 0.5, 0.5]})
            released = await client.call_tool("release_all", {})
            return status, shot, released

    status, shot, released = run(go)
    assert '"ready": true' in texts(status)[0]
    assert len(images(shot)) == 1
    assert "released" in texts(released)[0]
    assert manager.current().controller.backend.cancel_count == 1


def test_release_all_unconfirmed_is_reported():
    server, manager = make()

    async def go():
        async with Client(server) as client:
            await client.call_tool("switch_status", {})  # connects the fake devices
            manager.current().controller.backend.confirm_release = False
            return await client.call_tool("release_all", {})

    result = run(go)
    assert not result.is_error
    assert "did not confirm" in texts(result)[0]

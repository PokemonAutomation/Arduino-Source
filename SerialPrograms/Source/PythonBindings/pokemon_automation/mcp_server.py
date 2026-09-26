"""MCP server that lets AI agents see and control a Nintendo Switch.

The server wraps one `Console` (controller + capture card). Tools are deliberately
coarse: a single call can send a whole input sequence and return a screenshot taken
after the game has reacted, because an agent needs seconds per turn and every round
trip counts.

Run over stdio (for Claude Code, Claude Desktop and most MCP clients):
    python -m pokemon_automation.mcp_server --serial /dev/cu.usbserial-0001 --video MiraBox

Run over HTTP (for remote agents):
    python -m pokemon_automation.mcp_server --serial ... --video ... \\
        --transport streamable-http --host 127.0.0.1 --port 8765

Try it without hardware:
    python -m pokemon_automation.mcp_server --fake

Register with Claude Code:
    claude mcp add switch -- /path/to/python -m pokemon_automation.mcp_server \\
        --serial /dev/cu.usbserial-0001 --video MiraBox

Safety: every input tool is capped (`--max-hold-ms`, `--max-sequence-ms`),
`release_all` works even while another tool is mid-sequence, and `--read-only`
disables inputs entirely. All inputs are logged (see `get_logs`).
"""

from __future__ import annotations

import argparse
import functools
import logging
import os
import threading
import time
from dataclasses import dataclass
from typing import Annotated, Any, Literal

import anyio
from pydantic import BaseModel, Field

try:  # mcp >= 2
    from mcp.server.mcpserver import Image, MCPServer as _Server
    from mcp.server.mcpserver.exceptions import ToolError
except ImportError:  # mcp 1.x
    from mcp.server.fastmcp import FastMCP as _Server, Image  # type: ignore[no-redef]
    from mcp.server.fastmcp.exceptions import ToolError  # type: ignore[no-redef]

from . import agent_tools
from . import buttons as btn
from ._core import core, core_available
from .console import Console
from .controller import InputStep, SwitchController, validate_steps
from .devices import list_serial_ports, list_video_devices
from .fake import FakeController, FakeVideoCapture
from .video import VideoSource, find_video_device, ocr_available

log = logging.getLogger("pokemon_automation.mcp")

# Tool names, descriptions, input schemas and agent instructions come from the
# shared AgentTools.json (see agent_tools.py), which the SerialPrograms app serves
# too. The functions below implement the tools; their signatures do the argument
# validation and must stay in line with the shared schemas (tests check this).

BoxArg = list[float] | None


class Step(BaseModel):
    """One `run_inputs` step (schema and descriptions: AgentTools.json)."""

    model_config = {"extra": "forbid"}

    buttons: str | None = None
    left_stick: str | list[float] | None = None
    right_stick: str | list[float] | None = None
    hold_ms: int = Field(80, ge=1)
    release_ms: int = Field(80, ge=0)
    repeat: int = Field(1, ge=1, le=100)
    wait_ms: int = Field(0, ge=0)

    def to_input_step(self) -> InputStep:
        return InputStep(
            buttons=self.buttons, left_stick=self.left_stick, right_stick=self.right_stick,
            hold_ms=self.hold_ms, release_ms=self.release_ms, repeat=self.repeat,
            wait_ms=self.wait_ms,
        )


@dataclass
class ServerConfig:
    serial_port: str | None = None
    video_device: str | None = None
    width: int = 1920
    height: int = 1080
    fake: bool = False
    read_only: bool = False
    max_hold_ms: int = 10_000
    max_sequence_ms: int = 60_000
    screenshot_width: int = 1280
    jpeg_quality: int = 75
    default_settle_ms: int = 500


class ConsoleManager:
    """Owns the `Console` and connects to devices lazily, so the server starts (and
    can report useful errors) even when a device is missing or unplugged."""

    def __init__(self, config: ServerConfig):
        self.config = config
        self._lock = threading.Lock()
        self._console: Console | None = None
        self._errors: dict[str, str] = {}

    def console(self) -> Console:
        with self._lock:
            if self._console is None:
                self._console = self._open(self.config.serial_port, self.config.video_device)
            return self._console

    def reconnect(self, serial_port: str | None, video_device: str | None) -> Console:
        with self._lock:
            if self._console is not None:
                self._console.close()
                self._console = None
            if serial_port is not None:
                self.config.serial_port = serial_port
            if video_device is not None:
                self.config.video_device = video_device
            self._console = self._open(self.config.serial_port, self.config.video_device)
            return self._console

    def current(self) -> Console | None:
        return self._console

    def errors(self) -> dict[str, str]:
        return dict(self._errors)

    def _open(self, serial_port: str | None, video_device: str | None) -> Console:
        self._errors = {}
        if self.config.fake:
            fake_controller = FakeController()
            return Console(
                controller=SwitchController(backend=fake_controller),
                video_source=VideoSource(backend=FakeVideoCapture(fake_controller)),
            )
        controller = video = None
        if serial_port and not self.config.read_only:
            try:
                controller = SwitchController(serial_port)
            except Exception as e:  # keep going so video still works
                self._errors["controller"] = str(e)
                log.warning("Controller: %s", e)
        if video_device is not None:
            try:
                video = VideoSource(video_device, self.config.width, self.config.height)
            except Exception as e:
                self._errors["video"] = str(e)
                log.warning("Video: %s", e)
        return Console(controller=controller, video_source=video)

    def close(self) -> None:
        with self._lock:
            if self._console is not None:
                self._console.close()
                self._console = None


def log_event(message: str) -> None:
    """Log to the core log (which `get_logs` returns and echoes to stderr), or to
    Python logging when the core module isn't available."""
    if core_available():
        core().log("[MCP] " + message)
    else:
        log.info(message)


def _image_from_bytes(data: bytes) -> Image:
    fmt = "png" if data.startswith(b"\x89PNG") else "jpeg"
    return Image(data=data, format=fmt)


def agent_errors(fn):
    """Report expected failures (bad arguments, missing devices, limits) to the agent
    as a readable tool error. The MCP SDK replaces the message of any other exception
    with a generic "Error executing tool", which leaves the agent guessing."""
    @functools.wraps(fn)
    async def wrapper(*args: Any, **kwargs: Any) -> Any:
        try:
            return await fn(*args, **kwargs)
        except (ValueError, RuntimeError, ImportError, OSError) as e:
            raise ToolError(str(e)) from e
    return wrapper


def create_server(config: ServerConfig) -> tuple[Any, ConsoleManager]:
    """Build the MCP server and its console manager. Separate from `main()` for tests."""
    manager = ConsoleManager(config)
    server = _Server(agent_tools.server_name(), instructions=agent_tools.instructions())
    shared_tools = agent_tools.tools_for("python")

    def tool(structured_output: bool | None = None):
        """Register a tool under its function name, with the description and input
        schema from AgentTools.json. Raises KeyError for a tool not in the file."""
        def decorator(fn):
            definition = shared_tools[fn.__name__]
            server.tool(description=definition["description"],
                        structured_output=structured_output)(fn)
            # Advertise the shared schema (the SDK would otherwise derive one from
            # the signature, which validates the same arguments but reads differently).
            server._tool_manager.get_tool(fn.__name__).parameters = definition["inputSchema"]
            return fn
        return decorator
    # Serializes input tools so two calls can't interleave their button presses.
    input_lock = anyio.Lock()

    def check_limits(steps: list[InputStep]) -> int:
        total = 0
        for s in steps:
            if not s.is_wait() and s.hold_ms > config.max_hold_ms:
                raise ValueError(f"hold_ms {s.hold_ms} exceeds the limit of {config.max_hold_ms} ms.")
            total += s.duration_ms()
        if total > config.max_sequence_ms:
            raise ValueError(
                f"The sequence lasts {total} ms, over the limit of {config.max_sequence_ms} ms. "
                "Split it into several calls.")
        return total

    def screenshot_content(console: Console, box: list[float] | None, max_width: int | None,
                           settle_ms: int) -> list[Any]:
        video = console.require_video()
        started = time.monotonic()
        frame = video.fresh_frame(settle_ms)
        data = video.jpeg(tuple(box) if box else None,
                          max_width or config.screenshot_width, config.jpeg_quality)
        waited = int((time.monotonic() - started) * 1000)
        return [
            f"Frame #{frame.sequence} ({frame.width}x{frame.height}), captured {waited} ms after the call.",
            _image_from_bytes(data),
        ]

    async def send_and_observe(steps: list[InputStep], observe: bool, settle_ms: int | None,
                               description: str) -> list[Any]:
        if config.read_only:
            raise ValueError("The server is in read-only mode; inputs are disabled.")
        validate_steps(steps)
        total = check_limits(steps)
        settle = config.default_settle_ms if settle_ms is None else settle_ms
        async with input_lock:
            def work() -> list[Any]:
                console = manager.console()
                console.require_controller()
                log_event(f"Input: {description}")
                console.send(steps, wait=True)
                result: list[Any] = [f"Done: {description} ({total} ms of input)."]
                if observe and console.video is not None:
                    result += screenshot_content(console, None, None, settle)
                return result
            return await anyio.to_thread.run_sync(work)

    # ---- status & setup ------------------------------------------------------

    @tool()
    @agent_errors
    async def switch_status() -> dict[str, Any]:
        def work() -> dict[str, Any]:
            console = manager.console()
            status: dict[str, Any] = {"control": "agent", "read_only": config.read_only,
                                      "fake": config.fake}
            if console.controller is not None:
                status["controller"] = {
                    "ready": console.controller.is_ready(),
                    "name": console.controller.name(),
                    "status": console.controller.status(),
                    "port": config.serial_port,
                }
            else:
                status["controller"] = {"ready": False, "error": manager.errors().get(
                    "controller", "No serial port configured.")}
            if console.video is not None:
                width, height = console.video.resolution
                status["video"] = {
                    "streaming": console.video.is_streaming(),
                    "resolution": [width, height],
                    "fps": round(console.video.fps(), 1),
                    "device": config.video_device,
                }
            else:
                status["video"] = {"streaming": False, "error": manager.errors().get(
                    "video", "No video device configured.")}
            status["limits"] = {"max_hold_ms": config.max_hold_ms,
                                "max_sequence_ms": config.max_sequence_ms}
            status["ocr_available"] = ocr_available()
            return status
        return await anyio.to_thread.run_sync(work)

    @tool()
    @agent_errors
    async def list_devices() -> dict[str, Any]:
        return {"serial_ports": list_serial_ports(), "video_devices": list_video_devices()}

    @tool()
    @agent_errors
    async def connect(
        serial_port: str | None = None,
        video_device: str | None = None,
    ) -> dict[str, Any]:
        def work() -> dict[str, Any]:
            if video_device is not None and not config.fake:
                find_video_device(video_device)  # fail fast on a bad name
            manager.reconnect(serial_port, video_device)
            return {"errors": manager.errors() or None}
        async with input_lock:
            return await anyio.to_thread.run_sync(work)

    # ---- observation ------------------------------------------------------------

    @tool(structured_output=False)
    @agent_errors
    async def screenshot(
        box: BoxArg = None,
        max_width: Annotated[int | None, Field(ge=64, le=3840)] = None,
    ) -> list[Any]:
        def work() -> list[Any]:
            return screenshot_content(manager.console(), box, max_width, 0)
        return await anyio.to_thread.run_sync(work)

    @tool(structured_output=False)
    @agent_errors
    async def wait_and_observe(
        duration_ms: Annotated[int, Field(ge=0, le=60_000)],
        box: BoxArg = None,
    ) -> list[Any]:
        def work() -> list[Any]:
            return screenshot_content(manager.console(), box, None, duration_ms)
        return await anyio.to_thread.run_sync(work)

    @tool()
    @agent_errors
    async def read_text(
        box: BoxArg = None,
        mode: Literal["block", "line", "word", "sparse"] = "block",
        language: str = "eng",
    ) -> str:
        def work() -> str:
            return manager.console().read_text(
                tuple(box) if box else None, language=language, mode=mode)
        return await anyio.to_thread.run_sync(work)

    # ---- input --------------------------------------------------------------------

    @tool(structured_output=False)
    @agent_errors
    async def press_buttons(
        buttons: str,
        hold_ms: Annotated[int, Field(ge=1)] = 80,
        release_ms: Annotated[int, Field(ge=0)] = 120,
        repeat: Annotated[int, Field(ge=1, le=100)] = 1,
        observe: bool = True,
        settle_ms: Annotated[int | None, Field(ge=0, le=10_000)] = None,
    ) -> list[Any]:
        step = InputStep(buttons=buttons, hold_ms=hold_ms, release_ms=release_ms, repeat=repeat)
        return await send_and_observe([step], observe, settle_ms,
                                      f"press {buttons}" + (f" x{repeat}" if repeat > 1 else ""))

    @tool(structured_output=False)
    @agent_errors
    async def move_stick(
        direction: str | list[float],
        duration_ms: Annotated[int, Field(ge=1)] = 500,
        stick: Literal["left", "right"] = "left",
        buttons: str | None = None,
        observe: bool = True,
        settle_ms: Annotated[int | None, Field(ge=0, le=10_000)] = None,
    ) -> list[Any]:
        btn.parse_stick(direction)  # validate early for a clear error
        step = InputStep(buttons=buttons, hold_ms=duration_ms, release_ms=0)
        setattr(step, stick + "_stick", direction)
        return await send_and_observe([step], observe, settle_ms,
                                      f"{stick} stick {direction} for {duration_ms} ms"
                                      + (f" holding {buttons}" if buttons else ""))

    @tool(structured_output=False)
    @agent_errors
    async def run_inputs(
        steps: Annotated[list[Step], Field(min_length=1, max_length=200)],
        observe: bool = True,
        settle_ms: Annotated[int | None, Field(ge=0, le=10_000)] = None,
    ) -> list[Any]:
        input_steps = [s.to_input_step() for s in steps]
        return await send_and_observe(input_steps, observe, settle_ms, f"{len(steps)} steps")

    @tool()
    @agent_errors
    async def release_all() -> str:
        console = manager.current()
        if console is None or console.controller is None:
            return "No controller connected."
        timeout_ms = 1000
        confirmed = await anyio.to_thread.run_sync(console.release_all, timeout_ms)
        log_event(f"release_all called (confirmed: {confirmed})")
        if confirmed:
            return "All inputs released; the device confirmed the neutral state."
        return (f"Release requested, but the device did not confirm within {timeout_ms} ms. "
                "Check switch_status; the controller may be disconnected.")

    @tool()
    @agent_errors
    async def get_logs(count: Annotated[int, Field(ge=1, le=500)] = 30) -> list[str]:
        if not core_available():
            return []
        return list(core().recent_logs(count))

    registered = {t.name for t in server._tool_manager.list_tools()}
    if registered != set(shared_tools):
        raise RuntimeError(
            f"Tools out of sync with AgentTools.json: missing {sorted(set(shared_tools) - registered)}, "
            f"extra {sorted(registered - set(shared_tools))}")
    return server, manager


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    p = argparse.ArgumentParser(
        prog="python -m pokemon_automation.mcp_server",
        description="MCP server exposing a Nintendo Switch (controller + capture card) to AI agents.")
    p.add_argument("--serial", default=os.environ.get("PA_SERIAL_PORT"),
                   help="Controller serial port, e.g. /dev/cu.usbserial-0001 (env PA_SERIAL_PORT).")
    p.add_argument("--video", default=os.environ.get("PA_VIDEO_DEVICE"),
                   help="Video device index or name substring (env PA_VIDEO_DEVICE).")
    p.add_argument("--width", type=int, default=1920)
    p.add_argument("--height", type=int, default=1080)
    p.add_argument("--fake", action="store_true", help="Use fake devices (no hardware).")
    p.add_argument("--read-only", action="store_true", help="Disable all controller input.")
    p.add_argument("--max-hold-ms", type=int, default=10_000)
    p.add_argument("--max-sequence-ms", type=int, default=60_000)
    p.add_argument("--screenshot-width", type=int, default=1280)
    p.add_argument("--jpeg-quality", type=int, default=75)
    p.add_argument("--settle-ms", type=int, default=500,
                   help="Default wait between inputs finishing and the screenshot.")
    p.add_argument("--transport", choices=["stdio", "streamable-http"], default="stdio")
    p.add_argument("--host", default="127.0.0.1")
    p.add_argument("--port", type=int, default=8765)
    p.add_argument("--log-file", default=os.environ.get("PA_LOG_FILE"),
                   help="Append internal logs to this file.")
    return p.parse_args(argv)


def main(argv: list[str] | None = None) -> None:
    args = parse_args(argv)
    # Logs go to stderr: over stdio, stdout is the protocol channel.
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(name)s: %(message)s")
    if core_available():
        core().set_log_stderr(True)
        if args.log_file:
            core().set_log_file(args.log_file)
    config = ServerConfig(
        serial_port=args.serial, video_device=args.video, width=args.width, height=args.height,
        fake=args.fake, read_only=args.read_only, max_hold_ms=args.max_hold_ms,
        max_sequence_ms=args.max_sequence_ms, screenshot_width=args.screenshot_width,
        jpeg_quality=args.jpeg_quality, default_settle_ms=args.settle_ms,
    )
    server, manager = create_server(config)
    try:
        if args.transport == "stdio":
            server.run("stdio")
        else:
            server.run("streamable-http", host=args.host, port=args.port)
    except KeyboardInterrupt:
        # Ctrl+C is the normal way to stop the server; don't print a traceback.
        log.info("Stopping the MCP server.")
    finally:
        # Releases all inputs and closes the devices.
        manager.close()


if __name__ == "__main__":
    main()

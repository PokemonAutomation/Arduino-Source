"""Locate and import the compiled `_pa_core` extension module.

The module is built by CMake (`-DPA_PYTHON_BINDINGS=ON`, target `_pa_core`) and copied
next to this file after every build. Set the environment variable `PA_CORE_PATH` to a
folder containing `_pa_core*.so` / `_pa_core*.pyd` to load it from somewhere else.

Importing is deferred until first use so that the pure-Python parts of the package
(button parsing, fake devices, the MCP server in `--fake` mode) work without it.
"""

from __future__ import annotations

import importlib
import os
import sys
from types import ModuleType

_module: ModuleType | None = None

BUILD_HINT = (
    "The compiled module pokemon_automation._pa_core was not found. Build it with:\n"
    "  cmake <SerialPrograms> -DPA_PYTHON_BINDINGS=ON -DPython_EXECUTABLE=" + sys.executable + "\n"
    "  cmake --build . --target _pa_core\n"
    "or set PA_CORE_PATH to the folder containing the built module."
)


def core() -> ModuleType:
    """Return the `_pa_core` module, importing it on first call.

    Raises ImportError with build instructions if the module can't be found.
    """
    global _module
    if _module is not None:
        return _module
    override = os.environ.get("PA_CORE_PATH")
    if override:
        sys.path.insert(0, override)
        try:
            _module = importlib.import_module("_pa_core")
        finally:
            sys.path.remove(override)
        return _module
    try:
        from . import _pa_core  # type: ignore[attr-defined]
    except ImportError as e:
        raise ImportError(BUILD_HINT) from e
    _module = _pa_core
    return _module


def core_available() -> bool:
    try:
        core()
        return True
    except ImportError:
        return False

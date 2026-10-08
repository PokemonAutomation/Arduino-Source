"""The shared MCP interface definition (AgentTools.json).

The SerialPrograms app and this package both serve MCP from the same definition file,
so agents see identical tools whichever server they connect to. The file lives in the
C++ source tree at SerialPrograms/Source/Integrations/AgentServer/AgentTools.json.

Search order:
1. `PA_AGENT_TOOLS` environment variable (path to the file).
2. The C++ source tree, when running from a source checkout, so edits to the file
   take effect without rebuilding.
3. A copy inside this package (made by the CMake build, and included in wheels).
"""

from __future__ import annotations

import copy
import json
import os
from functools import lru_cache
from pathlib import Path
from typing import Any

FILE_NAME = "AgentTools.json"
TEST_CASES_FILE_NAME = "AgentInputTestCases.json"

_PACKAGE_DIR = Path(__file__).resolve().parent
_SOURCE_TREE_DIR = _PACKAGE_DIR.parent.parent / "Integrations" / "AgentServer"


def find_file(name: str = FILE_NAME) -> Path:
    """Locate a shared definition file. Raises FileNotFoundError if it's nowhere."""
    candidates = []
    if name == FILE_NAME and os.environ.get("PA_AGENT_TOOLS"):
        candidates.append(Path(os.environ["PA_AGENT_TOOLS"]))
    candidates += [_SOURCE_TREE_DIR / name, _PACKAGE_DIR / name]
    for path in candidates:
        if path.is_file():
            return path
    raise FileNotFoundError(
        f"{name} not found. Looked in: " + ", ".join(str(p) for p in candidates))


def resolve_refs(schema: Any, definitions: dict[str, Any]) -> Any:
    """Return `schema` with every {"$ref": "#/definitions/<name>", ...} replaced by a
    copy of that definition, merged with the sibling keys (siblings win).

    Agents receive each tool's inputSchema on its own, without the file's shared
    `definitions`, so references must be inlined. Raises KeyError on unknown names.
    """
    if isinstance(schema, list):
        return [resolve_refs(item, definitions) for item in schema]
    if not isinstance(schema, dict):
        return schema
    if "$ref" in schema:
        ref = schema["$ref"]
        prefix = "#/definitions/"
        if not ref.startswith(prefix):
            raise KeyError(f"Unsupported $ref {ref!r}")
        merged = copy.deepcopy(definitions[ref[len(prefix):]])
        merged.update({k: v for k, v in schema.items() if k != "$ref"})
        return resolve_refs(merged, definitions)
    return {k: resolve_refs(v, definitions) for k, v in schema.items()}


@lru_cache(maxsize=None)
def load() -> dict[str, Any]:
    """Load AgentTools.json with every tool's inputSchema made self-contained."""
    data = json.loads(find_file().read_text(encoding="utf-8"))
    definitions = data.get("definitions", {})
    for tool in data["tools"]:
        tool["inputSchema"] = resolve_refs(tool["inputSchema"], definitions)
    return data


def instructions() -> str:
    return "\n".join(load()["instructions"])


def server_name() -> str:
    return load()["server_name"]


def tools_for(host: str) -> dict[str, dict[str, Any]]:
    """The tools a host ("app" or "python") implements, by name."""
    return {t["name"]: t for t in load()["tools"] if host in t["hosts"]}


def load_test_cases() -> dict[str, Any]:
    return json.loads(find_file(TEST_CASES_FILE_NAME).read_text(encoding="utf-8"))

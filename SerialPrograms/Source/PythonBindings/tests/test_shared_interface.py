"""AgentTools.json must be usable as-is, and the Python input vocabulary must pass
the shared AgentInputTestCases.json (also used by C++)."""

import json
import math

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

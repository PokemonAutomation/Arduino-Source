# Contributing to the Pokemon Automation Codebase

Thanks for your interest in helping! This guide explains how to get help and send a change.
For coding guidelines — how to build and test, and the project's hard rules (Qt, filesystem,
threading, line endings, ...) — see [AGENTS.md](AGENTS.md). It was written for AI coding
agents, but it applies to human contributors too.

We welcome bug fixes, new programs, and new features to existing programs.

## Getting Help

Ask questions on our [Discord server](https://discord.gg/cQ4gWxN). Discussion about the
program is usually in #automation-chat and #automation-help.

## By Kind of Change

- **New program:** pitch it in Discord first. A new program is a lot of code the
  maintainers have to live with, so we want to agree on the idea and its scope before it
  is written. Follow the patterns in `CodingAgentContext/AutomationProgramPatterns.md`.
- **Improvement to an existing program:** usually fine to just send as a PR.
- **Bug fix or improvement in shared code** (inference, navigation, controllers,
  `CommonFramework`, `Common/Cpp`, ...): shared code has a much higher quality bar because
  of its blast radius. Check which programs use the code you are touching, and make sure
  the change is correct for all of them.
- **Big changes or refactors:** pitch them in Discord first, and keep them out of
  behavior-change PRs — no unprompted refactoring of infra inside a fix or feature.

## Size and Shape

- One fix or one feature per PR; keep diffs small.
- Moves and refactors go in their own PR, separate from behavior changes.
- We prefer smaller PRs over bigger PRs.

## Testing

- CI builds on Windows (clang-cl) with `/W4 /WX` — every compiler warning is an error — in
  addition to Linux/clang. Warnings that a macOS or Linux build does not report are caught
  there, so assume the strictest flags apply.
- Unit tests are welcome for logic changes (the test suite runs in CI and in the GUI's
  "Unit Test Runner" program).
- Tests are strongly recommended for new inference classes. Include images that should
  be detected and, where practical, images that should not be detected.
- Tests are especially important when changing an existing inference class to fix a
  detection problem. You likely already have an image that demonstrates the failure.
  Add it as a regression test that fails before the fix and passes afterward, so the
  same problem can be caught if it returns. Run the existing tests for the affected
  inference code to check that the change preserves other expected behavior.
- Test images are stored in the separate
  [CommandLineTests repository](https://github.com/PokemonAutomation/CommandLineTests).
  Add images there and the corresponding test code and test cases in this repository.
- If you also tested on real hardware, say so in the PR description: what you ran, and for how
  long.

## Limits and Response Time

- Our devs work on the repo in their free time, so reviews can take a week or more — don't
  open more PRs while waiting on one.
- New contributors are asked to have at most two PRs open at any time; **drafts count**
  toward that limit. Work that is stacked on not-yet-merged work can live in your fork
  until it is ready.
- The limit is for ramping up: It lifts once you become a regular and trusted contributor.

## Using AI

Due to the advance of AI technology, many GitHub repo maintainers are overwhelmed with AI
generated PRs. We have the following policy as a response:

- If you use AI, you need to understand what the AI writes and can defend its decisions.
- Run and test the change yourself before sending it.
- Don't let an agent open PRs or reply to reviews on your behalf.
- When sending a pull request (PR), don't let AI generate a long description. Write your
  own PR description or edit an AI generated one. Keep it short, easy to read and on the
  point, and mention that a tool helped and which one.
- [AGENTS.md](AGENTS.md) contains the project's build/test steps and hard rules, so coding
  agents follow the same rules as human contributors.

## Coding Conventions and Program Patterns

Follow the conventions of nearby code. The notes below cover formatting and the recurring
patterns used by automation programs; the hard rules in [AGENTS.md](AGENTS.md) also apply.
For background, see the wiki's [development knowledge guide](https://pokemonautomation.github.io/Developer/Knowledge.html)
and [Git contribution guide](https://pokemonautomation.github.io/Developer/Git.html).

### Formatting and Naming

- Use four spaces for indentation, no tabs, and preserve the file's existing line endings
  (usually CRLF). Avoid reformatting unrelated code.
- Always use braces and multiline bodies for conditionals and loops, even for a single
  statement; do not write single-line `if` statements, with or without braces. Do not use
  `goto`. Put opening braces on the same line with no space between `)` and `{`, as in
  `if (condition){`; write `}else{` and `}else if (condition){`. See the explicit
  [control flow formatting rules](AGENTS.md#control-flow-formatting).
- Types generally use `PascalCase`; functions and local variables use `snake_case`.
  Ordinary member variables generally use an `m_` prefix. Program option members use
  `UPPER_SNAKE_CASE`, such as `NUM_ROUNDS` or `GO_HOME_WHEN_DONE`, even when mutable.
- Game files use names such as `PokemonLZA_RestaurantFarmer.h` and `.cpp`, under the
  corresponding game namespace inside `PokemonAutomation::NintendoSwitch`. Follow the
  existing file-header and `PokemonAutomation_<Name>_H` include-guard conventions.
- Follow [Include Sort Order](CodingAgentContext/IncludeOrder.md): library and framework
  headers precede game headers, and same-folder headers come last. A `.cpp` file's own
  header is normally last. The guide covers exceptions, including panel lists whose
  includes follow registration order. Include what you use and remove unused includes.

### Program Structure and Reuse

- A typical single-console program has a `ProgramName_Descriptor` derived from
  `SingleSwitchProgramDescriptor` and a `ProgramName` derived from
  `SingleSwitchProgramInstance`, with `using Descriptor = ProgramName_Descriptor`.
  The descriptor holds metadata and creates statistics; the instance holds options and
  implements `program(env, context)`. Use the corresponding multi-console framework when
  needed. See [Automation Program Patterns](CodingAgentContext/AutomationProgramPatterns.md)
  for a basic scaffold, and current programs for the APIs in use.
- Keep the main loop readable by extracting meaningful steps into helpers. Reuse existing
  navigation, battle, box, and inference routines before adding another implementation.
  Helpers used by several programs belong in a shared game module. Document their starting
  screen, ending screen, return values, and failure behavior so callers can compose them.
- Keep screen recognition in `Inference/`, reusable option definitions in `Options/`, and
  program orchestration in `Programs/`. A screen detector typically implements
  `StaticScreenDetector`; a watcher, often using `DetectorToFinder`, adds detection over
  time. Use existing image, OCR, and overlay helpers rather than embedding detection code
  in the main program loop.
- Program code must remain Qt-free. Use the option and session abstractions instead of
  creating widgets or storing program state in UI objects. Follow the UI/state separation
  rules in [AGENTS.md](AGENTS.md) when working on shared infrastructure.
- Add new source and header files to
  [SourceFiles.cmake](SerialPrograms/cmake/SourceFiles.cmake). Register a new program in its
  game's `*_Panels.cpp`; current single-console panels use
  `make_SingleSwitchProgram<ProgramName>()`.

### Options, Statistics, and Notifications

- Use the existing option types, initialize them in the constructor, and register them with
  `PA_ADD_OPTION`. Give options clear labels, useful defaults, and bounds where applicable.
  Choose `LockMode` deliberately: unlock an option during a run only if the program safely
  handles changes. When several decisions depend on one value, read it once for that step.
- Use the descriptor's `StatsTracker` and its atomic counters for progress and errors;
  call `env.update_stats()` at meaningful progress points. Use `env.log()` or the console
  logger for messages that help explain the current action or a failure.
- Use the framework's notification options and send helpers for status, completion, and
  errors. Reuse options such as `GoHomeWhenDoneOption` and `DeferredStopButtonOption` where
  appropriate. A deferred stop should be checked at a safe boundary, with its
  `ResetOnExit` guard so the button state is reset even if the run throws.

### Controller Commands and Feedback

- Send input through the controller context and existing `pbf_*` / `ssf_*` helpers.
  Prefer explicit durations such as `160ms` or `10s` so the units are clear.
  Commands are queued: use `context.wait_for_all_requests()` before making a decision that depends on those commands having finished.
- For programs with feedback, use detectors and `wait_until()` / `run_until()` to recognize
  game states and drive transitions. Handle both detected states and the no-detection
  result. Keep delays needed for animation or movement, and explain unusual timings or
  game-specific assumptions.
- Give waits and recovery attempts a timeout or retry limit, and report failure when it
  is reached. Use cancellable waits such as `context.wait_for()`; CPU-only loops that do
  not otherwise reach a cancellation point must check `throw_if_cancelled()`. Follow
  [AGENTS.md](AGENTS.md) for thread pools and synchronization.

### Errors and Recovery

- Use the project's exceptions for failures. For an unexpected game state, use
  `OperationFailedException` or `OperationFailedExceptionWithScreenshot` as appropriate,
  with a message describing the action and what failed. Preserve screenshots or other
  diagnostic context when useful.
- Recover from `OperationFailedException` only where there is a concrete recovery action,
  such as returning to a known screen and retrying. Bound repeated recovery attempts and
  propagate the failure when recovery is exhausted. If a catch only records statistics or
  diagnostics, rethrow afterward. Let cancellation and `ProgramFinishedException` propagate;
  do not swallow them in a general retry loop.
- Before sending a change, remove obsolete commented-out code and temporary debugging
  output. Explain non-obvious game behavior and helper contracts in comments.

## License

By contributing, you agree that your code is released under the MIT license, unless the
file says otherwise. See [README.md](README.md) for the dependency license rules.

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

## License

By contributing, you agree that your code is released under the MIT license, unless the
file says otherwise. See [README.md](README.md) for the dependency license rules.
